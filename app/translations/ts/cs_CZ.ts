<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="cs_CZ" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Chyba Anthropic</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>Chyba parsování streamu: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>Neplatný API klíč (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>Omezení rychlosti: %1</translation>
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
        <location filename="../../src/AI/Assistant.cpp" line="218"/>
        <source>Allow AI Device Control?</source>
        <translation>Povolit Ovládání Zařízení AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="219"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>Toto umožňuje AI asistentovi konfigurovat zařízení, otevírat a zavírat připojení a odesílat data do hardwaru.

Každá akce se zařízením stále vyžaduje explicitní schválení pro každé volání v chatu, i když je povoleno automatické schvalování. Povolte pouze v případě, že důvěřujete nakonfigurovanému poskytovateli AI s přístupem k hardwaru.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="417"/>
        <source>Switch AI provider?</source>
        <translation>Přepnout poskytovatele AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="418"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Přepnutí na jiného poskytovatele vymaže aktuální konverzaci. Chcete pokračovat?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="421"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="458"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI Asistent není v tomto sestavení k dispozici</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">AI Asistent vyžaduje licenci Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="463"/>
        <source>Set an API key first</source>
        <translation>Nejprve nastavte klíč API</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">Asistent AI vyžaduje licenci Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="169"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI Asistent není v tomto sestavení k dispozici</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="175"/>
        <source>AI subsystem not initialized</source>
        <translation>Subsystém AI není inicializován</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="181"/>
        <source>Already busy with a previous request</source>
        <translation>Již zpracovává předchozí požadavek</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="495"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Dosažen limit volání nástrojů pro toto kolo; žádné další nástroje nebudou spuštěny.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1093"/>
        <source>Waiting for %1 to respond. Loading the model and processing the prompt can take a while on local hardware...</source>
        <translation>Čekání na odpověď %1. Načítání modelu a zpracování dotazu může na lokálním hardwaru chvíli trvat...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1952"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>Dosáhli jste limitu volání nástrojů pro toto kolo. Nepožadujte další nástroje. Shrňte, co jste dosud zjistili, a pokud úloha není dokončena, uveďte, které kroky zbývají, aby vám uživatel mohl říct, abyste pokračovali.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Překročen limit volání nástrojů</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="940"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Model vrátil prázdnou odpověď. Zkuste přeformulovat dotaz, přepnout na jiný model nebo zkontrolovat, zda je požadavek povolen bezpečnostními filtry poskytovatele.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1097"/>
        <source>Sending request to %1...</source>
        <translation>Odesílání požadavku na %1…</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1110"/>
        <source>Provider returned no reply</source>
        <translation>Poskytovatel nevrátil žádnou odpověď</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="146"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Prompt zablokován bezpečnostním filtrem Gemini: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="200"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini se zastavil bez vytvoření odpovědi: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="262"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="265"/>
        <source>Invalid API key (%1)</source>
        <translation>Neplatný API klíč (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="267"/>
        <source>Rate limited: %1</source>
        <translation>Omezení rychlosti: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="269"/>
        <source>Invalid API key</source>
        <translation>Neplatný API klíč</translation>
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
        <translation>Neplatný API klíč (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="431"/>
        <source>Rate limited: %1</source>
        <translation>Omezení rychlosti: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="433"/>
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
        <translation>Exportovat Soubor Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>Nelze spustit GRPC server</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>Selhalo navázání na %1</translation>
    </message>
</context>
<context>
    <name>API::ProcessLauncher</name>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="82"/>
        <source>No program specified</source>
        <translation>Nebyl zadán žádný program</translation>
    </message>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="88"/>
        <source>Program "%1" not found in PATH</source>
        <translation>Program „%1" nebyl nalezen v PATH</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>Nelze spustit API TCP server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>Povolit Externí API Připojení?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Zpřístupnění API serveru externím hostitelům umožňuje ostatním zařízením ve vaší síti připojit se k Serial Studio na portu 7777.

Povolte pouze v důvěryhodných sítích. Nedůvěryhodní klienti mohou číst živá data nebo odesílat příkazy vašemu zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>Nelze restartovat API TCP server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>Povolit ovládání zařízení přes API?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Program používající lokální API Serial Studia žádá o povolení odesílat data do připojeného zařízení. Povolit klientům API zápis do zařízení?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>API server</source>
        <translation>API server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>Invalid pending connection</source>
        <translation>Neplatné čekající připojení</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>O Aplikaci</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>Verze %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Copyright © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>Všechna Práva Vyhrazena</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 je svobodný software: můžete jej redistribuovat a/nebo upravovat podle podmínek GNU General Public License publikované Free Software Foundation; buď verze 3 této licence, nebo (dle vaší volby) jakékoli pozdější verze.

%1 je distribuován v naději, že bude užitečný, avšak BEZ JAKÉKOLI ZÁRUKY; bez předpokládané záruky PRODEJNOSTI nebo VHODNOSTI PRO KONKRÉTNÍ ÚČEL. Další podrobnosti naleznete v GNU General Public License.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Tato konfigurace je licencována pro komerční a proprietární použití. Může být použita v aplikacích s uzavřeným zdrojovým kódem a komerčních aplikacích, v souladu s podmínkami komerční licence.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Tato konfigurace je pouze pro osobní použití a vyhodnocení. Komerční použití je zakázáno, pokud není aktivována platná komerční licence.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>Tento software je poskytován 'tak, jak je' bez záruky jakéhokoli druhu, výslovné či předpokládané, včetně, ale nejen, záruk prodejnosti nebo vhodnosti pro konkrétní účel. Autor v žádném případě nenese odpovědnost za škody vzniklé použitím tohoto softwaru.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Spravovat Licenci</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Přispět</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Zkontrolovat Aktualizace</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>Project Demo</source>
        <translation>Ukázka Projektu</translation>
    </message>
    <message>
        <source>What's New</source>
        <translation type="vanished">Co Je Nového</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">Tipy a Triky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="235"/>
        <source>License Agreement</source>
        <translation>Licenční Smlouva</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="244"/>
        <source>Report Bug</source>
        <translation>Nahlásit Chybu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="253"/>
        <source>Acknowledgements</source>
        <translation>Poděkování</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="262"/>
        <source>Benchmark</source>
        <translation>Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="270"/>
        <source>Website</source>
        <translation>Webové Stránky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="286"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="186"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="187"/>
        <source>Settings</source>
        <translation>Nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="245"/>
        <source>G-FORCE</source>
        <translation>G-SÍLA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="283"/>
        <source>PITCH ↕</source>
        <translation>NÁKLON ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="320"/>
        <source>ROLL ↔</source>
        <translation>KLOPENÍ ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>Konfigurace Akcelerometru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Nastavte rozsah zobrazení akcelerometru a vstupní jednotky.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Rozsah Zobrazení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>Max. G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Zadejte maximální hodnotu G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Konfigurace Vstupu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Vstup již v G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="34"/>
        <source>Acknowledgements</source>
        <translation>Poděkování</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="75"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="86"/>
        <source>About Qt…</source>
        <translation>O QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Změnit Ikonu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Změnit ikonu použitou pro tuto akci</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Duplikovat tuto akci se všemi jejími nastaveními</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Odstranit tuto akci z projektu</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Přidat Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Dostupné Widgety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Kliknutím na řádek jej přidáte do pracovního prostoru.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Hledat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Skupina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(celá skupina)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Již v pracovním prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Přidat do pracovního prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Žádné widgety nejsou k dispozici.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Žádné widgety neodpovídají.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 widgetů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 z %2 widgetů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Pásma Alarmu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <source>Info</source>
        <translation>Informace</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="72"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="152"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Warning</source>
        <translation>Varování</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>Kritické</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>Otáčkoměr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>Volnoběh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>Provoz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>Pozor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>Červené Pásmo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>Rychloměr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>Cestovní</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>Rychlý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>Maximální Rychlost</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>Teplota Motoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>Studený</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>Normální</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>Teplý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>Přehřátí</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>Tlak</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>Vakuum</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>Vysoký</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>Přetížení</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>Napětí Baterie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>Nízký</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>Nominální</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>Hladina Paliva</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>Prázdný</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>Rezerva</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>Síla Signálu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>Žádný Signál</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>Slabý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>Dobrý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>Zatížení CPU / Systému</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>Zaneprázdněný</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>Přetížení</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Varování / Kritické</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>Indikátor (Zap / Vyp)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>Zapnuto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>Indikátor Poruchy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>Porucha</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>Vybrat Barvu Pásma</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>Předvolby</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>Předvolba</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>Vybrat předvolbu…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>Blikat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>Obnovit na výchozí závažnost</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Klikněte pro výběr barvy. Pravým tlačítkem obnovíte výchozí barvu závažnosti.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>Klikněte pro výběr vlastní barvy.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>Blikat LED, když hodnota leží v tomto pásmu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Nejsou definována žádná pásma. Vyberte předvolbu výše nebo přidejte pásmo pro začátek.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>Použít</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>Použít změny na dataset.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Použít Předvolbu</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Nahradit aktuální pásma vybranou předvolbou, škálovanou na rozsah tohoto datasetu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>Rozsah</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>Pásma</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>Přidat Pásmo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Přidat nové pásmo navazující na poslední.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="437"/>
        <source>Min</source>
        <translation>Min.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="443"/>
        <source>Max</source>
        <translation>Max.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="449"/>
        <source>Severity</source>
        <translation>Závažnost</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>Barva</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>Popisek</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Vyberte vlastní barvu.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(volitelné)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>Posunout nahoru.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>Posunout dolů.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>Odstranit toto pásmo.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Nejsou definována žádná pásma. Použijte předvolbu nebo přidejte pásmo pro začátek.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>Náhled</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>Zahodit změny.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Uložit</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Uložit změny datasetu.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="79"/>
        <source>New chat</source>
        <translation>Nový chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>Toggle chat list</source>
        <translation>Přepnout seznam chatů</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="238"/>
        <source>Settings</source>
        <translation>Nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="274"/>
        <source>Manage API keys…</source>
        <translation>Spravovat API klíče…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="341"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>Export CSV vs. MDF4 — jaký je rozdíl?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="344"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Graf, Sloupcový Graf a Měřidlo — kdy použít každý?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="416"/>
        <source>How can I help with your project?</source>
        <translation>Jak mohu pomoci s vaším projektem?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="417"/>
        <source>Set up your API key to get started</source>
        <translation>Nastavte svůj API klíč pro začátek</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="429"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Popište, co chcete vytvořit, a já nakonfiguruji zdroje, skupiny, datasety, analyzátory rámců a transformace za vás.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>Pro zahájení konverzace vložte API klíč pro vybraného poskytovatele. Klíče jsou šifrovány na tomto počítači a nikdy neopustí váš počítač, kromě komunikace s poskytovatelem, kterého si zvolíte.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="453"/>
        <source>Open API Key Setup</source>
        <translation>Otevřít Nastavení API Klíče</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="463"/>
        <source>Get a key from %1</source>
        <translation>Získat klíč od %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="337"/>
        <source>List the sources in this project</source>
        <translation>Vypsat zdroje v tomto projektu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="334"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Pomozte mi objevit funkce Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="335"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Co tato aplikace umí s mou telemetrií?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="336"/>
        <source>Walk me through what this project already contains</source>
        <translation>Proveďte mě tím, co tento projekt již obsahuje</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="340"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Co je databáze relací a proč bych ji měl používat?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">Export CSV vs. MDF4 — jaký je rozdíl?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="342"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Co je parser rámců a kdy ho potřebuji?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="343"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>Kdy mám použít Lua vs. JavaScript pro parser?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Graf, Sloupcový Graf a Měřidlo — kdy použít každý?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="345"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Jaký je rozdíl mezi transformací a parserem rámců?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="348"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Přidat UART zdroj pro Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="349"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Nastavit IMU projekt od začátku</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="350"/>
        <source>Configure an MQTT subscriber</source>
        <translation>Nakonfigurovat MQTT odběratele</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="351"/>
        <source>Add a CAN bus source</source>
        <translation>Přidat zdroj sběrnice CAN</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="352"/>
        <source>Set up a Modbus poller</source>
        <translation>Nastavit Modbus poller</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="353"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Přidat síťový zdroj (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="354"/>
        <source>Write a CSV frame parser for me</source>
        <translation>Napsat CSV analyzátor rámců</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="355"/>
        <source>Help me parse a JSON frame</source>
        <translation>Pomoci mi parsovat JSON rámec</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="356"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>Přidat EMA vyhlazovací transformaci k datasetu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="357"/>
        <source>Decode hexadecimal frames</source>
        <translation>Dekódovat hexadecimální rámce</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="358"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Kalibrovat senzor s lineární transformací</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="361"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Navrhnout widgety dashboardu pro moje data</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="362"/>
        <source>Build an executive overview workspace</source>
        <translation>Vytvořit přehledový workspace</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="363"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Přidat widget painter pro vlastní vizualizaci</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="364"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Zobrazit Graf, FFT a Vodopád pro jeden dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="365"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Seskupit mé datasety do užitečných pracovních prostorů</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="644"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>Přetáhněte soubory nebo složky, aby je asistent mohl přečíst</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="689"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>Složka "%1" přidána – čitelná během této relace</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="690"/>
        <source>Added "%1" - readable this session</source>
        <translation>Soubor "%1" přidán – čitelný během této relace</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="774"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Zeptejte se Serial Studio na cokoliv…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Clear conversation</source>
        <translation>Vymazat konverzaci</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="838"/>
        <source>Stop generating</source>
        <translation>Zastavit generování</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="839"/>
        <source>Send message (Enter)</source>
        <translation>Odeslat zprávu (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="159"/>
        <source>Provider</source>
        <translation>Poskytovatel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="192"/>
        <source>Model selection</source>
        <translation>Výběr modelu</translation>
    </message>
    <message>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation type="vanished">Spouštět úpravy bez potvrzení. Blokované akce zůstanou blokovány.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Auto-approve edits</source>
        <translation>Automaticky schvalovat úpravy</translation>
    </message>
    <message>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation type="vanished">Umožnit AI konfigurovat zařízení, připojovat/odpojovat a odesílat data. Každá akce stále vyžaduje schválení.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="259"/>
        <source>Allow device control</source>
        <translation>Povolit ovládání zařízení</translation>
    </message>
    <message>
        <source>Manage API keys</source>
        <translation type="vanished">Spravovat API klíče</translation>
    </message>
    <message>
        <source>Working</source>
        <translation type="vanished">Zpracovává Se</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation type="vanished">Připraveno</translation>
    </message>
    <message>
        <source>  •  cache %1k tok</source>
        <translation type="vanished">•  cache %1k tok</translation>
    </message>
    <message>
        <source>  •  cache write %1k tok</source>
        <translation type="vanished">zápis cache %1k tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Nebyl Detekován Žádný Mikrofon</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Připojte mikrofon nebo zkontrolujte nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Vstupní Zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Vzorkovací Frekvence</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Formát Vzorku</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Kanály</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Výstupní Zařízení</translation>
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
        <translation type="vanished">Zadejte uživatelské jméno a heslo pro umístění stahování.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">&amp;Uživatelské jméno:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Heslo:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="65"/>
        <source>Axis Range Configuration</source>
        <translation>Konfigurace Rozsahu Osy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="179"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Nastavte viditelný rozsah os grafu. Hodnoty se aktualizují v reálném čase při psaní.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="187"/>
        <source>X Axis</source>
        <translation>Osa X</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="212"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="283"/>
        <source>Minimum:</source>
        <translation>Minimum:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="224"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="295"/>
        <source>Enter min value</source>
        <translation>Zadejte minimální hodnotu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="233"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="304"/>
        <source>Maximum:</source>
        <translation>Maximum:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="245"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="316"/>
        <source>Enter max value</source>
        <translation>Zadejte maximální hodnotu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="260"/>
        <source>Y Axis</source>
        <translation>Osa Y</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="335"/>
        <source>Reset</source>
        <translation>Resetovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="345"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Obnovit Zálohu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Bez Názvu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Projekt Načten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Automatické Ukládání</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Před Obnovením</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Před Smazáním Datasetu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Před Smazáním Skupiny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Před Novým Projektem</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Před Otevřením Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Před Načtením JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Před Použitím Šablony</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Před Smazáním Akce</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Před Smazáním Výstupního Widgetu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Před Přesunutím Datasetu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Před Přesunutím Skupiny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Před Odstraněním Pracovního Prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Před Vymazáním Všech Pracovních Prostorů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Před Odebráním Widgetu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Před Přeuspořádáním Pracovních Prostorů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Před Dávkovou Operací</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Před Přidáním Dlaždice</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (a %2 dalších)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>Kód parseru rámců se také liší a bude nahrazen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Název se mění z "%1" na "%2". Struktura skupiny beze změny.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Stejné skupiny a datasety, ale kód parseru rámců se liší — obnovení jej nahradí.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Stejné skupiny a datasety jako váš aktuální projekt. Obnovení může stále vrátit úpravy na úrovni polí.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Obnovení odstraní %1 a vrátí %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Obnovení odstraní %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Obnovení vrátí %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Vyberte zálohu k obnovení. Aktuální projekt je nejprve automaticky uložen, takže obnovení je reverzibilní.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Pro tento projekt zatím nejsou žádné zálohy. Upravte nebo uložte projekt pro spuštění rotující zálohy.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Otevřít Složku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Obnovit</translation>
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
        <translation>%1 rámců/s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="83"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="88"/>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>Úspěch</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>Selhání</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>Benchmark Kritické Cesty</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Měří, jak rychle tento počítač dokáže extrahovat, parsovat a vizualizovat rámce prostřednictvím datového pipeline Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>Rozhraní se během běhu zablokuje</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Každá fáze běží naplno v hlavním vlákně, takže okno přestane reagovat, dokud neskončí. Aktuální projekt se po dokončení benchmarku automaticky znovu načte.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>Snímků na fázi:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>Minimální trvání:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>Fáze</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>Parsery</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>Export dat</translation>
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
        <translation>Pouze číselné</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>Smíšený (numerický + text)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>Vyberte alespoň jednu fázi a jeden datový typ pro spuštění benchmarku.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>Spuštěno %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>Příprava...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>Propustnost</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>Čas</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>Výsledek</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>Spusťte test pro zobrazení výsledků</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>Úspěch/Neúspěch se vztahuje na fáze datového pipeline a parseru (datový pipeline a vestavěný numerický 1024 K snímků/s; vestavěný smíšený 512 K; Lua numerický 256 K; JavaScript numerický a Lua smíšený 128 K; JavaScript smíšený 64 K). Fáze exportu a dashboardu jsou informativní.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Úspěch/Neúspěch se vztahuje na fáze datového pipeline a parseru (datový pipeline a Vestavěný numerický 1024 K snímků/s; Vestavěný smíšený 512 K; Lua numerický 256 K; JavaScript numerický a Lua smíšený 128 K; JavaScript smíšený 64 K). Fáze exportu a dashboardu jsou informativní.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Úspěch/Selhání se vztahuje na fáze datového pipeline a parseru (datový pipeline a Nativní numerický 1024 K snímků/s; Nativní smíšený 512 K; Lua numerický 256 K; JavaScript numerický a Lua smíšený 128 K; JavaScript smíšený 64 K). Fáze exportu a dashboardu jsou informativní.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Úspěch/Neúspěch se vztahuje pouze na fáze datového pipeline a parseru (datový pipeline 1024 K snímků/s; numerický: Lua 256 K, JavaScript 128 K; smíšený: Lua 128 K, JavaScript 64 K). Fáze exportu a dashboardu jsou informativní.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Úspěch/Neúspěch se vztahuje pouze na fáze parseru (cíl Lua 256 K snímků/s, JavaScript 128 K). Fáze exportu a dashboardu jsou informativní.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>Probíhá...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>Spustit Benchmark</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>Datová pipeline</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">Lua parser</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">JavaScript parser</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + export dat</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + dashboard</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Nativní parser (numerický)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Nativní parser (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Vestavěný parser (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Vestavěný parser (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Lua parser (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>JavaScript parser (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Lua parser (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>JavaScript parser (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Vestavěný + export dat (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + export dat (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + export dat (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Vestavěný + export dat (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + export dat (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + export dat (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Vestavěný + dashboard (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + dashboard (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 K rámců</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 K rámců</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 K rámců</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 M rámců</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>1 sekunda</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>2 sekundy</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 sekund</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 sekund</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - Benchmark Kritické Cesty</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), zátěž: minimálně %3 snímků, minimálně %4 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>Propustnost</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>Cíl</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>Čas</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>Výsledek</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 rámců/s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>n/a</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>Úspěch</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>Selhání</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + dashboard (numerický)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>Vestavěný + dashboard (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + dashboard (smíšený)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + dashboard (smíšený)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Zobrazování dashboardu...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Služba</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Charakteristika</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Skenování…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Nebyl Detekován Žádný Bluetooth Adaptér</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Připojte Bluetooth adaptér nebo zkontrolujte systémová nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Tento OS Zatím Není Podporován.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio aktualizujeme pro tento operační systém, jakmile jej QT oficiálně podpoří</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Nebyly Nalezeny Žádné CAN Ovladače</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>Nainstalujte CAN hardwarové ovladače pro váš systém</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>CAN Ovladač</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="140"/>
        <source>Interface</source>
        <translation>Rozhraní</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="169"/>
        <source>Bitrate</source>
        <translation>Přenosová Rychlost</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="223"/>
        <source>Flexible Data-Rate</source>
        <translation>Flexibilní Datová Rychlost</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="243"/>
        <source>Loopback</source>
        <translation>Zpětná Smyčka</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="263"/>
        <source>Listen-Only</source>
        <translation>Pouze Naslouchání</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="283"/>
        <source>DBC Database</source>
        <translation>Databáze DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="287"/>
        <source>Import DBC File…</source>
        <translation>Importovat Soubor DBC…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="320"/>
        <source>No CAN Interfaces Found</source>
        <translation>Nenalezena Žádná Rozhraní CAN</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>Vybrat soubor CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>Soubory CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>Aktivní Připojení k Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Pro použití této funkce je nutné odpojit zařízení. Pokračovat?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>Zkontrolovat oprávnění a umístění souboru</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Nedostatečná Data v Souboru CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>Nelze přečíst soubor CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>Soubor CSV musí obsahovat alespoň jeden datový řádek. Zkontrolujte soubor a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>Neplatný CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>Soubor CSV neobsahuje žádná data ani záhlaví.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>Vyberte sloupec data/času</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>Nastavit interval ručně</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>Výběr Data/času CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Zvolte způsob zpracování dat data/času:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>Nastavit Interval</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Zadejte interval mezi řádky v milisekundách:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>Vybrat Sloupec Data/času</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Vyberte sloupec, který obsahuje data data/času:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>Neplatný Výběr</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>Vybraný sloupec není platný.</translation>
    </message>
</context>
<context>
    <name>ChatSidebar</name>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="44"/>
        <source>Chats</source>
        <translation>Chaty</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="57"/>
        <location filename="../../qml/AI/ChatSidebar.qml" line="115"/>
        <source>New chat</source>
        <translation>Nový chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="125"/>
        <source>%1 messages</source>
        <translation>Zpráv: %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="147"/>
        <source>Rename...</source>
        <translation>Přejmenovat…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="158"/>
        <source>Delete</source>
        <translation>Smazat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="197"/>
        <source>Rename chat</source>
        <translation>Přejmenovat chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="217"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="222"/>
        <source>Rename</source>
        <translation>Přejmenovat</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>Konzole</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>Export konzole je funkce Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Tato funkce vyžaduje licenci. Zakupte si ji pro povolení exportu konzole.</translation>
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
        <translation>Bez Ukončení Řádku</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>Nový Řádek</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>Návrat Vozíku</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>Prostý Text</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimální</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>Bez Kontrolního Součtu</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>Zařízení %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Vložit Konstantu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>Základní</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>Rychlost světla ve vakuu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Planckova konstanta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Elementární náboj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Avogadrova konstanta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Boltzmannova konstanta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Stefan-Boltzmannova konstanta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>Mechanika</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>Normální tíhové zrychlení</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Gravitační konstanta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Tlak</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Normální atmosféra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Barometrický tlak na hladině moře</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>Teplota</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>Absolutní nula (Celsius)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Bod tuhnutí vody</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Bod varu vody (1 atm)</translation>
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
        <translation>Plyny a Kapaliny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Univerzální plynová konstanta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Specifická plynová konstanta (suchý vzduch)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Specifická plynová konstanta (vodní pára)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Hustota vzduchu (hladina moře, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Hustota vody (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Rychlost zvuku ve vzduchu (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Poissonova konstanta (suchý vzduch)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>Elektromagnetismus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>Permitivita vakua</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Permeabilita vakua</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Matematika</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pí</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Eulerovo číslo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Zlatý řez</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Fyzikální Konstanty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>Přednastavené hodnoty v jednotkách SI. Kliknutím na řádek jej vložíte do %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Hledat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Symbol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Hodnota</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Kategorie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Žádné konstanty neodpovídají.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 z %2 konstant</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Řídicí Skript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Loop</source>
        <translation>Řídicí Smyčka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>Zpět</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>Znovu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>Vyjmout</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>Vložit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>Vybrat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>Formátovat Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>Formátovat Výběr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>Resetovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control loop</source>
        <translation>Obnovit výchozí řídicí smyčku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control loop file</source>
        <translation>Importovat soubor řídicí smyčky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control loop documentation</source>
        <translation>Otevřít dokumentaci řídicí smyčky</translation>
    </message>
    <message>
        <source>Reset to the default control script</source>
        <translation type="vanished">Obnovit výchozí řídicí skript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>Otevřít</translation>
    </message>
    <message>
        <source>Import a control script file</source>
        <translation type="vanished">Importovat soubor řídicího skriptu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>Vrátit poslední úpravu kódu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>Znovu provést předchozí vrácenou úpravu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>Vyjmout vybraný kód do schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>Zkopírovat vybraný kód do schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>Vložit kód ze schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>Nápověda</translation>
    </message>
    <message>
        <source>Open the control script documentation</source>
        <translation type="vanished">Otevřít dokumentaci řídicího skriptu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>Validovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Ověřit, že se skript správně zkompiluje</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Možnosti Obnovení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio se několikrát po sobě neočekávaně ukončil.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>Po sobě jdoucí pády: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Poslední hlášená fáze: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Zjištěno v: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Vyberte akci obnovení. Po jejím provedení se Serial Studio ukončí, aby příští spuštění začalo čistě.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Obnovit Výchozí Nastavení Vykreslovacího Backend</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Přeskočit Obnovení Naposledy Otevřeného Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Resetovat všechna nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Pokračovat I Přesto</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="35"/>
        <source>CSV Player</source>
        <translation>Přehrávač CSV</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>Náhled Souboru DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>Soubor DBC: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Zkontrolujte zprávy a signály CAN pro import do nového projektu Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Zprávy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Název Zprávy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>ID CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Signály</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>V souboru DBC nebyly nalezeny žádné zprávy.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Celkem: %1 zpráv, %2 signálů</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Vytvořit Projekt</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="262"/>
        <source>Dashboard %1</source>
        <translation>Dashboard %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>Odeslat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Není definována žádná funkce pro přenos</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="326"/>
        <source>Set Wallpaper…</source>
        <translation>Nastavit Pozadí…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="332"/>
        <source>Clear Wallpaper</source>
        <translation>Vymazat Pozadí</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="342"/>
        <source>Tile Windows</source>
        <translation>Uspořádat Okna Vedle Sebe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="361"/>
        <source>Pro features detected in this project.</source>
        <translation>Funkce Pro zjištěny v tomto projektu.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="363"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Aktivní jsou záložní widgety. Zakupte licenci pro plnou funkčnost.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="504"/>
        <source>Empty Workspace</source>
        <translation>Prázdný Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="518"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Použijte vyhledávací lištu k nalezení a přidání widgetů, nebo klikněte pravým tlačítkem na widget v jiném pracovním prostoru a přidejte jej sem.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="533"/>
        <source>Search Widgets</source>
        <translation>Hledat Widgety</translation>
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
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="215"/>
        <source>API Server Active (%1)</source>
        <translation>API Server Aktivní (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="216"/>
        <source>API Server Ready</source>
        <translation>API Server Připraven</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="217"/>
        <source>API Server Off</source>
        <translation>API Server Vypnut</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="155"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="307"/>
        <source>Send</source>
        <translation>Odeslat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="295"/>
        <source>Enter command…</source>
        <translation>Zadejte příkaz…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Není definována funkce pro odesílání</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Zadejte příkaz…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Odeslat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Není definována funkce pro odesílání</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>ZAP</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>VYP</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>Není definována funkce pro odesílání</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Resume</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="313"/>
        <source>Awaiting data…</source>
        <translation>Čekání na data…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="368"/>
        <source>Open %1 in a separate window</source>
        <translation>Otevřít %1 v samostatném okně</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="289"/>
        <source>Select Javascript file to import</source>
        <translation>Vyberte soubor Javascript k importu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="368"/>
        <source>Code Validation Failed</source>
        <translation>Validace Kódu Selhala</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="358"/>
        <source>Line %1: %2</source>
        <translation>Řádek %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="369"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>Skript musí definovat funkci setup() a/nebo loop().</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="374"/>
        <source>Code Validation Successful</source>
        <translation>Validace Kódu Úspěšná</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="375"/>
        <source>No syntax errors detected in the control loop.</source>
        <translation>V řídicí smyčce nebyly zjištěny žádné syntaktické chyby.</translation>
    </message>
    <message>
        <source>No syntax errors detected in the control script.</source>
        <translation type="vanished">V řídicím skriptu nebyly zjištěny žádné syntaktické chyby.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>Importovat Soubor DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>Soubory DBC (*.DBC);;Všechny Soubory (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>Nepodařilo se analyzovat soubor DBC: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>Ověřte formát souboru a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>Chyba Importu DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>Soubor DBC neobsahuje žádné zprávy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Vybraný soubor neobsahuje žádné definice zpráv CAN.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>Upozornění Importu DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>Přehled</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>Aktivní</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Nepodařilo se vytvořit dočasný soubor projektu</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">Zkontrolujte, zda má aplikace oprávnění k zápisu do dočasného adresáře.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>Soubor DBC úspěšně importován s %1 zprávami a %2 signály.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editor projektu je nyní otevřen pro přizpůsobení.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Nepodařilo se načíst importovaný projekt</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Vygenerovaný JSON projektu nelze načíst.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>Přeskočeno %1 signál(ů) používajících rozšířený multiplexing (SG_MUL_VAL_); podporován je pouze jednoduchý multiplexing.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>Import DBC Dokončen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="254"/>
        <source>CAN Bus</source>
        <translation>Sběrnice CAN</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="63"/>
        <source>Dataset Value Transform</source>
        <translation>Transformace Hodnoty Datasetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="100"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="100"/>
        <source>JavaScript</source>
        <translation>Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="129"/>
        <source>Language:</source>
        <translation>Jazyk:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="132"/>
        <source>Template:</source>
        <translation>Šablona:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Zadejte surovou hodnotu (např. 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="112"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="143"/>
        <source>Input:</source>
        <translation>Vstup:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="146"/>
        <source>Output:</source>
        <translation>Výstup:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="114"/>
        <source>Apply</source>
        <translation>Použít</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="115"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="224"/>
        <source>Transform — %1</source>
        <translation>Transformace — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="304"/>
        <source>The value transform has a syntax error and was not applied.</source>
        <translation>Transformace hodnoty obsahuje syntaktickou chybu a nebyla použita.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>The value transform must define a transform(value) function.</source>
        <translation>Transformace hodnoty musí definovat funkci transform(value).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="314"/>
        <source>Define a transform(value) function that returns a number, or use Clear to remove the transform.</source>
        <translation>Definujte funkci transform(value), která vrací číslo, nebo použijte Vymazat pro odstranění transformace.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="332"/>
        <source>Enter a value</source>
        <translation>Zadejte hodnotu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="339"/>
        <source>Invalid number</source>
        <translation>Neplatné číslo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="408"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formátovat Dokument	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="409"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formátovat Výběr	ctrl+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="520"/>
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
-- Definujte funkci transform(value), která přijímá aktuální
-- hodnotu datasetu a vrací transformované číslo. Pokud není
-- funkce transform() definována, ponechá se původní hodnota
-- a s projektem se neuloží nic.
--
-- Příklad:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Globální proměnné deklarované na nejvyšší úrovni přetrvávají
-- mezi snímky, což je užitečné pro filtry, akumulátory a jakékoliv
-- jiné stavové transformace, stejně jako u skriptu parseru snímků:
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
-- Použijte rozbalovací nabídku Šablona pro připravené příklady,
-- nebo klikněte na Test pro vyzkoušení vaší funkce.
--
</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="548"/>
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
 * Definujte funkci transform(value), která přijímá živé
 * čtení datasetu a vrací transformované číslo. Pokud není
 * funkce transform() definována, surová hodnota je zachována
 * a nic není uloženo s projektem.
 *
 * Příklad:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globální proměnné deklarované na nejvyšší úrovni přetrvávají
 * mezi snímky, což je užitečné pro filtry, akumulátory a jakékoli
 * jiné stavové transformace -- stejně jako skript parseru snímků:
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
 * Použijte rozbalovací nabídku Šablona pro připravené příklady,
 * nebo klikněte na Test pro vyzkoušení vaší funkce.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="608"/>
        <source>Failed to create the Lua engine.</source>
        <translation>Nepodařilo se vytvořit engine Lua.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="646"/>
        <source>Line %1: %2</source>
        <translation>Řádek %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="800"/>
        <source>Select Template…</source>
        <translation>Vybrat Šablonu…</translation>
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
 * Definujte funkci transform(value), která přijímá živé
 * čtení datasetu a vrací transformované číslo. Pokud není
 * funkce transform() definována, surová hodnota je zachována
 * a nic není uloženo s projektem.
 *
 * Příklad:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globální proměnné deklarované na nejvyšší úrovni přetrvávají
 * mezi snímky, což je užitečné pro filtry, akumulátory a jakékoli
 * jiné stavové transformace — stejně jako skript parseru snímků:
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
 * Použijte rozbalovací nabídku Šablona pro hotové příklady,
 * nebo klikněte na Test pro vyzkoušení vaší funkce.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <source>Engine error</source>
        <translation>Chyba enginu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="714"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="727"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="744"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="755"/>
        <source>Error: %1</source>
        <translation>Chyba: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="720"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="748"/>
        <source>Error: transform() not defined</source>
        <translation>Chyba: transform() není definována</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="732"/>
        <source>Error: transform() must return a number</source>
        <translation>Chyba: transform() musí vrátit číslo</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1705"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1819"/>
        <source>Channel %1</source>
        <translation>Kanál %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1830"/>
        <source>Audio Input</source>
        <translation>Zvukový Vstup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1714"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1835"/>
        <source>Quick Plot</source>
        <translation>Rychlý Graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1429"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>Transformace v JavaScriptu překročila limit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1430"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>Transformace datové sady trvala déle než %1 ms; zbývající datové sady ve frame byly převedeny na surové hodnoty až do dalšího frame. Profilujte nebo zjednodušte kód transformace.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="230"/>
        <source>Frame pool exhausted</source>
        <translation>Vyčerpán zásobník framů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="232"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>Následný spotřebitel (dashboard, export do CSV/MDF4, databáze sezení nebo API odběratel) nezpracovává framy dostatečně rychle. Serial Studio přechází na alokace po framech, dokud se nevyčistí fronta. Vypněte náročného spotřebitele nebo snižte datovou rychlost.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1666"/>
        <source>Device A</source>
        <translation>Zařízení A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1721"/>
        <source>Quick Plot Data</source>
        <translation>Data Rychlého Grafu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1733"/>
        <source>Multiple Plots</source>
        <translation>Více Grafů</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>Prostý text (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimální</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>Binární (surové bajty)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>Pouze koncový oddělovač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>Počáteční + koncový oddělovač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>Pouze počáteční oddělovač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Bez oddělovačů (celý blok)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>Bez Kontrolního Součtu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>Vybrat Šablonu Parseru Rámců</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>Vyberte šablonu k načtení:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Neplatný hexadecimální vstup.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>Nebyla vybrána žádná šablona.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>Extrahováno %1 rámců | spotřebováno %2 bajtů | uloženo %3 bajtů | zahozeno %4</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>Neplatný JSON: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>Parametry</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">Žádné</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Neplatný HEX Vstup</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Zadejte platné hexadecimální bajty.

Platný formát: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(žádné rámce)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">Extrakce nevytvořila úplný rámec. Zkontrolujte počáteční/koncové oddělovače a režim detekce.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">Extrahováno %1 rámců | spotřebováno %2 bajtů | uloženo %3 bajtů | zahozeno %4</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Konfigurace Pipeline</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Výsledky Zpracování</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Detekce</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Dekodér</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Kontrolní Součet</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Počáteční Oddělovač</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">Koncový Oddělovač</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Hexadecimální Oddělovače</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Pouze koncový oddělovač</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Počáteční + koncový oddělovač</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Pouze počáteční oddělovač</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Bez oddělovačů (celý blok)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Prostý text (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexadecimální</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binární (surové bajty)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Vymazat</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Vyhodnotit</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Zadejte surové bajty streamu zde...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Fáze</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Vstup Dat Rámce</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Výsledky Parseru Rámců</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Zadejte data rámce zde…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Index Datové Sady</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Hodnota</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Zadejte data rámce výše, v případě potřeby zapněte režim HEX a poté klikněte na „Vyhodnotit" pro spuštění parseru rámců.

Příklad (Text): a,b,c,d,e,f
Příklad (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Testovat Parser Rámců</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Zadejte hexadecimální bajty (např. 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(prázdné)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Nebyla vrácena žádná data</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Change Scripting Language?</source>
        <translation>Změnit Skriptovací Jazyk?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="220"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Přepnutí skriptovacího jazyka nahradí aktuální kód parseru ekvivalentní šablonou v novém jazyce.

Veškeré neuložené změny budou ztraceny. Pokračovat?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Javascript file to import</source>
        <translation>Vyberte soubor Javascript k importu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Lua file to import</source>
        <translation>Vyberte soubor Lua k importu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>Code Validation Successful</source>
        <translation>Validace Kódu Úspěšná</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="415"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>V kódu parseru nebyly zjištěny žádné syntaktické chyby.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Select Frame Parser Template</source>
        <translation>Vybrat Šablonu Parseru Rámců</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="526"/>
        <source>Choose a template to load:</source>
        <translation>Vyberte šablonu k načtení:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>Importovat Mapu Registrů Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Mapy Registrů Modbus (*.CSV *.XML *.JSON);;Soubory CSV (*.CSV);;Soubory XML (*.XML);;Soubory JSON (*.JSON);;Všechny Soubory (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>Nebyly nalezeny žádné registry</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Soubor se nepodařilo zpracovat nebo neobsahuje žádné definice registrů.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>Import Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>Přehled</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>Zapnuto</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Nepodařilo se načíst importovaný projekt</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Vygenerovaný JSON projektu nelze načíst.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Nepodařilo se vytvořit dočasný soubor projektu</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Zkontrolujte oprávnění k zápisu do dočasného adresáře.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>Úspěšně importováno %1 registrů v %2 skupinách.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editor projektu je nyní otevřen pro přizpůsobení.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>Import Modbus Dokončen</translation>
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
        <translation type="vanished">Prostý text (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexadecimální</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binární (surové bajty)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Pouze koncový oddělovač</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Počáteční + koncový oddělovač</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Pouze počáteční oddělovač</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Bez oddělovačů (celý blok)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">Bez Kontrolního Součtu</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">Neplatný hexadecimální vstup.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">Nebyla vybrána žádná šablona.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">Extrahováno %1 rámců | spotřebováno %2 bajtů | uloženo %3 bajtů | zahozeno %4</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">Parametry</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="288"/>
        <source>Select Javascript file to import</source>
        <translation>Vyberte soubor Javascript k importu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>Vyberte Šablonu Výstupního Widgetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>Zvolte šablonu k načtení:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>Vyberte soubor Javascript k importu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>Vyberte Šablonu Painter Widgetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>Zvolte šablonu k načtení:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>Přidat datasety pro tuto šablonu?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" očekává %2 dataset(ů); aktuální skupina má %3.

Přidat %4 dataset(ů) s výchozími hodnotami šablony?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2550"/>
        <source>Project Information</source>
        <translation>Informace o Projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2560"/>
        <source>Project Title</source>
        <translation>Název Projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2561"/>
        <source>Untitled Project</source>
        <translation>Nepojmenovaný Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2562"/>
        <source>Name or description of the project</source>
        <translation>Název nebo popis projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2693"/>
        <source>Time</source>
        <translation>Čas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2693"/>
        <source>Samples</source>
        <translation>Vzorky</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2705"/>
        <source>Plot every curve against time or against the sample number</source>
        <translation>Vykreslit každou křivku proti času nebo proti číslu vzorku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2721"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2723"/>
        <source>Web address to load in this widget</source>
        <translation>Webová adresa, která se načte v tomto widgetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2831"/>
        <source>Frame Detection</source>
        <translation>Detekce Rámců</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2845"/>
        <source>Frame Detection Method</source>
        <translation>Metoda Detekce Rámců</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2846"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Vyberte způsob identifikace příchozích datových rámců</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2856"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Hexadecimální Oddělovače</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Zadejte počáteční/koncové sekvence rámců jako hexadecimální hodnoty</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2873"/>
        <source>Frame Start Delimiter</source>
        <translation>Počáteční Oddělovač Rámce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2874"/>
        <source>e.g. /*</source>
        <translation>např. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2875"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Sekvence označující začátek datového rámce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2887"/>
        <source>Frame End Delimiter</source>
        <translation>Oddělovač Konce Rámce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2888"/>
        <source>e.g. */</source>
        <translation>např. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2889"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Sekvence označující konec datového rámce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2895"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Zpracování a Validace Datové Části</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2906"/>
        <source>Data Conversion Method</source>
        <translation>Metoda Konverze Dat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2907"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Vyberte, jak jsou příchozí binární data dekódována před parsováním</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2923"/>
        <source>Checksum Algorithm</source>
        <translation>Algoritmus Kontrolního Součtu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2924"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Vyberte algoritmus kontrolního součtu použitý k validaci rámců</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2580"/>
        <source>Group Information</source>
        <translation>Informace o Skupině</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2590"/>
        <source>Group Title</source>
        <translation>Název Skupiny</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2591"/>
        <source>Untitled Group</source>
        <translation>Skupina Bez Názvu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2592"/>
        <source>Title or description of this dataset group</source>
        <translation>Název nebo popis této skupiny datových sad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2763"/>
        <source>Composite Widget</source>
        <translation>Kompozitní Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2764"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Vyberte, jak má být tato skupina datových sad vizualizována (volitelné)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2642"/>
        <source>Image Configuration</source>
        <translation>Konfigurace Obrázku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3503"/>
        <source>Virtual Dataset</source>
        <translation>Virtuální Datová Sada</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3504"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Virtuální datové sady vypočítávají svou hodnotu z transformací a datových tabulek, nevyžadují index rámce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3907"/>
        <source>Fixed decimal places for the value display; overrides the format (-1 = auto)</source>
        <translation>Pevný počet desetinných míst pro zobrazení hodnoty; přepíše formát (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4036"/>
        <source>Auto-detect</source>
        <translation>Automatická Detekce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4036"/>
        <source>Manual Delimiters</source>
        <translation>Ruční Oddělovače</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2655"/>
        <source>Detection Mode</source>
        <translation>Režim Detekce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1471"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1474"/>
        <source>Frame Parser</source>
        <translation>Analyzátor Rámců</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation type="vanished">Skupiny</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1768"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1769"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1786"/>
        <source>Shared Memory</source>
        <translation>Sdílená Paměť</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1774"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1775"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5835"/>
        <source>Dataset Values</source>
        <translation>Hodnoty Datové Sady</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1935"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1948"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1949"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1964"/>
        <source>Workspaces</source>
        <translation>Pracovní Prostory</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1981"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1984"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1985"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Řídicí Skript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2058"/>
        <source>Publishing</source>
        <translation>Publikování</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2068"/>
        <source>Enable Publishing</source>
        <translation>Povolit Publikování</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2069"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Vysílat rámce, surové bajty a oznámení do brokeru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2080"/>
        <source>Payload</source>
        <translation>Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2081"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Vybírá, co se publikuje: parsovaná data dashboardu nebo surové RX bajty</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2091"/>
        <source>Publish Rate (Hz)</source>
        <translation>Frekvence Publikování (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2092"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>Kolikrát za sekundu publikovat (1-30 Hz). Vyšší frekvence zvyšují zátěž brokeru; data dashboardu jsou omezena frekvencí, takže pomalý broker nikdy neblokuje parsování rámců.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2104"/>
        <source>Topic Base</source>
        <translation>Základ Tématu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2105"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2106"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Základní téma používané pro publikování rámců a surových bajtů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2116"/>
        <source>Script Topic</source>
        <translation>Téma Skriptu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2117"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2141"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>Výchozí hodnota je Základní Téma, pokud je prázdné</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2118"/>
        <source>Topic the user script publishes to</source>
        <translation>Téma, do kterého uživatelský skript publikuje</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2128"/>
        <source>Publish Notifications</source>
        <translation>Publikovat Oznámení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2129"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Zrcadlit oznámení z dashboardu do vyhrazeného tématu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2140"/>
        <source>Notification Topic</source>
        <translation>Téma Oznámení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2142"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Téma, do kterého jsou zrcadlena oznámení z dashboardu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2155"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2165"/>
        <source>Hostname</source>
        <translation>Hostname</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2166"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2167"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>Hostname nebo IP adresa MQTT brokeru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2176"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2177"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>TCP port vystavený brokerem (1883 plain, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2187"/>
        <source>Custom Client ID</source>
        <translation>Vlastní ID Klienta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2189"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Vypnuto: při každém načtení projektu se vygeneruje nové náhodné ID. Zapnuto: použije se ID níže.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2200"/>
        <source>Client ID</source>
        <translation>ID Klienta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2201"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Identifikátor odeslaný brokeru při CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2214"/>
        <source>Protocol Version</source>
        <translation>Verze Protokolu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2215"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>Revize protokolu MQTT použitá při CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2224"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2225"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Sekundy mezi pakety PINGREQ při nečinnosti</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2234"/>
        <source>Clean Session</source>
        <translation>Vyčistit Relaci</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2235"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Zahodit jakýkoli trvalý stav relace při CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2250"/>
        <source>Username</source>
        <translation>Uživatelské Jméno</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2251"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Uživatelské jméno pro autentizaci brokera (ponechte prázdné pro anonymní přístup)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Password</source>
        <translation>Heslo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2262"/>
        <source>Password for broker authentication</source>
        <translation>Heslo pro autentizaci brokera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2273"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2283"/>
        <source>Use SSL/TLS</source>
        <translation>Použít SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2284"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Tunelovat připojení k brokeru přes TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2297"/>
        <source>Protocol</source>
        <translation>Protokol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2298"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Vyjednaná rodina TLS protokolu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2308"/>
        <source>Peer Verify</source>
        <translation>Ověření Protějšku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2309"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Jak přísně je validován řetězec certifikátů brokeru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2319"/>
        <source>Verify Depth</source>
        <translation>Hloubka Ověření</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2320"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Maximální délka akceptovaného řetězce certifikátů (0 = neomezeno)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2607"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3241"/>
        <source>Device %1</source>
        <translation>Zařízení %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2625"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2790"/>
        <source>Input Device</source>
        <translation>Vstupní Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2626"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Vyberte připojené zařízení, které poskytuje data pro tuto skupinu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2657"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Automatická detekce čte magické bajty JPEG/PNG; Ruční používá explicitní počáteční/koncové sekvence</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2667"/>
        <source>Start Sequence (Hex)</source>
        <translation>Počáteční Sekvence (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2668"/>
        <source>e.g. FF D8 FF</source>
        <translation>např. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2669"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Hexadecimální bajty označující začátek snímku obrazu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2678"/>
        <source>End Sequence (Hex)</source>
        <translation>Koncová Sekvence (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2679"/>
        <source>e.g. FF D9</source>
        <translation>např. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2680"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Hexadecimální bajty označující konec snímku obrazu</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">Identita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2800"/>
        <source>Device Name</source>
        <translation>Název Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2801"/>
        <source>Device 1</source>
        <translation>Zařízení 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2802"/>
        <source>Human-readable name for this input device</source>
        <translation>Čitelný název pro toto vstupní zařízení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2811"/>
        <source>Bus Type</source>
        <translation>Typ Sběrnice</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2812"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Vyberte hardwarové rozhraní pro toto vstupní zařízení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Serial Port</source>
        <translation>Sériový Port</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Network Socket</source>
        <translation>Síťový Soket</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Audio Input</source>
        <translation>Zvukový Vstup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>CAN Bus</source>
        <translation>Sběrnice CAN</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Raw USB</source>
        <translation>Přímé USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>HID Device</source>
        <translation>Zařízení HID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT Odběratel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2972"/>
        <source>Connection Settings</source>
        <translation>Nastavení Připojení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3209"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3479"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5502"/>
        <source>General Information</source>
        <translation>Obecné Informace</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3218"/>
        <source>Action Title</source>
        <translation>Název Akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3220"/>
        <source>Untitled Action</source>
        <translation>Nepojmenovaná Akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3221"/>
        <source>Name or description of this action</source>
        <translation>Název nebo popis této akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3230"/>
        <source>Action Icon</source>
        <translation>Ikona Akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3231"/>
        <source>Default Icon</source>
        <translation>Výchozí Ikona</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3232"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Ikona zobrazená pro tuto akci v dashboardu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3259"/>
        <source>Target Device</source>
        <translation>Cílové Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3260"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Vyberte připojené zařízení, kterému tato akce odesílá data</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3272"/>
        <source>Data Payload</source>
        <translation>Datová Část</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3283"/>
        <source>Send as Binary</source>
        <translation>Odeslat jako Binární</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3284"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Odeslat surová binární data při spuštění této akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3295"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3307"/>
        <source>Command</source>
        <translation>Příkaz</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3296"/>
        <source>Transmit Data (Hex)</source>
        <translation>Přenést Data (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3297"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Hexadecimální datová část k odeslání při spuštění akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3308"/>
        <source>Transmit Data</source>
        <translation>Přenést Data</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3309"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Textová datová část k odeslání při spuštění akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3320"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5561"/>
        <source>Text Encoding</source>
        <translation>Kódování Textu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3321"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Znakové kódování použité k serializaci textové datové části</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3345"/>
        <source>End-of-Line Sequence</source>
        <translation>Sekvence Konce Řádku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3346"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>Znaky EOL připojené ke zprávě (např. </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3358"/>
        <source>Execution Behavior</source>
        <translation>Chování Provádění</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3369"/>
        <source>Auto-Execute on Connect</source>
        <translation>Automatické Provedení při Připojení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3370"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Automaticky spustit tuto akci při připojení zařízení</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3376"/>
        <source>Timer Behavior</source>
        <translation>Chování Časovače</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3385"/>
        <source>Timer Mode</source>
        <translation>Režim Časovače</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3388"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Zvolte, kdy a jak se má tato akce automaticky opakovat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3395"/>
        <source>Interval (ms)</source>
        <translation>Interval (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3399"/>
        <source>Timer Interval (ms)</source>
        <translation>Interval Časovače (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3400"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Milisekundy mezi každým opakovaným spuštěním této akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3407"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3411"/>
        <source>Repeat Count</source>
        <translation>Počet Opakování</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3412"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Počet odeslání příkazu při každém spuštění</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3489"/>
        <source>Untitled Dataset</source>
        <translation>Nepojmenovaná Datová Sada</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3490"/>
        <source>Dataset Title</source>
        <translation>Název Datové Sady</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3491"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Název datové sady, používaný pro označení a identifikaci</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3520"/>
        <source>Hide on Dashboard</source>
        <translation>Skrýt na Dashboardu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Potlačit samostatnou dlaždici tohoto datasetu na dashboardu; painter widget může stále číst jeho hodnoty</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Dolní mez rozsahu hodnot datasetu; widgety a FFT ji použijí, pokud jejich vlastní rozsah není nastaven</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3580"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Horní mez rozsahu hodnot datasetu; widgety a FFT ji použijí, pokud jejich vlastní rozsah není nastaven</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Vyberte Čas nebo dataset pro řízení osy X v grafech</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Frequency Analysis</source>
        <translation>Frekvenční Analýza</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3698"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Vyberte Čas (výchozí) nebo jakýkoli dataset, jehož hodnota řídí osu Y -- vytváří Campbellův diagram při vazbě např. na otáčky</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3748"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3837"/>
        <source>Minimum Value (optional)</source>
        <translation>Minimální Hodnota (volitelné)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3749"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Dolní mez pro normalizaci dat; pokud není nastavena, použije se rozsah hodnot datasetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3761"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3850"/>
        <source>Maximum Value (optional)</source>
        <translation>Maximální Hodnota (volitelné)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3762"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Horní mez pro normalizaci dat; pokud není nastavena, použije se rozsah hodnot datasetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3838"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Dolní mez rozsahu měřidla nebo pruhu; pokud není nastavena, použije se rozsah hodnot datasetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3851"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Horní mez rozsahu měřidla nebo pruhu; pokud není nastavena, použije se rozsah hodnot datasetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3903"/>
        <source>Decimal Points</source>
        <translation>Desetinná Místa</translation>
    </message>
    <message>
        <source>Decimal places shown in the data grid value column (-1 = auto)</source>
        <translation type="vanished">Počet desetinných míst zobrazených ve sloupci hodnot datové mřížky (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3950"/>
        <source>On</source>
        <translation>Zapnuto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3994"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>LED se rozsvítí, když hodnota dosáhne nebo překročí tento práh; definujte alarmová pásma pro vícestavové barvy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4051"/>
        <source>Painter Widget</source>
        <translation>Painter Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4052"/>
        <source>Web View</source>
        <translation>Zobrazení Webu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5836"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Surové a transformované hodnoty pro každou datovou sadu (pouze pro čtení)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5845"/>
        <source>Shared table defined in this project</source>
        <translation>Sdílená tabulka definovaná v tomto projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6468"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>Odstranit 1 odkaz na widget, jehož cílová skupina nebo dataset již neexistuje?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6469"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>Odstranit %1 odkazů na widgety, jejichž cílové skupiny nebo datasety již neexistují?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6474"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Toto ovlivní pouze umístění dlaždic v pracovním prostoru; žádné skupiny, datasety ani data nebudou smazány.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6477"/>
        <source>Clean Up Workspaces</source>
        <translation>Vyčistit Pracovní Prostory</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3534"/>
        <source>Frame Index</source>
        <translation>Index Rámce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1638"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1639"/>
        <source>Dashboard Widgets</source>
        <translation>Widgety Dashboardu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2005"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2008"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2009"/>
        <source>Control Loop</source>
        <translation>Řídicí Smyčka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3535"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Pozice rámce použitá pro časové zarovnání datových sad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Measurement Unit</source>
        <translation>Měrná Jednotka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3545"/>
        <source>Volts, Amps, etc.</source>
        <translation>Volty, ampéry atd.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3546"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Měrná jednotka, například volty nebo ampéry (volitelné)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Plot Settings</source>
        <translation>Nastavení Grafu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Enable Plot Widget</source>
        <translation>Povolit Widget Grafu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3618"/>
        <source>Plot data in real-time</source>
        <translation>Vykreslovat data v reálném čase</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2704"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3637"/>
        <source>X-Axis Source</source>
        <translation>Zdroj Osy X</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Vybrat datovou sadu pro osu X v grafech</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Minimální Hodnota Grafu (volitelné)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Dolní mez rozsahu zobrazení grafu</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Maximální Hodnota Grafu (volitelné)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Horní mez rozsahu zobrazení grafu</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">Konfigurace FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3662"/>
        <source>Enable FFT Analysis</source>
        <translation>Povolit Analýzu FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3663"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Provést frekvenční analýzu datové sady</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3673"/>
        <source>Enable Waterfall Plot</source>
        <translation>Povolit Vodopádový Graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3674"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Zobrazit posuvný spektrogram frekvenčního obsahu v čase (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3697"/>
        <source>Waterfall Y Axis</source>
        <translation>Osa Y Vodopádového Grafu</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="obsolete">Vyberte Čas (výchozí) nebo jakýkoli dataset, jehož hodnota řídí osu Y — vytváří Campbellův diagram při vazbě např. na otáčky</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3725"/>
        <source>FFT Window Size</source>
        <translation>Velikost Okna FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3726"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Počet vzorků použitých pro každé výpočetní okno FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3737"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>Vzorkovací Frekvence FFT (Hz, povinné)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3738"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Vzorkovací frekvence použitá pro FFT (v Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Minimální Hodnota (doporučeno)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Dolní mez pro normalizaci dat</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Maximální Hodnota (doporučeno)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Horní mez pro normalizaci dat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3787"/>
        <source>Widget Settings</source>
        <translation>Nastavení Widgetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3810"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3811"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Vyberte vizuální widget použitý k zobrazení této datové sady</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Minimální Zobrazovaná Hodnota (povinné)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Dolní mez rozsahu zobrazení měřidla nebo pruhu</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Maximální Zobrazovaná Hodnota (povinné)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Horní mez rozsahu zobrazení měřidla nebo pruhu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3867"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3902"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4063"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3868"/>
        <source>Tick Count</source>
        <translation>Počet Dílků</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3872"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Počet hlavních dílků na stupnici ciferníku (0 = automatické přizpůsobení velikosti widgetu)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3891"/>
        <source>Label Format</source>
        <translation>Formát Popisku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3892"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Počet desetinných míst nebo notace použitá na popiscích dílků a zobrazení hodnoty</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Zobrazit Indikátor Hodnoty</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Přepnout zarámovaný číselný údaj, který se nachází pod widgetem nebo vedle něj</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Nastavení Alarmů</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Povolit Alarmy</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Spustí vizuální alarm, když hodnota překročí prahové hodnoty alarmu</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Dolní Práh</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Spustí vizuální alarm, když hodnota klesne pod tento práh</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Horní Práh</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Spustí vizuální alarm, když hodnota překročí tento práh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3967"/>
        <source>LED Display Settings</source>
        <translation>Nastavení LED Displeje</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3978"/>
        <source>Show in LED Panel</source>
        <translation>Zobrazit v LED Panelu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3979"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Umožní vizuální monitorování stavu pomocí LED displeje</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3993"/>
        <source>LED On Threshold (required)</source>
        <translation>Práh Zapnutí LED (povinné)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">LED se rozsvítí, když hodnota dosáhne nebo překročí tento práh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Off</source>
        <translation>Vypnuto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Auto Start</source>
        <translation>Automatický Start</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Start on Trigger</source>
        <translation>Spustit při Triggeru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Toggle on Trigger</source>
        <translation>Přepnout při Triggeru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4015"/>
        <source>Repeat N Times</source>
        <translation>Opakovat N-krát</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Plain Text (UTF8)</source>
        <translation>Prostý Text (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimální</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4019"/>
        <source>Binary (Direct)</source>
        <translation>Binární (Přímý)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4024"/>
        <source>No Checksum</source>
        <translation>Bez Kontrolního Součtu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4028"/>
        <source>End Delimiter Only</source>
        <translation>Pouze Koncový Oddělovač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4028"/>
        <source>Start Delimiter Only</source>
        <translation>Pouze Počáteční Oddělovač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4029"/>
        <source>Start + End Delimiter</source>
        <translation>Počáteční + Koncový Oddělovač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4029"/>
        <source>No Delimiters</source>
        <translation>Bez Oddělovačů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Button</source>
        <translation>Tlačítko</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Slider</source>
        <translation>Posuvník</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Toggle</source>
        <translation>Přepínač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Text Field</source>
        <translation>Textové Pole</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4040"/>
        <source>Knob</source>
        <translation>Otočný Ovladač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4044"/>
        <source>Data Grid</source>
        <translation>Datová Mřížka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4045"/>
        <source>GPS Map</source>
        <translation>Mapa GPS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4046"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4047"/>
        <source>Multiple Plot</source>
        <translation>Vícenásobný Graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4048"/>
        <source>Accelerometer</source>
        <translation>Akcelerometr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4049"/>
        <source>3D Plot</source>
        <translation>3D Graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4050"/>
        <source>Image View</source>
        <translation>Zobrazení Obrázku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4053"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4056"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4071"/>
        <source>None</source>
        <translation>Žádný</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4057"/>
        <source>Bar</source>
        <translation>Sloupcový</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4058"/>
        <source>Gauge</source>
        <translation>Ukazatel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4059"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4060"/>
        <source>Meter</source>
        <translation>Měřič</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Teploměr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4064"/>
        <source>Integer (0 decimals)</source>
        <translation>Celé Číslo (0 desetinných míst)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4065"/>
        <source>1 decimal</source>
        <translation>1 desetinné místo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4066"/>
        <source>2 decimals</source>
        <translation>2 desetinná místa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4067"/>
        <source>3 decimals</source>
        <translation>3 desetinná místa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4068"/>
        <source>Scientific</source>
        <translation>Vědecký</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4072"/>
        <source>New Line (\n)</source>
        <translation>Nový Řádek (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4073"/>
        <source>Carriage Return (\r)</source>
        <translation>Návrat Vozíku (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4074"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4077"/>
        <source>No</source>
        <translation>Ne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4078"/>
        <source>Yes</source>
        <translation>Ano</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4908"/>
        <source>(multiple)</source>
        <translation>(více)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4922"/>
        <source>Mixed</source>
        <translation>Smíšené</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5512"/>
        <source>Label</source>
        <translation>Popisek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5513"/>
        <source>Display label</source>
        <translation>Zobrazit popisek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5523"/>
        <source>Button Icon</source>
        <translation>Ikona Tlačítka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5532"/>
        <source>Colorize Icon</source>
        <translation>Obarvit Ikonu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5533"/>
        <source>Tint the icon with the button color</source>
        <translation>Obarvit ikonu barvou tlačítka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5550"/>
        <source>Initial Value</source>
        <translation>Počáteční Hodnota</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5562"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Kódování znaků použité, když transmit() vrací řetězcovou hodnotu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5580"/>
        <source>Value Range</source>
        <translation>Rozsah Hodnot</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3566"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5590"/>
        <source>Minimum Value</source>
        <translation>Minimální Hodnota</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3579"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5599"/>
        <source>Maximum Value</source>
        <translation>Maximální Hodnota</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5608"/>
        <source>Step Size</source>
        <translation>Velikost Kroku</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="644"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="653"/>
        <source>Lock Project</source>
        <translation>Zamknout Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="645"/>
        <source>Choose a password to lock the project:</source>
        <translation>Zvolte heslo pro zamknutí projektu:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="653"/>
        <source>Confirm the password:</source>
        <translation>Potvrďte heslo:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="658"/>
        <source>Passwords do not match</source>
        <translation>Hesla se neshodují</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="659"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>Zadaná hesla se neshodují. Projekt nebyl zamknut.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="693"/>
        <source>Unlock Project</source>
        <translation>Odemknout Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="694"/>
        <source>Enter the project password:</source>
        <translation>Zadejte heslo projektu:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="704"/>
        <source>Incorrect password</source>
        <translation>Nesprávné heslo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="705"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>Zadané heslo neodpovídá heslu uloženému v souboru projektu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="736"/>
        <source>New Project</source>
        <translation>Nový Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="936"/>
        <source>Samples</source>
        <translation>Vzorky</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1523"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Více zdrojů dat vyžaduje licenci Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1524"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro umožňuje připojení k více zařízením současně. Proveďte upgrade pro odemknutí této funkce.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1536"/>
        <source>Device %1</source>
        <translation>Zařízení %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Kopie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1806"/>
        <source>Do you want to save your changes?</source>
        <translation>Uložit změny?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1807"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Projekt obsahuje neuložené změny!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="516"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="538"/>
        <source>Project error</source>
        <translation>Chyba projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="516"/>
        <source>Project title cannot be empty!</source>
        <translation>Název projektu nesmí být prázdný!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <source>You need to add at least one group!</source>
        <translation>Je nutné přidat alespoň jednu skupinu!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="538"/>
        <source>You need to add at least one dataset!</source>
        <translation>Je nutné přidat alespoň jednu datovou sadu!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="585"/>
        <source>Your project needs a title</source>
        <translation>Váš projekt potřebuje název</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="587"/>
        <source>Add a group to get started</source>
        <translation>Přidejte skupinu pro začátek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="589"/>
        <source>Add a dataset to a group</source>
        <translation>Přidejte datovou sadu do skupiny</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="603"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Otevřete zobrazení Projektu v horní části stromu a zadejte název. Projekt můžete kdykoli přejmenovat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="606"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>Skupiny organizují datové sady do widgetů dashboardu. Použijte tlačítko Skupina v panelu nástrojů výše pro vytvoření skupiny a poté do ní přidejte datové sady.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="610"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>Datové sady jsou hodnoty, které se zobrazují na dashboardu. Vyberte skupinu ve stromu a použijte tlačítko Datová sada v panelu nástrojů pro přidání nové.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="935"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="984"/>
        <source>Time</source>
        <translation>Čas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1568"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Chcete smazat zdroj dat "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1569"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>Skupiny používající tento zdroj budou přesunuty na výchozí zdroj. Tuto akci nelze vrátit zpět.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1846"/>
        <source>Save Serial Studio Project</source>
        <translation>Uložit Projekt Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1848"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2512"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Soubory Projektu Serial Studio (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1869"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2110"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2503"/>
        <source>Untitled Project</source>
        <translation>Projekt Bez Názvu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2125"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2658"/>
        <source>Device A</source>
        <translation>Zařízení A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2337"/>
        <source>Select Project File</source>
        <translation>Vybrat Soubor Projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2339"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>Soubory Projektu (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2383"/>
        <source>JSON validation error</source>
        <translation>Chyba validace JSON</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2477"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Projekt byl aktualizován ze staršího formátu souboru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2478"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Tento projekt byl uložen se schématem verze %1; aktuální verze je %2. Výchozí hodnoty byly použity pro všechna nová pole. Uložte projekt pro potvrzení aktualizace.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2510"/>
        <source>Save Imported Project</source>
        <translation>Uložit Importovaný Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2702"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>Projekty s více zdroji vyžadují licenci Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2703"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Tento projekt obsahuje více zdrojů dat. Byl načten pouze první zdroj. Pro použití projektů s více zdroji je vyžadována licence Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2941"/>
        <source>Workspace IDs remapped on load</source>
        <translation>ID pracovních prostorů byly při načtení přečíslovány</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2942"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 vlastních ID pracovních prostorů se překrývalo s novým rezervovaným automatickým rozsahem a bylo přesunuto do uživatelského rozsahu. Uložte projekt pro trvalé zachování tohoto mapování.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3125"/>
        <source>Legacy frame parser function updated</source>
        <translation>Starší funkce parseru rámců aktualizována</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3126"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Váš projekt používal starší funkci parseru rámců s argumentem 'separator'. Byl automaticky migrován do nového formátu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3370"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Chcete smazat skupinu "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3371"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3416"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3448"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4186"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Tuto akci nelze vrátit zpět. Chcete pokračovat?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3415"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Chcete smazat akci "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3447"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Chcete smazat datovou sadu "%1"?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Kopie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4098"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4134"/>
        <source>Output Controls</source>
        <translation>Výstupní Ovládací Prvky</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4146"/>
        <source>New Button</source>
        <translation>Nové Tlačítko</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4149"/>
        <source>New Slider</source>
        <translation>Nový Posuvník</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4152"/>
        <source>New Toggle</source>
        <translation>Nový Přepínač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4155"/>
        <source>New Text Field</source>
        <translation>Nové Textové Pole</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4158"/>
        <source>New Knob</source>
        <translation>Nový Otočný Ovladač</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4185"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Chcete smazat výstupní widget „%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4350"/>
        <source>Group</source>
        <translation>Skupina</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4368"/>
        <source>New Dataset</source>
        <translation>Nová Datová Sada</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4371"/>
        <source>New Plot</source>
        <translation>Nový Graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4375"/>
        <source>New FFT Plot</source>
        <translation>Nový FFT Graf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4379"/>
        <source>New Level Indicator</source>
        <translation>Nový Indikátor Úrovně</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4383"/>
        <source>New Gauge</source>
        <translation>Nový Měřič</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4387"/>
        <source>New Compass</source>
        <translation>Nový Kompas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4393"/>
        <source>New Meter</source>
        <translation>Nový Měřič</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Nový Teploměr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4397"/>
        <source>New LED Indicator</source>
        <translation>Nový LED Indikátor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4401"/>
        <source>New Waterfall</source>
        <translation>Nový Vodopád</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4469"/>
        <source>Channel %1</source>
        <translation>Kanál %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4540"/>
        <source>New Action</source>
        <translation>Nová Akce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4681"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Opravdu chcete změnit widget na úrovni skupiny?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4683"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>Existující datasety pro tuto skupinu budou smazány</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4751"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4752"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4753"/>
        <source>Accelerometer %1</source>
        <translation>Akcelerometr %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <source>Gyro %1</source>
        <translation>Gyroskop %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Latitude</source>
        <translation>Zeměpisná Šířka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Longitude</source>
        <translation>Zeměpisná Délka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Altitude</source>
        <translation>Nadmořská Výška</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4798"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4812"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4798"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4812"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4798"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4812"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5070"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5996"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6268"/>
        <source>Workspace</source>
        <translation>Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5259"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5465"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6727"/>
        <source>Shared Table</source>
        <translation>Sdílená Tabulka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5336"/>
        <source>register</source>
        <translation>registr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5465"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6727"/>
        <source>New Shared Table</source>
        <translation>Nová Sdílená Tabulka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5465"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5488"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5507"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5531"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5558"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5577"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5599"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5621"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5996"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6017"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6251"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6268"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6290"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6485"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6506"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6710"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6727"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6748"/>
        <source>Name:</source>
        <translation>Název:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5488"/>
        <source>Rename Table</source>
        <translation>Přejmenovat Tabulku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5507"/>
        <source>Rename Group</source>
        <translation>Přejmenovat Skupinu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5531"/>
        <source>Rename Dataset</source>
        <translation>Přejmenovat Datovou Sadu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5558"/>
        <source>Rename Data Source</source>
        <translation>Přejmenovat Zdroj Dat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5577"/>
        <source>Rename Action</source>
        <translation>Přejmenovat Akci</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5598"/>
        <source>New Register</source>
        <translation>Nový Registr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5621"/>
        <source>Rename Register</source>
        <translation>Přejmenovat Registr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5655"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5680"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7389"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8173"/>
        <source>This action cannot be undone.</source>
        <translation>Tuto akci nelze vrátit zpět.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5656"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Tímto se odstraní %1 registr(ů) spolu s tabulkou. Tuto akci nelze vrátit zpět.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5659"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5679"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7388"/>
        <source>Delete "%1"?</source>
        <translation>Smazat „%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5662"/>
        <source>Delete Table</source>
        <translation>Smazat Tabulku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5682"/>
        <source>Delete Register</source>
        <translation>Smazat Registr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5704"/>
        <source>Export Table</source>
        <translation>Exportovat Tabulku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5706"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5747"/>
        <source>CSV files (*.csv)</source>
        <translation>Soubory CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5745"/>
        <source>Import Table</source>
        <translation>Importovat Tabulku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5996"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6268"/>
        <source>New Workspace</source>
        <translation>Nový Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6017"/>
        <source>Rename Workspace</source>
        <translation>Přejmenovat Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6063"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6251"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6351"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6485"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6569"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6710"/>
        <source>Folder</source>
        <translation>Složka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6251"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6485"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6710"/>
        <source>New Folder</source>
        <translation>Nová Složka</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6290"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6506"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6748"/>
        <source>Rename Folder</source>
        <translation>Přejmenovat Složku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6308"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6524"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6766"/>
        <source>Delete folder "%1"?</source>
        <translation>Smazat složku „%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6309"/>
        <source>The folder is removed; its workspaces and sub-folders move up to the parent.</source>
        <translation>Složka je odstraněna; její pracovní plochy a podsložky se přesunou do nadřazené složky.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6311"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6527"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6770"/>
        <source>Delete Folder</source>
        <translation>Smazat Složku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6525"/>
        <source>The folder is removed; its groups and sub-folders move up to the parent.</source>
        <translation>Složka je odstraněna; její skupiny a podsložky se přesunou do nadřazené složky.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6767"/>
        <source>The folder is removed; its tables and sub-folders move up to the parent. The accessor path of those tables changes accordingly.</source>
        <translation>Složka je odstraněna; její tabulky a podsložky se přesunou do nadřazené složky. Přístupová cesta těchto tabulek se odpovídajícím způsobem změní.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6855"/>
        <source>Overview</source>
        <translation>Přehled</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6864"/>
        <source>All Data</source>
        <translation>Všechna Data</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7157"/>
        <source>Discard workspace customisations?</source>
        <translation>Zahodit přizpůsobení pracovních prostorů?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7158"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Vypnutí přizpůsobení zahodí vaše úpravy a znovu vytvoří seznam pracovních prostorů ze skupin projektu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7161"/>
        <source>Customize Workspaces</source>
        <translation>Přizpůsobit Pracovní Prostory</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7391"/>
        <source>Delete Workspace</source>
        <translation>Smazat Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7787"/>
        <source>Project file removed from disk</source>
        <translation>Soubor projektu odstraněn z disku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7788"/>
        <source>%1 was deleted or renamed by another program. Save the project to recreate it.</source>
        <translation>%1 byl smazán nebo přejmenován jiným programem. Uložte projekt pro jeho opětovné vytvoření.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7809"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7830"/>
        <source>Project file changed on disk</source>
        <translation>Soubor projektu změněn na disku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7810"/>
        <source>%1 was modified by another program. The in-memory project was kept; reopen the file to load the external changes.</source>
        <translation>%1 byl upraven jiným programem. Projekt v paměti byl zachován; znovu otevřete soubor pro načtení externích změn.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7826"/>
        <source>The project file was modified by another program.

Reload it and discard your unsaved changes?</source>
        <translation>Soubor projektu byl upraven jiným programem.

Načíst jej znovu a zahodit neuložené změny?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7828"/>
        <source>The project file was modified by another program.

Reload it?</source>
        <translation>Soubor projektu byl upraven jiným programem.

Načíst jej znovu?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7859"/>
        <source>File save error</source>
        <translation>Chyba při ukládání souboru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8172"/>
        <source>Delete %1 selected items?</source>
        <translation>Smazat vybrané položky (%1)?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8175"/>
        <source>Delete Items</source>
        <translation>Smazat Položky</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2546"/>
        <source>File open error</source>
        <translation>Chyba při otevírání souboru</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Importovat Soubor Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Soubory Proto (*.proto);;Všechny Soubory (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Nepodařilo se otevřít soubor proto: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Ověřte cestu k souboru a oprávnění ke čtení a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Chyba Importu Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Soubor proto je příliš velký (limit je 10 MB).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>Ověřte, že jste vybrali správný definiční soubor .proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Nepodařilo se analyzovat soubor proto na řádku %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>Podporována je pouze syntaxe proto3. Ověřte formát souboru a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Soubor proto neobsahuje žádné definice zpráv</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Vybraný soubor neobsahuje žádné bloky `message` k importu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Upozornění Importu Protobuf</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Nepodařilo se načíst importovaný projekt</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Vygenerovaný JSON projektu se nepodařilo načíst.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>Úspěšně importováno %1 zpráv a %2 polí ze souboru proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editor projektu je nyní otevřen pro přizpůsobení.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Import Protobuf Dokončen</translation>
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
        <translation>Neplatný Hexadecimální Vstup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Zadejte platné hexadecimální bajty.

Platný formát: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Žádný kód funkce transmit k vyhodnocení.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>funkce transmit není volatelná</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>Vyhodnotit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>Vstupní Hodnota</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>Výstup Funkce Přenosu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>Zadejte hodnotu k odeslání…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>Zde se zobrazí výstup jako řetězec</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>Zde se zobrazí výstup jako hexadecimální bajty</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>Testovat Funkci Přenosu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Zadejte hexadecimální bajty (např. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(prázdné) Nevrácena žádná data</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 bajtů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 bajt(ů)</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>Sdílená Paměť</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="167"/>
        <source>Add Folder</source>
        <translation>Přidat Složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="169"/>
        <source>Add a top-level folder</source>
        <translation>Přidat složku nejvyšší úrovně</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="177"/>
        <source>Add Shared Table</source>
        <translation>Přidat Sdílenou Tabulku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="179"/>
        <source>Add shared table</source>
        <translation>Přidat sdílenou tabulku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="188"/>
        <source>Help</source>
        <translation>Nápověda</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="193"/>
        <source>Open help documentation for shared memory</source>
        <translation>Otevřít dokumentaci nápovědy pro sdílenou paměť</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="202"/>
        <source>Title</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="203"/>
        <source>Registers</source>
        <translation>Registry</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="vanished">Název</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="vanished">Popis</translation>
    </message>
    <message>
        <source>Entries</source>
        <translation type="vanished">Položky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="297"/>
        <source>No shared tables.</source>
        <translation>Žádné sdílené tabulky.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Relace</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Otevřít</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Otevřít soubor relace</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Zavřít soubor relace</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Přehrát</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Přehrát vybranou relaci na panelu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Smazat</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Odemkněte soubor relace pro mazání relací</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Smazat vybranou relaci</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Odemknout</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Zamknout</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Odemkněte soubor relace pro povolení mazání</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Nastavte heslo pro zamezení mazání relací</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>Exportovat CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Exportovat vybranou relaci do CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>Exportovat PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>Vygenerovat PDF zprávu pro vybranou relaci</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Obnovit Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Obnovit soubor projektu z tohoto souboru relace</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Načítání relace…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>Zpracovává Se…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="112"/>
        <source>Pro features detected in this project.</source>
        <translation>V tomto projektu zjištěny funkce Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="114"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Použity náhradní widgety. Zakupte licenci pro odemknutí plné funkčnosti.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="142"/>
        <source>Plots</source>
        <translation>Grafy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="147"/>
        <source>Plot</source>
        <translation>Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="151"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>Přepnout 2D vizualizaci grafu pro tento dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>FFT Plot</source>
        <translation>FFT Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="166"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>Přepnout FFT graf pro vizualizaci frekvenčního obsahu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="178"/>
        <source>Waterfall</source>
        <translation>Vodopád</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="182"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Přepnout vodopádový (spektrogram) graf — používá nastavení FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="199"/>
        <source>Widgets</source>
        <translation>Widgety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="205"/>
        <source>Bar/Level</source>
        <translation>Pruh/úroveň</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="209"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Přepnout indikátor pruhu/úrovně pro tento dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="220"/>
        <source>Gauge</source>
        <translation>Měřič</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="225"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Přepnout widget měřiče pro zobrazení v analogovém stylu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="237"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="241"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Přepnout widget kompasu pro směrová data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="252"/>
        <source>Meter</source>
        <translation>Měřič</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="257"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Přepnout widget analogového měřiče (půlkruhový)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="308"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>Definujte barevné rozsahy hodnot s úrovněmi závažnosti pro měřič nebo LED tohoto datasetu.</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Teploměr</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Přepnout widget teploměru pro teplotní data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="268"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="273"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>Přepnout LED indikátor pro binární nebo prahové hodnoty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="293"/>
        <source>Behavior</source>
        <translation>Chování</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="298"/>
        <source>Alarm Bands</source>
        <translation>Pásma Alarmu</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">Definujte barevné rozsahy hodnot s úrovněmi závažnosti pro měřič tohoto datasetu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Transform</source>
        <translation>Transformace</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="318"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Upravit výraz transformace hodnoty pro kalibraci, filtrování nebo převod jednotek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="331"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="336"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Duplikovat tento dataset se stejnou konfigurací</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="344"/>
        <source>Delete this dataset from the group</source>
        <translation>Odstranit tento dataset ze skupiny</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="36"/>
        <source>Support Serial Studio</source>
        <translation>Podpořit Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="85"/>
        <source>Support the development of %1!</source>
        <translation>Podpořte vývoj %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="96"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio je svobodný software s otevřeným zdrojovým kódem podporovaný dobrovolníky. Zvažte darování nebo získání licence Pro na podporu vývojového úsilí :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="109"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Tento projekt můžete také podpořit jeho sdílením, hlášením chyb a navrhováním nových funkcí!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="123"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="134"/>
        <source>Donate</source>
        <translation>Přispět</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="149"/>
        <source>Get Serial Studio Pro</source>
        <translation>Získat Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="143"/>
        <source>Stop</source>
        <translation>Zastavit</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="385"/>
        <source>Downloading updates</source>
        <translation>Stahování aktualizací</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="145"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="483"/>
        <source>Time remaining</source>
        <translation>Zbývající čas</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="145"/>
        <source>unknown</source>
        <translation>neznámý</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="251"/>
        <source>Error</source>
        <translation>Chyba</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="251"/>
        <source>Cannot find downloaded update!</source>
        <translation>Nelze najít staženou aktualizaci!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="267"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="268"/>
        <source>Download complete!</source>
        <translation>Stahování dokončeno!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="276"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Klikněte na „OK" pro zahájení instalace aktualizace</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="278"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>Pro instalaci aktualizace může být nutné ukončit aplikaci.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="269"/>
        <source>The installer opens separately</source>
        <translation>Instalátor se otevře samostatně</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="282"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>Pro instalaci aktualizace může být nutné ukončit aplikaci. Toto je povinná aktualizace, ukončení nyní zavře aplikaci.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="298"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Klikněte na tlačítko „Otevřít" pro aplikaci aktualizace</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="311"/>
        <source>Updater</source>
        <translation>Aktualizátor</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="315"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Opravdu chcete zrušit stahování?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="317"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Opravdu chcete zrušit stahování? Toto je povinná aktualizace, ukončení nyní zavře aplikaci</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="372"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="379"/>
        <source>%1 bytes</source>
        <translation>%1 bajtů</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="374"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="381"/>
        <source>%1 KB</source>
        <translation>%1 KB</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="376"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="383"/>
        <source>%1 MB</source>
        <translation>%1 MB</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="385"/>
        <source>of</source>
        <translation>z</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="430"/>
        <source>Downloading Updates</source>
        <translation>Stahování Aktualizací</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>Time Remaining</source>
        <translation>Zbývající Čas</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>Unknown</source>
        <translation>Neznámý</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="459"/>
        <source>about %1 hours</source>
        <translation>přibližně %1 hodin</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="461"/>
        <source>about one hour</source>
        <translation>přibližně jedna hodina</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="469"/>
        <source>%1 minutes</source>
        <translation>%1 minut</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="471"/>
        <source>1 minute</source>
        <translation>1 minuta</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="478"/>
        <source>%1 seconds</source>
        <translation>%1 sekund</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="480"/>
        <source>1 second</source>
        <translation>1 sekunda</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Zbývající čas: 0 minut</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Otevřít</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="33"/>
        <source>Examples Browser</source>
        <translation>Prohlížeč Příkladů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="119"/>
        <source>Back</source>
        <translation>Zpět</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="151"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="172"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Stáhnout a Otevřít</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="187"/>
        <source>View on GitHub</source>
        <translation>Zobrazit na GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="90"/>
        <source>Search in Examples…</source>
        <translation>Hledat v Příkladech…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="244"/>
        <source>Fetching examples…</source>
        <translation>Načítání příkladů…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="567"/>
        <source>Loading...</source>
        <translation>Načítání...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>No README available.</source>
        <translation>README není k dispozici.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="608"/>
        <source>Copied to Clipboard</source>
        <translation>Zkopírováno do Schránky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="671"/>
        <source>No screenshot available</source>
        <translation>Snímek obrazovky není k dispozici</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="703"/>
        <source>Details</source>
        <translation>Podrobnosti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="732"/>
        <source>Info</source>
        <translation>Informace</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="755"/>
        <source>Category:</source>
        <translation>Kategorie:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="768"/>
        <source>Difficulty:</source>
        <translation>Obtížnost:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="786"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="828"/>
        <source>No Results Found</source>
        <translation>Nebyly Nalezeny Žádné Výsledky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="839"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Zkontrolujte pravopis nebo zkuste jiný hledaný výraz.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="854"/>
        <source>%1 examples</source>
        <translation>%1 příkladů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="863"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="31"/>
        <source>Extension Manager</source>
        <translation>Správce Rozšíření</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="118"/>
        <source>Refresh</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="132"/>
        <source>Repos</source>
        <translation>Repozitáře</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="162"/>
        <source>Repository Settings</source>
        <translation>Nastavení Repozitáře</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="174"/>
        <source>Back</source>
        <translation>Zpět</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="215"/>
        <source>Install</source>
        <translation>Instalovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="232"/>
        <source>Uninstall</source>
        <translation>Odinstalovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="259"/>
        <source>Run</source>
        <translation>Spustit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="283"/>
        <source>Stop</source>
        <translation>Zastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="317"/>
        <source>Reset</source>
        <translation>Resetovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="77"/>
        <source>Search extensions…</source>
        <translation>Hledat rozšíření…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="367"/>
        <source>Fetching extensions…</source>
        <translation>Načítání rozšíření…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="604"/>
        <source>Running</source>
        <translation>Spuštěno</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="626"/>
        <source>Update</source>
        <translation>Aktualizovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="626"/>
        <source>Installed</source>
        <translation>Nainstalováno</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="643"/>
        <source>Unavailable</source>
        <translation>Nedostupné</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="822"/>
        <source>No description available.</source>
        <translation>Popis není k dispozici.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="863"/>
        <source>Details</source>
        <translation>Podrobnosti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="884"/>
        <source>Type:</source>
        <translation>Typ:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="897"/>
        <source>Author:</source>
        <translation>Autor:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="909"/>
        <source>Version:</source>
        <translation>Verze:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="921"/>
        <source>License:</source>
        <translation>Licence:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="982"/>
        <source>No preview</source>
        <translation>Žádný náhled</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1011"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>VÝSTUP PLUGINU</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1041"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Zatím žádný výstup. Spusťte plugin pro zobrazení jeho logu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1076"/>
        <source>No preview available</source>
        <translation>Náhled není k dispozici</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1120"/>
        <source>Repositories</source>
        <translation>Repozitáře</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1133"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>Přidejte URL vzdálených repozitářů nebo cesty k lokálním složkám.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1170"/>
        <source>LOCAL</source>
        <translation>LOKÁLNÍ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1227"/>
        <source>URL or local path…</source>
        <translation>URL nebo lokální cesta…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1258"/>
        <source>Browse…</source>
        <translation>Procházet…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1235"/>
        <source>Add</source>
        <translation>Přidat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1295"/>
        <source>No Results Found</source>
        <translation>Nebyly Nalezeny Žádné Výsledky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1306"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Zkontrolujte pravopis nebo zkuste jiný hledaný výraz.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1330"/>
        <source>No Extensions Available</source>
        <translation>Nejsou Dostupná Žádná Rozšíření</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1341"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Přidejte URL repozitáře nebo místní cestu v nastavení Repozitáře a poté obnovte.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1356"/>
        <source>%1 extensions</source>
        <translation>%1 rozšíření</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1365"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="167"/>
        <source>Interpolation: %1</source>
        <translation>Interpolace: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="195"/>
        <source>Show Area Under Plot</source>
        <translation>Zobrazit Plochu Pod Grafem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="213"/>
        <source>Show X Axis Label</source>
        <translation>Zobrazit Popisek Osy X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="225"/>
        <source>Show Y Axis Label</source>
        <translation>Zobrazit Popisek Osy Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="243"/>
        <source>Show Crosshair</source>
        <translation>Zobrazit Zaměřovač</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Resume</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="269"/>
        <source>Reset View</source>
        <translation>Resetovat Zobrazení</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="275"/>
        <source>Axis Range Settings</source>
        <translation>Nastavení Rozsahu Os</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="304"/>
        <source>Magnitude (dB)</source>
        <translation>Magnituda (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="305"/>
        <source>Frequency (Hz)</source>
        <translation>Frekvence (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Sem přetáhněte soubory projektů a CSV</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="33"/>
        <source>File Transmission</source>
        <translation>Přenos Souboru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="101"/>
        <source>Transfer Protocol:</source>
        <translation>Protokol Přenosu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="134"/>
        <source>File Selection:</source>
        <translation>Výběr Souboru:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="151"/>
        <source>Select File…</source>
        <translation>Vybrat Soubor…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="169"/>
        <source>Transmission Interval:</source>
        <translation>Interval Přenosu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="195"/>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="271"/>
        <source>msecs</source>
        <translation>ms</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="205"/>
        <source>Block Size:</source>
        <translation>Velikost Bloku:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="233"/>
        <source>bytes</source>
        <translation>bajtů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="243"/>
        <source>Timeout:</source>
        <translation>Časový Limit:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="281"/>
        <source>Max Retries:</source>
        <translation>Max. Opakování:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="339"/>
        <source>Progress: %1%</source>
        <translation>Průběh: %1 %</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="372"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 bajtů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="380"/>
        <source>Errors: %1</source>
        <translation>Chyby: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="460"/>
        <source>Activity Log</source>
        <translation>Protokol Aktivit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="464"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="418"/>
        <source>Pause Transmission</source>
        <translation>Pozastavit Přenos</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Resume Transmission</source>
        <translation>Obnovit Přenos</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="422"/>
        <source>Stop Transmission</source>
        <translation>Zastavit Přenos</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Begin Transmission</source>
        <translation>Zahájit Přenos</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="448"/>
        <source>Frame Parser</source>
        <translation>Analyzátor Rámců</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="455"/>
        <source>Device %1</source>
        <translation>Zařízení %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="575"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1862"/>
        <source>Output Panel</source>
        <translation>Výstupní Panel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="619"/>
        <source>Control</source>
        <translation>Ovládání</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="694"/>
        <source>Table</source>
        <translation>Tabulka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="708"/>
        <source>%1 regs</source>
        <translation>%1 reg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="406"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="708"/>
        <source>empty</source>
        <translation>prázdné</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Řídicí Skript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="843"/>
        <source>MQTT Publisher</source>
        <translation>Vydavatel MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1251"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Otevřít editor transformačního kódu pro tuto datovou sadu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1632"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1635"/>
        <source>Dataset Container</source>
        <translation>Kontejner Datových Sad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1644"/>
        <source>Multi-Plot</source>
        <translation>Vícenásobný Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1647"/>
        <source>Multiple Plot</source>
        <translation>Vícenásobný Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1656"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1659"/>
        <source>Accelerometer</source>
        <translation>Akcelerometr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1668"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1671"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1680"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1683"/>
        <source>GPS Map</source>
        <translation>Mapa GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1691"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1694"/>
        <source>3D Plot</source>
        <translation>3D Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1702"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1705"/>
        <source>Image View</source>
        <translation>Zobrazení Obrázku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1714"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1717"/>
        <source>Painter Widget</source>
        <translation>Painter Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1729"/>
        <source>Web View</source>
        <translation>Zobrazení Webu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1738"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1741"/>
        <source>Data Grid</source>
        <translation>Datová Mřížka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1754"/>
        <source>Generic</source>
        <translation>Obecný</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1767"/>
        <source>Plot</source>
        <translation>Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1780"/>
        <source>FFT Plot</source>
        <translation>FFT Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1793"/>
        <source>Gauge</source>
        <translation>Ukazatel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1806"/>
        <source>Level Indicator</source>
        <translation>Indikátor Úrovně</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1819"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1832"/>
        <source>Meter</source>
        <translation>Měřidlo</translation>
    </message>
    <message>
        <source>Edit Control Script…</source>
        <translation type="vanished">Upravit Řídicí Skript…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Teploměr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="404"/>
        <source>Control Loop</source>
        <translation>Řídicí Smyčka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="799"/>
        <source>Shared Memory</source>
        <translation>Sdílená Paměť</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1845"/>
        <source>LED Indicator</source>
        <translation>LED Indikátor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1874"/>
        <source>Slider</source>
        <translation>Posuvník</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1887"/>
        <source>Toggle</source>
        <translation>Přepínač</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1900"/>
        <source>Knob</source>
        <translation>Otočný Ovladač</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1913"/>
        <source>Text Field</source>
        <translation>Textové Pole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1926"/>
        <source>Button</source>
        <translation>Tlačítko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1950"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2026"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2114"/>
        <source>Add Group</source>
        <translation>Přidat Skupinu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1966"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2042"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2130"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2175"/>
        <source>Add Dataset</source>
        <translation>Přidat Datovou Sadu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1980"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2056"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2144"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2189"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2396"/>
        <source>Add Output</source>
        <translation>Přidat Výstup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1996"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2069"/>
        <source>Add Action</source>
        <translation>Přidat Akci</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2005"/>
        <source>Add Data Source</source>
        <translation>Přidat Zdroj Dat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Add Data Table</source>
        <translation>Přidat Datovou Tabulku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2080"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2216"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2283"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2411"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2445"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2501"/>
        <source>Rename…</source>
        <translation>Přejmenovat…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2088"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2246"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2316"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2368"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2419"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2475"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2099"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2257"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2328"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2380"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2430"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2486"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2512"/>
        <source>Delete…</source>
        <translation>Odstranit…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2160"/>
        <source>Edit Frame Parser…</source>
        <translation>Upravit Analyzátor Rámců…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2202"/>
        <source>Edit Painter Code…</source>
        <translation>Upravit Kód Malíře…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2224"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2292"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2344"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2453"/>
        <source>Move Up</source>
        <translation>Posunout Nahoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2235"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2304"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2356"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2464"/>
        <source>Move Down</source>
        <translation>Posunout Dolů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2272"/>
        <source>Edit Transform Code…</source>
        <translation>Upravit Kód Transformace…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2527"/>
        <source>Edit Code…</source>
        <translation>Upravit Kód…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2542"/>
        <source>Edit Control Loop…</source>
        <translation>Upravit Řídicí Smyčku…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="245"/>
        <source>Group</source>
        <translation>Skupina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="372"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="756"/>
        <source>Folder</source>
        <translation>Složka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="517"/>
        <source>Action</source>
        <translation>Akce</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">Zatím nejsou definovány žádné skupiny</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Testovat Parser Rámců</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>Rámec %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>Dekodér</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>Řádky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 řádek/řádky/řádků</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Řádek %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Konfigurace Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Detekce</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Začátek</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Konec</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Kontrolní Součet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Hexadecimální Oddělovače</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Vstup Dat Rámce</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Zadejte hexadecimální bajty (např. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Zadejte surové bajty streamu zde...</translation>
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
        <translation>Vzorek neobsahuje nakonfigurované oddělovače rámce, takže nebude extrahován žádný rámec. Zadejte je do vzorku (např. 
 pro nový řádek) nebo upravte režim detekce.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Výsledky Zpracování</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Fáze</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Hodnota</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>Extrakce nevytvořila kompletní rámec. Zkontrolujte počáteční/koncové oddělovače a režim detekce.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Zadejte vzorová data výše a stiskněte Vyhodnotit pro náhled zpracovaného výstupu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Vyhodnotit</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Zpět</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Znovu</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Vyjmout</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Kopírovat</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Vložit</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Vybrat Vše</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Formátovat Dokument</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Formátovat Výběr</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Obnovit</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Obnovit na výchozí skript parsování</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Otevřít</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Importovat soubor skriptu pro parsování dat</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Otevřít nápovědu k parsování dat</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Jazyk:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Vybrat Šablonu…</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Testovat Se Vzorovými Daty</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Vyhodnotit</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Vrátit poslední úpravu kódu</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Znovu provést předchozí vrácenou úpravu</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Vyjmout vybraný kód do schránky</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Zkopírovat vybraný kód do schránky</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Vložit kód ze schránky</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Nápověda</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Automatické Centrování</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Vykreslit Trajektorii</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Přiblížit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Oddálit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Zobrazit Počasí</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>Vrstva Počasí NASA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Základní Mapa: %1</translation>
    </message>
</context>
<context>
    <name>GroupFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Složka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="132"/>
        <source>Add Sub-folder</source>
        <translation>Přidat Podsložku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="134"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Přidat podsložku do této složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="142"/>
        <source>Add Group</source>
        <translation>Přidat Skupinu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="144"/>
        <source>Add a group inside this folder</source>
        <translation>Přidat skupinu do této složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="154"/>
        <source>Rename</source>
        <translation>Přejmenovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="156"/>
        <source>Rename folder</source>
        <translation>Přejmenovat složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="164"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="166"/>
        <source>Delete folder</source>
        <translation>Odstranit složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="179"/>
        <source>Title</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="180"/>
        <source>Contents</source>
        <translation>Obsah</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="270"/>
        <source>This folder is empty. Use the toolbar to add a group or sub-folder.</source>
        <translation>Tato složka je prázdná. Použijte panel nástrojů k přidání skupiny nebo podsložky.</translation>
    </message>
</context>
<context>
    <name>GroupTemplateMenu</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="43"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="45"/>
        <source>Dataset Container</source>
        <translation>Kontejner Datových Sad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="51"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="53"/>
        <source>Data Grid</source>
        <translation>Datová Mřížka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="59"/>
        <source>Multi-Plot</source>
        <translation>Vícenásobný Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="61"/>
        <source>Multiple Plot</source>
        <translation>Vícenásobný Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="67"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="69"/>
        <source>3D Plot</source>
        <translation>3D Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="75"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="77"/>
        <source>Accelerometer</source>
        <translation>Akcelerometr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="83"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="85"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="91"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="93"/>
        <source>GPS Map</source>
        <translation>Mapa GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="99"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="101"/>
        <source>Image View</source>
        <translation>Zobrazení Obrázku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="107"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="109"/>
        <source>Web View</source>
        <translation>Zobrazení Webu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="115"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="117"/>
        <source>Painter Widget</source>
        <translation>Painter Widget</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="100"/>
        <source>Pro features detected in this project.</source>
        <translation>Funkce Pro zjištěny v tomto projektu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="102"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Používají se náhradní widgety. Zakupte licenci pro odemknutí plné funkčnosti.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="132"/>
        <source>Add Dataset</source>
        <translation>Přidat Datovou Sadu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="142"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Přidat obecný dataset do aktuální skupiny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="149"/>
        <source>Plot</source>
        <translation>Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="153"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>Přidat 2D graf pro vizualizaci numerických dat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="161"/>
        <source>FFT Plot</source>
        <translation>FFT Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="166"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>Přidat FFT graf pro vizualizaci frekvenční domény</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="172"/>
        <source>Waterfall</source>
        <translation>Vodopád</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="177"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Přidat vodopádový (spektrogram) graf — používá nastavení FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="183"/>
        <source>Bar/Level</source>
        <translation>Pruh/úroveň</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="187"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Přidat pruh nebo indikátor úrovně pro škálované hodnoty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Měřidlo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="198"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Přidat widget měřidla pro analogovou vizualizaci</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="205"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="209"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Přidat kompas pro zobrazení směrových nebo úhlových dat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="215"/>
        <source>Meter</source>
        <translation>Měřidlo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="219"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Přidat widget analogového měřidla (půlkruh)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Teploměr</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Přidat widget teploměru pro teplotní data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="226"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="231"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>Přidat LED indikátor pro binární stavové signály</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="244"/>
        <source>Add Control</source>
        <translation>Přidat Ovládací Prvek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="250"/>
        <source>Button</source>
        <translation>Tlačítko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="254"/>
        <source>Add a button that sends a command on click</source>
        <translation>Přidat tlačítko, které při kliknutí odešle příkaz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="260"/>
        <source>Slider</source>
        <translation>Posuvník</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="264"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Přidat posuvník pro odesílání škálovaných číselných hodnot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="270"/>
        <source>Toggle</source>
        <translation>Přepínač</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="274"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Přidat přepínač pro příkazy zapnuto/vypnuto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Text Field</source>
        <translation>Textové Pole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="284"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Přidat textové pole pro psaní a odesílání příkazů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="290"/>
        <source>Knob</source>
        <translation>Otočný Knoflík</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="294"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Přidat otočný knoflík pro ovládání nastavené hodnoty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="313"/>
        <source>Edit Code</source>
        <translation>Upravit Kód</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="317"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Upravit JavaScript, který vykresluje tento painter widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="330"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="334"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Duplikovat aktuální skupinu a její obsah</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="339"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="344"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Odstranit aktuální skupinu a všechny obsažené datasety</translation>
    </message>
</context>
<context>
    <name>GroupsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="33"/>
        <source>Dashboard Widgets</source>
        <translation>Widgety Panelu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="127"/>
        <source>Add Folder</source>
        <translation>Přidat Složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="129"/>
        <source>Add a top-level folder</source>
        <translation>Přidejte složku nejvyšší úrovně</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="137"/>
        <source>Add Group</source>
        <translation>Přidat Skupinu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="139"/>
        <source>Add a group from a template</source>
        <translation>Přidejte skupinu ze šablony</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="154"/>
        <source>Title</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="155"/>
        <source>Contents</source>
        <translation>Obsah</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="245"/>
        <source>No groups yet. Use the toolbar to add a group or a folder.</source>
        <translation>Zatím žádné skupiny. Použijte panel nástrojů k přidání skupiny nebo složky.</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="410"/>
        <source>ROLL ↔</source>
        <translation>NÁKLON ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="440"/>
        <source>YAW ↻</source>
        <translation>ZATÁČENÍ ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="470"/>
        <source>PITCH ↕</source>
        <translation>KLOPENÍ ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>Zařízení HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Stránka Použití</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Použití</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Připojte gamepady, joysticky, volanty, letové ovladače a další USB zařízení třídy HID.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>Tabulky Použití HID (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="32"/>
        <source>Help Center</source>
        <translation>Centrum Nápovědy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="94"/>
        <source>Fetching help pages…</source>
        <translation>Načítání stránek nápovědy…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="128"/>
        <source>Search…</source>
        <translation>Hledat…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="246"/>
        <source>Loading…</source>
        <translation>Načítání…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="290"/>
        <source>Select a page from the sidebar</source>
        <translation>Vyberte stránku z postranního panelu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="320"/>
        <source>Copied to Clipboard</source>
        <translation>Zkopírováno do Schránky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="352"/>
        <source>View Online</source>
        <translation>Zobrazit Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="371"/>
        <source>%1 pages</source>
        <translation>%1 stránek</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="380"/>
        <source>Close</source>
        <translation>Zavřít</translation>
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
        <translation>Síťový Socket</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="271"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="274"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>CAN Bus</source>
        <translation>Sběrnice CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>USB Device</source>
        <translation>Zařízení USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>HID Device</source>
        <translation>Zařízení HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT Odběratel</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="685"/>
        <source>Your trial period has ended.</source>
        <translation>Zkušební období skončilo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="686"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Pro pokračování v používání Serial Studio aktivujte licenci.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="752"/>
        <source>channels</source>
        <translation>kanály</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="805"/>
        <source> channels</source>
        <translation>kanály</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="953"/>
        <source>Unsigned 8-bit</source>
        <translation>Bez znaménka 8-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="954"/>
        <source>Signed 16-bit</source>
        <translation>Se znaménkem 16-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="955"/>
        <source>Signed 24-bit</source>
        <translation>Se znaménkem 24-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="956"/>
        <source>Signed 32-bit</source>
        <translation>Se znaménkem 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="957"/>
        <source>Float 32-bit</source>
        <translation>Plovoucí řádová čárka 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="960"/>
        <source>Mono</source>
        <translation>Mono</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="961"/>
        <source>Stereo</source>
        <translation>Stereo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1342"/>
        <source>Input Device</source>
        <translation>Vstupní Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1350"/>
        <source>Sample Rate</source>
        <translation>Vzorkovací Frekvence</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1358"/>
        <source>Sample Format</source>
        <translation>Formát Vzorku</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1366"/>
        <source>Channels</source>
        <translation>Kanály</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>Chyba BLE I/O Modulu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>Vybrat Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>Vybrat Službu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>Vybrat Charakteristiku</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>Chyba při konfiguraci BLE služby</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>Chyba operace</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>Chyba zápisu charakteristiky</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>Chyba zápisu deskriptoru</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>Neznámá chyba</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>Chyba čtení charakteristiky</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>Chyba čtení deskriptoru</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>BLE Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>Služba</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>Notifikační Charakteristika</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>Charakteristika</translation>
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
        <translation>CAN Sběrnice Není Dostupná</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="331"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>V systému nebyly nalezeny žádné pluginy pro CAN sběrnici.

Podpora CAN sběrnice v macOS je omezená a může vyžadovat ovladače hardwaru třetích stran.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="336"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Na této platformě nejsou dostupné žádné pluginy pro CAN sběrnici.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="348"/>
        <source>Invalid CAN Configuration</source>
        <translation>Neplatná Konfigurace CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="356"/>
        <source>Invalid Selection</source>
        <translation>Neplatný Výběr</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="365"/>
        <source>No Devices Available</source>
        <translation>Žádná Dostupná Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="274"/>
        <source>CAN Device Creation Failed</source>
        <translation>Vytvoření CAN Zařízení Selhalo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="301"/>
        <source>CAN Connection Failed</source>
        <translation>CAN Připojení Selhalo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="319"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>V systému nebyly nalezeny žádné CAN moduly.

V Linuxu se ujistěte, že jsou načteny moduly jádra SOCKETCAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="325"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>V systému nebyly nalezeny žádné CAN moduly.

Ve Windows nainstalujte ovladače CAN hardware (PEAK, VECTOR atd.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="349"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>Konfigurace CAN sběrnice je neúplná. Vyberte platný modul a rozhraní.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="357"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>Vybraný modul nebo rozhraní již není dostupné. Obnovte seznamy a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="366"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>Seznam modulů nebo rozhraní je prázdný. Obnovte seznamy a ujistěte se, že je CAN hardware připojen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="275"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>Nelze vytvořit CAN zařízení. Zkontrolujte hardware a ovladače.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Nelze se připojit k CAN zařízení. Zkontrolujte hardwarové připojení a nastavení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="698"/>
        <source>CAN Bus Error</source>
        <translation>Chyba Sběrnice CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="699"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>Došlo k chybě, ale zařízení CAN již není dostupné.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="706"/>
        <source>Error code: %1</source>
        <translation>Kód chyby: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="709"/>
        <source>CAN Bus Communication Error</source>
        <translation>Chyba Komunikace Sběrnice CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="788"/>
        <source>No CAN driver selected</source>
        <translation>Nebyl vybrán žádný ovladač CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="728"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>Nejprve načtěte moduly jádra SOCKETCAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="724"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>Připojte adaptér %1 a poté obnovte</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="731"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Nejprve nastavte virtuální rozhraní CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="733"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="753"/>
        <source>No interfaces found for %1</source>
        <translation>Pro %1 nebyla nalezena žádná rozhraní</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="737"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>Nainstalujte &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;ovladače PEAK CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="741"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>Nainstalujte &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;ovladače VECTOR CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="745"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>Nainstalujte &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;ovladače SysTec CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="748"/>
        <source>Install %1 drivers</source>
        <translation>Nainstalujte ovladače %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="751"/>
        <source>Install %1 drivers for macOS</source>
        <translation>Nainstalujte ovladače %1 pro macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="765"/>
        <source>

If the interface is down, bring it up first:
sudo ip link set %1 up type can bitrate %2</source>
        <translation>Pokud je rozhraní vypnuté, nejprve jej zapněte:
sudo ip link set %1 up type can bitrate %2

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="872"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="880"/>
        <source>Interface</source>
        <translation>Rozhraní</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="888"/>
        <source>Bitrate</source>
        <translation>Přenosová Rychlost</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="897"/>
        <source>CAN FD</source>
        <translation>CAN FD</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="904"/>
        <source>Loopback</source>
        <translation>Zpětná Smyčka</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="911"/>
        <source>Listen-Only</source>
        <translation>Pouze Naslouchání</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="382"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>Inicializace knihovny libusb pro adaptér CANable se nezdařila.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="410"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>Nelze vytvořit výčet USB zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="430"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>Vybraný adaptér CANable již není připojen, nebo jej má otevřený jiná aplikace. Na Windows musí zařízení používat ovladač WinUSB (candleLight jej instaluje automaticky).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="443"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>Nelze získat USB rozhraní CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="510"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>Adaptér CANable není otevřen pro zápis.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="545"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>Nepodařilo se odeslat CAN rámec do adaptéru.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="561"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>Chyba sběrnice CAN hlášená adaptérem CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="616"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>CAN rámec nebyl potvrzen na sběrnici.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="715"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>Adaptér CANable odmítl handshake formátu hostitele.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="722"/>
        <source>Could not read CANable timing constants.</source>
        <translation>Nelze přečíst časové konstanty CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="728"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>Přenosová rychlost %1 bps není tímto adaptérem CANable podporována.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="734"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>Adaptér CANable odmítl požadovanou přenosovou rychlost.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="748"/>
        <source>Could not start the CANable channel.</source>
        <translation>Nelze spustit kanál CANable.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>Neznámá chyba</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Ověřte, že váš uživatel je ve skupině 'plugdev' nebo že pravidlo udev uděluje přístup k tomuto zařízení.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>Nepodařilo se otevřít "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>Chyba Zařízení HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>Zařízení HID bylo odpojeno nebo došlo k závažné chybě čtení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>Vybrat Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>Zařízení HID</translation>
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
        <translation>TLS 1.3 nebo Novější</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 nebo Novější</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>Jakýkoliv Protokol</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>Pouze Zabezpečené Protokoly</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>Žádné</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>Dotázat Protějšek</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>Ověřit Protějšek</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>Automaticky Ověřit Protějšek</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>Funkce MQTT Vyžaduje Komerční Licenci</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">Přihlášení k MQTT brokeru je dostupné pouze s platnou komerční licencí Serial Studio (úroveň Hobbyist nebo vyšší).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>Přihlášení k MQTT brokeru je dostupné pouze s platnou licencí Serial Studio nebo aktivní zkušební verzí.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>Použít Systémovou Databázi</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>Načíst Ze Složky…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Vybrat Adresář Certifikátů PEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>Název Hostitele</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>Filtr Tématu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>ID Klienta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>Uživatelské Jméno</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>Heslo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>Verze MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Vyčistit Relaci</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Automatický Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS Povoleno</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>Protokol SSL</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>Režim Ověření Protějšku</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>Hloubka Ověření Protějšku</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>Chyba Odběru MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Nepodařilo se odebírat téma "%1".</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Neplatná Verze Protokolu MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>Broker odmítl nakonfigurovanou verzi protokolu MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>ID Klienta Odmítnuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>Broker odmítl ID klienta. Zkuste jiný identifikátor.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>MQTT Server Nedostupný</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>Broker je momentálně nedostupný. Opakujte akci později.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>Chyba Autentizace</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>Poskytnuté přihlašovací údaje byly brokerem odmítnuty.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>Chyba Autorizace</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>Účet nemá oprávnění pro tuto operaci.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>Chyba Sítě nebo Transportní Vrstvy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Problém síťové/transportní vrstvy při připojování k brokeru.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>Porušení Protokolu MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>Broker nahlásil porušení protokolu a ukončil spojení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>Chyba MQTT 5</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>Došlo k chybě protokolu MQTT 5.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>Chyba MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>Došlo k neočekávané chybě MQTT.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>Neplatný Sériový Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Inicializace Modbus Selhala</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Připojení Modbus Selhalo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>Vybraný sériový port „%1" již není dostupný. Obnovte seznam portů a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Nelze vytvořit zařízení Modbus. Zkontrolujte konfiguraci systému a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Nelze se připojit k „%1". Zkontrolujte nastavení připojení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="609"/>
        <source>None</source>
        <translation>Žádná</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="610"/>
        <source>Even</source>
        <translation>Sudá</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="611"/>
        <source>Odd</source>
        <translation>Lichá</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="612"/>
        <source>Space</source>
        <translation>Mezera</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="613"/>
        <source>Mark</source>
        <translation>Značka</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="665"/>
        <source>Holding Registers (0x03)</source>
        <translation>Holding Registry (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="666"/>
        <source>Input Registers (0x04)</source>
        <translation>Input Registry (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="667"/>
        <source>Coils (0x01)</source>
        <translation>Cívky (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="668"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Diskrétní Vstupy (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>No register groups configured</source>
        <translation>Nejsou nakonfigurovány žádné skupiny registrů</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="851"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Před vygenerováním projektu přidejte alespoň jednu skupinu registrů.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="865"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="890"/>
        <source>Modbus Project Generator</source>
        <translation>Generátor Projektů Modbus</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Nepodařilo se vytvořit dočasný soubor projektu</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Zkontrolujte oprávnění k zápisu do dočasného adresáře.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="862"/>
        <source>Failed to load generated project</source>
        <translation>Nepodařilo se načíst vygenerovaný projekt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="863"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>Vygenerovaný projekt JSON se nepodařilo načíst.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="885"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Projekt úspěšně vygenerován s %1 skupinami a %2 datovými sadami.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="888"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editor projektu je nyní otevřen pro přizpůsobení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="903"/>
        <source>Modbus Project</source>
        <translation>Projekt Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="928"/>
        <source>Holding Registers</source>
        <translation>Holding Registry</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="929"/>
        <source>Input Registers</source>
        <translation>Vstupní Registry</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="930"/>
        <source>Coils</source>
        <translation>Cívky</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="931"/>
        <source>Discrete Inputs</source>
        <translation>Diskrétní Vstupy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="945"/>
        <source>Unknown</source>
        <translation>Neznámý</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="958"/>
        <source>Register %1</source>
        <translation>Registr %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Coil %1</source>
        <translation>Cívka %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Discrete %1</source>
        <translation>Diskrétní %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1372"/>
        <source>Error code: %1</source>
        <translation>Kód chyby: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1375"/>
        <source>Modbus Communication Error</source>
        <translation>Chyba Komunikace Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1387"/>
        <source>Select Port</source>
        <translation>Vybrat Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1540"/>
        <source>Protocol</source>
        <translation>Protokol</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1548"/>
        <source>Slave Address</source>
        <translation>Adresa Slave</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1557"/>
        <source>Poll Interval (ms)</source>
        <translation>Interval Dotazování (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1597"/>
        <source>Host / IP</source>
        <translation>Host / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1604"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1619"/>
        <source>Serial Port</source>
        <translation>Sériový Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1627"/>
        <source>Baud Rate</source>
        <translation>Přenosová Rychlost</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1635"/>
        <source>Parity</source>
        <translation>Parita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1643"/>
        <source>Data Bits</source>
        <translation>Datové Bity</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1651"/>
        <source>Stop Bits</source>
        <translation>Stop Bity</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="547"/>
        <source>Network socket error</source>
        <translation>Chyba síťového socketu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="563"/>
        <source>Socket Type</source>
        <translation>Typ Socketu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="571"/>
        <source>Remote Address</source>
        <translation>Vzdálená Adresa</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="579"/>
        <source>TCP Port</source>
        <translation>TCP Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="588"/>
        <source>UDP Local Port</source>
        <translation>UDP Lokální Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="597"/>
        <source>UDP Remote Port</source>
        <translation>UDP Vzdálený Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="606"/>
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
        <translation>Nepodařilo se spustit proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Spustitelný soubor „%1" nebyl nalezen v PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>Vybrat Spustitelný Soubor</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>Vybrat Pracovní Adresář</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Vybrat Pojmenovanou Rouru / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>Proces havaroval.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>Ukončovací kód: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>Proces „%1" zastaven</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>Neznámá chyba</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>Chyba Procesu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>Chyba Roury</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>Nelze otevřít pojmenovanou rouru: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>Režim</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>Spustit Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>Pojmenovaná Roura</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>Spustitelný Soubor</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>Argumenty</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>Pracovní Adresář</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>Cesta k Rouře</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="206"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>Přenosová rychlost %1 bps není podporována analyzátorem USB-CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="216"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Nelze otevřít sériový port %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="227"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>Inicializace analyzátoru USB-CAN se nezdařila.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="264"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>Analyzátor USB-CAN není otevřen pro zápis.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="310"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>Analyzátor USB-CAN hlásí chybu sběrnice CAN.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="167"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>Přenosová rychlost %1 bps není standardní rychlost slcan.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="177"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Nelze otevřít sériový port %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="190"/>
        <source>Failed to open the slcan channel.</source>
        <translation>Otevření kanálu slcan se nezdařilo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="229"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>Adaptér slcan není otevřen pro zápis.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="267"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>Chyba sběrnice CAN nahlášená adaptérem slcan.</translation>
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
        <translation>Žádná</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="349"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="733"/>
        <source>Select Port</source>
        <translation>Vybrat Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="392"/>
        <source>Even</source>
        <translation>Sudá</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="393"/>
        <source>Odd</source>
        <translation>Lichá</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="394"/>
        <source>Space</source>
        <translation>Mezera</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="395"/>
        <source>Mark</source>
        <translation>Značka</translation>
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
        <translation>„%1" není platná cesta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="563"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Zadejte jinou cestu pro registraci vlastního sériového zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="823"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Zadané zařízení nebylo nalezeno. Zkontrolujte připojení a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="830"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>Došlo k neznámé chybě. Zkontrolujte zařízení a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="832"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Zařízení není otevřené. Před provedením této operace otevřete zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="261"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Připojení k sériovému portu „%1" se nezdařilo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="798"/>
        <source>Unknown</source>
        <translation>Neznámá</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="799"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Kritická chyba na sériovém portu „%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="800"/>
        <source>Unknown error</source>
        <translation>Neznámá chyba</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="822"/>
        <source>No error occurred.</source>
        <translation>Nedošlo k žádné chybě.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="824"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Přístup odepřen. Ujistěte se, že aplikace má potřebná přístupová práva k zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="825"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Otevření zařízení selhalo. Může být již používáno nebo nedostupné.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="826"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>Při zápisu dat do zařízení došlo k chybě.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>Při čtení dat ze zařízení došlo k chybě.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>Došlo ke kritické chybě prostředku. Zařízení mohlo být odpojeno nebo již není přístupné.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>Požadovaná operace není na tomto zařízení podporována.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="831"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>Operace vypršela. Zařízení možná neodpovídá.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Serial Port</source>
        <translation>Sériový Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Baud Rate</source>
        <translation>Přenosová Rychlost</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Parity</source>
        <translation>Parita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Data Bits</source>
        <translation>Datové Bity</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>Stop Bits</source>
        <translation>Stop Bity</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1034"/>
        <source>Flow Control</source>
        <translation>Řízení Toku</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1042"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1049"/>
        <source>Auto-Reconnect</source>
        <translation>Automatické Připojení</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="156"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="164"/>
        <source>USB Error</source>
        <translation>Chyba USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="157"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Inicializace subsystému USB se nezdařila. Zkontrolujte, zda je v systému dostupná knihovna libusb.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="214"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="572"/>
        <source>USB Device Error</source>
        <translation>Chyba Zařízení USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>Zařízení USB nelze otevřít: %1.

V Linuxu zajistěte oprávnění ke čtení/zápisu uzlu zařízení (přidejte pravidlo udev nebo spusťte jako root). V macOS může být nutné nejprve odpojit ovladač jádra.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="165"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Není vybráno žádné zařízení USB. Vyberte zařízení a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="171"/>
        <source>Unknown Device</source>
        <translation>Neznámé Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <source>Failed to open "%1"</source>
        <translation>Otevření „%1" se nezdařilo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="215"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Rozhraní %1 na zařízení USB nelze získat.

Jiný ovladač nebo aplikace jej může mít již otevřené. V Linuxu zkuste uvolnit ovladač jádra (např. cdc_acm) nebo přidat pravidlo udev.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="360"/>
        <source>Select Device</source>
        <translation>Vybrat Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="379"/>
        <source>Select IN Endpoint</source>
        <translation>Vybrat Vstupní Koncový Bod</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="390"/>
        <source>None (Read-only)</source>
        <translation>Žádný (Pouze Čtení)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="464"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Povolit pokročilé řídící přenosy USB?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="465"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Toto umožňuje řídící přenosy kromě hromadných přenosů. Odesílání nesprávných řídících požadavků může způsobit pád nebo poškození připojeného hardwaru. Povolit pouze v případě, že víte, co děláte.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="469"/>
        <source>Advanced USB Mode</source>
        <translation>Pokročilý Režim USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="573"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>Zařízení USB bylo odpojeno nebo došlo k závažné chybě čtení.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="710"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Na tomto zařízení nebyl nalezen žádný izochronní IN endpoint, ale jsou k dispozici hromadné endpointy.

Přepněte režim přenosu na „Hromadný proud" a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="715"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Na tomto zařízení nebyl nalezen žádný hromadný IN endpoint, ale jsou k dispozici izochronní endpointy.

Přepněte režim přenosu na „Izochronní" a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="719"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Na tomto zařízení nebyl nalezen žádný použitelný IN endpoint.

Zařízení nemusí vystavovat datové endpointy ve své aktivní konfiguraci nebo může vyžadovat specifický ovladač.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1211"/>
        <source>USB Device</source>
        <translation>Zařízení USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1219"/>
        <source>Transfer Mode</source>
        <translation>Režim Přenosu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Bulk Stream</source>
        <translation>Hromadný Proud</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Advanced Control</source>
        <translation>Pokročilé Řízení</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Isochronous</source>
        <translation>Izochronní</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1227"/>
        <source>IN Endpoint</source>
        <translation>Koncový Bod IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1235"/>
        <source>OUT Endpoint</source>
        <translation>Koncový Bod OUT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1243"/>
        <source>ISO Packet Size</source>
        <translation>Velikost ISO Paketu</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>Nebyl vybrán žádný soubor…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>Prostý Text</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Binární Data</translation>
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
        <translation>Vybrat soubor k přenosu</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>Vybraný soubor: %1 (%2 bajtů)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>Chyba při otevírání souboru: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>Zahajování přenosu %1…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>Přenos dokončen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>Přenos prostého textu dokončen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Přenos binárních dat dokončen (%1 bajtů)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>Přenos dokončen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Přenos úspěšně dokončen (%1 bajtů)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>Přenos selhal: %1</translation>
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
        <translation>Zahozené rámce</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>Příchozí data přicházejí rychleji, než je Serial Studio schopno zpracovat; %1 rámec(ů) bylo zahozeno. Snižte datový tok nebo vypněte náročného spotřebitele.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>Nelze otevřít soubor: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>Přenos zrušen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>Přenos zrušen uživatelem</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>Čekání na příjemce…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Příjemce připraven (režim CRC), odesílání dat…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Příliš mnoho opakování, přenos přerušen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>Překročen maximální počet opakování</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>Přijat NAK, opakování bloku %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>Selhalo posunutí v souboru</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Přenos zrušen příjemcem</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Příjemce zrušil přenos</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>Přenos dokončen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>Čtení souboru vrátilo více dat, než bylo požadováno</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Odesílání bloku %1 (%2 bajtů)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>Odesílání EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Časový limit, opakování (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>Časový limit přenosu vypršel</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>Časový limit: žádná odpověď od příjemce</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>Nelze otevřít soubor: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Přenos zrušen příjemcem</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Příjemce zrušil přenos</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>Čekání na příjemce…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>Odesílání prvního EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Příliš mnoho opakování, přenos přerušen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>Překročen maximální počet opakování</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>Přijat NAK, opakování bloku %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>Nelze vyhledat v souboru</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>Odesílání druhého EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>Odesílání značky konce dávky…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>Přenos dokončen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Odesílání hlavičky souboru: %1 (%2 bajtů)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Odesílání bloku %1 (%2/%3 bajtů)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>Nelze otevřít soubor: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Soubor je příliš velký pro ZMODEM (%1 bajtů, limit 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>Přenos zrušen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>Přenos zrušen uživatelem</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>Neshoda CRC hlavičky hex, zahazuji rámec</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Odesílám informace o souboru: %1 (%2 bajtů)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>Selhalo přesunutí na pozici %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>Čtení souboru vrátilo více dat, než bylo požadováno</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Příjemce požaduje data od pozice %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>Příjemce přeskočil soubor</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>Příliš mnoho chyb, přenos přerušen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>Překročen maximální počet opakování</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>Odesílám ZRQINIT, čekám na příjemce…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Data souboru odeslána, čekám na potvrzení…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>Odesílání ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Přijímač připraven, odesílání informací o souboru…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>Přijat NAK, opakování pokusu (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>Přenos dokončen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Přenos zrušen přijímačem</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Přijímač zrušil přenos</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>Přijímač hlásí chybu souboru</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>Časový limit přenosu vypršel</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>Časový limit: žádná odpověď od přijímače</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Časový limit vypršel, opakování pokusu (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="41"/>
        <source>Select Icon</source>
        <translation>Vybrat Ikonu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="118"/>
        <source>Search Online…</source>
        <translation>Hledat Online…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="131"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="143"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>Normální</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>Stupně Šedi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Vysoký Kontrast</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Živé</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Noční Vidění</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Infračervené</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Tmavě Modrá</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Jantarová</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Exportovat Obrázky</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Otevřít Složku Exportu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Přiblížit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Oddálit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Zobrazit Zaměřovač</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>Čekání na Obrázek…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>API Klíče</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. Výchozí je Claude Haiku 4.5 ($1 vstup / $5 výstup na milion tokenů). K dispozici jsou také Sonnet 4.6 a OPUS 4.7. Podporuje streamování, použití nástrojů, rozšířené myšlení a cachování promptů.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. Výchozí je GPT-4o mini ($0.15 vstup / $0.60 výstup na milion tokenů). K dispozici jsou také GPT-4o, GPT-4 Turbo a o1-mini.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. Výchozí je Gemini 2.0 Flash, který má velkorysou bezplatnou úroveň (podléhá omezení rychlosti). K dispozici jsou také Gemini 1.5 Pro a Gemini 1.5 Flash.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Použijte vlastní API klíče. Jsou šifrovány v klidu pomocí klíče specifického pro počítač a nikdy neopustí váš počítač kromě komunikace s vybraným poskytovatelem.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>Klíč nastaven</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>Žádný klíč</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">Klíč je uložen — vložte nový pro jeho nahrazení</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. Výchozí je GPT-5 mini pro rychlou a cenově výhodnou agentní práci. GPT-5.2 je silnější univerzální varianta a GPT-5.2 Chat sleduje model aktuálně používaný v ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. API kompatibilní s OpenAI. Výchozí je deepseek-chat (V3). K dispozici je také deepseek-reasoner (R1). Často nejlevnější cloudová varianta pro použití nástrojů.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. Jeden klíč, ~200 modelů od Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen a dalších. Modely s bezplatnou úrovní (s příponou :free) mají omezení rychlosti, ale nevyžadují další fakturaci. Zbytek je placený podle spotřeby.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Hardwarově akcelerované odvozování (LPU) pro velmi rychlé modely Llama, Mixtral, Gemma, DeepSeek-R1 distill a Qwen. Velkorysá bezplatná úroveň s denními limity tokenů.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. Výchozí je Mistral Large. Codestral je zaměřen na doplňování kódu, Pixtral zpracovává obraz a modely Ministral jsou optimalizovány pro hraniční/nízkolatentní použití.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Lokální modelový server. Funguje s jakýmkoli endpointem kompatibilním s OpenAI -- Ollama, llama-server z llama.cpp, LM Studio nebo vLLM. Nic neopouští váš počítač. Seznam modelů je dotazován živě ze serveru.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>Klíč je uložen -- vložte nový pro jeho nahrazení</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>Vložte sem svůj API klíč</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>Skrýt klíč</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>Zobrazit klíč při psaní</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>Získat klíč</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Otevřít konzoli poskytovatele pro vytvoření nového klíče</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>Uložit</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>Odebrat uložený klíč pro %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Instalovat Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Otevřít stránku stahování Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>Použít</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>Znovu načíst seznam modelů</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Zatím nejsou nakonfigurovány žádné API klíče. Přidejte klíč pro začátek.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">Zatím nejsou nakonfigurovány žádné API klíče. Přidejte alespoň jeden výše pro začátek.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>Jeden poskytovatel je připraven.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 poskytovatelů je připraveno.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Licencování</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Čekejte prosím…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Aktivovat Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Vložte níže svůj licenční klíč pro odemčení funkcí Pro, jako jsou MQTT, 3D grafy a další.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>Vaše licence zahrnuje 5 aktivací zařízení.
Plány zahrnují měsíční, roční a doživotní možnosti.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Vložte sem svůj licenční klíč…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="333"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="382"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Vložit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="339"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="388"/>
        <source>Select All</source>
        <translation>Vybrat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="235"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="426"/>
        <source>Product</source>
        <translation>Produkt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="242"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="253"/>
        <source>Licensee</source>
        <translation>Držitel Licence</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="272"/>
        <source>Licensee E-Mail</source>
        <translation>E-mail Držitele Licence</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="289"/>
        <source>Device Usage</source>
        <translation>Využití Zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 zařízení v provozu (Neomezený tarif)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="298"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 z %2 zařízení využito</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="308"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="443"/>
        <source>Device ID</source>
        <translation>ID Zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="355"/>
        <source>License Key</source>
        <translation>Licenční Klíč</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="433"/>
        <source>Serial Studio %1 (Offline)</source>
        <translation>Serial Studio %1 (Offline)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="460"/>
        <source>Expires</source>
        <translation>Vyprší</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="467"/>
        <source>%1 (%2 days left)</source>
        <translation>%1 (zbývá %2 dní)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="480"/>
        <source>Your offline license expires soon. Request a new certificate to stay activated.</source>
        <translation>Vaše offline licence brzy vyprší. Vyžádejte si nový certifikát pro zachování aktivace.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="497"/>
        <source>Customer Portal</source>
        <translation>Zákaznický Portál</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="508"/>
        <source>Buy License</source>
        <translation>Koupit Licenci</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="515"/>
        <source>Activate</source>
        <translation>Aktivovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="526"/>
        <source>Activate Offline…</source>
        <translation>Aktivovat Offline…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="535"/>
        <source>Deactivate</source>
        <translation>Deaktivovat</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="518"/>
        <source>There was an issue validating your license.</source>
        <translation>Při ověřování licence došlo k problému.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="536"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="717"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="836"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>Zadaný licenční klíč nepatří k Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="537"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Zkontrolujte, že jste licenci zakoupili v oficiálním obchodě Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="548"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="726"/>
        <source>This license key was activated on a different device.</source>
        <translation>Tento licenční klíč byl aktivován na jiném zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="549"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="727"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Nejprve jej tam deaktivujte nebo kontaktujte podporu.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="560"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>This license is not currently active.</source>
        <translation>Tato licence není aktuálně aktivní.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="561"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="738"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Mohla vypršet nebo být deaktivována (stav: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="571"/>
        <source>Something went wrong on the server.</source>
        <translation>Na serveru došlo k chybě.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="572"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="748"/>
        <source>No activation ID was returned.</source>
        <translation>Nebylo vráceno ID aktivace.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="582"/>
        <source>Could not validate your license at this time.</source>
        <translation>Licenci se nyní nepodařilo ověřit.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="583"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="757"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="846"/>
        <source>Try again later.</source>
        <translation>Zkuste to znovu později.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="718"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="837"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Zkontrolujte, že jste licenci zakoupili v oficiálním obchodě Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="747"/>
        <source>Something went wrong on the server…</source>
        <translation>Na serveru se něco pokazilo…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="601"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="603"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="654"/>
        <source>Your license has been successfully activated.</source>
        <translation>Vaše licence byla úspěšně aktivována.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="655"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Děkujeme za podporu Serial Studio!
Nyní máte přístup ke všem prémiové funkcím.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="709"/>
        <source>There was an issue activating your license.</source>
        <translation>Při aktivaci licence došlo k problému.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="756"/>
        <source>Could not activate your license at this time.</source>
        <translation>Licenci nelze v tuto chvíli aktivovat.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="828"/>
        <source>There was an issue deactivating your license.</source>
        <translation>Při deaktivaci licence došlo k problému.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="845"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>V tuto chvíli nelze deaktivovat vaši licenci.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="855"/>
        <source>Your license has been deactivated.</source>
        <translation>Vaše licence byla deaktivována.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="856"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Přístup k funkcím Pro byl odebrán.
Děkujeme znovu za podporu Serial Studio!</translation>
    </message>
</context>
<context>
    <name>Licensing::OfflineLicense</name>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="186"/>
        <source>Remove the offline license from this device?</source>
        <translation>Odstranit offline licenci z tohoto zařízení?</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="187"/>
        <source>This deletes the local activation certificate, but it does not free up your license seat automatically.

To reuse this seat on another device, email alex@serial-studio.com with your device ID or your .ssmachine file so the seat can be released manually.</source>
        <translation>Tímto se odstraní místní aktivační certifikát, ale automaticky se neuvolní vaše licenční místo.

Chcete-li toto místo použít na jiném zařízení, napište e-mail na alex@serial-studio.com s vaším ID zařízení nebo souborem .ssmachine, aby mohlo být místo uvolněno ručně.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="192"/>
        <source>Offline Activation</source>
        <translation>Offline Aktivace</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="213"/>
        <source>Could not open the certificate file.</source>
        <translation>Nelze otevřít soubor certifikátu.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="220"/>
        <source>The certificate file is too large to be valid.</source>
        <translation>Soubor certifikátu je příliš velký, aby byl platný.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="247"/>
        <source>Could not write the device file.</source>
        <translation>Nelze zapsat soubor zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="285"/>
        <source>The stored certificate could not be decrypted on this machine.</source>
        <translation>Uložený certifikát nelze na tomto počítači dešifrovat.</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>Export MDF4 je funkce Pro.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Tato funkce vyžaduje licenci. Zakupte si ji prosím pro povolení exportu MDF4.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>Vybrat soubor MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>Soubory MDF4 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>Odpojit od zařízení?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>Před otevřením souboru MDF4 se musíte odpojit od aktuálního zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>Nelze otevřít soubor MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Soubor může být poškozen nebo v nepodporovaném formátu.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>Neplatný soubor MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Nepodařilo se přečíst strukturu souboru. Soubor může být poškozen.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>Žádná data v souboru</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>Soubor MDF4 neobsahuje žádná měřicí data.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>Hostname</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>např. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Filtr Tématu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>např. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>ID Klienta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Regenerovat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Uživatelské Jméno</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Heslo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>Verze</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Vyčistit Relaci</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>Použít SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>SSL Protokol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Ověření Protějšku</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Hloubka Ověření</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>CA Certifikáty</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Načíst Ze Složky…</translation>
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
        <translation type="vanished">TLS 1.3 nebo Novější</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 nebo Novější</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">Jakýkoliv Protokol</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">Pouze Zabezpečené Protokoly</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Žádné</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">Dotázat Protějšek</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">Ověřit Protějšek</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">Automaticky Ověřit Protějšek</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">Použít Systémovou Databázi</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">MQTT Odběratel</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">Vydavatel MQTT</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">Funkce MQTT vyžaduje komerční licenci</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">Načíst Ze Složky…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">Připojení k MQTT brokerům je dostupné pouze s platnou komerční licencí Serial Studio (úroveň Hobbyist nebo vyšší).

Pro odemknutí této funkce aktivujte licenci nebo navštivte obchod.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">Chybí Téma MQTT</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">Před připojením jako vydavatel musíte zadat téma.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">Chyba Konfigurace</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">Téma MQTT Není Nastaveno</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">Nebudete přijímat žádné zprávy, dokud nebude nakonfigurováno téma.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">Upozornění Konfigurace</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">Neplatné MQTT Téma</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Téma "%1" není platné.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">Vybrat Adresář Certifikátů PEM</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">Chyba Odběru</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Nepodařilo se odebírat téma "%1".</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">Neplatná Verze Protokolu MQTT</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">MQTT broker odmítl připojení kvůli nepodporované verzi protokolu. Ujistěte se, že klient i broker podporují stejnou verzi protokolu.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">ID Klienta Odmítnuto</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">Broker odmítl ID klienta. Může být nesprávně formátováno, příliš dlouhé nebo již používáno. Zkuste použít jiné ID klienta.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">MQTT Server Nedostupný</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">Síťové připojení bylo navázáno, ale broker je momentálně nedostupný. Ověřte stav brokeru a zkuste to znovu později.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">Chyba Autentizace</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">Zadané uživatelské jméno nebo heslo je nesprávné nebo chybně formátované. Zkontrolujte přihlašovací údaje a zkuste to znovu.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">Chyba Autorizace</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">MQTT broker odmítl připojení z důvodu nedostatečných oprávnění. Ujistěte se, že účet má potřebná přístupová práva.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">Chyba Sítě nebo Transportní Vrstvy</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">Došlo k problému na síťové nebo transportní vrstvě, což způsobilo neočekávané selhání připojení. Zkontrolujte síťové připojení a nastavení brokeru.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">Porušení Protokolu MQTT</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">Klient detekoval porušení protokolu MQTT a ukončil připojení. Zkontrolujte implementaci MQTT z hlediska souladu s protokolem.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">Neznámá Chyba</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">Došlo k neočekávané chybě. Zkontrolujte protokoly nebo restartujte aplikaci.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">Chyba MQTT 5</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">Došlo k chybě protokolu MQTT úrovně 5. Zkontrolujte protokoly brokeru nebo kódy důvodu.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">Autentizace MQTT Selhala</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">Autentizace selhala: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">Je vyžadována rozšířená autentizace, ale MQTT 5.0 není povoleno.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">Neznámý</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">Vyžadována Autentizace MQTT</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">Broker MQTT vyžaduje autentizaci metodou: "%1".

Zadejte potřebná pověření.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">Zadejte Uživatelské Jméno MQTT</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">Uživatelské Jméno:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">Zadejte Heslo MQTT</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">Heslo:</translation>
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
        <translation>TLS 1.3 nebo Novější</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 nebo Novější</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>Jakýkoliv Protokol</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>Pouze Zabezpečené Protokoly</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>Žádná</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>Dotázat Protějšek</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>Ověřit Protějšek</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>Automaticky Ověřit Protějšek</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>Surová RX Data</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>Vlastní Skript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Data Dashboardu (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Data Dashboardu (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>MQTT publisher není dostupný</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>Pro použití MQTT publikování je vyžadována platná komerční licence.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>Test Připojení MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Vybrat Adresář Certifikátů PEM</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>MQTT broker je dostupný</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>MQTT broker není dostupný</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>Připojení k MQTT brokeru selhalo</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>Vydavatel MQTT</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="50"/>
        <source>MQTT Publisher Script</source>
        <translation>Skript Vydavatele MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="87"/>
        <source>JavaScript</source>
        <translation>Javascript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="87"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="93"/>
        <source>Sample frame bytes (text or hex)</source>
        <translation>Ukázkové bajty rámce (text nebo hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="100"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Apply</source>
        <translation>Použít</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="103"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="112"/>
        <source>Language:</source>
        <translation>Jazyk:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="115"/>
        <source>Template:</source>
        <translation>Šablona:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="126"/>
        <source>Frame:</source>
        <translation>Rámec:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="130"/>
        <source>Output:</source>
        <translation>Výstup:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="270"/>
        <source>Enter a frame</source>
        <translation>Zadejte rámec</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="277"/>
        <source>Invalid hex</source>
        <translation>Neplatný hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formátovat Dokument	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="361"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formátovat Výběr	ctrl+i</translation>
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
-- Definujte funkci mqtt(frame), která přijímá surové bajty
-- jednoho parsovaného rámce a vrací payload pro publikování
-- na broker, nebo nil pro přeskočení tohoto rámce.
--
-- Argument frame odpovídá tomu, co vidí skript Frame Parser:
-- Lua řetězec obsahující bajty mezi oddělovači FrameReader.
--
-- Příklad:
--   function mqtt(frame)
--     return frame    -- přeposlat beze změny
--   end
--
-- Použijte rozbalovací nabídku Šablona pro hotové příklady.
--

</translation>
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
 * Definujte funkci mqtt(frame), která přijímá surové bajty
 * jednoho parsovaného rámce a vrací payload pro publikování
 * na broker, nebo null pro přeskočení tohoto rámce.
 *
 * Argument frame odpovídá tomu, co vidí skript Frame Parser:
 * řetězec obsahující bajty mezi oddělovači FrameReader.
 *
 * Příklad:
 *   function mqtt(frame) {
 *     return frame;   // přeposlat beze změny
 *   }
 *
 * Použijte rozbalovací nabídku Šablona pro hotové příklady.
 */
</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="615"/>
        <source>Script is empty</source>
        <translation>Skript je prázdný</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="622"/>
        <source>Lua engine error</source>
        <translation>Chyba Lua enginu</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="696"/>
        <source>Error: %1</source>
        <translation>Chyba: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="652"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() není definována</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- rámec přeskočen)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="671"/>
        <source>(non-string return)</source>
        <translation>(non-string return)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="701"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- rámec přeskočen)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="779"/>
        <source>Select Template…</source>
        <translation>Vybrat Šablonu…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Před testováním připojení nakonfigurujte název hostitele a port brokeru.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Úspěšně připojeno k %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Časový limit vypršel po 5 sekundách bez dosažení brokeru.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">Nastavení MQTT</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT je funkce Pro</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">Aktivujte licenci nebo navštivte obchod pro odemčení podpory MQTT.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">Obecné</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">Autentizace</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">Možnosti MQTT</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">Vlastnosti SSL</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">Hostitel</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">Port</translation>
    </message>
    <message>
        <source>Client ID</source>
        <translation type="vanished">ID Klienta</translation>
    </message>
    <message>
        <source>Keep Alive (s)</source>
        <translation type="vanished">Keep Alive (s)</translation>
    </message>
    <message>
        <source>Clean Session</source>
        <translation type="vanished">Vyčistit Relaci</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">Uživatelské Jméno</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">Uživatelské Jméno MQTT</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">Heslo</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">Heslo MQTT</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">Verze</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">Režim</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">Téma</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">např. sensors/temperature nebo home/+/status</translation>
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
        <translation type="vanished">Will Téma</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">např. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will Zpráva</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">např. Zařízení bylo neočekávaně odpojeno</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">Povolit SSL</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">Protokol SSL</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">Hloubka Ověření</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">Režim Ověření</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Zavřít</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">Odpojit</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">Připojit</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="205"/>
        <source>Console Only Mode</source>
        <translation>Režim Pouze Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="208"/>
        <source>Quick Plot Mode</source>
        <translation>Režim Rychlého Grafu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="215"/>
        <source>Empty Project</source>
        <translation>Prázdný Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="696"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="704"/>
        <source>Waiting for data…</source>
        <translation>Čekání na data…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="705"/>
        <source>Connecting to device…</source>
        <translation>Připojování k zařízení…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Ukázka kódu</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Doplňovač</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Zvýrazňovač</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Styl</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>mezer</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>Zkopírováno do Schránky</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="23"/>
        <source>MDF4 Player</source>
        <translation>Přehrávač MDF4</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Chyba</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Vy</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Zjišťování</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Provádění</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>Schválit %1 akcí v %2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Tato volání budou provedena společně. Rozbalte jednotlivé karty pro kontrolu argumentů.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Schválit vše</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Zamítnout vše</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Kopírovat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Vybrat Vše</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>Vy</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>Chyba</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Zjišťování</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Provádění</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>Spuštěno</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>Čeká na schválení</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Hotovo</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Zamítnuto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Blokováno</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Schválit</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Zamítnout</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Schválit vše</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Zamítnout vše</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Argumenty</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Výsledek</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>Schválit {n} akcí v {family}?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Tyto volání budou provedena společně. Rozbalte každou kartu pro kontrolu argumentů.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
</context>
<context>
    <name>Misc::DemoLauncher</name>
    <message>
        <location filename="../../src/Misc/DemoLauncher.cpp" line="194"/>
        <source>Cannot start the demo</source>
        <translation>Nelze spustit demo</translation>
    </message>
    <message>
        <location filename="../../src/Misc/DemoLauncher.cpp" line="195"/>
        <source>The demo project could not be staged or opened. Check the application data directory permissions.</source>
        <translation>Projekt demo nemohl být připraven nebo otevřen. Zkontrolujte oprávnění adresáře aplikačních dat.</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>Nepodařilo se načíst README: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>Motiv</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>Analyzátor Rámců</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>Šablona Projektu</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>Všechny Typy</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>Resetovat Rozšíření</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Toto odinstaluje všechna rozšíření, odstraní všechny vlastní repozitáře a obnoví výchozí nastavení. Pokračovat?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>Vybrat Složku Repozitáře Rozšíření</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1017"/>
        <source>Installed (repository no longer available)</source>
        <translation>Nainstalováno (repozitář již není dostupný)</translation>
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
        <translation>Chyba Pluginu</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Plugin "%1" není nainstalován.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1335"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>Rozšíření "%1" není plugin (typ: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1356"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Nelze načíst soubor metadat pluginu:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Plugin "%1" vyžaduje GRPC, ale toto sestavení neobsahuje podporu GRPC.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Tento plugin používá GRPC pro vysokovýkonné streamování dat. API server musí být povolen.

Chcete jej nyní povolit?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1393"/>
        <source>API Server Required</source>
        <translation>Vyžadován API Server</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1422"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Plugin "%1" nemá pole 'entry' v info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Entry point not found:
%1</source>
        <translation>Vstupní bod nenalezen:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Plugin "%1" má neplatnou cestu ke vstupnímu bodu.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1484"/>
        <source>Missing Dependency</source>
        <translation>Chybějící Závislost</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Tento plugin vyžaduje "%1", ale nebyl nalezen ve vašem systému.

Chcete otevřít stránku pro stažení?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>Pluginy potřebují API server pro komunikaci se Serial Studio. Chcete jej nyní povolit?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Vyžadován Restart</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>Nové vykreslovací rozhraní se projeví po restartování Serial Studio. Restartovat nyní pro použití změny?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>Načtení stránky selhalo: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="152"/>
        <source>Invalid icon identifier</source>
        <translation>Neplatný identifikátor ikony</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="222"/>
        <source>Empty SVG data received</source>
        <translation>Přijata prázdná SVG data</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Zástupce Windows (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>Aplikace macOS (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Položka Plochy (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Pro nejostřejší výsledek ve Finderu a Docku použijte ikonu .icns.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Ponechte ikonu prázdnou pro zdědění ikony spustitelného souboru Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Umístěte soubor do ~/.local/share/applications/ pro zobrazení ve spouštěči aplikací.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Obraz Ikony Apple (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Ikona Windows (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>Vektorový nebo Rastrový Obrázek (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>Pro generování zástupců je vyžadována licence Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>Nebyla poskytnuta výstupní cesta.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>Nepodařilo se zapsat soubor zástupce.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Nelze nahradit existující zástupce na %1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Nelze vytvořit adresářovou strukturu balíčku .app.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Nelze zapsat spouštěč balíčku: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Nelze označit spouštěč balíčku jako spustitelný.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Nelze zapsat Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Zapisovač zástupců Windows není na této platformě dostupný.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Zapisovač zástupců Linux není na této platformě dostupný.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>Nelze inicializovat COM (vyžadováno pro zápis zástupců .lnk).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) selhalo (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) selhalo (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Uložení souboru .lnk selhalo (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>Zapisovač zástupců macOS není na této platformě dostupný.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Nelze otevřít cestu zástupce pro zápis: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Zástupce Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Nelze označit zástupce jako spustitelný.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>Systém</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">Kontrolovat aktualizace automaticky?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">Má %1 automaticky kontrolovat aktualizace? Aktualizace lze vždy zkontrolovat ručně v dialogu „O aplikaci"</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>Uložit</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>Uložit vše</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>Otevřít</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>Ano</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>Ano pro vše</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>Ne</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>Ne všem</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>Přerušit</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>Opakovat</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>Ignorovat</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>Zahodit</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>Nápověda</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>Použít</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>Resetovat</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>Obnovit výchozí</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>Vybrat Umístění Pracovního Prostoru</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>Protokol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="70"/>
        <source>Serial Port</source>
        <translation>Sériový Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="93"/>
        <source>Baud Rate</source>
        <translation>Přenosová Rychlost</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="184"/>
        <source>Parity</source>
        <translation>Parita</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="205"/>
        <source>Data Bits</source>
        <translation>Datové Bity</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="226"/>
        <source>Stop Bits</source>
        <translation>Stop Bity</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="247"/>
        <source>Host</source>
        <translation>Hostitel</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="257"/>
        <source>IP Address</source>
        <translation>IP Adresa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="271"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="280"/>
        <source>TCP Port</source>
        <translation>TCP Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="308"/>
        <source>Slave Address</source>
        <translation>Adresa Slave</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="313"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="361"/>
        <source>Configure Register Groups…</source>
        <translation>Konfigurovat Skupiny Registrů…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="371"/>
        <source>Import Register Map…</source>
        <translation>Importovat Mapu Registrů…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="386"/>
        <source>%1 group(s) configured</source>
        <translation>Nakonfigurováno skupin: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="387"/>
        <source>No groups configured</source>
        <translation>Žádné skupiny nejsou nakonfigurovány</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="328"/>
        <source>Poll Interval (ms)</source>
        <translation>Interval Dotazování (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="333"/>
        <source>Polling interval</source>
        <translation>Interval dotazování</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Skupiny Registrů Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Nakonfigurujte více skupin registrů pro sekvenční dotazování různých typů registrů.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Přidat Novou Skupinu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Typ Registru:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Počáteční Adresa:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Počet Registrů:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Přidat Skupinu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Nakonfigurované Skupiny</translation>
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
        <translation>Začátek</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>Počet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Akce</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Odebrat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Žádné skupiny nejsou nakonfigurovány.
Přidejte skupiny výše pro dotazování více typů registrů.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Celkem skupin: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Generovat Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Vymazat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Náhled Mapy Registrů Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>Soubor: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Zkontrolujte registry pro import do nového projektu Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>Registry</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Adresa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Datový Typ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Jednotky</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>V souboru nebyly nalezeny žádné registry.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Celkem: %1 registrů v %2 skupinách</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Vytvořit Projekt</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>Vydavatel MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>Připojeno k brokeru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Nepřipojeno</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Testovat Připojení</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Otestovat broker s aktuálním nastavením</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Nejprve povolit publikování</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Upravit Skript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Upravit skript publikátora (Lua nebo JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Načíst CA Certifikáty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>Přidat PEM certifikáty ze složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>Nejprve povolit SSL/TLS</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolovat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="305"/>
        <source>Interpolation: %1</source>
        <translation>Interpolace: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Zobrazit Legendy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="327"/>
        <source>Show X Axis Label</source>
        <translation>Zobrazit Popisek Osy X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="338"/>
        <source>Show Y Axis Label</source>
        <translation>Zobrazit Popisek Osy Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="350"/>
        <source>Show Crosshair</source>
        <translation>Zobrazit Zaměřovač</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Resume</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="374"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Režim Sweep / Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="386"/>
        <source>Trigger Settings</source>
        <translation>Nastavení Triggeru</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="410"/>
        <source>Reset View</source>
        <translation>Resetovat Zobrazení</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="416"/>
        <source>Axis Range Settings</source>
        <translation>Nastavení Rozsahu Os</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Vzorky</translation>
    </message>
</context>
<context>
    <name>MultiSelectionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="35"/>
        <source>%1 items selected</source>
        <translation>Vybráno položek: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="118"/>
        <source>Plots</source>
        <translation>Grafy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="123"/>
        <source>Plot</source>
        <translation>Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="127"/>
        <source>Toggle 2D plot for every selected dataset</source>
        <translation>Přepnout 2D graf pro každou vybranou datovou sadu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="139"/>
        <source>FFT Plot</source>
        <translation>FFT Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="142"/>
        <source>Toggle FFT plot for every selected dataset</source>
        <translation>Přepnout FFT graf pro každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="154"/>
        <source>Waterfall</source>
        <translation>Vodopád</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="157"/>
        <source>Toggle waterfall for every selected dataset</source>
        <translation>Přepnout vodopád pro každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="173"/>
        <source>Widgets</source>
        <translation>Widgety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="179"/>
        <source>Bar/Level</source>
        <translation>Pruh/úroveň</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="182"/>
        <source>Set bar/level for every selected dataset</source>
        <translation>Nastavit pruh/úroveň pro každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Měřič</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="197"/>
        <source>Set gauge for every selected dataset</source>
        <translation>Nastavit měřič pro každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="209"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="212"/>
        <source>Set compass for every selected dataset</source>
        <translation>Nastavit kompas pro každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="223"/>
        <source>Meter</source>
        <translation>Měřič</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="227"/>
        <source>Set meter for every selected dataset</source>
        <translation>Nastavit měřič pro každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="238"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="242"/>
        <source>Toggle LED for every selected dataset</source>
        <translation>Přepnout LED pro každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="266"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="268"/>
        <source>Duplicate every selected dataset</source>
        <translation>Duplikovat každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="276"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="279"/>
        <source>Delete every selected dataset</source>
        <translation>Odstranit každý vybraný dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="319"/>
        <source>Editing %1 items. Shared fields apply to all; per-item fields are locked.</source>
        <translation>Úprava %1 položek. Sdílená pole platí pro všechny; pole jednotlivých položek jsou uzamčena.</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>Bajtů na hodnotu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Počet bajtů sloučených do každé hodnoty kanálu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="301"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="439"/>
        <source>Endianness</source>
        <translation>Endianita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="302"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="440"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>Pořadí bajtů použité při kombinování vícebajtových hodnot.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>Znaménkové hodnoty</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Interpretuje každou hodnotu jako znaménkovou ve dvojkovém doplňku.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>Směrovací tabulka značek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Položky značka:index oddělené čárkami, např. 1:0,2:1,3:2. Značky mohou být desítkové nebo hexadecimální s prefixem 0x.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>Ověřit kontrolní součet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Odmítá zprávy s neplatným Fletcherovým kontrolním součtem.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Odmítá zprávy s neplatným aditivním kontrolním součtem.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>Verze protokolu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Vybírá očekávanou počáteční značku (0xFE pro v1, 0xFD pro v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>Validovat CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Odmítá rámce s neplatným kontrolním součtem CRC-24Q.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>Počet kanálů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Počet výstupních kanálů (registry nebo cívky).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>Offset registru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Adresový offset odečtený od ech jednoduchých zápisů (40001 pro starší mapy Modicon).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>Znaménkové registry</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Interpretuje 16bitové registry jako znaménkové hodnoty v dvojkovém doplňku.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>Rozvržení payloadu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>Pole emituje každý prvek v pořadí; mapa směruje klíče přes seznam klíčů.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>Klíče (režim mapy)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Klíče mapy oddělené čárkami v pořadí kanálů. Používá se pouze pro rozložení mapy.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Skalární pole</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Pole JSON oddělená čárkami opakovaná v každém generovaném rámci.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Pole vzorkového pole</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>Pole JSON obsahující dávkové pole vzorků.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Pole pole záznamů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>Pole JSON obsahující pole objektů záznamů.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Pole záznamů (v pořadí kanálů)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Pole záznamů oddělená čárkami. Pozice každého pole určuje jeho index kanálu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>Šířky sloupců</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Počty znaků oddělené čárkami pro každé pole. Ponechte prázdné pro rozdělení podle mezer.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>Oříznout mezery</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Odstraní odsazení kolem každého rozděleného pole.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>Klíče (v pořadí kanálů)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Názvy klíčů oddělené čárkami. Pozice každého klíče určuje jeho index kanálu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>Oddělovač párů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>Znak mezi páry klíč=hodnota.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>Oddělovač klíč-hodnota</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>Znak mezi klíčem a jeho hodnotou.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>Pouze číselné hodnoty</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Ignoruje páry, jejichž hodnota není číslo.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>Směrovací tabulka příkazů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Středníkem oddělené položky seznamu NÁZEV:index, např. CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>Prefix vysílače</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>Dvoumístný identifikátor vysílače, např. GP pro GPS nebo GN pro přijímače s více konstelacemi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Odmítá věty, jejichž kontrolní součet *hh neodpovídá.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Čárkami oddělené názvy parametrů. Pozice každého klíče určuje jeho index kanálu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>Pole (v pořadí kanálů)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Čárkami oddělené názvy polí. Pozice každého pole určuje jeho index kanálu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>Značky (v pořadí kanálů)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Názvy značek oddělené čárkami. Pozice každé značky určuje její index kanálu.</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">Bloky Registrů</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">Dotazované bloky registrů s typovanými, škálovanými položkami. Vytvořeno importérem mapy registrů Modbus.</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">Mapa Signálů</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">CAN zprávy s jejich bitovými rozloženími signálů a škálováním. Vytvořeno importérem DBC.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Typ Socketu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Vzdálená Adresa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Místní Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>Zadejte 0 pro automatický port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Vzdálený Port</translation>
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
        <translation>Filtrovat podle kanálu…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Vymazat všechna oznámení</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(bez názvu)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Zatím žádná oznámení</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">Transformace datových sad a skripty výstupních widgetů mohou zde publikovat události pomocí notifyInfo / notifyWarning / notifyCritical.</translation>
    </message>
</context>
<context>
    <name>OfflineActivation</name>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="37"/>
        <source>Activate Offline</source>
        <translation>Aktivovat Offline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="55"/>
        <source>Activate Serial Studio Pro on a machine with no internet access. No account or connection is needed on this computer.</source>
        <translation>Aktivujte Serial Studio Pro na počítači bez přístupu k internetu. Na tomto počítači není potřeba účet ani připojení.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="86"/>
        <source>Save your device file</source>
        <translation>Uložte soubor zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="95"/>
        <source>Save this computer's device file. It identifies this machine and contains no personal information.</source>
        <translation>Uložte soubor zařízení tohoto počítače. Identifikuje tento stroj a neobsahuje žádné osobní informace.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="102"/>
        <source>Save Device File…</source>
        <translation>Uložit Soubor Zařízení…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="136"/>
        <source>Get your license file</source>
        <translation>Získejte licenční soubor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="145"/>
        <source>On another computer with internet access, go to the address below, upload the device file, and enter your email and license key.</source>
        <translation>Na jiném počítači s přístupem k internetu přejděte na níže uvedenou adresu, nahrajte soubor zařízení a zadejte svůj e-mail a licenční klíč.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="157"/>
        <source>Open in Browser</source>
        <translation>Otevřít v Prohlížeči</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="193"/>
        <source>Import your license</source>
        <translation>Importujte licenci</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="202"/>
        <source>Copy the license file to this computer and import it. Pro features unlock immediately.</source>
        <translation>Zkopírujte licenční soubor do tohoto počítače a importujte jej. Funkce Pro se odemknou okamžitě.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="209"/>
        <source>Import License File…</source>
        <translation>Importovat Licenční Soubor…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="224"/>
        <source>Save Device File</source>
        <translation>Uložit Soubor Zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="226"/>
        <source>Serial Studio device file (*.ssmachine)</source>
        <translation>Soubor zařízení Serial Studio (*.ssmachine)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="235"/>
        <source>Import License File</source>
        <translation>Importovat Soubor Licence</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="236"/>
        <source>Serial Studio license (*.sslic)</source>
        <translation>Licence Serial Studio (*.sslic)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="236"/>
        <source>All files (*)</source>
        <translation>Všechny soubory (*)</translation>
    </message>
</context>
<context>
    <name>OfflineLicense</name>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="182"/>
        <source>License certificate is valid.</source>
        <translation>Licenční certifikát je platný.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="184"/>
        <source>The certificate signature is invalid or corrupted.</source>
        <translation>Podpis certifikátu je neplatný nebo poškozený.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="187"/>
        <source>This certificate was issued for a different device.</source>
        <translation>Tento certifikát byl vydán pro jiné zařízení.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="190"/>
        <source>This certificate has expired.</source>
        <translation>Platnost tohoto certifikátu vypršela.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="192"/>
        <source>This certificate does not grant a valid license tier.</source>
        <translation>Tento certifikát neuděluje platnou úroveň licence.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="196"/>
        <source>The certificate file is malformed or unreadable.</source>
        <translation>Soubor certifikátu je poškozený nebo nečitelný.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="41"/>
        <source>Search Online Icons</source>
        <translation>Hledat Online Ikony</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="71"/>
        <source>Download failed: %1</source>
        <translation>Stahování selhalo: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="96"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Hledat ikony (např. teplota, šipka, přehrát)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="108"/>
        <source>Search</source>
        <translation>Hledat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="147"/>
        <source>Search for icons above to get started</source>
        <translation>Začněte hledáním ikon výše</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="248"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="258"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Výstupní widgety vyžadují licenci Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Tlačítko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Odeslat příkaz po kliknutí</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Posuvník</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Odesílat škálované číselné hodnoty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Přepínač</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Odesílat příkazy zapnuto/vypnuto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Textové Pole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Zadávat a odesílat libovolné příkazy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Knoflík</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Otočný vstup pro nastavované hodnoty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Výstupní widgety lze konfigurovat, ale na panelu se zobrazují pouze s licencí Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Duplikovat tento výstupní widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Odstranit tento výstupní widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Funkce Přenosu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Importovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Importovat funkci přenosu ze souboru .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Šablona</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Vybrat předpřipravenou šablonu funkce přenosu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Testovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Testovat funkci přenosu se vzorovým vstupem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Zpět</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Znovu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Vyjmout</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Vložit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Vybrat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Formátovat Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Formátovat Výběr</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Chyba Painter Widgetu</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Editor Kódu Painter Widgetu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Importovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Importovat kód painteru ze souboru .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Šablona</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Vybrat vestavěnou šablonu kreslícího kódu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Formátovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Přeformátovat kreslící kód</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Otevřít živý náhled se simulovanými hodnotami datasetu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Náhled</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Vyjmout</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Vložit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Vybrat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Zpět</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Znovu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Formátovat Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Formátovat Výběr</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Živý Náhled Malíře</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Náhled</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Simulované datové sady</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>Čekání na první rámec...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Vymazat konzoli</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolovat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="294"/>
        <source>Interpolation: %1</source>
        <translation>Interpolace: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="307"/>
        <source>Show Area Under Plot</source>
        <translation>Zobrazit Plochu Pod Grafem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="326"/>
        <source>Show X Axis Label</source>
        <translation>Zobrazit Popisek Osy X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="337"/>
        <source>Show Y Axis Label</source>
        <translation>Zobrazit Popisek Osy Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="349"/>
        <source>Show Crosshair</source>
        <translation>Zobrazit Zaměřovač</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Resume</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="373"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Režim Sweep / Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="385"/>
        <source>Trigger Settings</source>
        <translation>Nastavení Triggeru</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="409"/>
        <source>Reset View</source>
        <translation>Obnovit Zobrazení</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="415"/>
        <source>Axis Range Settings</source>
        <translation>Nastavení Rozsahu Os</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="202"/>
        <source>Interpolate</source>
        <translation>Interpolovat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="220"/>
        <source>Orbit Navigation</source>
        <translation>Orbitální Navigace</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="230"/>
        <source>Pan Navigation</source>
        <translation>Posuvná Navigace</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="241"/>
        <source>Orthogonal View</source>
        <translation>Ortogonální Zobrazení</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="247"/>
        <source>Top View</source>
        <translation>Pohled Shora</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="253"/>
        <source>Left View</source>
        <translation>Pohled Zleva</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="259"/>
        <source>Front View</source>
        <translation>Pohled Zepředu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="276"/>
        <source>Auto Center</source>
        <translation>Automatické Centrování</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="292"/>
        <source>Anaglyph 3D</source>
        <translation>Anaglyph 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="306"/>
        <source>Invert Eye Positions</source>
        <translation>Invertovat Pozice Očí</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="71"/>
        <source>None</source>
        <translation>Žádná</translation>
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
        <translation>Lineární</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1392"/>
        <source>Time</source>
        <translation>Čas</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1415"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Přetažením posunout, pravým tlačítkem vymazat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1418"/>
        <source>Click to place cursor</source>
        <translation>Kliknutím umístit kurzor</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1420"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Kliknutím umístit druhý kurzor — Přetažením posunout</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Navštívit Web</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Koupit Licenci</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Aktivovat</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Asistent — funkce Pro</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>Asistent je funkce Serial Studio Pro. Pro odemčení aktivujte licenci.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Aktivovat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>Režim</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>Spustit Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Pojmenovaná Roura</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Spustitelný Soubor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/cesta/k/spustitelnému/souboru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Procházet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Argumenty</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 hodnota1 --arg2 hodnota2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Pracovní Adresář</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(volitelné) /pracovní/adresář</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Cesta k Rouře</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Vybrat Běžící Proces…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Spustit podřízený proces a zachytit jeho stdout, nebo se připojit k pojmenované rouře vytvořené existujícím procesem.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Další informace o pojmenovaných rourách</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Vybrat Běžící Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Vyberte běžící proces pro odvození návrhu cesty k pojmenované rouře.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Filtrovat Procesy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Zadejte text pro filtrování podle názvu…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Běžící Procesy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>Žádné procesy neodpovídají filtru.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Nebyly nalezeny žádné běžící procesy.
Klikněte na Obnovit pro aktualizaci seznamu.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 proces(ů)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Vybrat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>změněno</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="367"/>
        <source>This project is password protected</source>
        <translation>Tento projekt je chráněn heslem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="368"/>
        <source>Editing is available in Project mode</source>
        <translation>Úpravy jsou dostupné v režimu Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="379"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Zadejte heslo pro provedení změn nebo otevřete jiný projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="380"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Přepněte do režimu Projekt pro načtení a úpravu projektu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="402"/>
        <source>Unlock</source>
        <translation>Odemknout</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="403"/>
        <source>Switch to Project Mode</source>
        <translation>Přepnout do Režimu Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="422"/>
        <source>Open Other Project</source>
        <translation>Otevřít Jiný Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="423"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="439"/>
        <source>Create New Project</source>
        <translation>Vytvořit Nový Projekt</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>Struktura Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>Hledat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="357"/>
        <source>Move Up</source>
        <translation>Posunout Nahoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="365"/>
        <source>Move Down</source>
        <translation>Posunout Dolů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="386"/>
        <source>Rename</source>
        <translation>Přejmenovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="404"/>
        <source>Hide Selected (%1)</source>
        <translation>Skrýt Vybrané (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="405"/>
        <source>Show Selected (%1)</source>
        <translation>Zobrazit Vybrané (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="406"/>
        <source>Hide</source>
        <translation>Skrýt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="406"/>
        <source>Show</source>
        <translation>Zobrazit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="429"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Duplikovat Vybrané (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="430"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="452"/>
        <source>Delete Selected (%1)</source>
        <translation>Smazat Vybrané (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="453"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="504"/>
        <source>New Folder</source>
        <translation>Nová Složka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="517"/>
        <source>New Sub-Folder</source>
        <translation>Nová Podsložka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="533"/>
        <source>Move to Folder</source>
        <translation>Přesunout do Složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="540"/>
        <source>Top Level</source>
        <translation>Nejvyšší Úroveň</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="573"/>
        <source>Move Here</source>
        <translation>Přesunout Sem</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Nový</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Vytvořit nový JSON projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Otevřít</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Otevřít existující JSON projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Uložit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Uložit aktuální projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Uložit Jako</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Uložit aktuální projekt pod novým názvem</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">Odemknout</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Zamknout</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">Odemknout Editor Projektu pomocí hesla projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Importovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Vygenerovat projekt ze schématu Protocol Buffers (.proto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Nastavit heslo a uzamknout Editor Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Přidat Zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Přidat nový zdroj dat (zařízení) do projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Akce</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Přidat novou akci do projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Výstup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Obnovit nedávný automatický snímek aktuálního projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Přidat nový výstupní ovládací panel s tlačítkem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Posuvník</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Přidat výstupní ovládací prvek posuvníku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Přepínač</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Přidat výstupní ovládací prvek přepínače</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Otočný Ovladač</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Přidat výstupní ovládací prvek otočného ovladače</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Textové Pole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Přidat výstupní ovládací prvek textového pole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Tlačítko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Přidat ovládací tlačítko výstupu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>Datová Sada</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>Přidat obecnou datovou sadu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>Přidat datovou sadu 2D grafu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>FFT Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Přidat graf rychlé Fourierovy transformace</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Měřidlo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Přidat widget měřidla pro číselná data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Indikátor Úrovně</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Přidat svislý indikátor úrovně</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Přidat widget kompasu pro směrová data</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>LED Indikátor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Přidat stavový indikátor ve stylu LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Skupina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Přidat skupinu kontejneru datových sad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Kontejner Datových Sad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Obrázek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Přidat prohlížeč obrazu/videa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Zobrazení Obrazu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="450"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Web View</source>
        <translation>Zobrazení Webu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="452"/>
        <source>Add an web viewer</source>
        <translation>Přidat prohlížeč webu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="462"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="466"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Přidat vlastní widget painter vykreslovaný JavaScriptem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="467"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Widgety painter vyžadují licenci Pro — přidání jednoho se vrátí k datové mřížce</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="468"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="480"/>
        <source>Table</source>
        <translation>Tabulka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Add a data table view</source>
        <translation>Přidat zobrazení datové tabulky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Data Grid</source>
        <translation>Datová Mřížka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="491"/>
        <source>Multi-Plot</source>
        <translation>Vícenásobný Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="493"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>Přidat 2D graf s více signály</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Multiple Plot</source>
        <translation>Vícenásobný Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="500"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>3D Plot</source>
        <translation>3D Graf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <source>Add a 3D plot visualization</source>
        <translation>Přidat vizualizaci 3D grafu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="511"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="515"/>
        <source>Accelerometer</source>
        <translation>Akcelerometr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>Přidat skupinu pro data z 3osého akcelerometru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="521"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="524"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>Přidat skupinu pro data z 3osého gyroskopu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="530"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="535"/>
        <source>GPS Map</source>
        <translation>Mapa GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="533"/>
        <source>Add a map widget for GPS data</source>
        <translation>Přidat widget mapy pro data GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="547"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="551"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="554"/>
        <source>Open the Assistant</source>
        <translation>Otevřít Asistenta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="560"/>
        <source>Help Center</source>
        <translation>Centrum Nápovědy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="564"/>
        <source>Open the Project Editor documentation</source>
        <translation>Otevřít dokumentaci Editoru projektů</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Souhrn Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>V tomto projektu zjištěny funkce Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Použity náhradní widgety. Zakupte licenci pro odemknutí plné funkčnosti.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Název Projektu:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Projekt Bez Názvu</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Body:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="147"/>
        <source>Settings</source>
        <translation>Nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="173"/>
        <source>Time Range:</source>
        <translation>Časový Rozsah:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="221"/>
        <source>Point Count:</source>
        <translation>Počet Bodů:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="240"/>
        <source>Change-Driven Transforms:</source>
        <translation>Transformace Řízené Změnou:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="250"/>
        <source>Run a dataset's transform only when one of its inputs changes. Speeds up large table-driven projects; off by default.</source>
        <translation>Spustit transformaci datové sady pouze při změně jednoho z jejích vstupů. Zrychluje velké projekty řízené tabulkami; ve výchozím nastavení vypnuto.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="297"/>
        <source>Source</source>
        <translation>Zdroj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="298"/>
        <source>Sources</source>
        <translation>Zdroje</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="303"/>
        <source>Group</source>
        <translation>Skupina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="304"/>
        <source>Groups</source>
        <translation>Skupiny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="309"/>
        <source>Dataset</source>
        <translation>Datová Sada</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="310"/>
        <source>Datasets</source>
        <translation>Datové Sady</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="315"/>
        <source>Action</source>
        <translation>Akce</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="316"/>
        <source>Actions</source>
        <translation>Akce</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="404"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Dvojklikem upravíte blok. Pravým tlačítkem kdekoli přidáte skupinu, datovou sadu, akci, datovou tabulku nebo zařízení.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Náhled Souboru Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Proto Soubor: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Procházejte zprávy níže a poté vytvořte projekt. Každá zpráva se stane skupinou dashboardu; bloky kanálů se shodným typem získají MultiPlot a zprávy se smíšenými typy získají DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Zobrazit pole pro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Pole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Název Pole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Vybraná zpráva neobsahuje žádná pole.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Celkem: %1 zpráv, %2 polí</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Vytvořit Projekt</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Žádná chyba</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Broker odmítl připojení kvůli nepodporované verzi protokolu. Slaďte verzi MQTT s brokerem a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Broker odmítl ID klienta. Může být nesprávně formátováno, příliš dlouhé nebo již používáno. Vygenerujte ho znovu a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>Síť dosáhla brokeru, ale broker je momentálně nedostupný. Ověřte jeho stav a zkuste to znovu později.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>Uživatelské jméno nebo heslo je nesprávné nebo chybně formátované. Zkontrolujte přihlašovací údaje a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Broker odmítl připojení z důvodu nedostatečných oprávnění. Ověřte, že účet má požadovaná ACL.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>Problém na úrovni sítě nebo transportní vrstvy zabránil připojení. Zkontrolujte konektivitu, porty a konfiguraci TLS.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>Klient detekoval porušení protokolu MQTT a ukončil připojení. Ověřte kompatibilitu brokeru a klienta.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>Došlo k neočekávané chybě. Zkontrolujte protokoly brokeru a konzoli aplikace.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>Došlo k chybě protokolu MQTT úrovně 5. Zkontrolujte kód důvodu brokeru.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Nespecifikovaná chyba MQTT (kód %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>Nepodařilo se načíst uvítací text :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="250"/>
        <source>Network error</source>
        <translation>Chyba sítě</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="253"/>
        <location filename="../../src/Licensing/Trial.cpp" line="269"/>
        <location filename="../../src/Licensing/Trial.cpp" line="301"/>
        <source>Trial Activation Error</source>
        <translation>Chyba Aktivace Zkušební Verze</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="266"/>
        <source>Invalid server response</source>
        <translation>Neplatná odpověď serveru</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>The server returned malformed data: %1</source>
        <translation>Server vrátil chybná data: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="298"/>
        <source>Unexpected server response</source>
        <translation>Neočekávaná odpověď serveru</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>The server response is missing required fields.</source>
        <translation>V odpovědi serveru chybí požadovaná pole.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Chyba Souboru Výstupu Konzole</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>Nelze otevřít soubor pro zápis!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Neplatný Bluetooth adaptér!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>Nepodporovaná platforma nebo operační systém</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>Nepodporovaná metoda zjišťování</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>Obecná chyba I/O</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>Analyzátor Rámců Zakázán</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Lua analyzátor rámců pro zdroj %1 vypršel %2 rámců za sebou a byl zakázán, aby Serial Studio zůstalo responzivní.

Nejpravděpodobnější příčina: nekonečná smyčka nebo extrémně pomalá operace v těle skriptu. Opravte skript a znovu načtěte projekt pro opětovné povolení analýzy.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>Syntaktická Chyba Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>Kód parseru obsahuje chybu:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>Běhová Chyba Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>Kód parseru vyvolal chybu:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>Chybějící Funkce Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>Funkce 'parse' není ve skriptu definována.

Ujistěte se, že váš kód obsahuje:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>Chyba Běhu Funkce Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>Funkce parse obsahuje chybu:

%1

Opravte chybu v těle funkce.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>JavaScript analyzátor rámců pro zdroj %1 vypršel %2 rámců za sebou a byl zakázán, aby Serial Studio zůstalo responzivní.

Nejpravděpodobnější příčina: nekonečná smyčka nebo extrémně pomalá operace v těle skriptu. Opravte skript a znovu načtěte projekt pro opětovné povolení analýzy.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>Časový Limit Javascriptu Vypršel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>Kód parseru nedokončil vyhodnocení do %1 ms a byl přerušen.

Nejpravděpodobnější příčina: nekonečná smyčka na nejvyšší úrovni skriptu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>Syntaktická Chyba Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>Kód parseru obsahuje syntaktickou chybu na řádku %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>Došlo k Výjimce Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>Kód parseru vyvolal následující výjimky:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>Funkce 'parse' není ve skriptu definována.

Ujistěte se, že váš kód obsahuje:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>Funkce parse obsahuje chybu na řádku %1:

%2

Opravte chybu v těle funkce.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>Neplatná Deklarace Funkce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Nenalezen export volatelné funkce 'parse'.

Definujte jednu z možností:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>Funkce 'parse' musí přijímat alespoň jeden parametr (datový rámec).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Nebyla nalezena platná deklarace funkce 'parse'.

Očekávaný formát:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>Neplatný Parametr Funkce</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Funkce 'parse' musí mít alespoň jeden parametr.

Očekávaný formát:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>Zastaralý Podpis Funkce</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>Funkce 'parse' používá starý formát se dvěma parametry: parse(%1, %2)

Tento formát již není podporován. Aktualizujte na nový formát s jedním parametrem:
function parse(%1) { ... }

Parametr oddělovače již není potřeba.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="192"/>
        <source>Critical</source>
        <translation>Kritické</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="192"/>
        <source>Warning</source>
        <translation>Varování</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="585"/>
        <source>Project file not found</source>
        <translation>Soubor projektu nenalezen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="586"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Soubor projektu odkazovaný touto zástupcem nebyl nalezen:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="589"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Chcete tento zástupce smazat?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="593"/>
        <source>Delete Shortcut</source>
        <translation>Smazat Zástupce</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="595"/>
        <source>Quit</source>
        <translation>Ukončit</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>Čas (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>Frekv: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>Čas: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Není nastaven API klíč OpenAI. Otevřete Správu klíčů a přidejte ho.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="240"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Není nastaven API klíč Anthropic. Otevřete Správu klíčů a přidejte ho.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Není nastaven API klíč Gemini. Otevřete Správu klíčů a přidejte ho.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="351"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Není nakonfigurována URL serveru lokálního modelu. Otevřete Správu klíčů a nastavte ji.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Není nastaven API klíč DeepSeek. Otevřete Správu klíčů a přidejte jej.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Není nastaven API klíč Mistral. Otevřete Správu klíčů a přidejte ho.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Není nastaven API klíč OpenRouter. Otevřete Správu klíčů a přidejte ho.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Není nastaven API klíč Groq. Otevřete Správu klíčů a přidejte ho.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1149"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Analyzátor rámců využívá více než %1% času CPU.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1151"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio zahazuje rámce, aby aplikace zůstala responzivní. Zjednodušte nebo optimalizujte skript analyzátoru rámců pro snížení jeho zátěže.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>Očekáváno %1, obdrženo '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Očekáván název výčtu po 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>Očekáván název oneof</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Značka pole '%1' mimo rozsah (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Očekáván typ klíče v map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Očekáván typ hodnoty v map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>Očekáván název pole mapy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>Očekávána značka pole mapy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>Očekáván typ pole, obdrženo '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>Očekáván název pole po typu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>Očekáváno číslo značky pole</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Vnořování zpráv příliš hluboké (limit %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>Očekáván název zprávy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Neočekávaný token '%1' v rozsahu souboru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Nepodporované klíčové slovo nejvyšší úrovně '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Automaticky (Výchozí pro Platformu)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Softwarově (Záložní Režim)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Řádek %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">Rámec %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Dekodér</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">Řádky</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 řádek/ů</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>Konfigurace nativního parseru není platný objekt JSON.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Neznámá šablona nativního parseru: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Chyba Vestavěného Parseru</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Chyba Nativního Parseru</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="52"/>
        <source>Offline Activation</source>
        <translation>Offline Aktivace</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append není podporován pro GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Otevření gzip pro čtení i zápis současně není podporováno</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>Gzip lze otevřít buď pro čtení, nebo pro zápis. Co to má být?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Nelze provést gzopen() souboru</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append není podporován pro QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite není podporován pro QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>Chyba ZIP/UNZIP API %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>Generovat PDF Sestavu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Generovat Sestavu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="71"/>
        <source>Solid</source>
        <translation>Plná</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="72"/>
        <source>Dashed</source>
        <translation>Čárkovaná</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="73"/>
        <source>Dotted</source>
        <translation>Tečkovaná</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="95"/>
        <source>A4 (210 × 297 mm)</source>
        <translation>A4 (210 × 297 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="96"/>
        <source>A3 (297 × 420 mm)</source>
        <translation>A3 (297 × 420 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="97"/>
        <source>A2 (420 × 594 mm)</source>
        <translation>A2 (420 × 594 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="98"/>
        <source>A1 (594 × 841 mm)</source>
        <translation>A1 (594 × 841 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="99"/>
        <source>A0 (841 × 1189 mm)</source>
        <translation>A0 (841 × 1189 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="100"/>
        <source>A5 (148 × 210 mm)</source>
        <translation>A5 (148 × 210 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="101"/>
        <source>A6 (105 × 148 mm)</source>
        <translation>A6 (105 × 148 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="102"/>
        <source>B4 (250 × 353 mm)</source>
        <translation>B4 (250 × 353 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>B5 (176 × 250 mm)</source>
        <translation>B5 (176 × 250 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="104"/>
        <source>Letter (8.5 × 11 in)</source>
        <translation>Letter (8,5 × 11 palců)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8,5 × 14 palců)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="106"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7,25 × 10,5 palců)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="107"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 palců)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="108"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 palců)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="123"/>
        <source>%1 — Session Report</source>
        <translation>%1 — Zpráva o Relaci</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="125"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="661"/>
        <source>Session Report</source>
        <translation>Zpráva o Relaci</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="563"/>
        <source>Branding</source>
        <translation>Značka</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="569"/>
        <source>Page</source>
        <translation>Stránka</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="575"/>
        <source>Sections</source>
        <translation>Sekce</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="581"/>
        <source>Datasets</source>
        <translation>Datové Sady</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="629"/>
        <source>Identity</source>
        <translation>Identita</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="643"/>
        <source>Company</source>
        <translation>Společnost</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="650"/>
        <source>e.g. Acme Test Systems</source>
        <translation>např. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="654"/>
        <source>Document title</source>
        <translation>Název dokumentu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="665"/>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="672"/>
        <source>Prepared by (optional)</source>
        <translation>Připravil (volitelné)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="681"/>
        <source>Logo</source>
        <translation>Logo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="694"/>
        <source>File</source>
        <translation>Soubor</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="705"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG nebo SVG (volitelné)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="707"/>
        <source>Browse…</source>
        <translation>Procházet…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="710"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="751"/>
        <source>Paper</source>
        <translation>Papír</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="763"/>
        <source>Page size</source>
        <translation>Velikost stránky</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="891"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Anotovat min., max. a průměrné hodnoty na grafech</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="927"/>
        <source>Include datasets</source>
        <translation>Zahrnout datové sady</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="941"/>
        <source>Expand All</source>
        <translation>Rozbalit Vše</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="950"/>
        <source>Collapse All</source>
        <translation>Sbalit Vše</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="962"/>
        <source>Search datasets</source>
        <translation>Hledat datové sady</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1087"/>
        <source>Loading datasets...</source>
        <translation>Načítání datových sad...</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1088"/>
        <source>No datasets match your search.</source>
        <translation>Žádné datové sady neodpovídají vašemu hledání.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1108"/>
        <source>Select at least one dataset to include.</source>
        <translation>Vyberte alespoň jednu datovou sadu k zahrnutí.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1125"/>
        <source>Export HTML</source>
        <translation>Exportovat HTML</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="879"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Titulní strana (logo, název dokumentu, podtitul testu)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="882"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Informace o testu (projekt, časové značky, klasifikace a poznámky)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="885"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Souhrn měření (min, max, průměr, směr. odchylka pro každý parametr)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="888"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Trendy parametrů (časový graf pro každý číselný parametr)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="778"/>
        <source>Plot appearance</source>
        <translation>Vzhled grafu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="792"/>
        <source>Line width</source>
        <translation>Šířka čáry</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="824"/>
        <source>Line style</source>
        <translation>Styl čáry</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="864"/>
        <source>Include</source>
        <translation>Zahrnout</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1117"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1125"/>
        <source>Export PDF</source>
        <translation>Exportovat PDF</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Generování Sestavy</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>Zpracovává Se…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Může to trvat několik sekund u relací s mnoha parametry. Okno se automaticky zavře, jakmile bude sestava připravena.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>Připojení Ztraceno</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>Zařízení Nedostupné</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>Připojení k vašemu zařízení bylo ztraceno.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio se nemohlo připojit k vašemu zařízení.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Zkontrolujte kabel, napájení a zda zařízení nepřevzala jiná aplikace. Můžete zkusit znovu připojit, přepnout na jiné zařízení nebo ukončit.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Ujistěte se, že je zapojeno, zapnuté a není již používáno jinou aplikací. Můžete to zkusit znovu, vybrat jiné zařízení nebo ukončit.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>Ukončit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>Vybrat Jiné Zařízení</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">Překonfigurovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>Zkusit Znovu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>Znovu Připojit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Vyberte správné zařízení a stiskněte Připojit.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>I/O Rozhraní: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>Připojit</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="338"/>
        <source>Data Grids</source>
        <translation>Datové Mřížky</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="341"/>
        <source>Multiple Data Plots</source>
        <translation>Vícenásobné Datové Grafy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="344"/>
        <source>Accelerometers</source>
        <translation>Akcelerometry</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="347"/>
        <source>Gyroscopes</source>
        <translation>Gyroskopy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="350"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="353"/>
        <source>FFT Plots</source>
        <translation>FFT Grafy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="356"/>
        <source>LED Panels</source>
        <translation>LED Panely</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="359"/>
        <source>Data Plots</source>
        <translation>Datové Grafy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="362"/>
        <source>Bars</source>
        <translation>Sloupcové Grafy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="365"/>
        <source>Gauges</source>
        <translation>Měřidla</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="368"/>
        <source>Terminal</source>
        <translation>Terminál</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="371"/>
        <source>Clock</source>
        <translation>Hodiny</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="374"/>
        <source>Stopwatch</source>
        <translation>Stopky</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="377"/>
        <source>Compasses</source>
        <translation>Kompasy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="380"/>
        <source>Meters</source>
        <translation>Měřidla</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Teploměry</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>3D Plots</source>
        <translation>3D Grafy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Web Views</source>
        <translation>Zobrazení Webu</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="390"/>
        <source>Image Views</source>
        <translation>Zobrazení Obrázků</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="393"/>
        <source>Output Panels</source>
        <translation>Výstupní Panely</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="396"/>
        <source>Notifications</source>
        <translation>Oznámení</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="399"/>
        <source>Waterfalls</source>
        <translation>Vodopády</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="402"/>
        <source>Painter Widgets</source>
        <translation>Widgety Kreslení</translation>
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
        <translation>Systém</translation>
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
        <translation>Shift-jis</translation>
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
        <translation>Podrobnosti Relace</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Vyberte relaci pro zobrazení podrobností.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Zahájeno:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Ukončeno:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(probíhá)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Rámce:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Poznámky</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Přidat poznámky k relaci…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Poznámky jsou u dokončených relací pouze pro čtení.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Nový štítek…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Odemkněte soubor relace pro mazání relací</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Štítky</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Přidat</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Přehrát</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>Exportovat CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Vygenerovat Sestavu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Smazat</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Relace</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Hledat</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>Datum</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>Rámce</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>Značky</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>Nebyly nalezeny žádné relace.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Není otevřen žádný soubor relace.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1026"/>
        <source>Select logo image</source>
        <translation>Vybrat obrázek loga</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1028"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Obrázky (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Open Session File</source>
        <translation>Otevřít Soubor Relace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="430"/>
        <source>Session files (*.db)</source>
        <translation>Soubory relací (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1225"/>
        <source>Cannot open session file</source>
        <translation>Nelze otevřít soubor relace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="655"/>
        <source>Delete session from %1?</source>
        <translation>Odstranit relaci z %1?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="658"/>
        <source>Delete Session</source>
        <translation>Odstranit Relaci</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1086"/>
        <source>No project data</source>
        <translation>Žádná data projektu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Všechna měření a nezpracovaná data této relace budou trvale odstraněna.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>Lock Session File</source>
        <translation>Zamknout Soubor Relace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="487"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Zvolte heslo pro zamknutí souboru relace:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="496"/>
        <source>Confirm the password:</source>
        <translation>Potvrďte heslo:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="504"/>
        <source>Passwords do not match</source>
        <translation>Hesla se neshodují</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="505"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>Zadaná hesla se neshodují. Soubor relace nebyl zamknut.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="541"/>
        <source>Unlock Session File</source>
        <translation>Odemknout Soubor Relace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="542"/>
        <source>Enter the session file password:</source>
        <translation>Zadejte heslo souboru relace:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="552"/>
        <source>Incorrect password</source>
        <translation>Nesprávné heslo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="553"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>Zadané heslo neodpovídá heslu uloženému v souboru relace.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="645"/>
        <source>Session file locked</source>
        <translation>Soubor relace zamknut</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="646"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Před odstraněním zaznamenaných relací odemkněte soubor relace.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">Tato relace neobsahuje vložený soubor projektu — dashboard se vrací k rychlému rozložení grafů.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>Export Session to CSV</source>
        <translation>Exportovat Relaci do CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>CSV files (*.csv)</source>
        <translation>Soubory CSV (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">Export Dokončen</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">Relace exportována do:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">Příprava exportu…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Done</source>
        <translation>Hotovo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="962"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Failed</source>
        <translation>Selhalo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="968"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1008"/>
        <source>Report Failed</source>
        <translation>Sestava Selhala</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1009"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Nelze vygenerovat sestavu. Zkontrolujte výstupní cestu a zkuste to znovu.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save PDF Report</source>
        <translation>Uložit PDF Zprávu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="860"/>
        <source>Loading session data…</source>
        <translation>Načítání dat relace…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save HTML Report</source>
        <translation>Uložit HTML Zprávu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>PDF files (*.pdf)</source>
        <translation>PDF soubory (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>HTML files (*.html)</source>
        <translation>HTML soubory (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1087"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Tento soubor relace neobsahuje vložený projekt.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1096"/>
        <source>Invalid project data</source>
        <translation>Neplatná data projektu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1097"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>Vložený JSON projektu je poškozený a nelze jej obnovit.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Restore Project</source>
        <translation>Obnovit Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Projekty Serial Studio (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Cannot write file</source>
        <translation>Nelze zapsat soubor</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Check file permissions and try again.</source>
        <translation>Zkontrolujte oprávnění souboru a zkuste to znovu.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="77"/>
        <source>Empty file path</source>
        <translation>Prázdná cesta k souboru</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="171"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="226"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="286"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="357"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="382"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="410"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="450"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="639"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="705"/>
        <source>Database not open</source>
        <translation>Databáze není otevřena</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="263"/>
        <source>Database not open or empty label</source>
        <translation>Databáze není otevřena nebo prázdný popisek</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="331"/>
        <source>Invalid label</source>
        <translation>Neplatný popisek</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="598"/>
        <source>Cancelled</source>
        <translation>Zrušeno</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="717"/>
        <source>Could not load session data</source>
        <translation>Nelze načíst data relace</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>Sestavování reportu…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>Zápis výstupu…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>Report Relace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>Nepojmenovaný projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>Připravil</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>Vygenerováno %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>ID Testu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>Trvání</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>Vzorky</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>Parametry</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>Zahájeno</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>Ukončeno</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>Identifikátor testu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>Čas zahájení</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>Čas ukončení</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>Celková doba trvání</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>Získané vzorky</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>Zaznamenané parametry</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>Klasifikace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>Poznámky</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>Informace o Testu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>Parametr</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>Jednotky</translation>
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
        <translation>Průměr</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>Směr. Odchylka</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>Souhrn Měření</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>klikněte na sloupec pro řazení</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 vzorků za %2 sekund</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>Kombinovaný Pohled Parametrů</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>klikněte na položky legendy pro přepnutí signálů</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>Trendy Parametrů</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="650"/>
        <source>Min</source>
        <translation>Min.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="651"/>
        <source>Max</source>
        <translation>Max.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="724"/>
        <source>Page %1 of %2</source>
        <translation>Stránka %1 z %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>Načítání vykreslovacího jádra…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>Vykreslování grafů…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>Generování PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>Otevřít Soubor Relace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>Soubory relací (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>Aktivní Připojení k Zařízení</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Pro použití této funkce je nutné odpojit zařízení. Chcete pokračovat?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>Nelze Otevřít Soubor Relace</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>Neznámá chyba</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>Žádná data projektu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Tato relace neobsahuje vložený soubor projektu — dashboard se vrací k rychlému rozložení grafů.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>Zkontrolujte oprávnění souboru a zkuste to znovu.</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">Nenalezeny Žádné Relace</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">Tento soubor neobsahuje žádné nahrané relace.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">Relace Nemá Sloupce</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">Ve vybrané relaci chybí definice sloupců.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">Relace Neobsahuje Žádná Data</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">Vybraná relace neobsahuje žádné snímky.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Prázdná cesta k souboru</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Tento soubor neobsahuje žádné nahrané relace.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Zrušeno</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>Vybrané relaci chybí definice sloupců.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>Vybraná relace neobsahuje žádné rámce.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Předvolby</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>Obecné</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="166"/>
        <source>Language</source>
        <translation>Jazyk</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="182"/>
        <source>Theme</source>
        <translation>Motiv</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="289"/>
        <source>Workspace Folder</source>
        <translation>Složka Pracovního Prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="585"/>
        <source>Automatically Check for Updates</source>
        <translation>Automaticky Kontrolovat Aktualizace</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="402"/>
        <source>Export…</source>
        <translation>Exportovat…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="635"/>
        <source>Data Plotting</source>
        <translation>Vykreslování Dat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="702"/>
        <source>Point Count</source>
        <translation>Počet Bodů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="727"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Obnovovací Frekvence UI (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1037"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Vždy Zobrazit Tlačítka na Hlavním Panelu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="858"/>
        <source>Show Actions Panel</source>
        <translation>Zobrazit Panel Akcí</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="336"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>Povolit API Server (Port 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="85"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1149"/>
        <source>Console</source>
        <translation>Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="151"/>
        <source>Appearance</source>
        <translation>Vzhled</translation>
    </message>
    <message>
        <source>Files &amp; Updates</source>
        <translation type="vanished">Soubory a Aktualizace</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="vanished">Pokročilé</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>Allow External API Connections</source>
        <translation>Povolit Externí API Připojení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="874"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Automaticky Skrýt Panel Nástrojů</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <source>Taskbar</source>
        <translation>Hlavní Panel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="470"/>
        <source>Rendering Backend</source>
        <translation>Vykreslovací Rozhraní</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="370"/>
        <source>API Access Token</source>
        <translation>Přístupový Token API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Startup</source>
        <translation>Spuštění</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="202"/>
        <source>Window</source>
        <translation>Okno</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="221"/>
        <source>Custom Window Decorations</source>
        <translation>Vlastní Dekorace Oken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Window Shadow</source>
        <translation>Stín Okna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="264"/>
        <source>Window decoration changes apply after restarting %1.</source>
        <translation>Změny dekorace oken se projeví po restartu %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="273"/>
        <source>Files</source>
        <translation>Soubory</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>API &amp; Plugins</source>
        <translation>API a Pluginy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="452"/>
        <source>Graphics</source>
        <translation>Grafika</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="508"/>
        <source>System</source>
        <translation>Systém</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="524"/>
        <source>Apply Performance Hints</source>
        <translation>Použít Tipy pro Výkon</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="538"/>
        <source>Keep Display Awake</source>
        <translation>Udržovat Displej Aktivní</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="558"/>
        <source>Performance hints raise process priority and opt out of OS power throttling. Changes take effect the next time Serial Studio starts.</source>
        <translation>Tipy pro výkon zvyšují prioritu procesu a vypínají omezování výkonu operačním systémem. Změny se projeví při příštím spuštění Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="569"/>
        <source>Updates &amp; News</source>
        <translation>Aktualizace a Novinky</translation>
    </message>
    <message>
        <source>Show What's New on Startup</source>
        <translation type="vanished">Zobrazit Co Je Nového při Spuštění</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="650"/>
        <source>Time Range</source>
        <translation>Časový Rozsah</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="789"/>
        <source>Small</source>
        <translation>Malé</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="789"/>
        <source>Normal</source>
        <translation>Normální</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="789"/>
        <source>Large</source>
        <translation>Velké</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="789"/>
        <source>Extra Large</source>
        <translation>Extra Velké</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="789"/>
        <source>Custom</source>
        <translation>Vlastní</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="843"/>
        <source>Layout</source>
        <translation>Rozvržení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="890"/>
        <source>Auto-Layout Margin</source>
        <translation>Okraj Automatického Rozvržení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="915"/>
        <source>Auto-Layout Spacing</source>
        <translation>Rozestup Automatického Rozvržení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="947"/>
        <source>Video Export</source>
        <translation>Export Videa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="965"/>
        <source>Save Videos by Default</source>
        <translation>Ukládat Videa ve Výchozím Nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1016"/>
        <source>Behavior</source>
        <translation>Chování</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1052"/>
        <source>Show Search Field</source>
        <translation>Zobrazit Vyhledávací Pole</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1067"/>
        <source>Auto-hide Taskbar</source>
        <translation>Automaticky Skrýt Hlavní Panel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1085"/>
        <source>Hide Delay (ms)</source>
        <translation>Prodleva Skrytí (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1109"/>
        <source>Pinned Buttons</source>
        <translation>Připnutá Tlačítka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1127"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Přetažením připnutého tlačítka na hlavním panelu změníte jeho pořadí.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1148"/>
        <source>Settings</source>
        <translation>Nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1151"/>
        <source>Clock</source>
        <translation>Hodiny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1152"/>
        <source>Stopwatch</source>
        <translation>Stopky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1153"/>
        <source>Pause / Resume</source>
        <translation>Pozastavit / Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1154"/>
        <source>File Transmission</source>
        <translation>Přenos Souboru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1155"/>
        <source>AI Assistant</source>
        <translation>AI Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1284"/>
        <source>Display</source>
        <translation>Zobrazení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1299"/>
        <source>Display Mode</source>
        <translation>Režim Zobrazení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="764"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1312"/>
        <source>Font Family</source>
        <translation>Rodina Písma</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="92"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1150"/>
        <source>Notifications</source>
        <translation>Oznámení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="400"/>
        <source>Export Protobuf File</source>
        <translation>Exportovat Soubor Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="749"/>
        <source>Dashboard Font</source>
        <translation>Písmo Panelu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="779"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1327"/>
        <source>Font Size</source>
        <translation>Velikost Písma</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">Export Obrázků</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">Ukládat Obrázky ve Výchozím Nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1344"/>
        <source>Show Timestamps</source>
        <translation>Zobrazit Časové Značky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1363"/>
        <source>Data Transmission</source>
        <translation>Přenos Dat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1378"/>
        <source>Line Ending</source>
        <translation>Ukončení Řádku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1391"/>
        <source>Input Mode</source>
        <translation>Režim Vstupu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1404"/>
        <source>Text Encoding</source>
        <translation>Kódování Textu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1417"/>
        <source>Checksum</source>
        <translation>Kontrolní Součet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1430"/>
        <source>Echo Sent Data</source>
        <translation>Ozvěna Odeslaných Dat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1449"/>
        <source>Escape Codes</source>
        <translation>Escape Sekvence</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1464"/>
        <source>VT100 Emulation</source>
        <translation>Emulace VT100</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1483"/>
        <source>ANSI Colors</source>
        <translation>Barvy ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1541"/>
        <source>Delivery</source>
        <translation>Doručení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1556"/>
        <source>System Notifications</source>
        <translation>Systémová Oznámení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1577"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Zobrazit události Varování/Kritické jako systémová oznámení, když Serial Studio není v popředí.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1587"/>
        <source>Application Logs</source>
        <translation>Protokoly Aplikace</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1602"/>
        <source>Route Warnings to Notifications</source>
        <translation>Směrovat Varování do Oznámení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1623"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Vypnuto ve výchozím nastavení — QT a QML často generují varování a jejich povolení může přehlušit skutečné alarmy. Kritické zprávy jsou vždy směrovány bez ohledu na toto nastavení.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1642"/>
        <source>Reset</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1682"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1690"/>
        <source>Apply</source>
        <translation>Použít</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Nastavení Zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>API Server Aktivní (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>API Server Připraven</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>API Server Vypnut</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Parsování Rámců</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Pouze Konzole (Bez Parsování)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Rychlý Graf (Hodnoty Oddělené Čárkami)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Parsovat Pomocí Souboru Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Změnit Soubor Projektu (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Vybrat Soubor Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Export Dat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>Tabulka CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Záznam Relace</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>Záznam MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Protokol Konzole</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">Soubor CSV</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">Databáze Relací</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">Soubor MDF4</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">Výpis Konzole</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">Vytvořit Soubor CSV</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">Vytvořit Soubor MDF4</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">Vytvořit Záznam Relace</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">Exportovat Data Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>I/O Rozhraní: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Projekt s Více Zařízeními</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Tento projekt streamuje data z %1 nezávislých zařízení.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Každé zařízení má vlastní nastavení připojení. Nakonfigurujte je v Editoru Projektu na kartě Zdroje.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Otevřít Editor Projektu</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">Nová Zkratka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>Vyberte Ikonu</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">Uložit Zkratku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>Nové Nasazení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="109"/>
        <source>Save Deployment</source>
        <translation>Uložit Nasazení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>General</source>
        <translation>Obecné</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Taskbar</source>
        <translation>Hlavní Panel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="164"/>
        <source>Logging</source>
        <translation>Protokolování</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="221"/>
        <source>Identity</source>
        <translation>Identita</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="277"/>
        <source>Click to choose an icon</source>
        <translation>Klikněte pro výběr ikony</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="286"/>
        <source>Name:</source>
        <translation>Název:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="295"/>
        <source>Deployment Name</source>
        <translation>Název Nasazení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="386"/>
        <source>Theme</source>
        <translation>Motiv</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="396"/>
        <source>Same as Serial Studio</source>
        <translation>Stejný jako Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="418"/>
        <source>Actions Panel</source>
        <translation>Panel Akcí</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="429"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="638"/>
        <source>File Transmission</source>
        <translation>Přenos Souboru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="445"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Dvojité kliknutí na toto nasazení přenese uživatele přímo na živý dashboard tohoto projektu. Není zde žádný panel nástrojů ani nastavení, pouze data, a Serial Studio se ukončí, jakmile se zařízení odpojí.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="491"/>
        <source>Visibility</source>
        <translation>Viditelnost</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="506"/>
        <source>Mode</source>
        <translation>Režim</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="515"/>
        <source>Always shown</source>
        <translation>Vždy Zobrazeno</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="516"/>
        <source>Auto-hide</source>
        <translation>Automaticky Skrýt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="517"/>
        <source>Hidden</source>
        <translation>Skryto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Skrytí panelu úloh odstraní tlačítka pro minimalizaci/maximalizaci/zavření okna a vynutí automatické rozvržení, takže dashboard vždy vyplní dostupnou oblast.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="536"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>Panel úloh se vysune, když uživatel přesune kurzor blízko dolního okraje.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="538"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>Panel úloh je trvale viditelný ve spodní části dashboardu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="551"/>
        <source>Pinned Buttons</source>
        <translation>Připnutá Tlačítka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="568"/>
        <source>Console</source>
        <translation>Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="582"/>
        <source>Notifications</source>
        <translation>Oznámení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="596"/>
        <source>Clock</source>
        <translation>Hodiny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="610"/>
        <source>Stopwatch</source>
        <translation>Stopky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="624"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="749"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Nahrávky jsou uloženy ve složce pracovního prostoru Serial Studio</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">Název Zástupce</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="304"/>
        <source>Change Icon…</source>
        <translation>Změnit Ikonu…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="321"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="339"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="349"/>
        <source>Choose a project file to begin</source>
        <translation>Vyberte soubor projektu pro začátek</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="371"/>
        <source>Behavior</source>
        <translation>Chování</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="408"/>
        <source>Fullscreen</source>
        <translation>Celá Obrazovka</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">Dvojité kliknutí na tuto zástupce přenese uživatele přímo na živý dashboard tohoto projektu. Není zde žádný panel nástrojů ani nastavení, pouze data, a Serial Studio se ukončí, jakmile se zařízení odpojí.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">Vložit Projekt</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">Dvojklik na tento zástupce otevře přímo živý dashboard pro tento projekt. Není zde panel nástrojů ani nastavení, pouze data, a Serial Studio se ukončí, jakmile se zařízení odpojí.

Zapněte Vložit Projekt pro zabudování projektu do zástupce, aby fungoval i po přesunutí nebo smazání původního souboru.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="688"/>
        <source>Recorders</source>
        <translation>Záznamníky</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="703"/>
        <source>CSV File</source>
        <translation>Soubor CSV</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="713"/>
        <source>MDF4 File</source>
        <translation>Soubor MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="723"/>
        <source>Session Database</source>
        <translation>Databáze Relací</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="733"/>
        <source>Console Log</source>
        <translation>Protokol Konzole</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">Záznamy jsou ukládány do výchozího umístění každého modulu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="778"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="787"/>
        <source>Save</source>
        <translation>Uložit</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>Zpět</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>Znovu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>Vyjmout</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>Vložit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>Vybrat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>Formátovat Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>Formátovat Výběr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>Resetovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>Validovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Ověřit, že se skript správně zkompiluje</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">Obnovit parametry šablony na výchozí hodnoty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>Resetovat na výchozí skript parsování</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>Otevřít</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>Importovat soubor skriptu pro parsování dat</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Otevřít dokumentaci nápovědy pro parsování dat</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Jazyk:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Nativní</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>Vybrat Šablonu…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>Vrátit zpět poslední úpravu kódu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>Znovu provést předchozí vrácenou úpravu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>Vyjmout vybraný kód do schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>Zkopírovat vybraný kód do schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>Vložit kód ze schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>Nápověda</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>Platforma:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>Vestavěný</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">Šablona:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>Otestovat Se Vzorovými Daty</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Vyhodnotit</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Duplikovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Vytvořit kopii tohoto zdroje dat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Odebrat tento zdroj dat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>Primární zdroj dat nelze odebrat</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="24"/>
        <source>Session Player</source>
        <translation>Přehrávač Relací</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="91"/>
        <source>Loading session…</source>
        <translation>Načítání relace…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="278"/>
        <source>Workspaces</source>
        <translation>Pracovní Prostory</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="415"/>
        <source>Actions</source>
        <translation>Akce</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="438"/>
        <source>No Actions Available</source>
        <translation>Žádné Dostupné Akce</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="468"/>
        <source>Plugins</source>
        <translation>Pluginy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="516"/>
        <source>No Plugins Installed</source>
        <translation>Žádné Nainstalované Pluginy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="101"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="552"/>
        <source>Auto Layout</source>
        <translation>Automatické Rozvržení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="109"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="564"/>
        <source>Full Screen</source>
        <translation>Celá Obrazovka</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="115"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="576"/>
        <source>Add External Window</source>
        <translation>Přidat Externí Okno</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="157"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="861"/>
        <source>Help Center</source>
        <translation>Centrum Nápovědy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="692"/>
        <source>Tools</source>
        <translation>Nástroje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="821"/>
        <source>No Tools Available</source>
        <translation>Žádné Nástroje k Dispozici</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="889"/>
        <source>Reset</source>
        <translation>Resetovat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Quit</source>
        <translation>Ukončit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="947"/>
        <source>Delete</source>
        <translation>Smazat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="948"/>
        <source>Hide</source>
        <translation>Skrýt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="366"/>
        <source>New Workspace…</source>
        <translation>Nový Pracovní Prostor…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="135"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="765"/>
        <source>Clock</source>
        <translation>Hodiny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="143"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Stopwatch</source>
        <translation>Stopky</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="163"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="789"/>
        <source>Sessions</source>
        <translation>Relace</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="170"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>File Transmission</source>
        <translation>Přenos Souboru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="177"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="806"/>
        <source>AI Assistant</source>
        <translation>AI Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="352"/>
        <source>Show "%1"</source>
        <translation>Zobrazit "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Zobrazit Všechny Skryté Pracovní Prostory</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="380"/>
        <source>No Workspaces Available</source>
        <translation>Žádné Pracovní Prostory Nejsou k Dispozici</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="506"/>
        <source>Manage Plugins…</source>
        <translation>Spravovat Pluginy…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="596"/>
        <source>Export</source>
        <translation>Exportovat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="627"/>
        <source>CSV File</source>
        <translation>Soubor CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="633"/>
        <source>MDF4 File</source>
        <translation>Soubor MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="639"/>
        <source>Console Transcript</source>
        <translation>Přepis Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="648"/>
        <source>Session Database</source>
        <translation>Databáze Relací</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="662"/>
        <source>No Export Formats Available</source>
        <translation>Žádné Exportní Formáty k Dispozici</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="121"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="748"/>
        <source>Console</source>
        <translation>Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="127"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Notifications</source>
        <translation>Oznámení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="151"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="780"/>
        <source>Preferences</source>
        <translation>Předvolby</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="936"/>
        <source>Edit…</source>
        <translation>Upravit…</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="882"/>
        <source>Resume</source>
        <translation>Pokračovat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="883"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Disconnect</source>
        <translation>Odpojit</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Zastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>Začátek</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>Kolo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Resetovat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Celkem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Žádná kola nezaznamenána</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Stiskněte Kolo během běhu stopek</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="160"/>
        <source>No Data Available</source>
        <translation>Žádná Data k Dispozici</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Hodnoty Datových Sad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Hledat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Skupina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Datová Sada</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Jednotky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(virtuální)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Zkopírovat přístupový kód %1 do schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Přístupový kód datové sady zkopírován</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>V tomto projektu nejsou definovány žádné datové sady.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Žádné datové sady neodpovídají vašemu hledání.</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>Parametr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>Hodnota</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Vlastní Ikona)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>No</source>
        <translation>Ne</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>Yes</source>
        <translation>Ano</translation>
    </message>
</context>
<context>
    <name>TableFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Složka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="126"/>
        <source>Add Sub-folder</source>
        <translation>Přidat Podsložku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="128"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Přidat podsložku do této složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="136"/>
        <source>Add Shared Table</source>
        <translation>Přidat Sdílenou Tabulku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="138"/>
        <source>Add a shared table inside this folder</source>
        <translation>Přidat sdílenou tabulku do této složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="148"/>
        <source>Rename</source>
        <translation>Přejmenovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="150"/>
        <source>Rename folder</source>
        <translation>Přejmenovat složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="158"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="160"/>
        <source>Delete folder</source>
        <translation>Odstranit složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="173"/>
        <source>Title</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="174"/>
        <source>Registers</source>
        <translation>Registry</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="264"/>
        <source>This folder is empty. Use the toolbar to add a table or sub-folder.</source>
        <translation>Tato složka je prázdná. Použijte panel nástrojů k přidání tabulky nebo podsložky.</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Nabídka Start</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Nabídka</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Hledat…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Nastavení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Konzole</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Oznámení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Hodiny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Stopky</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>AI Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="986"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Připojeno k %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="987"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Nepřipojeno</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1011"/>
        <source>MQTT Publisher</source>
        <translation>Vydavatel MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1021"/>
        <source>Status:</source>
        <translation>Stav:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1029"/>
        <source>Connected</source>
        <translation>Připojeno</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1030"/>
        <source>Disconnected</source>
        <translation>Odpojeno</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1037"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1050"/>
        <source>Mode:</source>
        <translation>Režim:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1063"/>
        <source>Messages sent:</source>
        <translation>Odesláno zpráv:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1077"/>
        <source>Open MQTT Settings</source>
        <translation>Otevřít Nastavení MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Přenos Souboru</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">Hledat widgety…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Odebrat z pracovního prostoru</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Vybrat vše</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Vymazat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Odeslat data do zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Zobrazit Časovou Značku</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Echo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>Emulovat VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Barvy ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Zobrazení: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">Věděli Jste?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">Udržujte firmware jednoduchý odesíláním surových dat a nechte Serial Studio analyzovat je v JavaScriptu, Lua nebo pomocí vestavěných šablon bez kódu.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">Přiřaďte každému kanálu vlastní funkci pro kalibraci, filtrování nebo převod jednotek. Přeneste matematiku na Serial Studio a udržujte firmware štíhlý.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">Potřebujete hodnotu, kterou zařízení nikdy neposílá? Virtuální dataset vypočítá vlastní kanál, například výkon z napětí a proudu, vykreslený a zaznamenávaný jako data.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">Zachyťte poruchy jako na osciloskopu. Grafy s časovou osou mají režim zametání a spouštění a úroveň spouštění můžete přetáhnout přímo na grafu.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">Přestaňte hledat správný widget. Seskupte je do vlastních pracovních prostorů a přepínejte mezi nimi pomocí vyhledávání na hlavním panelu.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">Už nikdy neztratíte testovací běh. Zaznamenávejte relace do lokální databáze, poté je procházejte, označujte a přehrávejte, kdykoli je potřebujete.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">Předejte týmu vyleštěnou zprávu během několika sekund. Exportujte jakoukoli relaci do HTML nebo PDF, kompletní s grafy a statistikami min/max/průměr.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">Uzavřete smyčku bez dalších nástrojů. Výstupní Ovládací Prvky umožňují odesílat příkazy zpět do zařízení přímo z dashboardu.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">Vytvořte vizualizaci, kterou nikdo jiný nemá. Widget Painter spouští vlastní skript pro kreslení plně přizpůsobené grafiky z příchozích dat.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">Jeden nástroj pro každé připojení. Serial Studio čte z UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audia, USB, HID, MQTT a Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">Přeskočte tanec s terminálem. Odesílejte a přijímejte soubory přes sériové připojení pomocí vestavěných protokolů XMODEM, YMODEM a ZMODEM.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">Už máte mapu registrů Modbus nebo soubor DBC? Vygenerujte z ní automaticky projekt připravený k použití místo ručního vytváření.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">Popište, co chcete, a nechte AI Asistenta to vytvořit. Může vytvářet a upravovat projekty za vás napříč osmi poskytovateli modelů.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">Tip %1 z %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">Zjistit Více</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">Zobrazit Tipy při Spuštění</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">Předchozí</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">Další</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Zavřít</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>Čeká na schválení</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Hotovo</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Chyba</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Zamítnuto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Blokováno</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>Spuštěno</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Schválit</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Zamítnout</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>Argumenty</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>Kopírovat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Kopírovat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Vybrat Vše</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Výsledek</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Editor Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Otevřít Editor projektu pro vytvoření nebo úpravu JSON layoutu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>Přehrát CSV soubor, jako by se jednalo o živá data ze senzorů</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Předvolby</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Otevřít nastavení a předvolby aplikace</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>Otevřít CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>Otevřít MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Relace</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Procházet, přehrávat a exportovat zaznamenané relace</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">Zkratky</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">Vytvořit zkratku operátora pro aktuální projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Rozšíření</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Procházet a instalovat rozšíření</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">Konfigurovat připojení MQTT (publikovat nebo odebírat)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Nasadit</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">Vytvořit nasazení operátora pro aktuální projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Vybrat komunikaci přes sériový port (UART)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Vybrat vstupní zvukové zařízení (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Vybrat přímou komunikaci USB (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Síť</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>Vybrat síťovou komunikaci TCP/UDP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>Vybrat komunikaci MODBUS (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>Vybrat komunikaci se zařízením HID (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Vybrat komunikaci Bluetooth Low Energy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>Sběrnice CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>Vybrat komunikaci po sběrnici CAN (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Vybrat komunikaci přes rouru procesu (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Příklady</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Procházet ukázkové projekty</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Centrum Nápovědy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Procházet dokumentaci, FAQ a wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Zobrazit podrobnou dokumentaci a klást otázky na DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Připojit nebo odpojit nakonfigurované zařízení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>O Aplikaci</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Otevřít Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Otevřít existující JSON projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Přehrát MDF4 soubor jako živá data ze senzorů (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>Chatujte s AI pro vytvoření a úpravu vašeho projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Vytvořit operátorskou aplikaci pro aktuální projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Zobrazit informace o aplikaci a licenční údaje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI Wiki a Chat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Spravovat licenci a aktivovat Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Odpojit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Připojit</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">Připojit nebo odpojit od zařízení nebo MQTT brokeru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Aktivovat</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Nastavení Triggeru</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Udržet průběh nehybný zarovnáním každého průchodu k události triggeru.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Uzamkne opakující se signál na místě, jako tlačítko Auto na osciloskopu. Každý průběh začíná ve stejném bodě průběhu, takže zůstává nehybný místo rolování.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>Trigger</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">Režim:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>Režim</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>Normální</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Jednorázový</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto běží volně, když nic nepřekročí úroveň.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal čeká na překročení.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single zachytí jeden průběh a poté se zastaví.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Slope:</source>
        <translation>Sklon:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="273"/>
        <source>Trigger on a downward crossing</source>
        <translation>Trigger při sestupném překročení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="319"/>
        <source>Timebase:</source>
        <translation>Časová Základna:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="388"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Ponechte časovou základnu prázdnou pro použití časového rozsahu grafu; snižte pro přiblížení rychlého signálu. Holdoff ignoruje nové triggery na chvíli po každém.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Signál:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Hodnota pro překročení</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Hrana:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="254"/>
        <source>Rising</source>
        <translation>Náběžná</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="258"/>
        <source>Trigger on an upward crossing</source>
        <translation>Trigger při vzestupném překročení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="269"/>
        <source>Falling</source>
        <translation>Sestupná</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Nový průběh začíná pokaždé, když signál překročí úroveň ve zvoleném směru. Auto také běží volně, když není nalezeno žádné překročení.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="292"/>
        <source>Timing</source>
        <translation>Časování</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Časová Základna (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="332"/>
        <source>Match time range</source>
        <translation>Shodovat Časový Rozsah</translation>
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
        <translation>Prodleva:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="364"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">Časová základna nastavuje, kolik času zobrazuje jeden průběh; ponechte prázdné pro použití časového rozsahu grafu. Snižte pro přiblížení rychlého signálu. Holdoff ignoruje nové triggery na chvíli po každém.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="403"/>
        <source>Capture Next</source>
        <translation>Zachytit Další</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="405"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Aktivovat pro Další Jednorázové Zachycení</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Úroveň:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Úroveň spouštění</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Prodleva (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Čas prodlevy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Zdroj:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Znovu Aktivovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="418"/>
        <source>Close</source>
        <translation>Zavřít</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>COM Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="79"/>
        <source>Baud Rate</source>
        <translation>Přenosová Rychlost</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="163"/>
        <source>Data Bits</source>
        <translation>Datové Bity</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="184"/>
        <source>Parity</source>
        <translation>Parita</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="205"/>
        <source>Stop Bits</source>
        <translation>Stop Bity</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="226"/>
        <source>Flow Control</source>
        <translation>Řízení Toku</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="258"/>
        <source>Auto Reconnect</source>
        <translation>Automatické Připojení</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="276"/>
        <source>Send DTR Signal</source>
        <translation>Odeslat Signál DTR</translation>
    </message>
</context>
<context>
    <name>UI::AlarmMonitor</name>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="195"/>
        <source>Alarm</source>
        <translation>Alarm</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>critical</source>
        <translation>kritické</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>varování</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Hodnota %1%2 vstoupila do pásma %3 (%4–%5).</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="205"/>
        <source>Alarms</source>
        <translation>Alarmy</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1717"/>
        <source>Console</source>
        <translation>Konzole</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1725"/>
        <source>Notifications</source>
        <translation>Oznámení</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1733"/>
        <source>Clock</source>
        <translation>Hodiny</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1740"/>
        <source>Stopwatch</source>
        <translation>Stopky</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1786"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1801"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Záložní)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1823"/>
        <source>LED Panel (%1)</source>
        <translation>Panel LED (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="161"/>
        <source>Invalid</source>
        <translation>Neplatný</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1069"/>
        <source>Select Background Image</source>
        <translation>Vybrat Obrázek na Pozadí</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1071"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Obrázky (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>Zařízení USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Režim Přenosu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Hromadný Proud</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Pokročilý (Hromadný + Řídící)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Izochronní</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Připojení k zařízením USB pomocí hromadných, řídících nebo izochronních přenosů. Vhodné pro datové loggery, zařízení s vlastním firmwarem a USB přístroje.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>Specifikace USB (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>Koncový Bod IN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>Koncový Bod OUT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>Maximální Velikost Paketu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Přenosy Řízení Povoleny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>Odesílání nesprávných požadavků řízení může způsobit pád nebo poškození připojeného hardwaru. Používejte opatrně.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Informace o přenosech řízení USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>Velikost paketu by měla odpovídat maximální velikosti přenosu hlášené koncovým bodem. Typické hodnoty: 192 B (FS audio), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Stáhnout aktualizaci nyní?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Stáhnout aktualizaci nyní?&lt;br /&gt;Toto je povinná aktualizace, ukončení nyní zavře aplikaci.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Seznam změn:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>Verze %1 aplikace %2 byla vydána!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Momentálně nejsou k dispozici žádné aktualizace</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Gratulujeme! Používáte nejnovější verzi aplikace %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="172"/>
        <source>Add Register</source>
        <translation>Přidat Registr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="175"/>
        <source>Add register</source>
        <translation>Přidat registr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="182"/>
        <source>Insert Constant</source>
        <translation>Vložit Konstantu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="185"/>
        <source>Insert constant</source>
        <translation>Vložit konstantu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="192"/>
        <source>Import</source>
        <translation>Importovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="195"/>
        <source>Import registers from CSV</source>
        <translation>Importovat registry z CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="202"/>
        <source>Export</source>
        <translation>Exportovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="205"/>
        <source>Export registers to CSV</source>
        <translation>Exportovat registry do CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="217"/>
        <source>Rename</source>
        <translation>Přejmenovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="220"/>
        <source>Rename table</source>
        <translation>Přejmenovat tabulku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="227"/>
        <source>Delete</source>
        <translation>Smazat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="230"/>
        <source>Delete table</source>
        <translation>Smazat tabulku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="244"/>
        <source>Help</source>
        <translation>Nápověda</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="249"/>
        <source>Open help documentation for shared memory</source>
        <translation>Otevřít dokumentaci nápovědy pro sdílenou paměť</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="289"/>
        <source>Permissions</source>
        <translation>Oprávnění</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="290"/>
        <source>Register Name</source>
        <translation>Název Registru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="291"/>
        <source>Default Value</source>
        <translation>Výchozí Hodnota</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read-Only</source>
        <translation>Pouze pro Čtení</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read/Write</source>
        <translation>Čtení/zápis</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="468"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Zkopírovat přístupový kód %1 do schránky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="501"/>
        <source>Delete register</source>
        <translation>Smazat registr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="518"/>
        <source>No registers.</source>
        <translation>Žádné registry.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="568"/>
        <source>Register access code copied</source>
        <translation>Přístupový kód registru zkopírován</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="237"/>
        <source>Show Colorbar</source>
        <translation>Zobrazit Barevnou Škálu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="250"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Zobrazit Osy a Mřížku</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="263"/>
        <source>Show Crosshair</source>
        <translation>Zobrazit Zaměřovač</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Pause</source>
        <translation>Pozastavit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Resume</source>
        <translation>Obnovit</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">Vymazat Historii</translation>
    </message>
</context>
<context>
    <name>WebView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="70"/>
        <source>Web View Unavailable</source>
        <translation>Webové Zobrazení Nedostupné</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="81"/>
        <source>This build of Serial Studio was compiled without Qt WebEngine, so web pages cannot be displayed.</source>
        <translation>Toto sestavení Serial Studio bylo zkompilováno bez QT WebEngine, takže webové stránky nelze zobrazit.</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Vítejte v %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio je výkonný nástroj pro vizualizaci v reálném čase, vytvořený pro inženýry, studenty a kutily.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>Můžete spustit plně funkční 14denní zkušební verzi, aktivovat ji pomocí licenčního klíče nebo si stáhnout a zkompilovat zdrojový kód GPLv3 sami.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Zakoupením verze Pro přímo podpoříte autora a pomůžete financovat budoucí vývoj.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Sestavení verze GPLv3 vlastními silami pomáhá růstu komunity a podporuje technické příspěvky.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Čekejte prosím…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>Zbývá %1 dní zkušební verze.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">Aktuálně používáte plně funkční zkušební verzi %1 Pro. Je platná 14 dní pro osobní, nekomerční použití.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>Aktuálně používáte plně funkční zkušební verzi %1 Pro. Je platná 14 dní pro osobní, nekomerční použití.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Přejděte na placenou verzi, abyste mohli nadále používat Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Nebo zkompilujte zdrojový kód GPLv3 a používejte jej zdarma.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Dostupné předplatné zobrazíte kliknutím na „Upgradovat nyní" níže.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Neupozorňovat na zkušební verzi.
Chápu, že po jejím skončení budu muset zakoupit licenci nebo sestavit verzi GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>Zkušební verze %1 vypršela.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Zkušební období skončilo. Pro pokračování v používání %1 se všemi funkcemi Pro přejděte na placenou verzi.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>Případně můžete zkompilovat open-source verzi pod licencí GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Děkujeme, že jste vyzkoušeli %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Upgradovat Nyní</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Aktivovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="472"/>
        <source>See it in Action</source>
        <translation>Zobrazit v Akci</translation>
    </message>
    <message>
        <source>See it in action</source>
        <translation type="vanished">Zobrazit v akci</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="491"/>
        <source>Open in Limited Mode</source>
        <translation>Otevřít v Omezeném Režimu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="491"/>
        <source>Continue</source>
        <translation>Pokračovat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="491"/>
        <source>Start Trial</source>
        <translation>Spustit Zkušební Verzi</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <source>What's New in %1</source>
        <translation type="vanished">Co je nového v %1</translation>
    </message>
    <message>
        <source>Lua &amp; Built-In Parsers</source>
        <translation type="vanished">Lua a Vestavěné Parsery</translation>
    </message>
    <message>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation type="vanished">Parsování rámců v Lua 5.4 nebo pomocí vestavěných šablon bez kódu, společně s JavaScript.</translation>
    </message>
    <message>
        <source>AI Assistant</source>
        <translation type="vanished">AI Asistent</translation>
    </message>
    <message>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation type="vanished">Asistent v aplikaci napříč osmi poskytovateli, který může vytvářet a upravovat projekty za vás.</translation>
    </message>
    <message>
        <source>Device Control Loops</source>
        <translation type="vanished">Řídicí Smyčky Zařízení</translation>
    </message>
    <message>
        <source>Automate your device with an Arduino-style setup() and loop() routine that can read, write, and drive the dashboard.</source>
        <translation type="vanished">Automatizujte své zařízení pomocí rutiny setup() a loop() ve stylu Arduino, která může číst, zapisovat a ovládat dashboard.</translation>
    </message>
    <message>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation type="vanished">Sweep a Trigger Osciloskopu</translation>
    </message>
    <message>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation type="vanished">Sweep ve stylu osciloskopu s animovaným kurzorem triggeru, který lze přetahovat na grafu.</translation>
    </message>
    <message>
        <source>Output Controls</source>
        <translation type="vanished">Výstupní Ovládací Prvky</translation>
    </message>
    <message>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation type="vanished">Přenášejte zpět do vašeho zařízení pomocí ovládacích widgetů a pomocného protokolového enginu.</translation>
    </message>
    <message>
        <source>Dashboard Workspaces</source>
        <translation type="vanished">Pracovní Prostory Dashboardu</translation>
    </message>
    <message>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation type="vanished">Seskupte widgety do vlastních dashboardů a najděte je pomocí vyhledávání v hlavním panelu.</translation>
    </message>
    <message>
        <source>Session Database &amp; Reports</source>
        <translation type="vanished">Databáze Relací a Reporty</translation>
    </message>
    <message>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation type="vanished">Zaznamenávejte relace do SQLITE, přehrávejte je a exportujte HTML nebo PDF reporty.</translation>
    </message>
    <message>
        <source>Operator Deployments</source>
        <translation type="vanished">Nasazení pro Operátory</translation>
    </message>
    <message>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation type="vanished">Dodejte operátorům uzamčenou verzi pro jediný účel s režimem pouze pro běh.</translation>
    </message>
    <message>
        <source>New Dashboard Widgets</source>
        <translation type="vanished">Nové Widgety Dashboardu</translation>
    </message>
    <message>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation type="vanished">Ciferníky měřidel a ukazatelů s živými hodnotami, plus Hodiny, Stopky a Vodopád.</translation>
    </message>
    <message>
        <source>Dataset Transforms</source>
        <translation type="vanished">Transformace Datasetů</translation>
    </message>
    <message>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation type="vanished">Kalibrujte, filtrujte a převádějte každý kanál pomocí funkce pro každý dataset, nebo přidejte virtuální datasety, které vypočítají nové kanály.</translation>
    </message>
    <message>
        <source>Painter Widget</source>
        <translation type="vanished">Painter Widget</translation>
    </message>
    <message>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation type="vanished">Vykreslujte plně vlastní grafiku z příchozích dat pomocí vlastního skriptu pro kreslení.</translation>
    </message>
    <message>
        <source>Data Tables</source>
        <translation type="vanished">Datové Tabulky</translation>
    </message>
    <message>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation type="vanished">Živé tabulky ve stylu registrů s virtuálními datasety a editovatelnými buňkami.</translation>
    </message>
    <message>
        <source>Image Widget</source>
        <translation type="vanished">Widget Obrázku</translation>
    </message>
    <message>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation type="vanished">Dekódujte a zobrazujte snímky obrázků streamované přímo z vašeho zařízení.</translation>
    </message>
    <message>
        <source>Notifications &amp; Alarms</source>
        <translation type="vanished">Notifikace a Alarmy</translation>
    </message>
    <message>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation type="vanished">Víceúrovňové alarmy datasetů s úrovněmi závažnosti, směrované do Centra Notifikací.</translation>
    </message>
    <message>
        <source>Project Lock</source>
        <translation type="vanished">Zámek Projektu</translation>
    </message>
    <message>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation type="vanished">Uzamkněte projekt pro oddělení provozu od úprav pomocí přístupového kódu.</translation>
    </message>
    <message>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation type="vanished">MQTT, Protobuf a Přenos Souborů</translation>
    </message>
    <message>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation type="vanished">MQTT vstup a publikování, import Protobuf a přenosy XMODEM / YMODEM / ZMODEM.</translation>
    </message>
    <message>
        <source>Welcome to %1!</source>
        <translation type="vanished">Vítejte v %1!</translation>
    </message>
    <message>
        <source>Here's what's new in version %1.</source>
        <translation type="vanished">Co je nového ve verzi %1.</translation>
    </message>
    <message>
        <source>Show on Startup</source>
        <translation type="vanished">Zobrazit při Spuštění</translation>
    </message>
    <message>
        <source>See it in action</source>
        <translation type="vanished">Zobrazit v akci</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Zavřít</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Odebrat z Pracovního Prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="345"/>
        <source>Device Disconnected</source>
        <translation>Zařízení Odpojeno</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <source>Alarm</source>
        <translation type="vanished">Alarm</translation>
    </message>
    <message>
        <source>critical</source>
        <translation type="vanished">kritické</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">varování</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">Hodnota %1%2 vstoupila do pásma %3 (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Hodnota %1%2 dosáhla horního prahu alarmu %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Hodnota %1%2 dosáhla dolního prahu alarmu %3%2</translation>
    </message>
    <message>
        <source>Alarms</source>
        <translation type="vanished">Alarmy</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="166"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="189"/>
        <source>N</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="169"/>
        <source>NE</source>
        <translation>SV</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>E</source>
        <translation>V</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>SE</source>
        <translation>JV</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>S</source>
        <translation>J</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="181"/>
        <source>SW</source>
        <translation>JZ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="184"/>
        <source>W</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="187"/>
        <source>NW</source>
        <translation>SZ</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>Hodnota</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">Čekání na data…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>Satelitní Snímky</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Satelitní Snímky s Popisky</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>Silniční Mapa</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>Topografická Mapa</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>Terén</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>Plátno Světle Šedé</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>Plátno Tmavě Šedé</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>Další mapové vrstvy jsou dostupné pouze pro uživatele verze Pro.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Nemůžeme nabídnout neomezený přístup, protože klíč API ArcGIS má reálné náklady.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Time (s)</source>
        <translation>Čas (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Samples</source>
        <translation>Vzorky</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Časový limit skriptu přenosu vypršel po %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>Datová část překračuje maximální velikost</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="90"/>
        <source>Time (s)</source>
        <translation>Čas (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="116"/>
        <source>Samples</source>
        <translation>Vzorky</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="950"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Interval mřížky: %1 jednotek</translation>
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
        <translation>Stupně Šedi</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>Neznámá</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Upravit Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Nový Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Název:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>Můj Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Vyberte widgety k zahrnutí:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Filtrovat widgety…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Zrušit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="309"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
</context>
<context>
    <name>WorkspaceFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="42"/>
        <source>Folder</source>
        <translation>Složka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="127"/>
        <source>Add Sub-folder</source>
        <translation>Přidat Podsložku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="130"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Přidat podsložku do této složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="138"/>
        <source>Add Workspace</source>
        <translation>Přidat Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="141"/>
        <source>Add a workspace inside this folder</source>
        <translation>Přidat pracovní prostor do této složky</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="151"/>
        <source>Rename</source>
        <translation>Přejmenovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="153"/>
        <source>Rename folder</source>
        <translation>Přejmenovat složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="162"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="164"/>
        <source>Delete folder</source>
        <translation>Odstranit složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="178"/>
        <source>Title</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="179"/>
        <source>Contents</source>
        <translation>Obsah</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="269"/>
        <source>This folder is empty. Use the toolbar to add a workspace or sub-folder.</source>
        <translation>Tato složka je prázdná. Pomocí panelu nástrojů přidejte pracovní prostor nebo podsložku.</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="43"/>
        <source>Workspace</source>
        <translation>Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Add Widget</source>
        <translation>Přidat Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="146"/>
        <source>Add widget to workspace</source>
        <translation>Přidat widget do pracovního prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="157"/>
        <source>Move Up</source>
        <translation>Posunout Nahoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="159"/>
        <source>Move workspace up</source>
        <translation>Posunout pracovní prostor nahoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="169"/>
        <source>Move Down</source>
        <translation>Posunout Dolů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="171"/>
        <source>Move workspace down</source>
        <translation>Posunout pracovní prostor dolů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="181"/>
        <source>Change Icon</source>
        <translation>Změnit Ikonu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Change workspace icon</source>
        <translation>Změnit ikonu pracovního prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="198"/>
        <source>Rename</source>
        <translation>Přejmenovat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="200"/>
        <source>Rename workspace</source>
        <translation>Přejmenovat pracovní prostor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="209"/>
        <source>Delete</source>
        <translation>Odstranit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="211"/>
        <source>Delete workspace</source>
        <translation>Odstranit pracovní prostor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="233"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="239"/>
        <source>Group</source>
        <translation>Skupina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="234"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="240"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="235"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="241"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="285"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="323"/>
        <source>(unknown)</source>
        <translation>(neznámý)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="303"/>
        <source>(group widget)</source>
        <translation>(skupinový widget)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="353"/>
        <source>Remove widget from workspace</source>
        <translation>Odebrat widget z pracovního prostoru</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">Odebrat z pracovního prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="373"/>
        <source>No widgets in this workspace.</source>
        <translation>V tomto pracovním prostoru nejsou žádné widgety.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Pracovní Prostory</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="124"/>
        <source>Add Folder</source>
        <translation>Přidat Složku</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="126"/>
        <source>Add a top-level folder</source>
        <translation>Přidat složku nejvyšší úrovně</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="151"/>
        <source>Customize</source>
        <translation>Přizpůsobit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="153"/>
        <source>Edit workspaces manually</source>
        <translation>Upravit pracovní prostory ručně</translation>
    </message>
    <message>
        <source>Move Up</source>
        <translation type="vanished">Posunout Nahoru</translation>
    </message>
    <message>
        <source>Move the selected workspace up</source>
        <translation type="vanished">Posunout vybraný pracovní prostor nahoru</translation>
    </message>
    <message>
        <source>Move Down</source>
        <translation type="vanished">Posunout Dolů</translation>
    </message>
    <message>
        <source>Move the selected workspace down</source>
        <translation type="vanished">Posunout vybraný pracovní prostor dolů</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="135"/>
        <source>Add Workspace</source>
        <translation>Přidat Pracovní Prostor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="137"/>
        <source>Add workspace</source>
        <translation>Přidat pracovní prostor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="167"/>
        <source>Cleanup</source>
        <translation>Vyčistit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="170"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>Odstranit %1 odkazů na widgety, jejichž cílová skupina nebo dataset již neexistuje</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>No stale widget references in any workspace</source>
        <translation>Žádné zastaralé odkazy na widgety v žádném pracovním prostoru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>Title</source>
        <translation>Název</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="189"/>
        <source>Contents</source>
        <translation>Obsah</translation>
    </message>
    <message>
        <source>Widgets</source>
        <translation type="vanished">Widgety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="282"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>Žádné pracovní prostory. Přidejte jeden pomocí panelu nástrojů výše nebo obnovte automatické rozvržení.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Projekt nemá žádné vhodné skupiny -- přidejte skupinu s widgety pro naplnění pracovních prostorů.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="290"/>
        <source>Reset to Auto Layout</source>
        <translation>Obnovit Automatické Rozvržení</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">Žádné pracovní prostory.</translation>
    </message>
</context>
</TS>