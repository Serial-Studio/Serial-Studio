<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="nl_NL" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Anthropic-fout</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>Stream-parsefout: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>Ongeldige API-sleutel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>Snelheidslimiet bereikt: %1</translation>
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
        <translation>AI-apparaatbesturing toestaan?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="192"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>Hiermee kan de AI-assistent apparaten configureren, verbindingen openen en sluiten, en gegevens naar je hardware verzenden.

Elke apparaatactie vereist nog steeds je expliciete goedkeuring per aanroep in de chat, zelfs wanneer automatisch goedkeuren is ingeschakeld. Schakel dit alleen in als je de geconfigureerde AI-provider vertrouwt met hardwaretoegang.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Switch AI provider?</source>
        <translation>Van AI-provider wisselen?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="391"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Wisselen naar een andere provider wist het huidige gesprek. Doorgaan?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="394"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="431"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI-assistent is niet beschikbaar in deze build</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">AI-assistent vereist een Pro-licentie</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="436"/>
        <source>Set an API key first</source>
        <translation>Stel eerst een API-sleutel in</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">AI-assistent vereist een Pro-licentie</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="172"/>
        <source>AI Assistant is not available in this build</source>
        <translation>AI-assistent is niet beschikbaar in deze build</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="178"/>
        <source>AI subsystem not initialized</source>
        <translation>AI-subsysteem niet geïnitialiseerd</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="184"/>
        <source>Already busy with a previous request</source>
        <translation>Al bezig met een eerder verzoek</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="500"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Budgetlimiet voor tool-aanroepen bereikt voor deze beurt; er worden geen verdere tools uitgevoerd.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1098"/>
        <source>Waiting for %1 to respond. Loading the model and processing the prompt can take a while on local hardware...</source>
        <translation>Wachten op reactie van %1. Het laden van het model en verwerken van de prompt kan even duren op lokale hardware...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1956"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>U heeft de budgetlimiet voor tool-aanroepen voor deze beurt bereikt. Vraag geen verdere tools aan. Vat samen wat u tot nu toe heeft gevonden, en als de taak onvolledig is, geef dan aan welke stappen resteren zodat de gebruiker u kan vragen door te gaan.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Budget voor tool-aanroepen overschreden</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="945"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Het model heeft een lege respons geretourneerd. Probeer de vraag anders te formuleren, schakel over naar een ander model of controleer of het verzoek is toegestaan door de veiligheidsfilters van de provider.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1102"/>
        <source>Sending request to %1...</source>
        <translation>Verzoek wordt verzonden naar %1...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1114"/>
        <source>Provider returned no reply</source>
        <translation>Provider heeft geen antwoord geretourneerd</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="146"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Prompt geblokkeerd door Gemini-veiligheidsfilter: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="200"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini gestopt zonder een respons te produceren: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="262"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="265"/>
        <source>Invalid API key (%1)</source>
        <translation>Ongeldige API-sleutel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="267"/>
        <source>Rate limited: %1</source>
        <translation>Snelheidslimiet bereikt: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="269"/>
        <source>Invalid API key</source>
        <translation>Ongeldige API-sleutel</translation>
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
        <translation>Ongeldige API-sleutel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="431"/>
        <source>Rate limited: %1</source>
        <translation>Snelheidslimiet bereikt: %1</translation>
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
        <translation>Protobuf-bestand Exporteren</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>Kan GRPC-server niet starten</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>Binding aan %1 mislukt</translation>
    </message>
</context>
<context>
    <name>API::ProcessLauncher</name>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="82"/>
        <source>No program specified</source>
        <translation>Geen programma opgegeven</translation>
    </message>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="88"/>
        <source>Program "%1" not found in PATH</source>
        <translation>Programma "%1" niet gevonden in PATH</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>Kan API TCP-server niet starten</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>Externe API-Verbindingen Toestaan?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Het blootstellen van de API-server aan externe hosts stelt andere apparaten op uw netwerk in staat verbinding te maken met Serial Studio op poort 7777.

Schakel dit alleen in op vertrouwde netwerken. Niet-vertrouwde clients kunnen live data lezen of commando's naar uw apparaat sturen.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>Kan API TCP-server niet herstarten</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>API-apparaatbesturing toestaan?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Een programma dat de lokale API van Serial Studio gebruikt, vraagt toestemming om gegevens naar het aangesloten apparaat te verzenden. API-clients toestaan naar het apparaat te schrijven?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>API server</source>
        <translation>API-server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>Invalid pending connection</source>
        <translation>Ongeldige wachtende verbinding</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>Over</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>Versie %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Copyright © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>Alle Rechten Voorbehouden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 is vrije software: u kunt het herdistribueren en/of wijzigen onder de voorwaarden van de GNU General Public License zoals gepubliceerd door de Free Software Foundation; ofwel versie 3 van de Licentie, of (naar uw keuze) een latere versie.

%1 wordt gedistribueerd in de hoop dat het nuttig zal zijn, maar ZONDER ENIGE GARANTIE; zelfs zonder de impliciete garantie van VERKOOPBAARHEID of GESCHIKTHEID VOOR EEN BEPAALD DOEL. Zie de GNU General Public License voor meer details.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Deze configuratie is gelicentieerd voor commercieel en eigendomsgebruik. Het mag worden gebruikt in closed-source en commerciële toepassingen, onder voorbehoud van de voorwaarden van de commerciële licentie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Deze configuratie is uitsluitend voor persoonlijk gebruik en evaluatiedoeleinden. Commercieel gebruik is verboden tenzij een geldige commerciële licentie is geactiveerd.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>Deze software wordt geleverd 'zoals het is' zonder enige garantie, expliciet of impliciet, inclusief maar niet beperkt tot garanties van verkoopbaarheid of geschiktheid voor een bepaald doel. In geen geval is de auteur aansprakelijk voor schade voortvloeiend uit het gebruik van deze software.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Licentie Beheren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Doneren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Controleren op Updates</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>Wat Is Nieuw</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">Tips &amp; Trucs</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>Licentieovereenkomst</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>Fout Melden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>Dankbetuigingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>Website</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="186"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="187"/>
        <source>Settings</source>
        <translation>Instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="245"/>
        <source>G-FORCE</source>
        <translation>G-KRACHT</translation>
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
        <translation>Accelerometer Configuratie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Configureer het weergavebereik en de invoereenheden van de accelerometer.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Weergavebereik</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>Max G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Voer maximale G-waarde in</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Invoerconfiguratie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Invoer reeds in G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>Dankbetuigingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>Over QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Pictogram Wijzigen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Wijzig het pictogram dat voor deze actie wordt gebruikt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Dupliceren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Dupliceer deze actie met alle instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Verwijder deze actie uit het project</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Widget Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Beschikbare Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Klik op een rij om deze aan de werkruimte toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Zoeken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Groep</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(volledige groep)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Reeds in werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Toevoegen aan werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Geen widgets beschikbaar.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Geen widgets komen overeen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 van %2 widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Alarmbanden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <source>Info</source>
        <translation>Info</translation>
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
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>Kritiek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>Toerenteller</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>Stationair</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>Bedrijf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>Voorzichtig</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>Roodgebied</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>Snelheidsmeter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>Cruise</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>Snel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>Topsnelheid</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>Motortemperatuur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>Koud</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>Normaal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>Warm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>Oververhitting</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>Druk</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>Vacuüm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>Hoog</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>Piek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>Batterijspanning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>Laag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>Nominaal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>Brandstofniveau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>Leeg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>Reserve</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>Signaalsterkte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>Geen Signaal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>Zwak</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>Goed</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>CPU / Systeembelasting</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>Bezig</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>Overbelasting</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Waarschuwing / Kritiek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>Indicator (Aan / Uit)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>Aan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>Foutindicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>Fout</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>Kies Bandkleur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>Voorinstellingen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>Voorinstelling</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>Kies voorinstelling…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>Knipperen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>Herstel naar standaard ernst</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Klik om een kleur te kiezen. Rechtsklik om te herstellen naar standaard ernst.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>Klik om een aangepaste kleur te kiezen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>Laat de LED knipperen terwijl de waarde in dit bereik zit.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Geen banden gedefinieerd. Kies een voorinstelling hierboven of voeg een band toe om te beginnen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>Toepassen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>Wijzigingen in de dataset toepassen.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Voorinstelling Toepassen</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Vervang de huidige banden door de geselecteerde voorinstelling, geschaald naar het bereik van deze dataset.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>Bereik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>Banden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>Band Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Voeg een nieuwe band toe die aansluit op de laatste.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="437"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="443"/>
        <source>Max</source>
        <translation>Max</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="449"/>
        <source>Severity</source>
        <translation>Ernst</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>Kleur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>Label</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Kies een aangepaste kleur.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(optioneel)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>Omhoog verplaatsen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>Omlaag verplaatsen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>Verwijder deze band.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Geen banden gedefinieerd. Pas een voorinstelling toe of voeg een band toe om te beginnen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>Voorbeeld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>Wijzigingen verwerpen.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Opslaan</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Wijzigingen in de dataset opslaan.</translation>
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
        <translation>CSV vs MDF4 export - wat is het verschil?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Bar en Gauge - wanneer gebruik je elk?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>How can I help with your project?</source>
        <translation>Hoe kan ik helpen met je project?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="205"/>
        <source>Set up your API key to get started</source>
        <translation>Stel je API-sleutel in om te beginnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="217"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Beschrijf wat je wilt bouwen, en ik configureer de bronnen, groepen, datasets, frame-parsers en transformaties voor je.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="220"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>Om te beginnen met chatten, plak een API-sleutel voor de geselecteerde provider. Sleutels worden versleuteld op deze machine en verlaten nooit je computer, behalve om te communiceren met de provider die je kiest.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="241"/>
        <source>Open API Key Setup</source>
        <translation>Open API-sleutelinstellingen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="251"/>
        <source>Get a key from %1</source>
        <translation>Verkrijg een sleutel van %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>List the sources in this project</source>
        <translation>Toon de bronnen in dit project</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Help me de functies van Serial Studio te ontdekken</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Wat kan deze app doen voor mijn telemetrie?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>Walk me through what this project already contains</source>
        <translation>Leid me door wat dit project al bevat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Wat is een sessiedatabase, en waarom zou ik er een gebruiken?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">CSV vs MDF4 export — wat is het verschil?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Wat is een frame-parser, en wanneer heb ik er een nodig?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>Wanneer moet ik Lua vs JavaScript gebruiken voor de parser?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Plot, Bar en Gauge — wanneer gebruik je elk?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="133"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Wat is het verschil tussen een transform en een frame-parser?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Voeg een UART-bron toe voor een Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Stel een IMU-project vanaf nul in</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Configure an MQTT subscriber</source>
        <translation>Configureer een MQTT-abonnee</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Add a CAN bus source</source>
        <translation>Voeg een CAN-busbron toe</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Set up a Modbus poller</source>
        <translation>Stel een Modbus-poller in</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Voeg een netwerkbron (TCP/UDP) toe</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Write a CSV frame parser for me</source>
        <translation>Schrijf een CSV frame-parser voor me</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Help me parse a JSON frame</source>
        <translation>Help me een JSON-frame te parsen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>Voeg een EMA-afvlakkingstransformatie toe aan een dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Decode hexadecimal frames</source>
        <translation>Decodeer hexadecimale frames</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="146"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Kalibreer een sensor met een lineaire transformatie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Stel dashboardwidgets voor mijn gegevens voor</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Build an executive overview workspace</source>
        <translation>Bouw een executive overview-werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Voeg een painter-widget toe voor een aangepaste visualisatie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Toon Plot, FFT en Waterfall voor één dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="153"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Groepeer mijn datasets in nuttige werkruimten</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>Sleep bestanden of mappen om de assistent ze te laten lezen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="477"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>Map "%1" toegevoegd - leesbaar deze sessie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="478"/>
        <source>Added "%1" - readable this session</source>
        <translation>"%1" toegevoegd - leesbaar deze sessie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="562"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Vraag Serial Studio alles…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="582"/>
        <source>Clear conversation</source>
        <translation>Gesprek wissen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="626"/>
        <source>Stop generating</source>
        <translation>Genereren stoppen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="627"/>
        <source>Send message (Enter)</source>
        <translation>Bericht verzenden (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="669"/>
        <source>Provider</source>
        <translation>Provider</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="702"/>
        <source>Model selection</source>
        <translation>Modelselectie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="748"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>Voer bewerkingsacties uit zonder elke keer te vragen. Geblokkeerde acties blijven geblokkeerd.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="750"/>
        <source>Auto-approve edits</source>
        <translation>Bewerkingen automatisch goedkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="766"/>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation>Laat de AI apparaten configureren, verbinden/verbreken en gegevens verzenden. Elke actie vraagt nog steeds om je goedkeuring.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="768"/>
        <source>Allow device control</source>
        <translation>Apparaatbesturing toestaan</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Manage API keys</source>
        <translation>API-sleutels beheren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="815"/>
        <source>Working</source>
        <translation>Bezig</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="816"/>
        <source>Ready</source>
        <translation>Gereed</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="817"/>
        <source>  •  cache %1k tok</source>
        <translation>•  cache %1k tok</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="818"/>
        <source>  •  cache write %1k tok</source>
        <translation>cache schrijven %1k tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Geen Microfoon Gedetecteerd</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Sluit een microfoon aan of controleer de instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Invoerapparaat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Samplefrequentie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Sampleformaat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Kanalen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Uitvoerapparaat</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">Dialoogvenster</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">Geef de gebruikersnaam en het wachtwoord op voor de downloadlocatie.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">Gebr&amp;uikersnaam:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Wachtwoord:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="65"/>
        <source>Axis Range Configuration</source>
        <translation>Asbereik Configuratie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="179"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Configureer het zichtbare bereik voor de plotassen. Waarden worden in realtime bijgewerkt tijdens het typen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="187"/>
        <source>X Axis</source>
        <translation>X-as</translation>
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
        <translation>Voer minimale waarde in</translation>
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
        <translation>Voer maximale waarde in</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="260"/>
        <source>Y Axis</source>
        <translation>Y-as</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="335"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="345"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Back-up Herstellen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Naamloos</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Project Geladen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Automatisch Opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Vóór Herstellen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Vóór Dataset Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Vóór Groep Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Vóór Nieuw Project</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Vóór Project Openen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Vóór JSON Laden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Vóór Sjabloon Toepassen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Vóór Actie Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Vóór Uitvoerwidget Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Voor Dataset Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Voor Groep Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Voor Werkruimte Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Voor Alle Werkruimtes Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Voor Widget Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Voor Werkruimtes Herschikken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Voor Batchbewerking</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Voor Tegel Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (en %2 meer)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>De frame-parsercode verschilt ook en wordt vervangen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Titel wijzigt van "%1" naar "%2". Groepstructuur ongewijzigd.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Dezelfde groepen en datasets, maar de frame-parsercode verschilt — herstellen vervangt deze.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Dezelfde groepen en datasets als uw huidige project. Herstellen kan nog steeds wijzigingen op veldniveau terugdraaien.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Herstellen verwijdert %1 en brengt %2 terug.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Herstellen verwijdert %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Herstellen brengt %1 terug.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Kies een back-up om te herstellen. Het huidige project wordt automatisch eerst opgeslagen, dus het herstellen is omkeerbaar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Nog geen back-ups voor dit project. Bewerk of sla het project op om de doorlopende back-up te starten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Map Openen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Herstellen</translation>
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
        <translation>%1 frames/s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="83"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="88"/>
        <source>n/a</source>
        <translation>n.v.t.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>Geslaagd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>Mislukt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>Hotpath Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Meet hoe snel deze computer frames kan extraheren, parseren en visualiseren via de datapijplijn van Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>De interface bevriest tijdens het uitvoeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Elke fase draait op volle snelheid op de hoofdthread, waardoor het venster niet meer reageert totdat de fase is voltooid. Het huidige project wordt automatisch opnieuw geladen wanneer de benchmark eindigt.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>Frames per fase:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>Minimale duur:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>Fasen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>Parsers</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>Data-export</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>Gegevens</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>Alleen numeriek</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>Gemengd (numeriek + tekst)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>Selecteer ten minste één fase en één datatype om een benchmark uit te voeren.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>%1 Actief...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>Voorbereiden...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>Doorvoer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>Tijd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>Resultaat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>Voer een test uit om resultaten te zien</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>Geslaagd/Mislukt is van toepassing op de data-pipeline- en parserfasen (data-pipeline en Ingebouwde numeriek 1024 K frames/s; Ingebouwde gemengd 512 K; Lua numeriek 256 K; JavaScript numeriek en Lua gemengd 128 K; JavaScript gemengd 64 K). De export- en dashboardfasen zijn informatief.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Geslaagd/Mislukt is van toepassing op de data-pipeline- en parserfasen (data-pipeline en ingebouwde numeriek 1024 K frames/s; ingebouwde gemengd 512 K; Lua numeriek 256 K; JavaScript numeriek en Lua gemengd 128 K; JavaScript gemengd 64 K). De export- en dashboardfasen zijn informatief.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail is van toepassing op de data-pipeline en parserfasen (data-pipeline en Native numeriek 1024 K frames/s; Native gemengd 512 K; Lua numeriek 256 K; JavaScript numeriek en Lua gemengd 128 K; JavaScript gemengd 64 K). De export- en dashboardfasen zijn informatief.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Geslaagd/Mislukt is alleen van toepassing op de data-pipeline- en parserfasen (data-pipeline 1024 K frames/s; numeriek: Lua 256 K, JavaScript 128 K; gemengd: Lua 128 K, JavaScript 64 K). De export- en dashboardfasen zijn informatief.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Geslaagd/Mislukt is alleen van toepassing op de parserfasen (Lua-doel 256 K frames/s, JavaScript 128 K). De export- en dashboardfasen zijn informatief.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>Bezig...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>Benchmark Uitvoeren</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>Datapijplijn</translation>
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
        <translation type="vanished">Lua + data-export</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + dashboard</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Native parser (numeriek)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Native parser (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Ingebouwde parser (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Ingebouwde parser (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Lua-parser (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>JavaScript-parser (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Lua-parser (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>JavaScript-parser (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Ingebouwd + data-export (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + data-export (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + data-export (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Ingebouwd + data-export (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + data-export (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + data-export (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Ingebouwd + dashboard (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + dashboard (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 K frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 K frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 K frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 M frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>1 seconde</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>2 seconden</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 seconden</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 seconden</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - Hotpath-benchmark</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), werklast: minimaal %3 frames, minimaal %4 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>Doorvoer</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>Doel</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>Tijd</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>Resultaat</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 frames/s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>n.v.t.</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>Geslaagd</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + dashboard (numeriek)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>Ingebouwd + dashboard (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + dashboard (gemengd)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + dashboard (gemengd)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Dashboard weergeven...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Apparaat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Service</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Characteristic</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Scannen…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Geen Bluetooth-adapter Gedetecteerd</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Sluit een Bluetooth-adapter aan of controleer de systeeminstellingen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Dit Besturingssysteem wordt nog niet Ondersteund.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio wordt bijgewerkt om met dit besturingssysteem te werken zodra QT het officieel ondersteunt</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Geen CAN-stuurprogramma's Gevonden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>Installeer CAN-hardwarestuurprogramma's voor dit systeem</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>CAN-stuurprogramma</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="140"/>
        <source>Interface</source>
        <translation>Interface</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="169"/>
        <source>Bitrate</source>
        <translation>Bitsnelheid</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="223"/>
        <source>Flexible Data-Rate</source>
        <translation>Flexibele Datasnelheid</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="243"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="263"/>
        <source>Listen-Only</source>
        <translation>Alleen-luisteren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="283"/>
        <source>DBC Database</source>
        <translation>DBC-database</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="287"/>
        <source>Import DBC File…</source>
        <translation>DBC-bestand Importeren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="320"/>
        <source>No CAN Interfaces Found</source>
        <translation>Geen CAN-interfaces Gevonden</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>Selecteer CSV-bestand</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-bestanden (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>Apparaatverbinding Actief</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Om deze functie te gebruiken moet de verbinding met het apparaat worden verbroken. Doorgaan?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>Controleer bestandsrechten en locatie</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Onvoldoende Gegevens in CSV-Bestand</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>Kan CSV-bestand niet lezen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>Het CSV-bestand moet ten minste één gegevensrij bevatten om door te gaan. Controleer het bestand en probeer het opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>Ongeldig CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>Het CSV-bestand bevat geen gegevens of headers.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>Selecteer een datum/tijd-kolom</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>Interval handmatig instellen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>CSV Datum/tijd-selectie</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Kies hoe de datum/tijd-gegevens moeten worden verwerkt:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>Interval Instellen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Voer het interval tussen rijen in milliseconden in:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>Selecteer Datum/tijd-kolom</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Selecteer de kolom die de datum/tijd-gegevens bevat:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>Ongeldige Selectie</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>De geselecteerde kolom is niet geldig.</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>Console-export is een Pro-functie.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Deze functie vereist een licentie. Koop er een om console-export in te schakelen.</translation>
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
        <translation>Geen Regeleindiging</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>Nieuwe Regel</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>Carriage Return</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>Platte Tekst</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimaal</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>Geen Checksum</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>Apparaat %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Constante Invoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>Fundamenteel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>Lichtsnelheid in vacuüm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Planck-constante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Elementaire lading</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Avogadro-constante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Boltzmann-constante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Stefan-Boltzmann-constante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>Mechanica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>Standaardzwaartekracht</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Gravitatieconstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Druk</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Standaardatmosfeer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Barometrische druk op zeeniveau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>Temperatuur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>Absoluut nulpunt (Celsius)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Vriespunt van water</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Kookpunt van water (1 atm)</translation>
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
        <translation>Gassen en Vloeistoffen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Universele gasconstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Specifieke gasconstante (droge lucht)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Specifieke gasconstante (waterdamp)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Luchtdichtheid (zeeniveau, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Waterdichtheid (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Geluidssnelheid in lucht (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Warmtecapaciteitsverhouding (droge lucht)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>Elektromagnetisme</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>Vacuümpermittiviteit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Vacuümpermeabiliteit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Wiskunde</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Getal van Euler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Gulden snede</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Natuurkundige Constanten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>SI-eenheid vooringestelde waarden. Klik op een rij om deze in te voegen in %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Zoeken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Symbool</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Waarde</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Categorie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Geen overeenkomende constanten.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 constanten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 van %2 constanten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Controlescript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Loop</source>
        <translation>Besturingslus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>Ongedaan Maken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>Opnieuw</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>Knippen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>Plakken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>Alles Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>Document Formatteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>Selectie Formatteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control loop</source>
        <translation>Herstel naar de standaard besturingslus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control loop file</source>
        <translation>Importeer een besturingslus-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control loop documentation</source>
        <translation>Open de besturingslusdocumentatie</translation>
    </message>
    <message>
        <source>Reset to the default control script</source>
        <translation type="vanished">Herstel naar het standaard controlescript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>Openen</translation>
    </message>
    <message>
        <source>Import a control script file</source>
        <translation type="vanished">Importeer een controlescriptbestand</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>Maak de laatste codebewerking ongedaan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>Voer de eerder ongedaan gemaakte bewerking opnieuw uit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>Knip geselecteerde code naar klembord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>Kopieer geselecteerde code naar klembord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>Plak code vanaf klembord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <source>Open the control script documentation</source>
        <translation type="vanished">Open de controlescriptdocumentatie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>Valideren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Controleer of het script correct compileert</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Herstelopties</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio is onverwacht meerdere keren achter elkaar afgesloten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>Opeenvolgende crashes: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Laatst gerapporteerde fase: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Gedetecteerd op: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Kies een herstelactie. Serial Studio sluit na het toepassen hiervan zodat het volgende opstarten schoon begint.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Rendering-backend Resetten naar Standaard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Overslaan van het Herstellen van het Laatst Geopende Project</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Alle Voorkeuren Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Toch Doorgaan</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>CSV-speler</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>DBC-bestandsvoorbeeld</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>DBC-bestand: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Bekijk de CAN-berichten en signalen om te importeren in een nieuw Serial Studio-project.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Berichten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Berichtnaam</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>CAN-ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Signalen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>Geen berichten gevonden in DBC-bestand.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Totaal: %1 berichten, %2 signalen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Project Aanmaken</translation>
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
        <translation>Verzenden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Geen zendfunctie gedefinieerd</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="326"/>
        <source>Set Wallpaper…</source>
        <translation>Achtergrond Instellen…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="332"/>
        <source>Clear Wallpaper</source>
        <translation>Achtergrond Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="342"/>
        <source>Tile Windows</source>
        <translation>Vensters Naast Elkaar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="361"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-functies gedetecteerd in dit project.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="363"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Fallback-widgets zijn actief. Koop een licentie voor volledige functionaliteit.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="504"/>
        <source>Empty Workspace</source>
        <translation>Lege Werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="518"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Gebruik de zoekbalk om widgets te vinden en toe te voegen, of klik met de rechtermuisknop op een widget in een andere werkruimte om deze hier toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="533"/>
        <source>Search Widgets</source>
        <translation>Widgets Zoeken</translation>
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
        <translation>API-server Actief (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="216"/>
        <source>API Server Ready</source>
        <translation>API-server Gereed</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="217"/>
        <source>API Server Off</source>
        <translation>API-server Uit</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="155"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="307"/>
        <source>Send</source>
        <translation>Verzenden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="295"/>
        <source>Enter command…</source>
        <translation>Voer commando in…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Geen verzendfunctie gedefinieerd</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Voer commando in…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Verzenden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Geen verzendfunctie gedefinieerd</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>AAN</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>UIT</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>Geen verzendfunctie gedefinieerd</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Resume</source>
        <translation>Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="313"/>
        <source>Awaiting data…</source>
        <translation>Wachten op gegevens…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="368"/>
        <source>Open %1 in a separate window</source>
        <translation>Open %1 in een apart venster</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="289"/>
        <source>Select Javascript file to import</source>
        <translation>Selecteer Javascript-bestand om te importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="368"/>
        <source>Code Validation Failed</source>
        <translation>Codevalidatie Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="358"/>
        <source>Line %1: %2</source>
        <translation>Regel %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="369"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>Het script moet een setup()- en/of loop()-functie definiëren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="374"/>
        <source>Code Validation Successful</source>
        <translation>Codevalidatie Geslaagd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="375"/>
        <source>No syntax errors detected in the control loop.</source>
        <translation>Geen syntaxfouten gedetecteerd in de besturingslus.</translation>
    </message>
    <message>
        <source>No syntax errors detected in the control script.</source>
        <translation type="vanished">Geen syntaxfouten gedetecteerd in het controlescript.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>Importeer DBC-bestand</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>DBC-Bestanden (*.DBC);;Alle Bestanden (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>Parseren van DBC-bestand mislukt: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>Controleer het bestandsformaat en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>DBC-importfout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>DBC-bestand bevat geen berichten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Het geselecteerde bestand bevat geen CAN-berichtdefinities.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>DBC-importwaarschuwing</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>Overzicht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>Actief</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Aanmaken van tijdelijk projectbestand mislukt</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">Controleer of de applicatie schrijfrechten heeft voor de tijdelijke map.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>DBC-bestand succesvol geïmporteerd met %1 berichten en %2 signalen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>De projecteditor is nu geopend voor aanpassing.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Kan geïmporteerd project niet laden</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">De gegenereerde project-JSON kon niet worden geladen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>%1 signaal/signalen overgeslagen die uitgebreide multiplexing gebruiken (SG_MUL_VAL_); alleen eenvoudige multiplexing wordt ondersteund.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>DBC-import Voltooid</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="254"/>
        <source>CAN Bus</source>
        <translation>CAN-bus</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="60"/>
        <source>Dataset Value Transform</source>
        <translation>Dataset Waardetransformatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="97"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="97"/>
        <source>JavaScript</source>
        <translation>Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="121"/>
        <source>Language:</source>
        <translation>Taal:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="124"/>
        <source>Template:</source>
        <translation>Sjabloon:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="103"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Voer ruwe waarde in (bijv. 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="108"/>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="109"/>
        <source>Clear</source>
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="135"/>
        <source>Input:</source>
        <translation>Invoer:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="138"/>
        <source>Output:</source>
        <translation>Uitvoer:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Apply</source>
        <translation>Toepassen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="112"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="216"/>
        <source>Transform — %1</source>
        <translation>Transformatie — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="298"/>
        <source>Enter a value</source>
        <translation>Voer een waarde in</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="305"/>
        <source>Invalid number</source>
        <translation>Ongeldig getal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="374"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Document Formatteren	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="375"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Selectie Formatteren	ctrl+i</translation>
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
-- Definieer een transform(value)-functie die de live
-- datasetwaarde ontvangt en een getransformeerd getal retourneert.
-- Als er geen transform()-functie is gedefinieerd, wordt de
-- ruwe waarde behouden en wordt er niets met het project opgeslagen.
--
-- Voorbeeld:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Globale variabelen die op het hoogste niveau worden gedeclareerd,
-- blijven bestaan tussen frames, wat nuttig is voor filters,
-- accumulatoren en andere stateful transformaties, net als een
-- frame-parserscript:
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
-- Gebruik het Sjabloon-vervolgkeuzemenu voor kant-en-klare
-- voorbeelden, of klik op Test om uw functie uit te proberen.
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
 * Definieer een transform(value) functie die de live
 * dataset-uitlezing ontvangt en een getransformeerd getal
 * retourneert. Als er geen transform() functie is gedefinieerd,
 * wordt de ruwe waarde behouden en wordt er niets met het
 * project opgeslagen.
 *
 * Voorbeeld:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globale variabelen die op het hoogste niveau zijn gedeclareerd
 * blijven bestaan tussen frames, wat nuttig is voor filters,
 * accumulatoren en elke andere stateful transformatie -- net als
 * een frame-parserscript:
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
 * Gebruik het Template-vervolgkeuzemenu voor kant-en-klare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="757"/>
        <source>Select Template…</source>
        <translation>Selecteer Sjabloon…</translation>
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
 * Definieer een transform(value)-functie die de live
 * dataset-uitlezing ontvangt en een getransformeerd getal
 * retourneert. Als er geen transform()-functie is gedefinieerd,
 * wordt de ruwe waarde behouden en wordt er niets met het
 * project opgeslagen.
 *
 * Voorbeeld:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globale variabelen die op het hoogste niveau zijn gedeclareerd
 * blijven bestaan tussen frames, wat nuttig is voor filters,
 * accumulatoren en elke andere stateful transform — net als een
 * frame-parserscript:
 *
 *   var alpha = 0.1;
 *   var ema;
 *
 *
 *   function transform(value) {
 *     if (ema === undefined) ema = value;
 *     ema = alpha * value + (1 - alpha) * ema;
 *     return ema;
 *   }
 *</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="648"/>
        <source>Engine error</source>
        <translation>Engine-fout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="671"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="684"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="701"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="712"/>
        <source>Error: %1</source>
        <translation>Fout: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="677"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="705"/>
        <source>Error: transform() not defined</source>
        <translation>Fout: transform() niet gedefinieerd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="689"/>
        <source>Error: transform() must return a number</source>
        <translation>Fout: transform() moet een getal retourneren</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1622"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1736"/>
        <source>Channel %1</source>
        <translation>Kanaal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1747"/>
        <source>Audio Input</source>
        <translation>Audio-invoer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1631"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1752"/>
        <source>Quick Plot</source>
        <translation>Snelle Plot</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1348"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>JavaScript-transformatie overschreed het budget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1349"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>Een datasettransformatie duurde langer dan %1 ms; resterende datasets in de frame zijn teruggevallen op ruwe waarden tot de volgende frame. Profiler of vereenvoudig de transformatiecode.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="199"/>
        <source>Frame pool exhausted</source>
        <translation>Framepool uitgeput</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="201"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>Een downstream-consument (dashboard, CSV/MDF4-export, sessie-DB of API-abonnee) verwerkt frames niet snel genoeg. Serial Studio schakelt tijdelijk over op per-frame-allocaties tot de achterstand is weggewerkt. Schakel een zware consument uit of verlaag de gegevenssnelheid.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1583"/>
        <source>Device A</source>
        <translation>Apparaat A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1638"/>
        <source>Quick Plot Data</source>
        <translation>Snelle Plotgegevens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1650"/>
        <source>Multiple Plots</source>
        <translation>Meerdere Grafieken</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>Platte tekst (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimaal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>Binair (ruwe bytes)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>Alleen eindscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>Start- + eindscheidingstekens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>Alleen startscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Geen scheidingstekens (hele chunk)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>Geen Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>Selecteer Frame-parsersjabloon</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>Kies een sjabloon om te laden:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Ongeldige hexadecimale invoer.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>Geen sjabloon geselecteerd.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 frame(s) geëxtraheerd | %2 byte(s) verwerkt | %3 byte(s) gebufferd | %4 verwijderd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>Ongeldige JSON: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>Parameters</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">Geen</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Ongeldige Hex-invoer</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Voer geldige hexadecimale bytes in.

Geldig formaat: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(geen frames)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">Extractie heeft geen volledig frame opgeleverd. Controleer de start-/eindscheidingstekens en de detectiemodus.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 frame(s) geëxtraheerd | %2 byte(s) verwerkt | %3 byte(s) gebufferd | %4 verwijderd</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Pipelineconfiguratie</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Pipelineresultaten</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Detectie</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Decoder</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Checksum</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Startscheidingsteken</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">Eindscheidingsteken</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Hex-scheidingstekens</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Alleen eindscheidingsteken</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Start- + eindscheidingstekens</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Alleen startscheidingsteken</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Geen scheidingstekens (hele chunk)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Platte tekst (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexadecimaal</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binair (ruwe bytes)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Wissen</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Evalueren</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Voer hier ruwe stream-bytes in...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Klaarzetten</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Frame-gegevensinvoer</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Frame-parserresultaten</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Voer hier frame-gegevens in…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Dataset-index</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Waarde</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Voer framegegevens hierboven in, schakel HEX-modus in indien nodig en klik op "Evalueren" om de frameparser uit te voeren.

Voorbeeld (Tekst): a,b,c,d,e,f
Voorbeeld (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Frameparser Testen</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Voer hex-bytes in (bijv. 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(leeg)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Geen gegevens geretourneerd</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Change Scripting Language?</source>
        <translation>Scripttaal Wijzigen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="220"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Het wisselen van scripttaal vervangt de huidige parsercode door de equivalente sjabloon in de nieuwe taal.

Niet-opgeslagen wijzigingen gaan verloren. Doorgaan?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Javascript file to import</source>
        <translation>Selecteer Javascript-bestand om te importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Lua file to import</source>
        <translation>Selecteer Lua-bestand om te importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>Code Validation Successful</source>
        <translation>Codevalidatie Geslaagd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="415"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>Geen syntaxisfouten gedetecteerd in de parsercode.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Select Frame Parser Template</source>
        <translation>Selecteer Frame-parsersjabloon</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="526"/>
        <source>Choose a template to load:</source>
        <translation>Kies een sjabloon om te laden:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>Modbus-registerkaart Importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Modbus-Registerkaarten (*.CSV *.XML *.JSON);;CSV-Bestanden (*.CSV);;XML-Bestanden (*.XML);;JSON-Bestanden (*.JSON);;Alle Bestanden (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>Geen registers gevonden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Het bestand kon niet worden geparseerd of bevat geen registerdefinities.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>Modbus-import</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>Overzicht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>Aan</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Kan geïmporteerd project niet laden</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">De gegenereerde project-JSON kon niet worden geladen.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Aanmaken van tijdelijk projectbestand mislukt</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Controleer schrijfrechten voor de tijdelijke map.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>Succesvol %1 registers in %2 groepen geïmporteerd.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>De projecteditor is nu geopend voor aanpassing.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>Modbus-import Voltooid</translation>
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
        <translation type="vanished">Platte tekst (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexadecimaal</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binair (ruwe bytes)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Alleen eindscheidingsteken</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Start- + eindscheidingstekens</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Alleen startscheidingsteken</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Geen scheidingstekens (hele chunk)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">Geen Checksum</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">Ongeldige hexadecimale invoer.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">Geen sjabloon geselecteerd.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 frame(s) geëxtraheerd | %2 byte(s) verwerkt | %3 byte(s) gebufferd | %4 verwijderd</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">Parameters</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="288"/>
        <source>Select Javascript file to import</source>
        <translation>Selecteer Javascript-bestand om te importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>Selecteer Sjabloon voor Output Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>Kies een sjabloon om te laden:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>Selecteer Javascript-bestand om te importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>Selecteer Sjabloon voor Painter Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>Kies een sjabloon om te laden:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>Datasets toevoegen voor deze sjabloon?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" verwacht %2 dataset(s); de huidige groep heeft er %3.

Voeg %4 dataset(s) toe met de standaardwaarden van de sjabloon?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2187"/>
        <source>Project Information</source>
        <translation>Projectinformatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2197"/>
        <source>Project Title</source>
        <translation>Projecttitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2198"/>
        <source>Untitled Project</source>
        <translation>Naamloos Project</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2199"/>
        <source>Name or description of the project</source>
        <translation>Naam of beschrijving van het project</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2330"/>
        <source>Time</source>
        <translation>Tijd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2330"/>
        <source>Samples</source>
        <translation>Samples</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2342"/>
        <source>Plot every curve against time or against the sample number</source>
        <translation>Plot elke curve tegen tijd of tegen het samplenummer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2358"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2360"/>
        <source>Web address to load in this widget</source>
        <translation>Webadres om te laden in deze widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2468"/>
        <source>Frame Detection</source>
        <translation>Framedetectie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2482"/>
        <source>Frame Detection Method</source>
        <translation>Framedetectiemethode</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2483"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Selecteer hoe inkomende dataframes worden geïdentificeerd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2493"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Hexadecimale Scheidingstekens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2494"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Voer frame start/einde-sequenties in als hexadecimale waarden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2510"/>
        <source>Frame Start Delimiter</source>
        <translation>Frame Startscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2511"/>
        <source>e.g. /*</source>
        <translation>bijv. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2512"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Sequentie die het begin van een dataframe markeert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2524"/>
        <source>Frame End Delimiter</source>
        <translation>Frame-eindscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2525"/>
        <source>e.g. */</source>
        <translation>bijv. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2526"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Reeks die het einde van een dataframe markeert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2532"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Payload-verwerking &amp; Validatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2543"/>
        <source>Data Conversion Method</source>
        <translation>Dataconversiemethode</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2544"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Selecteer hoe binnenkomende binaire data wordt gedecodeerd vóór het parsen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2560"/>
        <source>Checksum Algorithm</source>
        <translation>Checksum-algoritme</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2561"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Selecteer het checksum-algoritme dat wordt gebruikt om frames te valideren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2217"/>
        <source>Group Information</source>
        <translation>Groepsinformatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2227"/>
        <source>Group Title</source>
        <translation>Groepstitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2228"/>
        <source>Untitled Group</source>
        <translation>Naamloze Groep</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2229"/>
        <source>Title or description of this dataset group</source>
        <translation>Titel of beschrijving van deze datasetgroep</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2400"/>
        <source>Composite Widget</source>
        <translation>Samengestelde Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2401"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Selecteer hoe deze groep datasets moet worden gevisualiseerd (optioneel)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2279"/>
        <source>Image Configuration</source>
        <translation>Afbeelding Configuratie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3140"/>
        <source>Virtual Dataset</source>
        <translation>Virtuele Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3141"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Virtuele datasets berekenen hun waarde uit transformaties en datatabellen, ze hebben geen frame-index nodig</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Fixed decimal places for the value display; overrides the format (-1 = auto)</source>
        <translation>Vaste decimalen voor de waarde-weergave; overschrijft het formaat (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3673"/>
        <source>Auto-detect</source>
        <translation>Automatisch Detecteren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3673"/>
        <source>Manual Delimiters</source>
        <translation>Handmatige Scheidingstekens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2292"/>
        <source>Detection Mode</source>
        <translation>Detectiemodus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1307"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1310"/>
        <source>Frame Parser</source>
        <translation>Frame-parser</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1449"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1450"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1492"/>
        <source>Groups</source>
        <translation>Groepen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1522"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1535"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1536"/>
        <source>Shared Memory</source>
        <translation>Gedeeld Geheugen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1522"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1541"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1542"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5059"/>
        <source>Dataset Values</source>
        <translation>Datasetwaarden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1584"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1597"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1598"/>
        <source>Workspaces</source>
        <translation>Werkruimten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1636"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1639"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1640"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Controlescript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1713"/>
        <source>Publishing</source>
        <translation>Publiceren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1723"/>
        <source>Enable Publishing</source>
        <translation>Publiceren Inschakelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1724"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Verzend frames, ruwe bytes en meldingen naar de broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1735"/>
        <source>Payload</source>
        <translation>Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1736"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Selecteert wat wordt gepubliceerd: geparseerde dashboardgegevens of ruwe RX-bytes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1746"/>
        <source>Publish Rate (Hz)</source>
        <translation>Publicatiesnelheid (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1747"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>Hoe vaak per seconde wordt gepubliceerd (1-30 Hz). Hogere snelheden verhogen de belasting van de broker; dashboardgegevens zijn snelheidsbegrensd zodat een trage broker het parsen van frames nooit blokkeert.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1759"/>
        <source>Topic Base</source>
        <translation>Topic Basis</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1760"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1761"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Basis-topic gebruikt voor frame- en raw-byte-publicatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1771"/>
        <source>Script Topic</source>
        <translation>Script-topic</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1772"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1796"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>Standaard naar Basis-Topic wanneer leeg</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1773"/>
        <source>Topic the user script publishes to</source>
        <translation>Topic waarnaar het gebruikersscript publiceert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1783"/>
        <source>Publish Notifications</source>
        <translation>Publiceer Meldingen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1784"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Spiegel dashboardmeldingen naar een toegewijd topic</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1795"/>
        <source>Notification Topic</source>
        <translation>Meldingtopic</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1797"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Topic waar dashboardmeldingen worden gespiegeld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1810"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1820"/>
        <source>Hostname</source>
        <translation>Hostnaam</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1821"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1822"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>Hostnaam of IP-adres van de MQTT-broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1831"/>
        <source>Port</source>
        <translation>Poort</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1832"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>TCP-poort aangeboden door de broker (1883 plain, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1842"/>
        <source>Custom Client ID</source>
        <translation>Aangepaste Client-ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1844"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Uit: een nieuwe willekeurige ID wordt gegenereerd bij elke projectlading. Aan: gebruik onderstaande ID.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1855"/>
        <source>Client ID</source>
        <translation>Client-ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1856"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Identificatie verzonden naar de broker bij CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1869"/>
        <source>Protocol Version</source>
        <translation>Protocolversie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1870"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>MQTT-protocolrevisie gebruikt bij CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1879"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1880"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Seconden tussen PINGREQ-pakketten bij inactiviteit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1889"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1890"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Verwijder alle persistente sessiegegevens bij CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1905"/>
        <source>Username</source>
        <translation>Gebruikersnaam</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1906"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Gebruikersnaam voor broker-authenticatie (leeg laten voor anoniem)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1916"/>
        <source>Password</source>
        <translation>Wachtwoord</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1917"/>
        <source>Password for broker authentication</source>
        <translation>Wachtwoord voor broker-authenticatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1928"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1938"/>
        <source>Use SSL/TLS</source>
        <translation>SSL/TLS Gebruiken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1939"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Tunnel de brokerverbinding over TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1952"/>
        <source>Protocol</source>
        <translation>Protocol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1953"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Onderhandeld TLS-protocolfamilie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1963"/>
        <source>Peer Verify</source>
        <translation>Peer Verifiëren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1964"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Hoe strikt de certificaatketen van de broker wordt gevalideerd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1974"/>
        <source>Verify Depth</source>
        <translation>Verificatiediepte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1975"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Maximale geaccepteerde certificaatketenlengte (0 = onbeperkt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2244"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2878"/>
        <source>Device %1</source>
        <translation>Apparaat %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2262"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2427"/>
        <source>Input Device</source>
        <translation>Invoerapparaat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2263"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Selecteer welk aangesloten apparaat gegevens levert voor deze groep</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2294"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Auto-detectie leest JPEG/PNG magic bytes; Handmatig gebruikt expliciete start-/eindreeksen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2304"/>
        <source>Start Sequence (Hex)</source>
        <translation>Startreeks (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2305"/>
        <source>e.g. FF D8 FF</source>
        <translation>bijv. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2306"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Hexadecimale bytes die het begin van een imageframe markeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2315"/>
        <source>End Sequence (Hex)</source>
        <translation>Eindreeks (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2316"/>
        <source>e.g. FF D9</source>
        <translation>bijv. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2317"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Hexadecimale bytes die het einde van een imageframe markeren</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">Identiteit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2437"/>
        <source>Device Name</source>
        <translation>Apparaatnaam</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2438"/>
        <source>Device 1</source>
        <translation>Apparaat 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2439"/>
        <source>Human-readable name for this input device</source>
        <translation>Leesbare naam voor dit invoerapparaat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2448"/>
        <source>Bus Type</source>
        <translation>Bustype</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2449"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Selecteer de hardware-interface voor dit invoerapparaat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>Serial Port</source>
        <translation>Seriële Poort</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>Network Socket</source>
        <translation>Netwerksocket</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>Audio Input</source>
        <translation>Audio-invoer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>CAN Bus</source>
        <translation>CAN-bus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2453"/>
        <source>Raw USB</source>
        <translation>Raw USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2454"/>
        <source>HID Device</source>
        <translation>HID-apparaat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2454"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2454"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT-abonnee</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2609"/>
        <source>Connection Settings</source>
        <translation>Verbindingsinstellingen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2846"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3116"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4755"/>
        <source>General Information</source>
        <translation>Algemene Informatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2855"/>
        <source>Action Title</source>
        <translation>Actietitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Untitled Action</source>
        <translation>Naamloze Actie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2858"/>
        <source>Name or description of this action</source>
        <translation>Naam of beschrijving van deze actie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2867"/>
        <source>Action Icon</source>
        <translation>Actiepictogram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2868"/>
        <source>Default Icon</source>
        <translation>Standaardpictogram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2869"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Pictogram dat voor deze actie in het dashboard wordt weergegeven</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2896"/>
        <source>Target Device</source>
        <translation>Doelapparaat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2897"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Selecteer naar welk verbonden apparaat deze actie gegevens verzendt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2909"/>
        <source>Data Payload</source>
        <translation>Gegevenslading</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2920"/>
        <source>Send as Binary</source>
        <translation>Verzenden als Binair</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2921"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Ruwe binaire gegevens verzenden wanneer deze actie wordt geactiveerd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2932"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2944"/>
        <source>Command</source>
        <translation>Commando</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2933"/>
        <source>Transmit Data (Hex)</source>
        <translation>Gegevens Verzenden (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2934"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Hexadecimale lading om te verzenden wanneer de actie wordt geactiveerd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2945"/>
        <source>Transmit Data</source>
        <translation>Gegevens Verzenden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2946"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Tekstlading om te verzenden wanneer de actie wordt geactiveerd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2957"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4814"/>
        <source>Text Encoding</source>
        <translation>Tekstcodering</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2958"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Tekencodering gebruikt om de tekstlading te serialiseren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2982"/>
        <source>End-of-Line Sequence</source>
        <translation>Regeleindigingsreeks</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2983"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>EOL-tekens om aan het bericht toe te voegen (bijv. </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2995"/>
        <source>Execution Behavior</source>
        <translation>Uitvoeringsgedrag</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3006"/>
        <source>Auto-Execute on Connect</source>
        <translation>Automatisch Uitvoeren bij Verbinding</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3007"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Deze actie automatisch activeren wanneer het apparaat verbinding maakt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3013"/>
        <source>Timer Behavior</source>
        <translation>Timergedrag</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3022"/>
        <source>Timer Mode</source>
        <translation>Timermodus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3025"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Kies wanneer en hoe deze actie automatisch moet herhalen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3032"/>
        <source>Interval (ms)</source>
        <translation>Interval (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3036"/>
        <source>Timer Interval (ms)</source>
        <translation>Timerinterval (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3037"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Milliseconden tussen elke herhaalde trigger van deze actie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3044"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3048"/>
        <source>Repeat Count</source>
        <translation>Herhalingsaantal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3049"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Aantal keren dat het commando bij elke trigger wordt verzonden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3126"/>
        <source>Untitled Dataset</source>
        <translation>Dataset Zonder Titel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3127"/>
        <source>Dataset Title</source>
        <translation>Dataset Titel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3128"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Naam van de dataset, gebruikt voor labeling en identificatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3157"/>
        <source>Hide on Dashboard</source>
        <translation>Verbergen op Dashboard</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3158"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Onderdruk de standalone dashboardtegel van deze dataset; de painter widget kan de waarden nog steeds uitlezen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3204"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Ondergrens van het waardebereik van de dataset; widgets en FFT vallen hierop terug wanneer hun eigen bereik niet is ingesteld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3217"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Bovengrens van het waardebereik van de dataset; widgets en FFT vallen hierop terug wanneer hun eigen bereik niet is ingesteld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3275"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Kies Tijd of een dataset om de X-as in grafieken aan te sturen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3288"/>
        <source>Frequency Analysis</source>
        <translation>Frequentieanalyse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3335"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Kies Tijd (standaard) of een dataset waarvan de waarde de Y-as aanstuurt -- produceert een Campbell-diagram wanneer gekoppeld aan bijv. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3385"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3474"/>
        <source>Minimum Value (optional)</source>
        <translation>Minimumwaarde (optioneel)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3386"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Ondergrens voor datanormalisatie; valt terug op het waardebereik van de dataset wanneer niet ingesteld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3398"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3487"/>
        <source>Maximum Value (optional)</source>
        <translation>Maximumwaarde (optioneel)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3399"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Bovengrens voor datanormalisatie; valt terug op het waardebereik van de dataset wanneer niet ingesteld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3475"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Ondergrens van het meter- of staafbereik; valt terug op het waardebereik van de dataset wanneer niet ingesteld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3488"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Bovengrens van het meter- of staafbereik; valt terug op het waardebereik van de dataset wanneer niet ingesteld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3540"/>
        <source>Decimal Points</source>
        <translation>Decimale Punten</translation>
    </message>
    <message>
        <source>Decimal places shown in the data grid value column (-1 = auto)</source>
        <translation type="vanished">Decimalen weergegeven in de waardekolom van het gegevensraster (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3587"/>
        <source>On</source>
        <translation>Aan</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3631"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>LED licht op wanneer waarde deze drempel bereikt of overschrijdt; definieer alarmbereiken voor meertoestandskleuren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3688"/>
        <source>Painter Widget</source>
        <translation>Painter Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3689"/>
        <source>Web View</source>
        <translation>Webweergave</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5060"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Ruwe en getransformeerde waarden voor elke dataset (alleen-lezen)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5069"/>
        <source>Shared table defined in this project</source>
        <translation>Gedeelde tabel gedefinieerd in dit project</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5428"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>1 widgetreferentie verwijderen waarvan de doelgroep of dataset niet meer bestaat?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5429"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>%1 widgetreferenties verwijderen waarvan de doelgroepen of datasets niet meer bestaan?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5434"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Dit heeft alleen invloed op de plaatsing van werkruimtetegels; geen groepen, datasets of gegevens worden verwijderd.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5437"/>
        <source>Clean Up Workspaces</source>
        <translation>Werkruimten Opschonen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3171"/>
        <source>Frame Index</source>
        <translation>Frame Index</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1660"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1663"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1664"/>
        <source>Control Loop</source>
        <translation>Besturingslus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3172"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Frame-positie gebruikt voor het uitlijnen van datasets in tijd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3181"/>
        <source>Measurement Unit</source>
        <translation>Meeteenheid</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3182"/>
        <source>Volts, Amps, etc.</source>
        <translation>Volt, Ampère, enz.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3183"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Meeteenheid, zoals volt of ampère (optioneel)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3230"/>
        <source>Plot Settings</source>
        <translation>Plotinstellingen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3253"/>
        <source>Enable Plot Widget</source>
        <translation>Plotwidget Inschakelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3255"/>
        <source>Plot data in real-time</source>
        <translation>Gegevens in realtime plotten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2341"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3274"/>
        <source>X-Axis Source</source>
        <translation>X-as Bron</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Kies welke dataset te gebruiken voor de X-as in plots</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Minimale Plotwaarde (optioneel)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Ondergrens voor weergavebereik van plot</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Maximale Plotwaarde (optioneel)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Bovengrens voor weergavebereik van plot</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">FFT Configuratie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3299"/>
        <source>Enable FFT Analysis</source>
        <translation>FFT Analyse Inschakelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3300"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Voer frequentiedomeinanalyse uit op de dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3310"/>
        <source>Enable Waterfall Plot</source>
        <translation>Watervalgrafiek Inschakelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3311"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Toon een scrollend spectrogram van frequentie-inhoud over tijd (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3334"/>
        <source>Waterfall Y Axis</source>
        <translation>Waterval Y-as</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="obsolete">Kies Tijd (standaard) of een dataset waarvan de waarde de Y-as aanstuurt — produceert een Campbell-diagram wanneer gekoppeld aan bijv. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3362"/>
        <source>FFT Window Size</source>
        <translation>FFT Venstergrootte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3363"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Aantal samples gebruikt voor elk FFT berekeningsvenster</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3374"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>FFT Bemonsteringsfrequentie (Hz, vereist)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3375"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Bemonsteringsfrequentie gebruikt voor FFT (in Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Minimumwaarde (aanbevolen)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Ondergrens voor datanormalisatie</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Maximumwaarde (aanbevolen)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Bovengrens voor datanormalisatie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3424"/>
        <source>Widget Settings</source>
        <translation>Widgetinstellingen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3447"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3448"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Selecteer de visuele widget die wordt gebruikt om deze dataset weer te geven</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Minimale Weergavewaarde (vereist)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Ondergrens van het weergavebereik van de meter of balk</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Maximale Weergavewaarde (vereist)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Bovengrens van het weergavebereik van de meter of balk</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3504"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3539"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3700"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3505"/>
        <source>Tick Count</source>
        <translation>Aantal Tikken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3509"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Aantal grote tikken op de wijzerschaal (0 = automatisch aanpassen aan widgetgrootte)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3528"/>
        <source>Label Format</source>
        <translation>Label Formatteren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3529"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Decimalen of notatie gebruikt op tiklabels en de waarde-weergave</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Waarde-indicator Tonen</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Schakel de omkaderde numerieke weergave in die onder of naast de widget staat</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Alarminstellingen</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Alarmen Inschakelen</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Activeert een visueel alarm wanneer de waarde de alarmdrempels overschrijdt</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Onderste Drempel</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Activeert een visueel alarm wanneer de waarde onder deze drempel daalt</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Bovenste Drempel</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Activeert een visueel alarm wanneer de waarde deze drempel overschrijdt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>LED Display Settings</source>
        <translation>LED-weergave-instellingen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3615"/>
        <source>Show in LED Panel</source>
        <translation>Tonen in LED-Paneel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Schakel visuele statusbewaking in met een LED-weergave</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3630"/>
        <source>LED On Threshold (required)</source>
        <translation>LED Aan-drempel (vereist)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">LED licht op wanneer waarde deze drempel bereikt of overschrijdt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Off</source>
        <translation>Uit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Auto Start</source>
        <translation>Automatisch Starten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Start on Trigger</source>
        <translation>Starten bij Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Toggle on Trigger</source>
        <translation>Schakelen bij Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3652"/>
        <source>Repeat N Times</source>
        <translation>N Keer Herhalen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3655"/>
        <source>Plain Text (UTF8)</source>
        <translation>Platte Tekst (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3655"/>
        <source>Hexadecimal</source>
        <translation>Hexadecimaal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3655"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3656"/>
        <source>Binary (Direct)</source>
        <translation>Binair (Direct)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3661"/>
        <source>No Checksum</source>
        <translation>Geen Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3665"/>
        <source>End Delimiter Only</source>
        <translation>Alleen Eindscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3665"/>
        <source>Start Delimiter Only</source>
        <translation>Alleen Startscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3666"/>
        <source>Start + End Delimiter</source>
        <translation>Start- + Eindscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3666"/>
        <source>No Delimiters</source>
        <translation>Geen Scheidingstekens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Button</source>
        <translation>Knop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Slider</source>
        <translation>Schuifregelaar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Toggle</source>
        <translation>Schakelaar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3676"/>
        <source>Text Field</source>
        <translation>Tekstveld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3677"/>
        <source>Knob</source>
        <translation>Draaiknop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3681"/>
        <source>Data Grid</source>
        <translation>Gegevensraster</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3682"/>
        <source>GPS Map</source>
        <translation>GPS-kaart</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3683"/>
        <source>Gyroscope</source>
        <translation>Gyroscoop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3684"/>
        <source>Multiple Plot</source>
        <translation>Meervoudige Plot</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3685"/>
        <source>Accelerometer</source>
        <translation>Accelerometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3686"/>
        <source>3D Plot</source>
        <translation>3D-plot</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3687"/>
        <source>Image View</source>
        <translation>Afbeeldingsweergave</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3690"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3693"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3708"/>
        <source>None</source>
        <translation>Geen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3694"/>
        <source>Bar</source>
        <translation>Balk</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3695"/>
        <source>Gauge</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3696"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3697"/>
        <source>Meter</source>
        <translation>Meter</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3701"/>
        <source>Integer (0 decimals)</source>
        <translation>Integer (0 decimalen)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3702"/>
        <source>1 decimal</source>
        <translation>1 decimaal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3703"/>
        <source>2 decimals</source>
        <translation>2 decimalen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3704"/>
        <source>3 decimals</source>
        <translation>3 decimalen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3705"/>
        <source>Scientific</source>
        <translation>Wetenschappelijk</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3709"/>
        <source>New Line (\n)</source>
        <translation>Nieuwe Regel (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3710"/>
        <source>Carriage Return (\r)</source>
        <translation>Carriage Return (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3711"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3714"/>
        <source>No</source>
        <translation>Nee</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3715"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4765"/>
        <source>Label</source>
        <translation>Label</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4766"/>
        <source>Display label</source>
        <translation>Weergavelabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4776"/>
        <source>Button Icon</source>
        <translation>Knoppictogram</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4785"/>
        <source>Colorize Icon</source>
        <translation>Pictogram Inkleuren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4786"/>
        <source>Tint the icon with the button color</source>
        <translation>Kleur het pictogram met de knopkleur</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4803"/>
        <source>Initial Value</source>
        <translation>Beginwaarde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4815"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Tekencodering gebruikt wanneer transmit() een tekenreekswaarde retourneert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4833"/>
        <source>Value Range</source>
        <translation>Waardebereik</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3203"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4843"/>
        <source>Minimum Value</source>
        <translation>Minimumwaarde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3216"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4852"/>
        <source>Maximum Value</source>
        <translation>Maximumwaarde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4861"/>
        <source>Step Size</source>
        <translation>Stapgrootte</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="539"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="548"/>
        <source>Lock Project</source>
        <translation>Project Vergrendelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="540"/>
        <source>Choose a password to lock the project:</source>
        <translation>Kies een wachtwoord om het project te vergrendelen:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="548"/>
        <source>Confirm the password:</source>
        <translation>Bevestig het wachtwoord:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="553"/>
        <source>Passwords do not match</source>
        <translation>Wachtwoorden komen niet overeen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="554"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>De twee ingevoerde wachtwoorden komen niet overeen. Het project is niet vergrendeld.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="588"/>
        <source>Unlock Project</source>
        <translation>Project Ontgrendelen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="589"/>
        <source>Enter the project password:</source>
        <translation>Voer het projectwachtwoord in:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="599"/>
        <source>Incorrect password</source>
        <translation>Onjuist wachtwoord</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="600"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>Het ingevoerde wachtwoord komt niet overeen met het wachtwoord in het projectbestand.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="631"/>
        <source>New Project</source>
        <translation>Nieuw Project</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="831"/>
        <source>Samples</source>
        <translation>Samples</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1340"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Meerdere gegevensbronnen vereisen een Pro-licentie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1341"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro maakt het mogelijk om gelijktijdig verbinding te maken met meerdere apparaten. Upgrade om deze functie te ontgrendelen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1353"/>
        <source>Device %1</source>
        <translation>Apparaat %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Kopie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1623"/>
        <source>Do you want to save your changes?</source>
        <translation>Wijzigingen opslaan?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1624"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Dit project bevat niet-opgeslagen wijzigingen!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="411"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="420"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="433"/>
        <source>Project error</source>
        <translation>Projectfout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="411"/>
        <source>Project title cannot be empty!</source>
        <translation>Projecttitel mag niet leeg zijn!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="420"/>
        <source>You need to add at least one group!</source>
        <translation>Voeg minimaal één groep toe!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="433"/>
        <source>You need to add at least one dataset!</source>
        <translation>Voeg minimaal één dataset toe!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="480"/>
        <source>Your project needs a title</source>
        <translation>Je project heeft een titel nodig</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="482"/>
        <source>Add a group to get started</source>
        <translation>Voeg een groep toe om te beginnen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="484"/>
        <source>Add a dataset to a group</source>
        <translation>Voeg een dataset toe aan een groep</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="498"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Open de Projectweergave bovenaan de boom en voer een naam in. Je kunt het project op elk moment hernoemen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="501"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>Groepen organiseren datasets in dashboardwidgets. Gebruik de Groep-knop in de werkbalk hierboven om er een te maken en voeg er vervolgens datasets aan toe.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="505"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>Datasets zijn de waarden die op het dashboard verschijnen. Selecteer een groep in de boom en gebruik de Dataset-knop in de werkbalk om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="830"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="879"/>
        <source>Time</source>
        <translation>Tijd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1385"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Wil je gegevensbron "%1" verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1386"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>Groepen die deze bron gebruiken worden verplaatst naar de standaardbron. Deze actie kan niet ongedaan worden gemaakt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1663"/>
        <source>Save Serial Studio Project</source>
        <translation>Serial Studio-project Opslaan</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1665"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2287"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Serial Studio-projectbestanden (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1686"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1905"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2278"/>
        <source>Untitled Project</source>
        <translation>Naamloos Project</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1917"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2433"/>
        <source>Device A</source>
        <translation>Apparaat A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2116"/>
        <source>Select Project File</source>
        <translation>Selecteer Projectbestand</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2118"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>Projectbestanden (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2162"/>
        <source>JSON validation error</source>
        <translation>JSON-validatiefout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2252"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Project geüpgraded vanaf een ouder bestandsformaat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2253"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Dit project is opgeslagen met schema-versie %1; de huidige versie is %2. Standaardwaarden zijn toegepast op nieuwe velden. Sla het project op om de upgrade vast te leggen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2285"/>
        <source>Save Imported Project</source>
        <translation>Geïmporteerd Project Opslaan</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2477"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>Multi-source-projecten vereisen een Pro-licentie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2478"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Dit project bevat meerdere gegevensbronnen. Alleen de eerste bron is geladen. Een Serial Studio Pro-licentie is vereist om multi-source-projecten te gebruiken.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2711"/>
        <source>Workspace IDs remapped on load</source>
        <translation>Workspace-ID's opnieuw toegewezen bij het laden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2712"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 aangepaste werkruimte-ID's overlapten met het nieuwe gereserveerde automatische bereik en zijn verplaatst naar het gebruikersbereik. Sla het project op om de herindeling permanent te maken.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2854"/>
        <source>Legacy frame parser function updated</source>
        <translation>Verouderde frame-parserfunctie bijgewerkt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2855"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Je project gebruikte een verouderde frame-parserfunctie met een 'separator'-argument. Deze is automatisch gemigreerd naar het nieuwe formaat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3053"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Wil je groep "%1" verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3054"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3099"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3131"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3869"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Deze actie kan niet ongedaan worden gemaakt. Doorgaan?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3098"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Wil je actie "%1" verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3130"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Wil je dataset "%1" verwijderen?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Kopie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3781"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3817"/>
        <source>Output Controls</source>
        <translation>Uitvoerbesturing</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3829"/>
        <source>New Button</source>
        <translation>Nieuwe Knop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3832"/>
        <source>New Slider</source>
        <translation>Nieuwe Schuifregelaar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3835"/>
        <source>New Toggle</source>
        <translation>Nieuwe Schakelaar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3838"/>
        <source>New Text Field</source>
        <translation>Nieuw Tekstveld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3841"/>
        <source>New Knob</source>
        <translation>Nieuwe Knop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3868"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Wilt u outputwidget "%1" verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4033"/>
        <source>Group</source>
        <translation>Groep</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4051"/>
        <source>New Dataset</source>
        <translation>Nieuwe Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4054"/>
        <source>New Plot</source>
        <translation>Nieuwe Plot</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4058"/>
        <source>New FFT Plot</source>
        <translation>Nieuwe FFT-plot</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4062"/>
        <source>New Level Indicator</source>
        <translation>Nieuwe Niveauindicator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4066"/>
        <source>New Gauge</source>
        <translation>Nieuwe Meter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4070"/>
        <source>New Compass</source>
        <translation>Nieuw Kompas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4076"/>
        <source>New Meter</source>
        <translation>Nieuwe Meter</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Nieuwe Thermometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4080"/>
        <source>New LED Indicator</source>
        <translation>Nieuwe LED-indicator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4084"/>
        <source>New Waterfall</source>
        <translation>Nieuwe Waterval</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4152"/>
        <source>Channel %1</source>
        <translation>Kanaal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4223"/>
        <source>New Action</source>
        <translation>Nieuwe Actie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4361"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Weet u zeker dat u de widget op groepsniveau wilt wijzigen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4363"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>Bestaande datasets voor deze groep worden verwijderd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4431"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4432"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4433"/>
        <source>Accelerometer %1</source>
        <translation>Accelerometer %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4448"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4448"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4448"/>
        <source>Gyro %1</source>
        <translation>GYRO %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4463"/>
        <source>Latitude</source>
        <translation>Breedtegraad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4463"/>
        <source>Longitude</source>
        <translation>Lengtegraad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4463"/>
        <source>Altitude</source>
        <translation>Hoogte</translation>
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
        <translation>Werkruimte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4915"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5122"/>
        <source>Shared Table</source>
        <translation>Gedeelde Tabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4993"/>
        <source>register</source>
        <translation>register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5122"/>
        <source>New Shared Table</source>
        <translation>Nieuwe Gedeelde Tabel</translation>
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
        <translation>Naam:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5139"/>
        <source>Rename Table</source>
        <translation>Tabel Hernoemen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5158"/>
        <source>Rename Group</source>
        <translation>Groep Hernoemen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5182"/>
        <source>Rename Dataset</source>
        <translation>Dataset Hernoemen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5209"/>
        <source>Rename Data Source</source>
        <translation>Gegevensbron Hernoemen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5228"/>
        <source>Rename Action</source>
        <translation>Actie Hernoemen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5249"/>
        <source>New Register</source>
        <translation>Nieuw Register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5272"/>
        <source>Rename Register</source>
        <translation>Register Hernoemen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5309"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5334"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6155"/>
        <source>This action cannot be undone.</source>
        <translation>Deze actie kan niet ongedaan worden gemaakt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5310"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Dit verwijdert %1 register(s) samen met de tabel. Deze actie kan niet ongedaan worden gemaakt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5313"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5333"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6154"/>
        <source>Delete "%1"?</source>
        <translation>"%1" Verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5316"/>
        <source>Delete Table</source>
        <translation>Tabel Verwijderen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5336"/>
        <source>Delete Register</source>
        <translation>Register Verwijderen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5359"/>
        <source>Export Table</source>
        <translation>Tabel Exporteren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5361"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5403"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-bestanden (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5401"/>
        <source>Import Table</source>
        <translation>Tabel Importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5632"/>
        <source>New Workspace</source>
        <translation>Nieuwe Werkruimte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5653"/>
        <source>Rename Workspace</source>
        <translation>Werkruimte Hernoemen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5736"/>
        <source>Overview</source>
        <translation>Overzicht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5745"/>
        <source>All Data</source>
        <translation>Alle Gegevens</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5923"/>
        <source>Discard workspace customisations?</source>
        <translation>Werkruimte-aanpassingen verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5924"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Aanpassen uitschakelen verwijdert uw bewerkingen en herbouwt de werkruimtelijst vanuit de groepen van het project.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5927"/>
        <source>Customize Workspaces</source>
        <translation>Werkruimten Aanpassen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6157"/>
        <source>Delete Workspace</source>
        <translation>Werkruimte Verwijderen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6538"/>
        <source>Project file removed from disk</source>
        <translation>Projectbestand verwijderd van schijf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6539"/>
        <source>%1 was deleted or renamed by another program. Save the project to recreate it.</source>
        <translation>%1 is verwijderd of hernoemd door een ander programma. Sla het project op om het opnieuw aan te maken.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6560"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6581"/>
        <source>Project file changed on disk</source>
        <translation>Projectbestand gewijzigd op schijf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6561"/>
        <source>%1 was modified by another program. The in-memory project was kept; reopen the file to load the external changes.</source>
        <translation>%1 is gewijzigd door een ander programma. Het project in het geheugen is behouden; heropen het bestand om de externe wijzigingen te laden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6577"/>
        <source>The project file was modified by another program.

Reload it and discard your unsaved changes?</source>
        <translation>Het projectbestand is gewijzigd door een ander programma.

Opnieuw laden en niet-opgeslagen wijzigingen verwerpen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6579"/>
        <source>The project file was modified by another program.

Reload it?</source>
        <translation>Het projectbestand is gewijzigd door een ander programma.

Opnieuw laden?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6610"/>
        <source>File save error</source>
        <translation>Fout bij opslaan bestand</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2321"/>
        <source>File open error</source>
        <translation>Fout bij openen bestand</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Protocol Buffers-bestand Importeren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Proto-Bestanden (*.proto);;Alle Bestanden (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Kan proto-bestand niet openen: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Controleer het bestandspad en leesrechten en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Protobuf-importfout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Proto-bestand is te groot (de limiet is 10 MB).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>Controleer of u het juiste .proto-definitiebestand hebt geselecteerd.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Parseren van proto-bestand mislukt op regel %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>Alleen proto3-syntaxis wordt ondersteund. Controleer het bestandsformaat en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Proto-bestand bevat geen berichtdefinities</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Het geselecteerde bestand heeft geen `message`-blokken om te importeren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Protobuf-importwaarschuwing</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Laden van geïmporteerd project mislukt</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">De gegenereerde project-JSON kon niet worden geladen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>%1 bericht(en) en %2 veld(en) succesvol geïmporteerd uit het proto-bestand.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>De projecteditor is nu geopend voor aanpassing.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Protobuf-import Voltooid</translation>
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
        <translation>Ongeldige Hex-invoer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Voer geldige hexadecimale bytes in.

Geldig formaat: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Geen verzendfunctiecode om te evalueren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>verzendfunctie is niet aanroepbaar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>Evalueren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>Invoerwaarde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>Uitvoer Verzendfunctie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>Voer waarde in om te verzenden…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>Onbewerkte tekenreeksuitvoer verschijnt hier</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>Hexadecimale byte-uitvoer verschijnt hier</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>Test Verzendfunctie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Voer hexadecimale bytes in (bijv. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(leeg) Geen gegevens geretourneerd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 bytes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 byte(s)</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>Gedeeld Geheugen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>Gedeelde Tabel Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>Gedeelde tabel toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>Helpdocumentatie voor gedeeld geheugen openen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>Beschrijving</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>Vermeldingen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="274"/>
        <source>No shared tables.</source>
        <translation>Geen gedeelde tabellen.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Sessies</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Openen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Open een sessiebestand</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Sluit sessiebestand</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Afspelen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Speel geselecteerde sessie af op het dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Ontgrendel het sessiebestand om sessies te verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Verwijder de geselecteerde sessie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Ontgrendelen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Vergrendelen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Ontgrendel het sessiebestand om verwijderingen toe te staan</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Stel een wachtwoord in om sessieverwijderingen te voorkomen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>Exporteer CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Exporteer geselecteerde sessie naar CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>Exporteer PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>Genereer een PDF-rapport voor de geselecteerde sessie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Project Herstellen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Herstel het projectbestand uit dit sessiebestand</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Sessie laden…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>Bezig…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-functies gedetecteerd in dit project.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Fallback-widgets worden gebruikt. Koop een licentie om volledige functionaliteit te ontgrendelen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>Grafieken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>Schakel 2D-plotvisualisatie voor deze dataset in of uit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>FFT-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>Schakel FFT-grafiek in om frequentie-inhoud te visualiseren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>Waterval</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Waterval (spectrogram) plot in-/uitschakelen — gebruikt de FFT-instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>Balk/niveau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Schakel balk-/niveau-indicator in voor deze dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Schakel meterwidget in voor analoge weergave</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Schakel kompaswidget in voor richtingsgegevens</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Schakel analoge meterwidget (halve boog) in</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="305"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>Definieer gekleurde waardebereiken met ernstniveaus voor de meter of LED van deze dataset.</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Schakel thermometerwidget in voor temperatuurgegevens</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>Schakel LED-indicator in voor binaire of drempelwaarden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>Gedrag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>Alarmbanden</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">Definieer gekleurde waardebereiken met ernstniveaus voor de meter van deze dataset.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="311"/>
        <source>Transform</source>
        <translation>Transformatie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="315"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Bewerk een waardetransformatie-expressie voor kalibratie, filtering of eenheidsconversie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="328"/>
        <source>Duplicate</source>
        <translation>Dupliceren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="333"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Dupliceer deze dataset met dezelfde configuratie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="338"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete this dataset from the group</source>
        <translation>Verwijder deze dataset uit de groep</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>Ondersteun Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>Ondersteun de ontwikkeling van %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio is gratis &amp; open-source software ondersteund door vrijwilligers. Overweeg te doneren of een Pro-licentie aan te schaffen om de ontwikkelingsinspanningen te ondersteunen :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Je kunt dit project ook ondersteunen door het te delen, bugs te melden en nieuwe functies voor te stellen!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>Doneren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>Serial Studio Pro Aanschaffen</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="142"/>
        <source>Stop</source>
        <translation>Stop</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="143"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="377"/>
        <source>Downloading updates</source>
        <translation>Updates downloaden</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="470"/>
        <source>Time remaining</source>
        <translation>Resterende tijd</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <source>unknown</source>
        <translation>onbekend</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Cannot find downloaded update!</source>
        <translation>Kan gedownloade update niet vinden!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="260"/>
        <source>Download complete!</source>
        <translation>Download voltooid!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="268"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Klik op "OK" om de installatie van de update te starten</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="270"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>Om de update te installeren moet u mogelijk de applicatie afsluiten.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="261"/>
        <source>The installer opens separately</source>
        <translation>Het installatieprogramma wordt apart geopend</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="274"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>Om de update te installeren moet u mogelijk de applicatie afsluiten. Dit is een verplichte update; nu afsluiten sluit de applicatie.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="290"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Klik op de knop "Openen" om de update toe te passen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="303"/>
        <source>Updater</source>
        <translation>Updater</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="307"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Weet u zeker dat u de download wilt annuleren?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="309"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Weet u zeker dat u de download wilt annuleren? Dit is een verplichte update; nu afsluiten sluit de applicatie</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="364"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="371"/>
        <source>%1 bytes</source>
        <translation>%1 bytes</translation>
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
        <translation>van</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="421"/>
        <source>Downloading Updates</source>
        <translation>Updates Downloaden</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Time Remaining</source>
        <translation>Resterende Tijd</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Unknown</source>
        <translation>Onbekend</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="446"/>
        <source>about %1 hours</source>
        <translation>ongeveer %1 uur</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="448"/>
        <source>about one hour</source>
        <translation>ongeveer een uur</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="456"/>
        <source>%1 minutes</source>
        <translation>%1 minuten</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="458"/>
        <source>1 minute</source>
        <translation>1 minuut</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="465"/>
        <source>%1 seconds</source>
        <translation>%1 seconden</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="467"/>
        <source>1 second</source>
        <translation>1 seconde</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Resterende tijd: 0 minuten</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Openen</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>Voorbeeldbrowser</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>Terug</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Downloaden &amp;&amp; Openen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>Bekijken op GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>Zoeken in Voorbeelden…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="245"/>
        <source>Fetching examples…</source>
        <translation>Voorbeelden ophalen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>Loading...</source>
        <translation>Laden...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="569"/>
        <source>No README available.</source>
        <translation>Geen README beschikbaar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="609"/>
        <source>Copied to Clipboard</source>
        <translation>Gekopieerd naar Klembord</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="672"/>
        <source>No screenshot available</source>
        <translation>Geen schermafbeelding beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="704"/>
        <source>Details</source>
        <translation>Details</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="733"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="756"/>
        <source>Category:</source>
        <translation>Categorie:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="769"/>
        <source>Difficulty:</source>
        <translation>Moeilijkheidsgraad:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="787"/>
        <source>Project:</source>
        <translation>Project:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="829"/>
        <source>No Results Found</source>
        <translation>Geen Resultaten Gevonden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="840"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Controleer de spelling of probeer een andere zoekterm.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="855"/>
        <source>%1 examples</source>
        <translation>%1 voorbeelden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="864"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>Extensiebeheer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>Vernieuwen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>Repo's</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>Repository-instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>Terug</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>Installeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>Uitvoeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>Stoppen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>Extensies zoeken…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>Extensies ophalen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>Actief</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>Bijwerken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>Geïnstalleerd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>Niet Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>Geen beschrijving beschikbaar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>Details</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>Type:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>Auteur:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>Versie:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>Licentie:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>Geen voorbeeld</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>PLUGIN-UITVOER</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Nog geen uitvoer. Voer de plugin uit om het logboek hier te zien.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>Geen voorbeeld beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>Repositories</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>Voeg URL's toe naar externe repositories of lokale mappaden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>LOKAAL</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL of lokaal pad…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>Bladeren…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>Geen Resultaten Gevonden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Controleer de spelling of probeer een andere zoekterm.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>Geen Extensies Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Voeg een repository-URL of lokaal pad toe in de Repos-instellingen en vernieuw daarna.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 extensies</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="167"/>
        <source>Interpolation: %1</source>
        <translation>Interpolatie: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="195"/>
        <source>Show Area Under Plot</source>
        <translation>Toon Gebied Onder Plot</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="213"/>
        <source>Show X Axis Label</source>
        <translation>Toon X-aslabel</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="225"/>
        <source>Show Y Axis Label</source>
        <translation>Toon Y-aslabel</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="243"/>
        <source>Show Crosshair</source>
        <translation>Toon Kruisdraad</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Resume</source>
        <translation>Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="269"/>
        <source>Reset View</source>
        <translation>Weergave Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="275"/>
        <source>Axis Range Settings</source>
        <translation>Asbereik Instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="304"/>
        <source>Magnitude (dB)</source>
        <translation>Magnitude (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="305"/>
        <source>Frequency (Hz)</source>
        <translation>Frequentie (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Projecten en CSV-bestanden hier neerzetten</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>Bestandsoverdracht</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>Overdrachtsprotocol:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>Bestandsselectie:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>Selecteer Bestand…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>Transmissie-interval:</translation>
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
        <translation>Blokgrootte:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>bytes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>Time-out:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>Max. Pogingen:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>Voortgang: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 bytes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>Fouten: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>Activiteitenlogboek</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>Transmissie Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>Transmissie Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>Transmissie Stoppen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>Transmissie Starten</translation>
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
        <translation>Apparaat %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="399"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1446"/>
        <source>Output Panel</source>
        <translation>Uitvoerpaneel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="443"/>
        <source>Control</source>
        <translation>Besturing</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="466"/>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="482"/>
        <source>%1 regs</source>
        <translation>%1 regs</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="483"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="505"/>
        <source>empty</source>
        <translation>leeg</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Controlescript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="528"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="931"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Open de transformcode-editor voor deze dataset.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1216"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1219"/>
        <source>Dataset Container</source>
        <translation>Datasetcontainer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1228"/>
        <source>Multi-Plot</source>
        <translation>Meervoudige Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1231"/>
        <source>Multiple Plot</source>
        <translation>Meervoudige Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1240"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <source>Accelerometer</source>
        <translation>Accelerometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1252"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1255"/>
        <source>Gyroscope</source>
        <translation>Gyroscoop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1264"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1267"/>
        <source>GPS Map</source>
        <translation>GPS-kaart</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1275"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <source>3D Plot</source>
        <translation>3D-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1286"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1289"/>
        <source>Image View</source>
        <translation>Afbeeldingsweergave</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1298"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1301"/>
        <source>Painter Widget</source>
        <translation>Painter Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1310"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1313"/>
        <source>Web View</source>
        <translation>Webweergave</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1322"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1325"/>
        <source>Data Grid</source>
        <translation>Gegevensraster</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1338"/>
        <source>Generic</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1351"/>
        <source>Plot</source>
        <translation>Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1364"/>
        <source>FFT Plot</source>
        <translation>FFT-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1377"/>
        <source>Gauge</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1390"/>
        <source>Level Indicator</source>
        <translation>Niveauindicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1403"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1416"/>
        <source>Meter</source>
        <translation>Meter</translation>
    </message>
    <message>
        <source>Edit Control Script…</source>
        <translation type="vanished">Controlescript Bewerken…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="503"/>
        <source>Control Loop</source>
        <translation>Besturingslus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1429"/>
        <source>LED Indicator</source>
        <translation>LED-indicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1458"/>
        <source>Slider</source>
        <translation>Schuifregelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1471"/>
        <source>Toggle</source>
        <translation>Schakelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1484"/>
        <source>Knob</source>
        <translation>Draaiknop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1497"/>
        <source>Text Field</source>
        <translation>Tekstveld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1510"/>
        <source>Button</source>
        <translation>Knop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1534"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1610"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1698"/>
        <source>Add Group</source>
        <translation>Groep Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1550"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1714"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1759"/>
        <source>Add Dataset</source>
        <translation>Dataset Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1564"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1640"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1728"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1773"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1980"/>
        <source>Add Output</source>
        <translation>Uitvoer Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1653"/>
        <source>Add Action</source>
        <translation>Actie Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1589"/>
        <source>Add Data Source</source>
        <translation>Gegevensbron Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1596"/>
        <source>Add Data Table</source>
        <translation>Gegevenstabel Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1664"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1800"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1867"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1995"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2029"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2085"/>
        <source>Rename…</source>
        <translation>Hernoemen…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1672"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1830"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1900"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1952"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2003"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2059"/>
        <source>Duplicate</source>
        <translation>Dupliceren</translation>
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
        <translation>Verwijderen…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1744"/>
        <source>Edit Frame Parser…</source>
        <translation>Frame-parser Bewerken…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1786"/>
        <source>Edit Painter Code…</source>
        <translation>Painter-code Bewerken…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1808"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1876"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1928"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2037"/>
        <source>Move Up</source>
        <translation>Omhoog Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1819"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1888"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1940"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2048"/>
        <source>Move Down</source>
        <translation>Omlaag Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1856"/>
        <source>Edit Transform Code…</source>
        <translation>Transform-code Bewerken…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2111"/>
        <source>Edit Code…</source>
        <translation>Code Bewerken…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2126"/>
        <source>Edit Control Loop…</source>
        <translation>Bewerk Besturingslus…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>Groep</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="349"/>
        <source>Action</source>
        <translation>Actie</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">Nog geen groepen gedefinieerd</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Frameparser Testen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>Frame %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>Decoder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>Rijen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 rij(en)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Rij %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Pipelineconfiguratie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Detectie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Einde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Hex-scheidingstekens</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Frame-gegevensinvoer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Voer hex-bytes in (bijv. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Voer hier ruwe stream-bytes in...</translation>
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
        <translation>Het voorbeeld bevat niet de geconfigureerde frame-scheidingstekens, dus er wordt geen frame geëxtraheerd. Typ ze in het voorbeeld (bijv. 
 voor een nieuwe regel) of pas de detectiemodus aan.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Pipelineresultaten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Klaarzetten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Waarde</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>Extractie heeft geen compleet frame opgeleverd. Controleer de start-/eindscheidingstekens en de detectiemodus.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Voer hierboven voorbeeldgegevens in en druk op Evalueren om de geparseerde uitvoer te bekijken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Evalueren</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Ongedaan Maken</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Opnieuw</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Knippen</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Kopiëren</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Plakken</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Alles Selecteren</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Document Formatteren</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Selectie Formatteren</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Resetten</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Resetten naar het standaard parsing-script</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Openen</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Importeer een scriptbestand voor gegevensparsing</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Helpdocumentatie voor gegevensparsing openen</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Taal:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Selecteer Sjabloon…</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Maak de laatste codebewerking ongedaan</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Voer de eerder ongedaan gemaakte bewerking opnieuw uit</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Knip geselecteerde code naar klembord</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Kopieer geselecteerde code naar klembord</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Plak code vanaf klembord</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Help</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Testen met Voorbeeldgegevens</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Evalueren</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Automatisch Centreren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Traject Plotten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Inzoomen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Uitzoomen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Weer Tonen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>NASA Weeroverlay</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Basiskaart: %1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-functies gedetecteerd in dit project.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Fallback-widgets worden gebruikt. Koop een licentie om volledige functionaliteit te ontgrendelen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>Dataset Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Voeg een generieke dataset toe aan de huidige groep</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>Voeg een 2D-plot toe om numerieke gegevens te visualiseren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>FFT-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>Voeg een FFT-plot toe voor frequentiedomeinvisualisatie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>Waterval</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Waterval (spectrogram) plot toevoegen — gebruikt de FFT-instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>Balk/niveau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Voeg een balk- of niveau-indicator toe voor geschaalde waarden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Voeg een meterwidget toe voor analoge visualisatie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Voeg een kompas toe om richtings- of hoekgegevens weer te geven</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Voeg een analoge meter (halve boog) widget toe</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Voeg een thermometer widget toe voor temperatuurgegevens</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>Voeg een LED-indicator toe voor binaire statussignalen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>Besturing Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>Knop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>Voeg een knop toe die bij klikken een commando verzendt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>Schuifregelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Voeg een schuifregelaar toe voor het verzenden van geschaalde numerieke waarden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>Schakelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Voeg een schakelaar toe voor aan/uit-commando's</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>Tekstveld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Voeg een tekstveld toe voor het typen en verzenden van commando's</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>Draaiknop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Voeg een draaiknop toe voor setpoint-besturing</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>Code Bewerken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Bewerk de JavaScript die deze painter-widget tekent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>Dupliceren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Dupliceer de huidige groep en de inhoud ervan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Verwijder de huidige groep en alle datasets erin</translation>
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
        <translation>HID-apparaat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Gebruikspagina</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Gebruik</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Verbind gamepads, joysticks, stuurwielen, flight controllers en andere HID-klasse USB-apparaten.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>HID-gebruikstabellen (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>Helpcentrum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>Helppagina's ophalen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>Zoeken…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>Laden…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>Selecteer een pagina uit de zijbalk</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>Gekopieerd naar Klembord</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>Online Bekijken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>%1 pagina's</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>Sluiten</translation>
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
        <translation>Netwerksocket</translation>
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
        <translation>CAN-bus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>USB Device</source>
        <translation>USB-apparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>HID Device</source>
        <translation>HID-apparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT-abonnee</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="685"/>
        <source>Your trial period has ended.</source>
        <translation>Uw proefperiode is afgelopen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="686"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Activeer uw licentie om Serial Studio te blijven gebruiken.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="752"/>
        <source>channels</source>
        <translation>kanalen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="805"/>
        <source> channels</source>
        <translation>kanalen</translation>
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
        <translation>Invoerapparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1350"/>
        <source>Sample Rate</source>
        <translation>Samplefrequentie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1358"/>
        <source>Sample Format</source>
        <translation>Sampleformaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1366"/>
        <source>Channels</source>
        <translation>Kanalen</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>BLE I/o-modulefout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>Selecteer Apparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>Selecteer Service</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>Selecteer Characteristic</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>Fout bij configureren van BLE-service</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>Bewerkingsfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>Characteristic-schrijffout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>Descriptor-schrijffout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>Onbekende fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>Characteristic-leesfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>Descriptor-leesfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>BLE-apparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>Service</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>Notify Characteristic</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>Characteristic</translation>
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
        <translation>CAN-bus Niet Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="331"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>Geen CAN-bus-plugins gevonden op dit systeem.

CAN-bus-ondersteuning op macOS is beperkt en kan hardware-drivers van derden vereisen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="336"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Geen CAN-bus-plugins beschikbaar op dit platform.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="348"/>
        <source>Invalid CAN Configuration</source>
        <translation>Ongeldige CAN-configuratie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="356"/>
        <source>Invalid Selection</source>
        <translation>Ongeldige Selectie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="365"/>
        <source>No Devices Available</source>
        <translation>Geen Apparaten Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="274"/>
        <source>CAN Device Creation Failed</source>
        <translation>CAN-apparaat Aanmaken Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="301"/>
        <source>CAN Connection Failed</source>
        <translation>CAN-verbinding Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="319"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>Geen CAN-bus-plugins gevonden op dit systeem.

Zorg er op Linux voor dat SOCKETCAN-kernelmodules zijn geladen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="325"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>Geen CAN-bus-plugins gevonden op dit systeem.

Installeer op Windows CAN-hardwarestuurprogramma's (PEAK, VECTOR, etc.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="349"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>De CAN-busconfiguratie is onvolledig. Selecteer een geldige plugin en interface.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="357"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>De geselecteerde plugin of interface is niet meer beschikbaar. Ververs de lijsten en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="366"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>De plugin- of interfacelijst is leeg. Ververs de lijsten en controleer of de CAN-hardware is aangesloten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="275"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>Kan CAN-busapparaat niet aanmaken. Controleer de hardware en stuurprogramma's.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Kan geen verbinding maken met CAN-busapparaat. Controleer de hardwareverbinding en instellingen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="695"/>
        <source>CAN Bus Error</source>
        <translation>CAN-busfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="696"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>Er is een fout opgetreden, maar het CAN-apparaat is niet meer beschikbaar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="703"/>
        <source>Error code: %1</source>
        <translation>Foutcode: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="706"/>
        <source>CAN Bus Communication Error</source>
        <translation>CAN-buscommunicatiefout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="783"/>
        <source>No CAN driver selected</source>
        <translation>Geen CAN-stuurprogramma geselecteerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="725"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>Laad eerst de SOCKETCAN-kernelmodules</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="721"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>Sluit een %1-adapter aan en vernieuw</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="728"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Stel eerst een virtuele CAN-interface in</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="730"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="750"/>
        <source>No interfaces found for %1</source>
        <translation>Geen interfaces gevonden voor %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="734"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>Installeer &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN-stuurprogramma's&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="738"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>Installeer &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;VECTOR CAN-stuurprogramma's&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="742"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>Installeer &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN-stuurprogramma's&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="745"/>
        <source>Install %1 drivers</source>
        <translation>Installeer %1-stuurprogramma's</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="748"/>
        <source>Install %1 drivers for macOS</source>
        <translation>Installeer %1-stuurprogramma's voor macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="762"/>
        <source>

If the interface is down, bring it up first:
sudo ip link set %1 up type can bitrate %2</source>
        <translation>Als de interface inactief is, activeer deze eerst:
sudo ip link set %1 up type can bitrate %2

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="864"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="872"/>
        <source>Interface</source>
        <translation>Interface</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="880"/>
        <source>Bitrate</source>
        <translation>Bitsnelheid</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="889"/>
        <source>CAN FD</source>
        <translation>CAN-FD</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="896"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="903"/>
        <source>Listen-Only</source>
        <translation>Alleen-luisteren</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="382"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>Initialisatie van libusb voor de CANable-adapter is mislukt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="410"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>Kan USB-apparaten niet opsommen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="430"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>De geselecteerde CANable-adapter is niet langer verbonden, of een andere toepassing heeft deze geopend. Op Windows moet het apparaat de WinUSB-driver gebruiken (candleLight installeert deze automatisch).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="443"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>Kon de CANable USB-interface niet claimen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="510"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>CANable-adapter is niet geopend voor schrijven.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="545"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>Verzenden van CAN-frame naar de adapter mislukt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="561"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>CAN-busfout gerapporteerd door de CANable-adapter.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="616"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>Een CAN-frame werd niet bevestigd op de bus.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="715"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>CANable-adapter heeft de host-format handshake afgewezen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="722"/>
        <source>Could not read CANable timing constants.</source>
        <translation>Kon CANable-timingconstanten niet lezen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="728"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>De bitsnelheid %1 bps wordt niet ondersteund door deze CANable-adapter.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="734"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>CANable-adapter heeft de gevraagde bitsnelheid afgewezen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="748"/>
        <source>Could not start the CANable channel.</source>
        <translation>Kan het CANable-kanaal niet starten.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>Onbekende fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Controleer of uw gebruiker in de groep 'plugdev' zit of dat een udev-regel toegang verleent tot dit apparaat.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>Openen van "%1" mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>HID-apparaatfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>Het HID-apparaat is losgekoppeld of heeft een fatale leesfout ondervonden.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>Selecteer Apparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>HID-apparaat</translation>
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
        <translation>TLS 1.3 of Later</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 of Later</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>Elk Protocol</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>Alleen Beveiligde Protocollen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>Geen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>Peer Opvragen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>Peer Verifiëren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>Peer Automatisch Verifiëren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>MQTT-Functie Vereist een Commerciële Licentie</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">Abonneren op een MQTT-broker is alleen beschikbaar met een geldige commerciële licentie van Serial Studio (Hobbyist-tier of hoger).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>Abonneren op een MQTT-broker is alleen beschikbaar met een geldige Serial Studio-licentie of een actieve proefperiode.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>Systeemdatabase Gebruiken</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>Laden Uit Map…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Selecteer Pem-certificatenmap</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>Hostnaam</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>Topicfilter</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>Client-ID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>Gebruikersnaam</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>Wachtwoord</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>MQTT-versie</translation>
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
        <translation>Automatische Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS Ingeschakeld</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>SSL-protocol</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>Peer-verificatiemodus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>Peer-verificatiediepte</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>MQTT-abonnementsfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Abonneren op topic "%1" mislukt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Ongeldige MQTT-protocolversie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>De broker heeft de geconfigureerde MQTT-protocolversie geweigerd.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>Client-ID Geweigerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>De broker heeft de client-ID geweigerd. Probeer een andere identificatie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>MQTT-server Niet Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>De broker is momenteel niet beschikbaar. Probeer het later opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>Authenticatiefout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>De opgegeven inloggegevens zijn geweigerd door de broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>Autorisatiefout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>Account heeft geen toestemming voor deze bewerking.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>Netwerk- of Transportfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Netwerk-/transportlaagprobleem tijdens verbinding met de broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>MQTT-protocolschending</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>De broker meldde een protocolschending en sloot de verbinding.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>MQTT 5-Fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>Er is een MQTT 5-protocolfout opgetreden.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>MQTT-fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>Er is een onverwachte MQTT-fout opgetreden.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>Ongeldige Seriële Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Modbus-initialisatie Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Modbus-verbinding Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>De geselecteerde seriële poort "%1" is niet langer beschikbaar. Ververs de poortenlijst en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Kan Modbus-apparaat niet aanmaken. Controleer de systeemconfiguratie en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Kan geen verbinding maken met "%1". Controleer de verbindingsinstellingen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="609"/>
        <source>None</source>
        <translation>Geen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="610"/>
        <source>Even</source>
        <translation>Even</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="611"/>
        <source>Odd</source>
        <translation>Oneven</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="612"/>
        <source>Space</source>
        <translation>Spatie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="613"/>
        <source>Mark</source>
        <translation>Markering</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="665"/>
        <source>Holding Registers (0x03)</source>
        <translation>Holding Registers (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="666"/>
        <source>Input Registers (0x04)</source>
        <translation>Input Registers (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="667"/>
        <source>Coils (0x01)</source>
        <translation>Coils (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="668"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Discrete Inputs (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>No register groups configured</source>
        <translation>Geen registergroepen geconfigureerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="851"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Voeg minimaal één registergroep toe voordat een project wordt gegenereerd.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="865"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="890"/>
        <source>Modbus Project Generator</source>
        <translation>Modbus Projectgenerator</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Aanmaken van tijdelijk projectbestand mislukt</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Controleer schrijfrechten voor de tijdelijke map.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="862"/>
        <source>Failed to load generated project</source>
        <translation>Gegenereerd project laden mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="863"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>De gegenereerde project-JSON kon niet worden geladen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="885"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Project succesvol gegenereerd met %1 groepen en %2 datasets.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="888"/>
        <source>The project editor is now open for customization.</source>
        <translation>De projecteditor is nu geopend voor aanpassing.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="903"/>
        <source>Modbus Project</source>
        <translation>Modbus Project</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="928"/>
        <source>Holding Registers</source>
        <translation>Holding Registers</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="929"/>
        <source>Input Registers</source>
        <translation>Invoerregisters</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="930"/>
        <source>Coils</source>
        <translation>Spoelen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="931"/>
        <source>Discrete Inputs</source>
        <translation>Discrete Ingangen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="945"/>
        <source>Unknown</source>
        <translation>Onbekend</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="958"/>
        <source>Register %1</source>
        <translation>Register %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Coil %1</source>
        <translation>Spoel %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Discrete %1</source>
        <translation>Discrete %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1372"/>
        <source>Error code: %1</source>
        <translation>Foutcode: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1375"/>
        <source>Modbus Communication Error</source>
        <translation>Modbus Communicatiefout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1387"/>
        <source>Select Port</source>
        <translation>Selecteer Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1540"/>
        <source>Protocol</source>
        <translation>Protocol</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1548"/>
        <source>Slave Address</source>
        <translation>Slave-adres</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1557"/>
        <source>Poll Interval (ms)</source>
        <translation>Poll-Interval (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1597"/>
        <source>Host / IP</source>
        <translation>Host / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1604"/>
        <source>Port</source>
        <translation>Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1619"/>
        <source>Serial Port</source>
        <translation>Seriële Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1627"/>
        <source>Baud Rate</source>
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1635"/>
        <source>Parity</source>
        <translation>Pariteit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1643"/>
        <source>Data Bits</source>
        <translation>Databits</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1651"/>
        <source>Stop Bits</source>
        <translation>Stopbits</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="533"/>
        <source>Network socket error</source>
        <translation>Netwerksocketfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="549"/>
        <source>Socket Type</source>
        <translation>Sockettype</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="557"/>
        <source>Remote Address</source>
        <translation>Extern Adres</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="565"/>
        <source>TCP Port</source>
        <translation>TCP-poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="574"/>
        <source>UDP Local Port</source>
        <translation>UDP Lokale Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="583"/>
        <source>UDP Remote Port</source>
        <translation>UDP Externe Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="592"/>
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
        <translation>Proces starten mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Uitvoerbaar bestand "%1" niet gevonden in PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>Selecteer Uitvoerbaar Bestand</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>Selecteer Werkmap</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Selecteer Named Pipe / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>Het proces is gecrasht.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>Exitcode: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>Proces "%1" gestopt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>Onbekende fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>Procesfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>Pipe-fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>Kan named pipe niet openen: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>Start Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>Uitvoerbaar Bestand</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>Argumenten</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>Werkmap</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>Pipe-pad</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="206"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>De bitsnelheid %1 bps wordt niet ondersteund door de USB-CAN Analyzer.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="216"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Kan seriële poort %1 niet openen: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="227"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>Initialiseren van de USB-CAN Analyzer mislukt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="264"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>USB-CAN Analyzer is niet geopend voor schrijven.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="310"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>CAN-busfout gerapporteerd door de USB-CAN Analyzer.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="167"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>De bitsnelheid %1 bps is geen standaard slcan-snelheid.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="177"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Kan seriële poort %1 niet openen: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="190"/>
        <source>Failed to open the slcan channel.</source>
        <translation>Openen van het slcan-kanaal mislukt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="229"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>slcan-adapter is niet geopend voor schrijven.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="267"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>CAN-busfout gemeld door de slcan-adapter.</translation>
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
        <translation>Geen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="349"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="733"/>
        <source>Select Port</source>
        <translation>Selecteer Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="392"/>
        <source>Even</source>
        <translation>Even</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="393"/>
        <source>Odd</source>
        <translation>Oneven</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="394"/>
        <source>Space</source>
        <translation>Spatie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="395"/>
        <source>Mark</source>
        <translation>Markering</translation>
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
        <translation>"%1" is geen geldig pad</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="563"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Voer een ander pad in om een aangepast serieel apparaat te registreren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="823"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Het opgegeven apparaat kon niet worden gevonden. Controleer de verbinding en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="830"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>Er is een onbekende fout opgetreden. Controleer het apparaat en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="832"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Het apparaat is niet geopend. Open het apparaat voordat u deze bewerking uitvoert.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="261"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Kan geen verbinding maken met seriële poort "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="798"/>
        <source>Unknown</source>
        <translation>Onbekend</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="799"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Kritieke fout op seriële poort "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="800"/>
        <source>Unknown error</source>
        <translation>Onbekende fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="822"/>
        <source>No error occurred.</source>
        <translation>Geen fout opgetreden.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="824"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Toegang geweigerd. Zorg ervoor dat de applicatie de benodigde toegangsrechten tot het apparaat heeft.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="825"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Openen van het apparaat mislukt. Het kan al in gebruik of niet beschikbaar zijn.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="826"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>Er is een fout opgetreden bij het schrijven van gegevens naar het apparaat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>Er is een fout opgetreden bij het lezen van gegevens van het apparaat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>Er is een kritieke resourcefout opgetreden. Het apparaat kan losgekoppeld zijn of is niet langer toegankelijk.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>De gevraagde bewerking wordt niet ondersteund op dit apparaat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="831"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>De bewerking is verlopen. Het apparaat reageert mogelijk niet.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Serial Port</source>
        <translation>Seriële Poort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Baud Rate</source>
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Parity</source>
        <translation>Pariteit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Data Bits</source>
        <translation>Databits</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>Stop Bits</source>
        <translation>Stopbits</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1034"/>
        <source>Flow Control</source>
        <translation>Stroomregeling</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1042"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1049"/>
        <source>Auto-Reconnect</source>
        <translation>Automatisch Opnieuw Verbinden</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="156"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="164"/>
        <source>USB Error</source>
        <translation>USB-fout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="157"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Initialisatie van het USB-subsysteem is mislukt. Controleer of libusb beschikbaar is op uw systeem.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="214"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="572"/>
        <source>USB Device Error</source>
        <translation>USB-apparaatfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>Kan het USB-apparaat niet openen: %1.

Controleer op Linux of u lees-/schrijfrechten hebt op het apparaatknooppunt (voeg een udev-regel toe of voer uit als root). Op macOS moet mogelijk eerst het kernelstuurprogramma worden losgekoppeld.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="165"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Geen USB-apparaat geselecteerd. Selecteer een apparaat en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="171"/>
        <source>Unknown Device</source>
        <translation>Onbekend Apparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <source>Failed to open "%1"</source>
        <translation>Kan "%1" niet openen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="215"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Kan interface %1 op het USB-apparaat niet claimen.

Een ander stuurprogramma of toepassing heeft deze mogelijk al geopend. Probeer op Linux het kernelstuurprogramma te verwijderen (bijv. cdc_acm) of voeg een udev-regel toe.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="360"/>
        <source>Select Device</source>
        <translation>Apparaat Selecteren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="379"/>
        <source>Select IN Endpoint</source>
        <translation>In-eindpunt Selecteren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="390"/>
        <source>None (Read-only)</source>
        <translation>Geen (Alleen-lezen)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="464"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Geavanceerde USB-besturingsoverdrachten Inschakelen?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="465"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Dit schakelt besturingsoverdrachten in naast bulkoverdrachten. Het verzenden van onjuiste besturingsverzoeken kan aangesloten hardware laten crashen of beschadigen. Schakel dit alleen in als u weet wat u doet.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="469"/>
        <source>Advanced USB Mode</source>
        <translation>Geavanceerde USB-modus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="573"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>Het USB-apparaat is losgekoppeld of heeft een fatale leesfout ondervonden.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="710"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Geen isochrone IN-eindpunt gevonden op dit apparaat, maar bulk-eindpunten zijn beschikbaar.

Schakel de Overdrachtsmodus over naar "Bulkstroom" en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="715"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Geen bulk IN-eindpunt gevonden op dit apparaat, maar isochrone eindpunten zijn beschikbaar.

Schakel de Overdrachtsmodus over naar "Isochroon" en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="719"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Geen bruikbaar IN-eindpunt gevonden op dit apparaat.

Het apparaat biedt mogelijk geen data-eindpunten in zijn actieve configuratie, of vereist mogelijk een specifiek stuurprogramma.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1211"/>
        <source>USB Device</source>
        <translation>USB-apparaat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1219"/>
        <source>Transfer Mode</source>
        <translation>Overdrachtsmodus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Bulk Stream</source>
        <translation>Bulkstroom</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Advanced Control</source>
        <translation>Geavanceerde Besturing</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Isochronous</source>
        <translation>Isochroon</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1227"/>
        <source>IN Endpoint</source>
        <translation>In-eindpunt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1235"/>
        <source>OUT Endpoint</source>
        <translation>Out-eindpunt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1243"/>
        <source>ISO Packet Size</source>
        <translation>ISO-pakketgrootte</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>Geen bestand geselecteerd…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>Platte Tekst</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Raw Binair</translation>
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
        <translation>Selecteer bestand om te verzenden</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>Bestand geselecteerd: %1 (%2 bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>Fout bij openen bestand: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>%1-overdracht starten…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>Overdracht voltooid</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>Platte Tekst-overdracht voltooid</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Ruwe binaire overdracht voltooid (%1 bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>Overdracht voltooid</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Overdracht succesvol voltooid (%1 bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>Overdracht mislukt: %1</translation>
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
        <translation>Frames verloren</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>Inkomende data arriveert sneller dan Serial Studio kan verwerken; %1 frame(s) zijn verloren gegaan. Verlaag de datasnelheid of schakel een zware verbruiker uit.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>Kan bestand niet openen: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>Overdracht geannuleerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>Overdracht geannuleerd door gebruiker</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>Wachten op ontvanger…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Ontvanger gereed (CRC-modus), gegevens worden verzonden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Te veel pogingen, overdracht afgebroken</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximum aantal pogingen overschreden</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>NAK ontvangen, blok %1 opnieuw verzenden (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>Zoeken in bestand mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Overdracht geannuleerd door ontvanger</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Ontvanger heeft de overdracht geannuleerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>Overdracht voltooid</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>Bestandslezing retourneerde meer gegevens dan aangevraagd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Blok %1 verzenden (%2 bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>EOT Verzenden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Time-out, opnieuw proberen (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>Overdracht verlopen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>Time-out: geen reactie van ontvanger</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>Kan bestand niet openen: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Overdracht geannuleerd door ontvanger</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Ontvanger heeft de overdracht geannuleerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>Wachten op ontvanger…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>Eerste EOT verzenden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Te veel pogingen, overdracht afgebroken</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximum aantal pogingen overschreden</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>NAK ontvangen, blok %1 opnieuw verzenden</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>Zoeken in bestand mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>Tweede EOT verzenden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>End-of-batch-markering verzenden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>Overdracht voltooid</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Bestandsheader verzenden: %1 (%2 bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Blok %1 verzenden (%2/%3 bytes)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>Kan bestand niet openen: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Bestand is te groot voor ZMODEM (%1 bytes, limiet 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>Overdracht geannuleerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>Overdracht geannuleerd door gebruiker</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>Hex-header CRC komt niet overeen, frame wordt verwijderd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Bestandsinformatie verzenden: %1 (%2 bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>Zoeken naar offset %1 mislukt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>Bestandslezing retourneerde meer gegevens dan aangevraagd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Ontvanger vraagt gegevens vanaf offset %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>Ontvanger heeft het bestand overgeslagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>Te veel fouten, overdracht afgebroken</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximum aantal pogingen overschreden</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>ZRQINIT verzenden, wachten op ontvanger…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Bestandsgegevens verzonden, wachten op bevestiging…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>ZFIN Wordt Verzonden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Ontvanger gereed, bestandsinfo wordt verzonden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>NAK ontvangen, opnieuw proberen (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>Overdracht voltooid</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Overdracht geannuleerd door ontvanger</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Ontvanger heeft de overdracht geannuleerd</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>Ontvanger meldde een bestandsfout</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>Time-out bij overdracht</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>Time-out: geen reactie van ontvanger</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Time-out, opnieuw proberen (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>Selecteer Pictogram</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>Online Zoeken…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>Normaal</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>Grijswaarden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Hoog Contrast</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Levendig</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Nachtzicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Infrarood</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Diepblauw</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Amber</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Afbeeldingen Exporteren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Exportmap Openen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Inzoomen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Uitzoomen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Toon Kruisdraad</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>Wachten op Afbeelding…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>API-sleutels</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. De standaard is Claude Haiku 4.5 ($1 invoer / $5 uitvoer per miljoen tokens). Sonnet 4.6 en OPUS 4.7 zijn ook beschikbaar. Ondersteunt streaming, tool-gebruik, uitgebreid denken en prompt-caching.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. De standaard is GPT-4o mini ($0,15 invoer / $0,60 uitvoer per miljoen tokens). GPT-4o, GPT-4 Turbo en o1-mini zijn ook beschikbaar.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. De standaard is Gemini 2.0 Flash, met een ruime gratis tier (onder voorbehoud van snelheidsbeperkingen). Gemini 1.5 Pro en Gemini 1.5 Flash zijn ook beschikbaar.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Gebruik eigen API-sleutels. Ze worden in rust versleuteld met een apparaatspecifieke sleutel en verlaten de computer nooit, behalve om te communiceren met de geselecteerde provider.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>Sleutel ingesteld</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>Geen sleutel</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">Er is een sleutel opgeslagen — plak een nieuwe om deze te vervangen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. De standaard is GPT-5 mini voor snel, kostenefficiënt agentisch werk. GPT-5.2 is de sterkere algemene optie, en GPT-5.2 Chat volgt het model dat momenteel in ChatGPT wordt gebruikt.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. OpenAI-compatibele API. De standaard is deepseek-chat (V3). deepseek-reasoner (R1) is ook beschikbaar. Vaak de goedkoopste cloudoptie voor tool-gebruik.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. Eén sleutel, ~200 modellen van Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen en anderen. Gratis modellen (met :free achtervoegsel) zijn rate-limited maar vereisen geen extra facturering. Pay-as-you-go voor de rest.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Hardware-versnelde inferentie (LPU's) voor zeer snelle Llama-, Mixtral-, Gemma-, DeepSeek-R1 distill- en Qwen-modellen. Royale gratis tier met dagelijkse tokenlimieten.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. De standaard is Mistral Large. Codestral richt zich op code-aanvulling, Pixtral verwerkt beelden, en de Ministral-modellen zijn afgestemd op edge- / lage-latentie-toepassingen.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Lokale modelserver. Werkt met elk OpenAI-compatibel eindpunt -- Ollama, llama.cpp's llama-server, LM Studio of vLLM. Niets verlaat je machine. De modellijst wordt live van de server opgehaald.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>Er is een sleutel opgeslagen -- plak een nieuwe om deze te vervangen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>Plak hier uw API-sleutel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>Sleutel verbergen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>Sleutel tonen tijdens typen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>Sleutel ophalen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Open de console van de provider om een nieuwe sleutel aan te maken</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>Opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>Verwijder de opgeslagen sleutel voor %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Installeer Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Open de Ollama-downloadpagina</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>Toepassen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>Modellijst opnieuw ophalen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Nog geen API-sleutels geconfigureerd. Voeg een sleutel toe om te beginnen.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">Nog geen API-sleutels geconfigureerd. Voeg er minimaal één toe om te beginnen.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>Eén provider is gereed.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 providers zijn gereed.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Licenties</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Even geduld…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Serial Studio Pro Activeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Plak hieronder uw licentiesleutel om Pro-functies zoals MQTT, 3D-plotting en meer te ontgrendelen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>Uw licentie omvat 5 apparaatactivaties.
Abonnementen zijn beschikbaar als Maandelijks, Jaarlijks en Levenslang.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Plak hier uw licentiesleutel…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Plakken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>Alles Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>Product</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>Licentiehouder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>E-mailadres Licentiehouder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>Apparaatgebruik</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 apparaten in gebruik (Onbeperkt abonnement)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 van %2 apparaten gebruikt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>Apparaat-ID</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>Licentiesleutel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>Klantportaal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>Licentie Kopen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>Activeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>Deactiveren</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="517"/>
        <source>There was an issue validating your license.</source>
        <translation>Er is een probleem opgetreden bij het valideren van uw licentie.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="535"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="716"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="835"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>De licentiesleutel die u heeft opgegeven behoort niet tot Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="536"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Controleer of u uw licentie heeft gekocht in de officiële Serial Studio-winkel.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="547"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="725"/>
        <source>This license key was activated on a different device.</source>
        <translation>Deze licentiesleutel is geactiveerd op een ander apparaat.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="548"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="726"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Deactiveer deze eerst daar of neem contact op met ondersteuning voor hulp.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="559"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="736"/>
        <source>This license is not currently active.</source>
        <translation>Deze licentie is momenteel niet actief.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="560"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Deze kan zijn verlopen of gedeactiveerd (status: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="570"/>
        <source>Something went wrong on the server.</source>
        <translation>Er is iets misgegaan op de server.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="571"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="747"/>
        <source>No activation ID was returned.</source>
        <translation>Geen activerings-ID geretourneerd.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="581"/>
        <source>Could not validate your license at this time.</source>
        <translation>Kon uw licentie op dit moment niet valideren.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="582"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="756"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="845"/>
        <source>Try again later.</source>
        <translation>Probeer later opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="717"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="836"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Controleer of u uw licentie hebt gekocht in de officiële Serial Studio-winkel.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <source>Something went wrong on the server…</source>
        <translation>Er is iets misgegaan op de server…</translation>
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
        <translation>Uw licentie is succesvol geactiveerd.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="654"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Bedankt voor het ondersteunen van Serial Studio!
U hebt nu toegang tot alle premium-functies.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="708"/>
        <source>There was an issue activating your license.</source>
        <translation>Er is een probleem opgetreden bij het activeren van uw licentie.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Could not activate your license at this time.</source>
        <translation>Uw licentie kon op dit moment niet worden geactiveerd.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="827"/>
        <source>There was an issue deactivating your license.</source>
        <translation>Er is een probleem opgetreden bij het deactiveren van uw licentie.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="844"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>Uw licentie kon op dit moment niet worden gedeactiveerd.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="854"/>
        <source>Your license has been deactivated.</source>
        <translation>Uw licentie is gedeactiveerd.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="855"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Toegang tot Pro-functies is verwijderd.
Nogmaals bedankt voor uw steun aan Serial Studio!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>MDF4-export is een Pro-functie.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Deze functie vereist een licentie. Koop er een om MDF4-export in te schakelen.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>Selecteer MDF4-bestand</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>MDF4-bestanden (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>Verbinding met apparaat verbreken?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>U moet de verbinding met het huidige apparaat verbreken voordat u een MDF4-bestand opent.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>Kan MDF4-bestand niet openen</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Het bestand kan beschadigd zijn of een niet-ondersteund formaat hebben.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>Ongeldig MDF4-bestand</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Bestandsstructuur lezen mislukt. Het bestand kan beschadigd zijn.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>Geen gegevens in bestand</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>Het MDF4-bestand bevat geen meetgegevens.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>Hostnaam</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>bijv. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Poort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Topicfilter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>bijv. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>Client-ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Opnieuw Genereren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Gebruikersnaam</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Wachtwoord</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>Versie</translation>
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
        <translation>SSL/TLS Gebruiken</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>SSL-protocol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Peer Verifiëren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Verificatiediepte</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>Ca-certificaten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Laden Uit Map…</translation>
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
        <translation type="vanished">TLS 1.3 of Later</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 of Later</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">Elk Protocol</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">Alleen Beveiligde Protocollen</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Geen</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">Peer Opvragen</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">Peer Verifiëren</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">Peer Automatisch Verifiëren</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">Systeemdatabase Gebruiken</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">MQTT-abonnee</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">MQTT Publisher</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">MQTT Functie Vereist een Commerciële Licentie</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">Laden Uit Map…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">Verbinding maken met MQTT brokers is alleen beschikbaar met een geldige Serial Studio commerciële licentie (Hobbyist tier of hoger).

Activeer uw licentie of bezoek de winkel om deze functie te ontgrendelen.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">MQTT Topic Ontbreekt</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">Er moet een topic worden opgegeven voordat verbinding kan worden gemaakt als publisher.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">Configuratiefout</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">MQTT Topic Niet Ingesteld</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">Er worden geen berichten ontvangen totdat een topic is geconfigureerd.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">Configuratiewaarschuwing</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">Ongeldig MQTT-topic</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Het topic "%1" is niet geldig.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">Selecteer Pem-certificatenmap</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">Abonnementsfout</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Abonneren op topic "%1" mislukt.</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">Ongeldige MQTT-protocolversie</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">De MQTT-broker heeft de verbinding geweigerd vanwege een niet-ondersteunde protocolversie. Zorg ervoor dat client en broker dezelfde protocolversie ondersteunen.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">Client-ID Geweigerd</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">De broker heeft de client-ID geweigerd. Deze kan onjuist geformatteerd, te lang of al in gebruik zijn. Probeer een andere client-ID.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">MQTT-server Niet Beschikbaar</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">De netwerkverbinding is tot stand gebracht, maar de broker is momenteel niet beschikbaar. Controleer de brokerstatus en probeer later opnieuw.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">Authenticatiefout</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">De opgegeven gebruikersnaam of wachtwoord is onjuist of ongeldig. Controleer uw inloggegevens en probeer opnieuw.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">Autorisatiefout</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">De MQTT-broker heeft de verbinding geweigerd wegens onvoldoende rechten. Zorg ervoor dat uw account over de benodigde toegangsrechten beschikt.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">Netwerk- of Transportfout</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">Er is een netwerk- of transportlaagprobleem opgetreden, waardoor de verbinding onverwacht is mislukt. Controleer uw netwerkverbinding en brokerinstellingen.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">MQTT-protocolschending</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">De client heeft een schending van het MQTT-protocol gedetecteerd en de verbinding gesloten. Controleer uw MQTT-implementatie op naleving.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">Onbekende Fout</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">Er is een onverwachte fout opgetreden. Controleer de logs voor meer details of herstart de applicatie.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">MQTT 5-Fout</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">Er is een MQTT-protocolfout op niveau 5 opgetreden. Controleer de brokerlogs of redencodes voor meer details.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">MQTT-authenticatie Mislukt</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">Authenticatie mislukt: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">Uitgebreide authenticatie is vereist, maar MQTT 5.0 is niet ingeschakeld.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">Onbekend</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">MQTT-authenticatie Vereist</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">De MQTT-broker vereist authenticatie met methode: "%1".

Geef de benodigde inloggegevens op.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">Voer MQTT-gebruikersnaam In</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">Gebruikersnaam:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">Voer MQTT-wachtwoord In</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">Wachtwoord:</translation>
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
        <translation>TLS 1.3 of Later</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 of Later</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>Elk Protocol</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>Alleen Beveiligde Protocollen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>Geen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>Peer Opvragen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>Peer Verifiëren</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>Peer Automatisch Verifiëren</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>Onbewerkte Rx-gegevens</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>Aangepast Script</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Dashboardgegevens (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Dashboardgegevens (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>MQTT-publisher niet beschikbaar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>Een geldige commerciële licentie is vereist om MQTT-publicatie te gebruiken.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>MQTT Testverbinding</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Selecteer Pem-certificatenmap</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>MQTT broker bereikbaar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>MQTT broker onbereikbaar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>MQTT broker verbinding mislukt</translation>
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
        <translation>MQTT Publisher Script</translation>
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
        <translation>Voorbeeld frame bytes (tekst of hex)</translation>
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
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Apply</source>
        <translation>Toepassen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="103"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="112"/>
        <source>Language:</source>
        <translation>Taal:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="115"/>
        <source>Template:</source>
        <translation>Sjabloon:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="126"/>
        <source>Frame:</source>
        <translation>Frame:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="130"/>
        <source>Output:</source>
        <translation>Uitvoer:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="270"/>
        <source>Enter a frame</source>
        <translation>Voer een frame in</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="277"/>
        <source>Invalid hex</source>
        <translation>Ongeldige hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Document Formatteren	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="361"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Selectie Formatteren	ctrl+i</translation>
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
-- Definieer een mqtt(frame) functie die de ruwe bytes
-- van één geparsed frame ontvangt en de payload retourneert
-- om naar de broker te publiceren, of nil om dit frame over te slaan.
--
-- Het frame-argument komt overeen met wat het Frame Parser-script
-- ziet: een Lua-string met de bytes tussen FrameReader-
-- scheidingstekens.
--
-- Voorbeeld:
--   function mqtt(frame)
--     return frame    -- doorsturen zoals het is
--   end
--
-- Gebruik de Template-dropdown voor kant-en-klare voorbeelden.
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
 * Definieer een mqtt(frame) functie die de ruwe bytes
 * van één geparsed frame ontvangt en de payload retourneert
 * om naar de broker te publiceren, of null om dit frame over te slaan.
 *
 * Het frame-argument komt overeen met wat het Frame Parser-script
 * ziet: een string met de bytes tussen FrameReader-
 * scheidingstekens.
 *
 * Voorbeeld:
 *   function mqtt(frame) {
 *     return frame;   // doorsturen zoals het is
 *   }
 *
 * Gebruik de Template-dropdown voor kant-en-klare voorbeelden.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="615"/>
        <source>Script is empty</source>
        <translation>Script is leeg</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="622"/>
        <source>Lua engine error</source>
        <translation>Lua engine-fout</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="696"/>
        <source>Error: %1</source>
        <translation>Fout: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="652"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() is niet gedefinieerd</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- frame overgeslagen)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="671"/>
        <source>(non-string return)</source>
        <translation>(niet-string retour)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="701"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- frame overgeslagen)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="779"/>
        <source>Select Template…</source>
        <translation>Selecteer Sjabloon…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Configureer brokerhostnaam en poort voordat de verbinding wordt getest.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Succesvol verbonden met %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Time-out na 5 seconden zonder de broker te bereiken.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">MQTT-configuratie</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT is een Pro-Functie</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">Activeer uw licentie of bezoek de winkel om MQTT-ondersteuning te ontgrendelen.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">Algemeen</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">Authenticatie</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">MQTT-opties</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">SSL-eigenschappen</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">Host</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">Poort</translation>
    </message>
    <message>
        <source>Client ID</source>
        <translation type="vanished">Client-ID</translation>
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
        <translation type="vanished">Gebruikersnaam</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">MQTT-gebruikersnaam</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">Wachtwoord</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">MQTT-wachtwoord</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">Versie</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">Modus</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">Topic</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">bijv. sensors/temperature of home/+/status</translation>
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
        <translation type="vanished">Will Topic</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">bijv. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will Message</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">bijv. Apparaat onverwacht losgekoppeld</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">SSL Inschakelen</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">SSL-protocol</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">Verificatiediepte</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">Verificatiemodus</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Sluiten</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">Verbinding Verbreken</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">Verbinden</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="205"/>
        <source>Console Only Mode</source>
        <translation>Alleen Consolemodus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="208"/>
        <source>Quick Plot Mode</source>
        <translation>Snelle Plotmodus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="215"/>
        <source>Empty Project</source>
        <translation>Leeg Project</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="696"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="704"/>
        <source>Waiting for data…</source>
        <translation>Wachten op gegevens…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="705"/>
        <source>Connecting to device…</source>
        <translation>Verbinden met apparaat…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Codevoorbeeld</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Aanvuller</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Syntaxmarkering</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Stijl</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>spaties</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>Gekopieerd naar Klembord</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>MDF4-speler</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Jij</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Ontdekking</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Uitvoering</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>%1 acties goedkeuren in %2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Deze aanroepen worden samen uitgevoerd. Vouw elke kaart hieronder uit om argumenten te inspecteren.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Alles goedkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Alles weigeren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Alles Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Alles Selecteren</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>Jij</translation>
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
        <translation>Fout</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Ontdekking</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Uitvoering</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>Actief</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>Wacht op goedkeuring</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Voltooid</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Geweigerd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Geblokkeerd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Goedkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Weigeren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Alles goedkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Alles weigeren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Argumenten</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Resultaat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>{n} acties in {family} goedkeuren?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Deze aanroepen worden samen uitgevoerd. Vouw elke kaart uit om argumenten te inspecteren.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>Laden van README mislukt: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>Thema</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>Frame-parser</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>Projectsjabloon</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>Alle Typen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>Extensies Resetten</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Dit verwijdert alle extensies, verwijdert alle aangepaste repositories en herstelt de standaardinstellingen. Doorgaan?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>Selecteer Map voor Extensie-repository</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1017"/>
        <source>Installed (repository no longer available)</source>
        <translation>Geïnstalleerd (repository niet meer beschikbaar)</translation>
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
        <translation>Plugin-fout</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Plugin "%1" is niet geïnstalleerd.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1335"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>Extensie "%1" is geen plugin (type: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1356"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Kan plugin-metadatabestand niet lezen:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Plugin "%1" vereist GRPC maar deze build bevat geen GRPC-ondersteuning.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Deze plugin gebruikt GRPC voor hoogwaardige datastreaming. De API-server moet worden ingeschakeld.

Wilt u deze nu inschakelen?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1393"/>
        <source>API Server Required</source>
        <translation>API-server Vereist</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1422"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Plugin "%1" heeft geen 'entry'-veld in info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Entry point not found:
%1</source>
        <translation>Ingangspunt niet gevonden:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Plugin "%1" heeft een ongeldig ingangspuntpad.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1484"/>
        <source>Missing Dependency</source>
        <translation>Ontbrekende Afhankelijkheid</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Deze plugin vereist "%1" maar deze is niet gevonden op uw systeem.

Wilt u de downloadpagina openen?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>Plugins hebben de API-server nodig om te communiceren met Serial Studio. Wilt u deze nu inschakelen?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Herstart Vereist</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>De nieuwe rendering-backend wordt actief na het herstarten van Serial Studio. Nu herstarten om de wijziging toe te passen?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>Laden van pagina mislukt: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="148"/>
        <source>Invalid icon identifier</source>
        <translation>Ongeldige pictogram-identificatie</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="218"/>
        <source>Empty SVG data received</source>
        <translation>Lege SVG-gegevens ontvangen</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Windows-snelkoppeling (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>macOS-applicatie (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Bureaubladitem (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Gebruik een .icns-pictogram voor het scherpste resultaat in Finder en het Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Laat het pictogram leeg om het pictogram van het Serial Studio-uitvoerbare bestand over te nemen.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Plaats het bestand onder ~/.local/share/applications/ om het beschikbaar te maken in de applicatiestarter.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Apple Icon Image (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Windows-pictogram (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>VECTOR- of rasterafbeelding (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>Een Pro-licentie is vereist om snelkoppelingen te genereren.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>Geen uitvoerpad opgegeven.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>Schrijven van snelkoppelingbestand mislukt.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Kon de bestaande snelkoppeling op %1 niet vervangen.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Kon de .app-bundelmapstructuur niet aanmaken.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Kon de bundellauncher niet schrijven: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Kon de bundellauncher niet als uitvoerbaar markeren.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Kon Info.plist niet schrijven: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Windows-snelkoppelingsschrijver is niet beschikbaar op dit platform.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Linux-snelkoppelingsschrijver is niet beschikbaar op dit platform.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>Kan COM niet initialiseren (vereist voor het schrijven van .lnk-snelkoppelingen).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) mislukt (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) mislukt (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Opslaan van .lnk-bestand mislukt (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>MacOS-snelkoppelingsschrijver is niet beschikbaar op dit platform.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Kan snelkoppelingspad niet openen voor schrijven: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Serial Studio-snelkoppeling</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Kan snelkoppeling niet als uitvoerbaar markeren.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>Systeem</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">Automatisch controleren op updates?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">Moet %1 automatisch controleren op updates? Controleren op updates kan altijd handmatig vanuit het "Over"-venster</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>Opslaan</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>Alles opslaan</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>Openen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>Ja op alles</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>Nee</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>Nee op alles</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>Afbreken</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>Opnieuw</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>Negeren</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>Verwerpen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>Toepassen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>Standaardinstellingen Herstellen</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>Werkruimtelocatie Selecteren</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>Protocol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="70"/>
        <source>Serial Port</source>
        <translation>Seriële Poort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="93"/>
        <source>Baud Rate</source>
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="184"/>
        <source>Parity</source>
        <translation>Pariteit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="205"/>
        <source>Data Bits</source>
        <translation>Databits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="226"/>
        <source>Stop Bits</source>
        <translation>Stopbits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="247"/>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="257"/>
        <source>IP Address</source>
        <translation>IP-adres</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="271"/>
        <source>Port</source>
        <translation>Poort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="280"/>
        <source>TCP Port</source>
        <translation>TCP-poort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="308"/>
        <source>Slave Address</source>
        <translation>Slave-adres</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="313"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="328"/>
        <source>Poll Interval (ms)</source>
        <translation>Poll-Interval (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="333"/>
        <source>Polling interval</source>
        <translation>Poll-interval</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="361"/>
        <source>Configure Register Groups…</source>
        <translation>Registergroepen Configureren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="371"/>
        <source>Import Register Map…</source>
        <translation>Registerkaart Importeren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="386"/>
        <source>%1 group(s) configured</source>
        <translation>%1 groep(en) geconfigureerd</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="387"/>
        <source>No groups configured</source>
        <translation>Geen groepen geconfigureerd</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Modbus-registergroepen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Configureer meerdere registergroepen om verschillende registertypes achtereenvolgens te pollen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Nieuwe Groep Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Registertype:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Startadres:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Aantal Registers:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Groep Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Geconfigureerde Groepen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>Aantal</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Actie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Geen groepen geconfigureerd.
Voeg hierboven groepen toe om meerdere registertypes te pollen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Totaal aantal groepen: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Project Genereren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Alles Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Voorbeeld Modbus-registerkaart</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>Bestand: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Controleer de registers om te importeren in een nieuw Serial Studio-project.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>Registers</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>Naam</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Adres</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Gegevenstype</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Eenheden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>Geen registers gevonden in bestand.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Totaal: %1 registers in %2 groepen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Project Aanmaken</translation>
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
        <translation>Verbonden met broker</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Niet verbonden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Test Verbinding</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Test de broker met de huidige instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Schakel publiceren eerst in</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Script Bewerken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Bewerk het publisher-script (Lua of JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Ca-certificaten Laden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>Voeg PEM-certificaten toe vanuit een map</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>Schakel SSL/TLS eerst in</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpoleren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="305"/>
        <source>Interpolation: %1</source>
        <translation>Interpolatie: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Toon Legenda's</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="327"/>
        <source>Show X Axis Label</source>
        <translation>Toon X-aslabel</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="338"/>
        <source>Show Y Axis Label</source>
        <translation>Toon Y-aslabel</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="350"/>
        <source>Show Crosshair</source>
        <translation>Toon Kruisdraad</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Resume</source>
        <translation>Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="374"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Sweep- / Triggermodus</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="386"/>
        <source>Trigger Settings</source>
        <translation>Triggerinstellingen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="410"/>
        <source>Reset View</source>
        <translation>Weergave Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="416"/>
        <source>Axis Range Settings</source>
        <translation>Asbereik Instellingen</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Samples</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>Bytes per waarde</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Aantal bytes dat wordt gecombineerd tot elke kanaalwaarde.</translation>
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
        <translation>Bytevolgorde die wordt gebruikt bij het combineren van multi-bytewaarden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>Getekende waarden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Interpreteert elke waarde als two's-complement getekend.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>Tag-routeringstabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Door komma's gescheiden tag:index-vermeldingen, bijv. 1:0,2:1,3:2. Tags kunnen decimaal zijn of met 0x-prefix hexadecimaal.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>Checksum valideren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Weigert berichten met een ongeldige Fletcher-checksum.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Weigert berichten met een ongeldige additieve checksum.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>Protocolversie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Selecteert de verwachte startmarkering (0xFE voor v1, 0xFD voor v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>Valideer CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Weigert frames met een ongeldige CRC-24Q checksum.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>Aantal kanalen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Aantal uitvoerkanalen (registers of coils).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>Register offset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Adres-offset afgetrokken van single-write echo's (40001 voor legacy Modicon-kaarten).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>Signed registers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Interpreteert 16-bit registers als two's-complement signed waarden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>Payload-indeling</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>Array zendt elk element op volgorde uit; map routeert sleutels via de sleutellijst.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>Sleutels (kaartmodus)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Kommagescheiden kaartsleutels in kanaalvolgorde. Alleen gebruikt voor de kaartindeling.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Scalaire velden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Kommagescheiden JSON-velden herhaald in elk gegenereerd frame.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Samplearray-veld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>JSON-veld met de gebatchte samplearray.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Records-array-veld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>JSON-veld met de array van recordobjecten.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Recordvelden (in kanaalvolgorde)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Kommagescheiden recordvelden. De positie van elk veld bepaalt de kanaalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>Kolombreedtes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Door komma's gescheiden tekentellingen per veld. Laat leeg om op witruimte te splitsen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>Witruimte verwijderen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Verwijdert opvulling rond elk gesneden veld.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>Sleutels (in kanaalvolgorde)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Door komma's gescheiden sleutelnamen. De positie van elke sleutel bepaalt de kanaalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>Paarscheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>Teken tussen sleutel=waarde-paren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>Sleutel-waarde-scheidingsteken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>Teken tussen een sleutel en de waarde.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>Alleen numerieke waarden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Negeert paren waarvan de waarde geen getal is.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>Commando-routeringstabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Puntkomma-gescheiden vermeldingen van NAAM:index lijst, bijv. CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>Talker-prefix</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>Tweeletterige talker-id, bijv. GP voor GPS of GN voor multi-constellatie ontvangers.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Weigert zinnen waarvan de *hh checksum niet overeenkomt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Komma-gescheiden parameternamen. De positie van elke sleutel bepaalt zijn kanaalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>Velden (in kanaalvolgorde)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Komma-gescheiden veldnamen. De positie van elk veld bepaalt zijn kanaalindex.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>Tags (in kanaalvolgorde)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Door komma's gescheiden tagnamen. De positie van elke tag bepaalt de kanaalindex.</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">Registerblokken</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">Gepolde registerblokken met getypeerde, geschaalde items. Geschreven door de Modbus-registerkaart-importeur.</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">Signaalkaart</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">CAN-berichten met hun signaal-bitindelingen en schaling. Geschreven door de DBC-importeur.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Sockettype</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Lokale Poort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>Typ 0 voor automatische poort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Extern Adres</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Externe Poort</translation>
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
        <translation>Filteren op kanaal…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Alle meldingen wissen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(geen titel)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Nog geen meldingen</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">Dataset-transformaties en output widget-scripts kunnen hier gebeurtenissen plaatsen via notifyInfo / notifyWarning / notifyCritical.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>Online Pictogrammen Zoeken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>Download mislukt: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Pictogrammen zoeken (bijv. temperatuur, pijl, afspelen)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>Zoeken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>Zoek hierboven naar pictogrammen om te beginnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Uitvoerwidgets vereisen een Pro-licentie.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Knop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Stuur een commando bij klikken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Schuifregelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Stuur geschaalde numerieke waarden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Schakelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Aan/uit-commando's verzenden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Tekstveld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Willekeurige commando's typen en verzenden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Draaiknop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Draai-invoer voor setpoints</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Uitvoerwidgets kunnen worden geconfigureerd, maar verschijnen alleen op het dashboard met een Pro-licentie.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Dupliceren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Deze uitvoerwidget dupliceren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Deze uitvoerwidget verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Verzendfunctie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Importeren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Verzendfunctie importeren vanuit een .js-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Sjabloon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Selecteer een voorgebouwde verzendfunctiesjabloon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Test de verzendfunctie met voorbeeldinvoer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Ongedaan Maken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Opnieuw</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Knippen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Plakken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Alles Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Document Formatteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Selectie Formatteren</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Painter-widget Fout</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Painter-widget Code-editor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Importeren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Importeer painter-code uit een .js-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Sjabloon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Selecteer een ingebouwd schildersjabloon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Formatteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Herformatteer de schildercode</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Test</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Open een live voorvertoning met gesimuleerde datasetwaardes</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Voorvertoning</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Knippen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Plakken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Alles Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Ongedaan Maken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Opnieuw</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Document Formatteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Selectie Formatteren</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Painter Live Voorbeeld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Voorbeeld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Gesimuleerde datasets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>Wachten op eerste frame...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Console wissen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpoleren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="294"/>
        <source>Interpolation: %1</source>
        <translation>Interpolatie: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="307"/>
        <source>Show Area Under Plot</source>
        <translation>Toon Gebied Onder Plot</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="326"/>
        <source>Show X Axis Label</source>
        <translation>Toon X-aslabel</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="337"/>
        <source>Show Y Axis Label</source>
        <translation>Toon Y-aslabel</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="349"/>
        <source>Show Crosshair</source>
        <translation>Toon Kruisdraad</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Resume</source>
        <translation>Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="373"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Sweep- / Triggermodus</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="385"/>
        <source>Trigger Settings</source>
        <translation>Triggerinstellingen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="409"/>
        <source>Reset View</source>
        <translation>Weergave Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="415"/>
        <source>Axis Range Settings</source>
        <translation>Asbereik Instellingen</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="202"/>
        <source>Interpolate</source>
        <translation>Interpoleren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="220"/>
        <source>Orbit Navigation</source>
        <translation>Orbit Navigatie</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="230"/>
        <source>Pan Navigation</source>
        <translation>Pan Navigatie</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="241"/>
        <source>Orthogonal View</source>
        <translation>Orthogonale Weergave</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="247"/>
        <source>Top View</source>
        <translation>Bovenaanzicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="253"/>
        <source>Left View</source>
        <translation>Linkeraanzicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="259"/>
        <source>Front View</source>
        <translation>Vooraanzicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="276"/>
        <source>Auto Center</source>
        <translation>Automatisch Centreren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="292"/>
        <source>Anaglyph 3D</source>
        <translation>Anaglyph 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="306"/>
        <source>Invert Eye Positions</source>
        <translation>Oogposities Omkeren</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="71"/>
        <source>None</source>
        <translation>Geen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="74"/>
        <source>ZOH</source>
        <translation>ZOH</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="77"/>
        <source>Stem</source>
        <translation>Stam</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="79"/>
        <source>Linear</source>
        <translation>Lineair</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1392"/>
        <source>Time</source>
        <translation>Tijd</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1415"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Sleep om te verplaatsen, rechtsklik om te wissen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1418"/>
        <source>Click to place cursor</source>
        <translation>Klik om cursor te plaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1420"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Klik om tweede cursor te plaatsen — Sleep om te verplaatsen</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Bezoek Website</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Licentie Kopen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Activeren</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Assistent — Pro-functie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>De Assistent is een Serial Studio Pro-functie. Activeer uw licentie om deze te ontgrendelen.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Activeren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>Start Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Uitvoerbaar Bestand</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/pad/naar/uitvoerbaar_bestand</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Bladeren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Argumenten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 waarde1 --arg2 waarde2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Werkmap</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(optioneel) /werk/map</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Pipe-pad</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Kies Actief Proces…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Start een onderliggend proces en leg de stdout vast, of maak verbinding met een named pipe die door een bestaand proces wordt geschreven.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Meer informatie over named pipes</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Selecteer Actief Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Selecteer een actief proces om een named-pipe-padsuggestie af te leiden.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Filter Processen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Typ om op naam te filteren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Vernieuwen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Actieve Processen</translation>
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
        <translation>Geen processen komen overeen met het filter.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Geen actieve processen gevonden.
Klik op Vernieuwen om de lijst bij te werken.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 proces(sen)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>gewijzigd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="362"/>
        <source>This project is password protected</source>
        <translation>Dit project is beveiligd met een wachtwoord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="363"/>
        <source>Editing is available in Project mode</source>
        <translation>Bewerken is beschikbaar in Projectmodus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="374"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Voer het wachtwoord in om wijzigingen aan te brengen, of open een ander project.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="375"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Schakel over naar Projectmodus om een project te laden en te bewerken.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="397"/>
        <source>Unlock</source>
        <translation>Ontgrendelen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="398"/>
        <source>Switch to Project Mode</source>
        <translation>Overschakelen naar Projectmodus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="417"/>
        <source>Open Other Project</source>
        <translation>Ander Project Openen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="418"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="434"/>
        <source>Create New Project</source>
        <translation>Nieuw Project Aanmaken</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>Projectstructuur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>Zoeken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>Omhoog Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>Omlaag Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Geselecteerde Dupliceren (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>Dupliceren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>Geselecteerde Verwijderen (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Nieuw</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Maak een nieuw JSON-project</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Openen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Een bestaand JSON-project openen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Het huidige project opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Opslaan Als</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Het huidige project opslaan onder een nieuwe naam</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">Ontgrendelen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Vergrendelen</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">Ontgrendel de Projecteditor met het projectwachtwoord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Importeren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Genereer een project vanuit een Protocol Buffers (.proto) schema</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Stel een wachtwoord in en vergrendel de Projecteditor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Apparaat Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Een nieuwe gegevensbron (apparaat) aan het project toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Actie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Een nieuwe actie aan het project toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Uitvoer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Herstellen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Herstel een recente automatische snapshot van het huidige project</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Voeg een nieuw uitvoercontrolepaneel toe met een knop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Schuifregelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Voeg een uitvoerschuifregelaar toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Schakelaar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Voeg een uitvoerschakelaar toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Draaiknop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Voeg een uitvoerdraaiknop toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Tekstveld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Voeg een uitvoertekstveld toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Knop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Voeg een uitvoerknopbesturing toe</translation>
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
        <translation>Voeg een generieke dataset toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>Voeg een 2D-plotdataset toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>FFT-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Voeg een Fast Fourier Transform-plot toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Meter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Voeg een meterwidget voor numerieke gegevens toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Niveauindicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Voeg een verticale staafniveauindicator toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Voeg een kompaswidget toe voor richtingsgegevens</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>LED-indicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Voeg een LED-statusindicator toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Groep</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Voeg een datasetcontainergroep toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Datasetcontainer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Afbeelding</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Voeg een afbeeldings-/videostream-viewer toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Afbeeldingsweergave</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="450"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Web View</source>
        <translation>Webweergave</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="452"/>
        <source>Add an web viewer</source>
        <translation>Een webviewer toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="462"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="466"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Voeg een aangepaste JavaScript-gerenderde painter-widget toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="467"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Painter-widgets vereisen een Pro-licentie — toevoegen valt terug op een gegevensraster</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="468"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="480"/>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Add a data table view</source>
        <translation>Voeg een datatabelweergave toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Data Grid</source>
        <translation>Gegevensraster</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="491"/>
        <source>Multi-Plot</source>
        <translation>Meervoudige Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="493"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>Voeg een 2D-plot met meerdere signalen toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Multiple Plot</source>
        <translation>Meervoudige Plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="500"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>3D Plot</source>
        <translation>3D-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <source>Add a 3D plot visualization</source>
        <translation>Voeg een 3D-plotvisualisatie toe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="511"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="515"/>
        <source>Accelerometer</source>
        <translation>Accelerometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>Voeg een groep toe voor 3-assige accelerometergegevens</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="521"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="524"/>
        <source>Gyroscope</source>
        <translation>Gyroscoop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>Voeg een groep toe voor 3-assige gyroscoopgegevens</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="530"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="535"/>
        <source>GPS Map</source>
        <translation>GPS-kaart</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="533"/>
        <source>Add a map widget for GPS data</source>
        <translation>Voeg een kaartwidget toe voor GPS-gegevens</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="547"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="551"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="554"/>
        <source>Open the Assistant</source>
        <translation>Open de Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="560"/>
        <source>Help Center</source>
        <translation>Helpcentrum</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="564"/>
        <source>Open the Project Editor documentation</source>
        <translation>Open de documentatie van de Project Editor</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Projectoverzicht</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-functies gedetecteerd in dit project.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Fallback-widgets worden gebruikt. Koop een licentie om volledige functionaliteit te ontgrendelen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Projecttitel:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Naamloos Project</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Punten:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>Tijdbereik:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="198"/>
        <source>Point Count:</source>
        <translation>Aantal Punten:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="252"/>
        <source>Source</source>
        <translation>Bron</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Sources</source>
        <translation>Bronnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="258"/>
        <source>Group</source>
        <translation>Groep</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="259"/>
        <source>Groups</source>
        <translation>Groepen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="264"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="265"/>
        <source>Datasets</source>
        <translation>Datasets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="270"/>
        <source>Action</source>
        <translation>Actie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="271"/>
        <source>Actions</source>
        <translation>Acties</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="359"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Dubbelklik op een blok om het te bewerken. Rechtsklik ergens om een groep, dataset, actie, datatabel of apparaat toe te voegen.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Voorbeeld Protocol Buffers-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Proto-bestand: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Blader door de onderstaande berichten en maak vervolgens het project aan. Elk bericht wordt een dashboardgroep; kanaalblokken met hetzelfde type krijgen een MultiPlot en berichten met gemengde types krijgen een DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Velden weergeven voor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Velden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Veldnaam</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Type</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Geen velden in het geselecteerde bericht.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Totaal: %1 berichten, %2 velden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Project Aanmaken</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Geen fout</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>De MQTT-broker heeft de verbinding geweigerd vanwege een niet-ondersteunde protocolversie. Zorg dat de MQTT-versie overeenkomt met die van de broker en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>De broker heeft de client-ID geweigerd. Deze kan onjuist geformatteerd, te lang of al in gebruik zijn. Genereer een nieuwe en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>Het netwerk heeft de broker bereikt, maar de broker is momenteel niet beschikbaar. Controleer de status en probeer later opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>De gebruikersnaam of wachtwoord is onjuist of ongeldig. Controleer de inloggegevens en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>De broker heeft de verbinding geweigerd wegens onvoldoende rechten. Controleer of het account over de vereiste ACL's beschikt.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>Een netwerk- of transportlaagprobleem heeft de verbinding verhinderd. Controleer connectiviteit, poorten en TLS-configuratie.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>De client heeft een schending van het MQTT-protocol gedetecteerd en de verbinding gesloten. Controleer de compatibiliteit van broker en client.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>Er is een onverwachte fout opgetreden. Controleer de brokerlogs en de applicatieconsole voor details.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>Er is een MQTT 5-protocolfout opgetreden. Raadpleeg de redencode van de broker voor details.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Niet-gespecificeerde MQTT-fout (code %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>Kan welkomsttekst niet laden :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="251"/>
        <source>Network error</source>
        <translation>Netwerkfout</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="254"/>
        <location filename="../../src/Licensing/Trial.cpp" line="270"/>
        <location filename="../../src/Licensing/Trial.cpp" line="302"/>
        <source>Trial Activation Error</source>
        <translation>Fout Bij Activeren van Proefperiode</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>Invalid server response</source>
        <translation>Ongeldig serverantwoord</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="268"/>
        <source>The server returned malformed data: %1</source>
        <translation>De server heeft misvormde gegevens geretourneerd: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>Unexpected server response</source>
        <translation>Onverwacht serverantwoord</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="300"/>
        <source>The server response is missing required fields.</source>
        <translation>Het serverantwoord mist vereiste velden.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Fout Bij Console-uitvoerbestand</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>Kan bestand niet openen voor schrijven!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Ongeldige Bluetooth-adapter!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>Niet-ondersteund platform of besturingssysteem</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>Niet-ondersteunde detectiemethode</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>Algemene I/O-fout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>Frame-parser Uitgeschakeld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>De Lua frame-parser voor bron %1 heeft %2 frames achter elkaar een time-out gegeven en is uitgeschakeld om Serial Studio responsief te houden.

Meest waarschijnlijke oorzaak: een oneindige lus of extreem trage bewerking in het script. Herstel het script en herlaad het project om parsing opnieuw in te schakelen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>Lua Syntaxisfout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>De parsercode bevat een fout:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>Lua Runtime-fout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>De parsercode heeft een fout veroorzaakt:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>Ontbrekende Parse-functie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>De 'parse'-functie is niet gedefinieerd in het script.

Zorg ervoor dat uw code het volgende bevat:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>Runtime-fout in Parse-functie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>De parse-functie bevat een fout:

%1

Corrigeer de fout in de functiebody.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>De JavaScript frame-parser voor bron %1 heeft %2 frames achter elkaar een time-out gegeven en is uitgeschakeld om Serial Studio responsief te houden.

Meest waarschijnlijke oorzaak: een oneindige lus of extreem trage bewerking in het script. Herstel het script en herlaad het project om parsing opnieuw in te schakelen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>Javascript Time-out</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>De parsercode is niet binnen %1 ms geëvalueerd en werd onderbroken.

Meest waarschijnlijke oorzaak: een oneindige lus op het hoogste niveau van het script.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>Javascript-syntaxisfout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>De parsercode bevat een syntaxisfout op regel %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>Javascript-uitzondering Opgetreden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>De parsercode heeft de volgende uitzonderingen veroorzaakt:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>De 'parse'-functie is niet gedefinieerd in het script.

Zorg ervoor dat uw code het volgende bevat:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>De parse-functie bevat een fout op regel %1:

%2

Corrigeer de fout in de functiebody.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>Ongeldige Functiedeclaratie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Geen aanroepbare 'parse'-export gevonden.

Definieer een van de volgende:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>De 'parse'-functie moet minimaal één parameter accepteren (de frame-payload).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Geen geldige 'parse'-functiedeclaratie gevonden.

Verwacht formaat:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>Ongeldige Functieparameter</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">De 'parse'-functie moet minimaal één parameter hebben.

Verwacht formaat:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>Verouderde Functiehandtekening</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>De 'parse'-functie gebruikt het oude twee-parameter formaat: parse(%1, %2)

Dit formaat wordt niet langer ondersteund. Werk bij naar het nieuwe enkel-parameter formaat:
function parse(%1) { ... }

De scheidingsteken-parameter is niet langer nodig.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Critical</source>
        <translation>Kritiek</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Warning</source>
        <translation>Waarschuwing</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="577"/>
        <source>Project file not found</source>
        <translation>Projectbestand niet gevonden</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="578"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Het projectbestand waarnaar deze snelkoppeling verwijst kon niet worden gevonden:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="581"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Wilt u deze snelkoppeling verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="585"/>
        <source>Delete Shortcut</source>
        <translation>Snelkoppeling Verwijderen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="587"/>
        <source>Quit</source>
        <translation>Afsluiten</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>Tijd (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>Freq: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>Tijd: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Geen OpenAI API-sleutel ingesteld. Open Sleutels Beheren om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="234"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Geen Anthropic API-sleutel ingesteld. Open Sleutels Beheren om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Geen Gemini API-sleutel ingesteld. Open Sleutels Beheren om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="351"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Geen lokale modelserver-URL geconfigureerd. Open Sleutels Beheren om er een in te stellen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Geen DeepSeek API-sleutel ingesteld. Open Sleutels Beheren om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Geen Mistral API-sleutel ingesteld. Open Sleutels Beheren om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Geen OpenRouter API-sleutel ingesteld. Open Sleutels Beheren om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Geen Groq API-sleutel ingesteld. Open Sleutels Beheren om er een toe te voegen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1100"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>De frame-parser gebruikt meer dan %1% van de CPU-tijd.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1102"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio laat frames vallen om de applicatie responsief te houden. Vereenvoudig of optimaliseer het frame-parser script om de werklast te verminderen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>Verwacht %1, ontvangen '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Enum-naam verwacht na 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>Oneof-naam verwacht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Veldtag '%1' buiten bereik (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Sleuteltype verwacht in map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Waardetype verwacht in map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>Map-veldnaam verwacht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>Map-veldtag verwacht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>Veldtype verwacht, '%1' ontvangen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>Veldnaam verwacht na type</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>Veldtagnummer verwacht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Berichtnesting te diep (limiet %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>Berichtnaam verwacht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Onverwacht token '%1' op bestandsniveau</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Niet-ondersteund sleutelwoord op hoofdniveau '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Automatisch (Platformstandaard)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Software (Fallback)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Rij %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">Frame %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Decoder</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">Rijen</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 rij(en)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>De native parserconfiguratie is geen geldig JSON-object.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Onbekende native parsersjabloon: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Ingebouwde Parserfout</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Native Parserfout</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append wordt niet ondersteund voor GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Gzip openen voor zowel lezen als schrijven wordt niet ondersteund</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>U kunt een gzip openen voor lezen of voor schrijven. Wat wordt het?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Kon gzopen() niet uitvoeren op bestand</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append wordt niet ondersteund voor QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite wordt niet ondersteund voor QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>ZIP/UNZIP API-fout %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>PDF-rapport Genereren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Rapport Genereren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>Ononderbroken</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>Gestreept</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>Gestippeld</translation>
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
        <translation>Letter (8,5 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="90"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8,5 × 14 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="91"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7,25 × 10,5 in)</translation>
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
        <translation>%1 — Sessierapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="280"/>
        <source>Session Report</source>
        <translation>Sessierapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="188"/>
        <source>Branding</source>
        <translation>Branding</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="194"/>
        <source>Page</source>
        <translation>Pagina</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="200"/>
        <source>Sections</source>
        <translation>Secties</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="248"/>
        <source>Identity</source>
        <translation>Identiteit</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="262"/>
        <source>Company</source>
        <translation>Bedrijf</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="269"/>
        <source>e.g. Acme Test Systems</source>
        <translation>bijv. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="273"/>
        <source>Document title</source>
        <translation>Documenttitel</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="284"/>
        <source>Author</source>
        <translation>Auteur</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="291"/>
        <source>Prepared by (optional)</source>
        <translation>Opgesteld door (optioneel)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="300"/>
        <source>Logo</source>
        <translation>Logo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="313"/>
        <source>File</source>
        <translation>Bestand</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="324"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG of SVG (optioneel)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="326"/>
        <source>Browse…</source>
        <translation>Bladeren…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="329"/>
        <source>Clear</source>
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="370"/>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="382"/>
        <source>Page size</source>
        <translation>Paginaformaat</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="510"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Annoteer min-, max- en gemiddelde waarden op grafieken</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="558"/>
        <source>Export HTML</source>
        <translation>HTML Exporteren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="498"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Voorblad (logo, documenttitel, testondertitel)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="501"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Testinformatie (project, tijdstempels, classificatie en notities)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="504"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Meetsamenvating (min, max, gemiddelde, standaarddeviatie per parameter)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="507"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Parametertrends (tijdreeksgrafiek per numerieke parameter)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="397"/>
        <source>Plot appearance</source>
        <translation>Plotweergave</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="411"/>
        <source>Line width</source>
        <translation>Lijnbreedte</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="443"/>
        <source>Line style</source>
        <translation>Lijnstijl</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="483"/>
        <source>Include</source>
        <translation>Opnemen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="533"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="558"/>
        <source>Export PDF</source>
        <translation>Exporteer PDF</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Rapport Genereren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>Bezig…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Dit kan enkele seconden duren voor sessies met veel parameters. Het venster sluit automatisch wanneer het rapport gereed is.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>Verbinding Verbroken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>Apparaat Niet Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>De verbinding met uw apparaat is verbroken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio kon uw apparaat niet bereiken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Controleer de kabel, voeding en of geen andere toepassing het apparaat heeft overgenomen. U kunt proberen opnieuw te verbinden, een ander apparaat kiezen of afsluiten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Controleer of het is aangesloten, ingeschakeld en niet al in gebruik is door een andere app. U kunt het opnieuw proberen, een ander apparaat kiezen of afsluiten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>Afsluiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>Kies Ander Apparaat</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">Opnieuw Configureren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>Opnieuw Proberen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>Opnieuw Verbinden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Kies het juiste apparaat en druk op Verbinden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>I/o-interface: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="338"/>
        <source>Data Grids</source>
        <translation>Gegevensrasters</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="341"/>
        <source>Multiple Data Plots</source>
        <translation>Meerdere Gegevensgrafieken</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="344"/>
        <source>Accelerometers</source>
        <translation>Accelerometers</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="347"/>
        <source>Gyroscopes</source>
        <translation>Gyroscopen</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="350"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="353"/>
        <source>FFT Plots</source>
        <translation>FFT-grafieken</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="356"/>
        <source>LED Panels</source>
        <translation>LED-panelen</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="359"/>
        <source>Data Plots</source>
        <translation>Gegevensgrafieken</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="362"/>
        <source>Bars</source>
        <translation>Balken</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="365"/>
        <source>Gauges</source>
        <translation>Meters</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="368"/>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="371"/>
        <source>Clock</source>
        <translation>Klok</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="374"/>
        <source>Stopwatch</source>
        <translation>Stopwatch</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="377"/>
        <source>Compasses</source>
        <translation>Kompassen</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="380"/>
        <source>Meters</source>
        <translation>Meters</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Thermometers</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>3D Plots</source>
        <translation>3D Grafieken</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Web Views</source>
        <translation>Webweergaves</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="390"/>
        <source>Image Views</source>
        <translation>Afbeeldingsweergaven</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="393"/>
        <source>Output Panels</source>
        <translation>Uitvoerpanelen</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="396"/>
        <source>Notifications</source>
        <translation>Meldingen</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="399"/>
        <source>Waterfalls</source>
        <translation>Watervallen</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="402"/>
        <source>Painter Widgets</source>
        <translation>Painter-widgets</translation>
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
        <translation>Systeem</translation>
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
        <translation>Sessiedetails</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Selecteer een sessie om details te bekijken.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Project:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Gestart:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Beëindigd:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(bezig)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Frames:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Notities</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Sessienotities toevoegen…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Notities zijn alleen-lezen voor voltooide sessies.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Nieuwe tag…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Ontgrendel het sessiebestand om sessies te verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Tags</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Afspelen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>Exporteer CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Genereer Rapport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Sessies</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Zoeken</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>Datum</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>Frames</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>Tags</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>Geen sessies gevonden.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Geen sessiebestand geopend.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1001"/>
        <source>Select logo image</source>
        <translation>Selecteer logo-afbeelding</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1003"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Afbeeldingen (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="426"/>
        <source>Open Session File</source>
        <translation>Open Sessiebestand</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Session files (*.db)</source>
        <translation>Sessiebestanden (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1200"/>
        <source>Cannot open session file</source>
        <translation>Kan sessiebestand niet openen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="653"/>
        <source>Delete session from %1?</source>
        <translation>Sessie van %1 verwijderen?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>Delete Session</source>
        <translation>Sessie Verwijderen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1061"/>
        <source>No project data</source>
        <translation>Geen projectgegevens</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="654"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Alle metingen en ruwe gegevens voor deze sessie worden permanent verwijderd.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="484"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="493"/>
        <source>Lock Session File</source>
        <translation>Sessiebestand Vergrendelen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="485"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Kies een wachtwoord om het sessiebestand te vergrendelen:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="494"/>
        <source>Confirm the password:</source>
        <translation>Bevestig het wachtwoord:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="502"/>
        <source>Passwords do not match</source>
        <translation>Wachtwoorden komen niet overeen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="503"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>De twee ingevoerde wachtwoorden komen niet overeen. Het sessiebestand is niet vergrendeld.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="539"/>
        <source>Unlock Session File</source>
        <translation>Ontgrendel Sessiebestand</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="540"/>
        <source>Enter the session file password:</source>
        <translation>Voer het sessiebestandwachtwoord in:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="550"/>
        <source>Incorrect password</source>
        <translation>Onjuist wachtwoord</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="551"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>Het ingevoerde wachtwoord komt niet overeen met het wachtwoord in het sessiebestand.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="643"/>
        <source>Session file locked</source>
        <translation>Sessiebestand vergrendeld</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="644"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Ontgrendel het sessiebestand voordat opgenomen sessies worden verwijderd.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">Deze sessie bevat geen ingesloten projectbestand — het dashboard valt terug op een snelle-plot-indeling.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>Export Session to CSV</source>
        <translation>Exporteer Sessie naar CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="778"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-bestanden (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">Export Voltooid</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">Sessie geëxporteerd naar:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">Export voorbereiden…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Done</source>
        <translation>Voltooid</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="937"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="973"/>
        <source>Failed</source>
        <translation>Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="943"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="983"/>
        <source>Report Failed</source>
        <translation>Rapport Mislukt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="945"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="984"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Kan het rapport niet genereren. Controleer het uitvoerpad en probeer opnieuw.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save PDF Report</source>
        <translation>PDF-rapport Opslaan</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="853"/>
        <source>Loading session data…</source>
        <translation>Sessiegegevens laden…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="873"/>
        <source>Save HTML Report</source>
        <translation>HTML-rapport Opslaan</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>PDF files (*.pdf)</source>
        <translation>PDF-bestanden (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="874"/>
        <source>HTML files (*.html)</source>
        <translation>HTML-bestanden (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1062"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Dit sessiebestand bevat geen ingesloten project.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1071"/>
        <source>Invalid project data</source>
        <translation>Ongeldige projectgegevens</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1072"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>De ingesloten project-JSON is onjuist geformatteerd en kan niet worden hersteld.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Restore Project</source>
        <translation>Project Herstellen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1082"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Serial Studio-projecten (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Cannot write file</source>
        <translation>Kan bestand niet schrijven</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1090"/>
        <source>Check file permissions and try again.</source>
        <translation>Controleer bestandsrechten en probeer opnieuw.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="76"/>
        <source>Empty file path</source>
        <translation>Leeg bestandspad</translation>
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
        <translation>Database niet geopend</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="262"/>
        <source>Database not open or empty label</source>
        <translation>Database niet geopend of leeg label</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="330"/>
        <source>Invalid label</source>
        <translation>Ongeldig label</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="597"/>
        <source>Cancelled</source>
        <translation>Geannuleerd</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="710"/>
        <source>Could not load session data</source>
        <translation>Kon sessiegegevens niet laden</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>Rapport samenstellen…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>Uitvoer schrijven…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>Sessierapport</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>Naamloos project</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>Opgesteld door</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>Gegenereerd op %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>Test-ID</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>Duur</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>Samples</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>Parameters</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>Gestart</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>Beëindigd</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>Project</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>Test-ID</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>Starttijd</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>Eindtijd</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>Totale Duur</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>Samples Verkregen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>Parameters Gelogd</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>Classificatie</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>Notities</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>Testinformatie</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>Parameter</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>Eenheden</translation>
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
        <translation>Gemiddelde</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>Standaarddeviatie</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>Meetoverzicht</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>klik op een kolom om te sorteren</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 samples over %2 seconden</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>Gecombineerde Parameterweergave</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>klik op legenda-items om signalen te schakelen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>Parametertrends</translation>
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
        <translation>Pagina %1 van %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>Rendering-engine laden…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>Grafieken renderen…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>PDF Genereren…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>Open Sessiebestand</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>Sessiebestanden (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>Apparaatverbinding Actief</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Om deze functie te gebruiken moet de verbinding met het apparaat worden verbroken. Doorgaan?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>Kan sessiebestand niet openen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>Onbekende fout</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>Geen projectgegevens</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Deze sessie bevat geen ingesloten projectbestand — het dashboard valt terug op een snelle-plot-indeling.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>Controleer bestandsrechten en probeer opnieuw.</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">Geen sessies gevonden</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">Dit bestand bevat geen opgenomen sessies.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">Sessie heeft geen kolommen</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">De geselecteerde sessie mist kolomdefinities.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">Sessie heeft geen metingen</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">De geselecteerde sessie bevat geen frames.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Leeg bestandspad</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Dit bestand bevat geen opgenomen sessies.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Geannuleerd</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>De geselecteerde sessie mist zijn kolomdefinities.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>De geselecteerde sessie bevat geen frames.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Voorkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="166"/>
        <source>Language</source>
        <translation>Taal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="182"/>
        <source>Theme</source>
        <translation>Thema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="217"/>
        <source>Workspace Folder</source>
        <translation>Werkruimtemap</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="280"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>API-server Inschakelen (Poort 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="529"/>
        <source>Automatically Check for Updates</source>
        <translation>Automatisch Controleren op Updates</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="346"/>
        <source>Export…</source>
        <translation>Exporteren…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="593"/>
        <source>Data Plotting</source>
        <translation>Gegevensweergave</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="660"/>
        <source>Point Count</source>
        <translation>Aantal Punten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="685"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Ui-verversingsfrequentie (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="816"/>
        <source>Show Actions Panel</source>
        <translation>Actiepaneel Tonen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="979"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Taakbalkknoppen Altijd Tonen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="85"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1091"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="151"/>
        <source>Appearance</source>
        <translation>Uiterlijk</translation>
    </message>
    <message>
        <source>Files &amp; Updates</source>
        <translation type="vanished">Bestanden &amp; Updates</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="248"/>
        <source>Advanced</source>
        <translation>Geavanceerd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="298"/>
        <source>Allow External API Connections</source>
        <translation>Externe API-verbindingen Toestaan</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="263"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Werkbalk Automatisch Verbergen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <source>Taskbar</source>
        <translation>Taakbalk</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="414"/>
        <source>Rendering Backend</source>
        <translation>Rendering-backend</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="314"/>
        <source>API Access Token</source>
        <translation>API-toegangstoken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Startup</source>
        <translation>Opstarten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="201"/>
        <source>Files</source>
        <translation>Bestanden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="396"/>
        <source>Graphics</source>
        <translation>Grafisch</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="452"/>
        <source>System</source>
        <translation>Systeem</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="468"/>
        <source>Apply Performance Hints</source>
        <translation>Prestatietips Toepassen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="482"/>
        <source>Keep Display Awake</source>
        <translation>Scherm Actief Houden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="502"/>
        <source>Performance hints raise process priority and opt out of OS power throttling. Changes take effect the next time Serial Studio starts.</source>
        <translation>Prestatietips verhogen de procesprioriteit en schakelen OS-energiebeperking uit. Wijzigingen worden van kracht bij de volgende start van Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="513"/>
        <source>Updates &amp; News</source>
        <translation>Updates &amp; Nieuws</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="543"/>
        <source>Show What's New on Startup</source>
        <translation>Wat Is Nieuw Tonen bij Opstarten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="608"/>
        <source>Time Range</source>
        <translation>Tijdsbereik</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="747"/>
        <source>Small</source>
        <translation>Klein</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="747"/>
        <source>Normal</source>
        <translation>Normaal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="747"/>
        <source>Large</source>
        <translation>Groot</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="747"/>
        <source>Extra Large</source>
        <translation>Extra Groot</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="747"/>
        <source>Custom</source>
        <translation>Aangepast</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="801"/>
        <source>Layout</source>
        <translation>Indeling</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="832"/>
        <source>Auto-Layout Margin</source>
        <translation>Marge Automatische Indeling</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="857"/>
        <source>Auto-Layout Spacing</source>
        <translation>Afstand Automatische Indeling</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="889"/>
        <source>Video Export</source>
        <translation>Video-export</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="907"/>
        <source>Save Videos by Default</source>
        <translation>Video's Standaard Opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="958"/>
        <source>Behavior</source>
        <translation>Gedrag</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="994"/>
        <source>Show Search Field</source>
        <translation>Zoekveld Tonen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1009"/>
        <source>Auto-hide Taskbar</source>
        <translation>Taakbalk Automatisch Verbergen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1027"/>
        <source>Hide Delay (ms)</source>
        <translation>Verbergvertraging (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1051"/>
        <source>Pinned Buttons</source>
        <translation>Vastgezette Knoppen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1069"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Sleep een vastgezette knop op de taakbalk om de volgorde te wijzigen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1090"/>
        <source>Settings</source>
        <translation>Instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1093"/>
        <source>Clock</source>
        <translation>Klok</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1094"/>
        <source>Stopwatch</source>
        <translation>Stopwatch</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1095"/>
        <source>Pause / Resume</source>
        <translation>Pauzeren / Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1096"/>
        <source>File Transmission</source>
        <translation>Bestandsoverdracht</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1097"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1226"/>
        <source>Display</source>
        <translation>Weergave</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1241"/>
        <source>Display Mode</source>
        <translation>Weergavemodus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="722"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1254"/>
        <source>Font Family</source>
        <translation>Lettertypefamilie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="92"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1092"/>
        <source>Notifications</source>
        <translation>Meldingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="344"/>
        <source>Export Protobuf File</source>
        <translation>Protobuf-bestand Exporteren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="707"/>
        <source>Dashboard Font</source>
        <translation>Dashboard-lettertype</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="737"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1269"/>
        <source>Font Size</source>
        <translation>Lettergrootte</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">Afbeelding Exporteren</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">Afbeeldingen Standaard Opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1286"/>
        <source>Show Timestamps</source>
        <translation>Tijdstempels Tonen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1305"/>
        <source>Data Transmission</source>
        <translation>Gegevensoverdracht</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1320"/>
        <source>Line Ending</source>
        <translation>Regeleindiging</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1333"/>
        <source>Input Mode</source>
        <translation>Invoermodus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1346"/>
        <source>Text Encoding</source>
        <translation>Tekstcodering</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1359"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1372"/>
        <source>Echo Sent Data</source>
        <translation>Echo Verzonden Gegevens</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1391"/>
        <source>Escape Codes</source>
        <translation>Escape-codes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1406"/>
        <source>VT100 Emulation</source>
        <translation>Vt100-emulatie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1425"/>
        <source>ANSI Colors</source>
        <translation>Ansi-kleuren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1483"/>
        <source>Delivery</source>
        <translation>Aflevering</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1498"/>
        <source>System Notifications</source>
        <translation>Systeemmeldingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1519"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Toon waarschuwingen/kritieke gebeurtenissen als OS-desktopmeldingen wanneer Serial Studio niet het actieve venster is.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1529"/>
        <source>Application Logs</source>
        <translation>Applicatielogboeken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1544"/>
        <source>Route Warnings to Notifications</source>
        <translation>Stuur Waarschuwingen naar Meldingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1565"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Standaard uit — QT en QML geven vaak waarschuwingen en het inschakelen hiervan kan echte alarmen overstemmen. Kritieke berichten worden altijd doorgestuurd, ongeacht deze instelling.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1584"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1624"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1632"/>
        <source>Apply</source>
        <translation>Toepassen</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Apparaatconfiguratie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>API-server Actief (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>API-server Gereed</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>API-server Uit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Frame-parsing</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Alleen Console (Geen Parsing)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Snelle Plot (Kommagescheiden Waarden)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Parseren via Projectbestand</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Wijzig Projectbestand (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Selecteer Projectbestand</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Data-export</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>CSV-spreadsheet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Sessie-opname</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>MDF4-opname</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Consolelog</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">CSV-bestand</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">Sessiedatabase</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">MDF4-bestand</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">Console-dump</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">Maak CSV-bestand</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">Maak MDF4-bestand</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">Maak Sessielogboek</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">Exporteer Consolegegevens</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>I/o-interface: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Multi-apparaat Project</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Dit project streamt gegevens van %1 onafhankelijke apparaten.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Elk apparaat heeft zijn eigen verbindingsinstellingen. Configureer deze in de Projecteditor onder het tabblad Bronnen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Projecteditor Openen</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">Nieuwe Snelkoppeling</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>Kies een Pictogram</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">Snelkoppeling Opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>Nieuwe Implementatie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="109"/>
        <source>Save Deployment</source>
        <translation>Implementatie Opslaan</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>General</source>
        <translation>Algemeen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Taskbar</source>
        <translation>Taakbalk</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="164"/>
        <source>Logging</source>
        <translation>Logboekregistratie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="221"/>
        <source>Identity</source>
        <translation>Identiteit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="277"/>
        <source>Click to choose an icon</source>
        <translation>Klik om een pictogram te kiezen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="286"/>
        <source>Name:</source>
        <translation>Naam:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="295"/>
        <source>Deployment Name</source>
        <translation>Implementatienaam</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="386"/>
        <source>Theme</source>
        <translation>Thema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="396"/>
        <source>Same as Serial Studio</source>
        <translation>Hetzelfde als Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="418"/>
        <source>Actions Panel</source>
        <translation>Actiepaneel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="429"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="638"/>
        <source>File Transmission</source>
        <translation>Bestandsoverdracht</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="445"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Dubbelklikken op deze implementatie brengt iemand direct naar het live dashboard voor dit project. Er is geen werkbalk of instellingenpaneel, alleen de gegevens, en Serial Studio sluit zodra het apparaat de verbinding verbreekt.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="491"/>
        <source>Visibility</source>
        <translation>Zichtbaarheid</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="506"/>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="515"/>
        <source>Always shown</source>
        <translation>Altijd Zichtbaar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="516"/>
        <source>Auto-hide</source>
        <translation>Automatisch Verbergen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="517"/>
        <source>Hidden</source>
        <translation>Verborgen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Het verbergen van de taakbalk verwijdert de knoppen voor minimaliseren/maximaliseren/sluiten en forceert automatische indeling, zodat het dashboard altijd het beschikbare gebied vult.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="536"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>De taakbalk schuift naar binnen wanneer de cursor naar de onderrand wordt bewogen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="538"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>De taakbalk is permanent zichtbaar onderaan het dashboard.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="551"/>
        <source>Pinned Buttons</source>
        <translation>Vastgezette Knoppen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="568"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="582"/>
        <source>Notifications</source>
        <translation>Meldingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="596"/>
        <source>Clock</source>
        <translation>Klok</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="610"/>
        <source>Stopwatch</source>
        <translation>Stopwatch</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="624"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="749"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Opnames worden opgeslagen in de Serial Studio-werkruimtemap</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">Snelkoppeling Naam</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="304"/>
        <source>Change Icon…</source>
        <translation>Pictogram Wijzigen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="321"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="339"/>
        <source>Project</source>
        <translation>Project</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="349"/>
        <source>Choose a project file to begin</source>
        <translation>Kies een projectbestand om te beginnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="371"/>
        <source>Behavior</source>
        <translation>Gedrag</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="408"/>
        <source>Fullscreen</source>
        <translation>Volledig Scherm</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">Dubbelklikken op deze snelkoppeling brengt iemand direct naar het live dashboard voor dit project. Er is geen werkbalk of instellingenpaneel, alleen de gegevens, en Serial Studio sluit zodra het apparaat de verbinding verbreekt.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">Project Insluiten</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">Dubbelklikken op deze snelkoppeling brengt iemand direct naar het live dashboard voor dit project. Er is geen werkbalk of instellingenpaneel, alleen de gegevens, en Serial Studio sluit zodra het apparaat wordt losgekoppeld.

Schakel Project Insluiten in om het project in de snelkoppeling in te bakken, zodat het blijft werken zelfs als het oorspronkelijke bestand wordt verplaatst of verwijderd.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="688"/>
        <source>Recorders</source>
        <translation>Recorders</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="703"/>
        <source>CSV File</source>
        <translation>CSV-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="713"/>
        <source>MDF4 File</source>
        <translation>MDF4-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="723"/>
        <source>Session Database</source>
        <translation>Sessiedatabase</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="733"/>
        <source>Console Log</source>
        <translation>Consolelog</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">Opnames worden opgeslagen op de standaardlocatie van elke module.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="778"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="787"/>
        <source>Save</source>
        <translation>Opslaan</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>Ongedaan Maken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>Opnieuw</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>Knippen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>Plakken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>Alles Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>Document Formatteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>Selectie Formatteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>Valideren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Controleer of het script correct compileert</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">Sjabloonparameters terugzetten naar standaardwaarden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>Resetten naar het standaard parsing-script</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>Openen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>Importeer een scriptbestand voor data-parsing</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Open hulpdocumentatie voor data-parsing</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Taal:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Native</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>Selecteer Sjabloon…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>Maak de laatste codebewerking ongedaan</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>Voer de eerder ongedaan gemaakte bewerking opnieuw uit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>Knip geselecteerde code naar klembord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>Kopieer geselecteerde code naar klembord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>Plak code van klembord</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>Platform:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>Ingebouwd</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">Sjabloon:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>Test met Voorbeeldgegevens</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Evalueren</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Dupliceren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Maak een kopie van deze gegevensbron</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Verwijder deze gegevensbron</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>De primaire gegevensbron kan niet worden verwijderd</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>Sessiespeler</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>Sessie laden…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="278"/>
        <source>Workspaces</source>
        <translation>Werkruimten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="374"/>
        <source>No Workspaces Available</source>
        <translation>Geen Werkruimten Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="408"/>
        <source>Actions</source>
        <translation>Acties</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="431"/>
        <source>No Actions Available</source>
        <translation>Geen Acties Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="461"/>
        <source>Plugins</source>
        <translation>Plugins</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="359"/>
        <source>New Workspace…</source>
        <translation>Nieuwe Werkruimte…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="499"/>
        <source>Manage Plugins…</source>
        <translation>Plugins Beheren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="509"/>
        <source>No Plugins Installed</source>
        <translation>Geen Plugins Geïnstalleerd</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="101"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="545"/>
        <source>Auto Layout</source>
        <translation>Automatische Indeling</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="109"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="557"/>
        <source>Full Screen</source>
        <translation>Volledig Scherm</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="115"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="569"/>
        <source>Add External Window</source>
        <translation>Extern Venster Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="135"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="758"/>
        <source>Clock</source>
        <translation>Klok</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="143"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="765"/>
        <source>Stopwatch</source>
        <translation>Stopwatch</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="163"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="782"/>
        <source>Sessions</source>
        <translation>Sessies</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="170"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="791"/>
        <source>File Transmission</source>
        <translation>Bestandsoverdracht</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="177"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="799"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="345"/>
        <source>Show "%1"</source>
        <translation>Toon "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="350"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Toon Alle Verborgen Werkruimten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="589"/>
        <source>Export</source>
        <translation>Exporteren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="620"/>
        <source>CSV File</source>
        <translation>CSV-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="626"/>
        <source>MDF4 File</source>
        <translation>MDF4-bestand</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="632"/>
        <source>Console Transcript</source>
        <translation>Console-transcript</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="641"/>
        <source>Session Database</source>
        <translation>Sessiedatabase</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="655"/>
        <source>No Export Formats Available</source>
        <translation>Geen Exportformaten Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="685"/>
        <source>Tools</source>
        <translation>Tools</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="814"/>
        <source>No Tools Available</source>
        <translation>Geen Tools Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="882"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="903"/>
        <source>Quit</source>
        <translation>Afsluiten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="929"/>
        <source>Edit…</source>
        <translation>Bewerken…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="121"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="741"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="127"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="750"/>
        <source>Notifications</source>
        <translation>Meldingen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="151"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="773"/>
        <source>Preferences</source>
        <translation>Voorkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="157"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="854"/>
        <source>Help Center</source>
        <translation>Helpcentrum</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="941"/>
        <source>Hide</source>
        <translation>Verbergen</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Resume</source>
        <translation>Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="876"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="903"/>
        <source>Disconnect</source>
        <translation>Verbinding Verbreken</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Stop</translation>
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
        <translation>Rondetijd</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Resetten</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Totaal</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Geen rondetijden opgenomen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Druk op Rondetijd terwijl de stopwatch loopt</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>Geen Gegevens Beschikbaar</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Datasetwaarden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Zoeken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Groep</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Eenheden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(virtueel)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Toegangscode %1 naar klembord kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Dataset-toegangscode gekopieerd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>Geen datasets gedefinieerd in dit project.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Geen datasets komen overeen met uw zoekopdracht.</translation>
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
        <translation>Waarde</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Aangepast Pictogram)</translation>
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
        <translation>Nee</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Startmenu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Menu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Zoeken…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Instellingen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Meldingen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Klok</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Stopwatch</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Hervatten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Verbonden met %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Niet verbonden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>Verbonden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>Niet Verbonden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1000"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Mode:</source>
        <translation>Modus:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1026"/>
        <source>Messages sent:</source>
        <translation>Verzonden berichten:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>MQTT-instellingen Openen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Bestandsoverdracht</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">Widgets zoeken…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Verwijderen uit Werkruimte</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Alles Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Wissen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Gegevens naar Apparaat Verzenden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Tijdstempel Tonen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Echo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>VT-100 Emuleren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Ansi-kleuren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Weergave: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">Wist Je Dat?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">Houd je firmware eenvoudig door ruwe data te verzenden en Serial Studio deze te laten verwerken in JavaScript, Lua, of codevrije Ingebouwde sjablonen.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">Geef elk kanaal zijn eigen functie om te kalibreren, filteren of eenheden om te zetten. Verplaats de berekeningen naar Serial Studio en houd je firmware slank.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">Heb je een waarde nodig die je apparaat nooit verstuurt? Een virtuele dataset berekent zijn eigen kanaal, zoals vermogen uit spanning en stroom, weergegeven en gelogd als data.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">Vang storingen zoals een oscilloscoop. Tijdas-grafieken hebben een sweep- en triggermodus, en je kunt het triggerniveau direct op de grafiek slepen.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">Stop met scrollen om de juiste widget te vinden. Groepeer ze in je eigen werkruimtes en spring ertussen via de taakbalk-zoekfunctie.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">Verlies nooit meer een testrun. Neem sessies op in een lokale database, blader, tag en speel ze af wanneer je ze nodig hebt.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">Lever in seconden een verzorgd rapport aan je team. Exporteer elke sessie naar HTML of PDF, compleet met grafieken en min/max/gemiddelde statistieken.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">Sluit de lus zonder extra gereedschap. Uitvoerbesturingen laten je commando's rechtstreeks vanuit het dashboard naar je apparaat sturen.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">Bouw een visualisatie die niemand anders heeft. De Painter-widget voert je eigen script uit om volledig aangepaste graphics te tekenen vanuit binnenkomende data.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">Eén tool voor elke verbinding. Serial Studio leest van UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT en Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">Sla de terminal-dans over. Verzend en ontvang bestanden via je seriële verbinding met de ingebouwde XMODEM-, YMODEM- en ZMODEM-protocollen.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">Heb je al een Modbus-registerkaart of een DBC-bestand? Genereer er automatisch een kant-en-klaar project van in plaats van er handmatig een te bouwen.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">Beschrijf wat je wilt en laat de AI-assistent het bouwen. Het kan projecten voor je maken en bewerken via acht modelproviders.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">Tip %1 van %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">Meer Informatie</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">Toon Tips bij Opstarten</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">Vorige</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">Volgende</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Sluiten</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>Wacht op goedkeuring</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Voltooid</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Fout</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Geweigerd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Geblokkeerd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>Actief</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Goedkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Weigeren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>Argumenten</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Alles Kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Alles Selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Resultaat</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Projecteditor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Open de Projecteditor om uw JSON-layout te maken of te wijzigen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Project Openen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Een bestaand JSON-project openen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>CSV Openen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>Een CSV-bestand afspelen alsof het live sensorgegevens zijn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>MDF4 Openen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Een MDF4-bestand afspelen alsof het live sensorgegevens zijn (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Sessies</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Opgenomen sessies bladeren, afspelen en exporteren</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">Snelkoppelingen</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">Maak een operatorsnelkoppeling voor het huidige project</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Extensies</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Extensies bladeren en installeren</translation>
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
        <translation>Chat met een AI om je project te bouwen en bewerken</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">MQTT-verbinding configureren (publiceren of abonneren)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Bouw een operator-app voor het huidige project</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Voorkeuren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Applicatie-instellingen en voorkeuren openen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Seriële poort (UART) communicatie selecteren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Audio-invoerapparaat selecteren (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Raw USB-communicatie selecteren (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Netwerk</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>Selecteer TCP/UDP-netwerkcommunicatie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Blader door voorbeeldprojecten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Helpcentrum</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Blader door documentatie, FAQ en wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>Selecteer Modbus-communicatie (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Implementeren</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">Bouw een operatorimplementatie voor het huidige project</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>Selecteer HID-apparaatcommunicatie (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Selecteer Bluetooth Low Energy-communicatie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>CAN-bus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>Selecteer CAN-bus-communicatie (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Selecteer proces-pipe-communicatie (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>Over</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Toon applicatie-informatie en licentiedetails</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Voorbeelden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI Wiki &amp; Chat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Bekijk gedetailleerde documentatie en stel vragen op DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Activeren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Beheer licentie en activeer Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Verbinding Verbreken</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Verbind of verbreek verbinding met het geconfigureerde apparaat</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">Verbind of verbreek verbinding met apparaat of MQTT-broker</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Triggerinstellingen</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Houd de golfvorm stil door elke sweep uit te lijnen op een triggergebeurtenis.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Vergrendel een herhalend signaal op zijn plaats, zoals de Auto-knop op een oscilloscoop. Elke sweep begint op hetzelfde punt van de golfvorm, zodat deze stilstaat in plaats van voorbij te scrollen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>Trigger</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">Modus:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>Normaal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Enkel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto loopt vrij door wanneer niets het niveau kruist.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normaal wacht op een kruising.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Enkel legt één sweep vast en stopt dan.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Slope:</source>
        <translation>Helling:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="273"/>
        <source>Trigger on a downward crossing</source>
        <translation>Trigger op een neerwaartse kruising</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="319"/>
        <source>Timebase:</source>
        <translation>Tijdbasis:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="388"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Laat tijdbasis leeg om het tijdsbereik van de plot te gebruiken; verlaag om in te zoomen op een snel signaal. Holdoff negeert nieuwe triggers even na elke trigger.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Signaal:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Te kruisen waarde</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Flank:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="254"/>
        <source>Rising</source>
        <translation>Stijgend</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="258"/>
        <source>Trigger on an upward crossing</source>
        <translation>Trigger op een opwaartse kruising</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="269"/>
        <source>Falling</source>
        <translation>Dalend</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Een nieuwe sweep begint telkens wanneer het signaal het niveau in de gekozen richting kruist. Auto loopt ook vrij door wanneer geen kruising wordt gevonden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="292"/>
        <source>Timing</source>
        <translation>Timing</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Tijdbasis (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="332"/>
        <source>Match time range</source>
        <translation>Tijdbereik matchen</translation>
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
        <translation>Holdoff:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="364"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">Tijdbasis stelt in hoeveel tijd één sweep toont; laat leeg om het tijdsbereik van de plot te gebruiken. Verlaag om in te zoomen op een snel signaal. Holdoff negeert nieuwe triggers even na elke trigger.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="403"/>
        <source>Capture Next</source>
        <translation>Volgende Vastleggen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="405"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Activeren voor Nog Eén Eenmalige Vastlegging</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Niveau:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Triggerniveau</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Holdoff (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Holdoff-tijd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Bron:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Opnieuw Activeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="418"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>Com-poort</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="79"/>
        <source>Baud Rate</source>
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="163"/>
        <source>Data Bits</source>
        <translation>Databits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="184"/>
        <source>Parity</source>
        <translation>Pariteit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="205"/>
        <source>Stop Bits</source>
        <translation>Stopbits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="226"/>
        <source>Flow Control</source>
        <translation>Stroomregeling</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="258"/>
        <source>Auto Reconnect</source>
        <translation>Automatisch Opnieuw Verbinden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="276"/>
        <source>Send DTR Signal</source>
        <translation>Dtr-signaal Verzenden</translation>
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
        <translation>kritiek</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>waarschuwing</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Waarde %1%2 is de %3-band binnengegaan (%4–%5).</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="205"/>
        <source>Alarms</source>
        <translation>Alarmen</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1717"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1725"/>
        <source>Notifications</source>
        <translation>Meldingen</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1733"/>
        <source>Clock</source>
        <translation>Klok</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1740"/>
        <source>Stopwatch</source>
        <translation>Stopwatch</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1786"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1801"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Terugval)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1823"/>
        <source>LED Panel (%1)</source>
        <translation>LED-paneel (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="161"/>
        <source>Invalid</source>
        <translation>Ongeldig</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1069"/>
        <source>Select Background Image</source>
        <translation>Achtergrondafbeelding Selecteren</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1071"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Afbeeldingen (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>USB-apparaat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Overdrachtsmodus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Bulkstroom</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Geavanceerd (Bulk + Control)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Isochroon</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Maak verbinding met USB-apparaten via bulk-, control- of isochrone overdrachten. Geschikt voor dataloggers, apparaten met aangepaste firmware en USB-instrumenten.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>USB-specificaties (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>In-eindpunt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>Out-eindpunt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>Maximale Pakketgrootte</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Besturingsoverdrachten Ingeschakeld</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>Het verzenden van onjuiste besturingsverzoeken kan aangesloten hardware laten crashen of beschadigen. Gebruik met voorzichtigheid.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Meer informatie over USB-besturingsoverdrachten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>Pakketgrootte moet overeenkomen met de maximale overdrachtsgrootte gerapporteerd door het eindpunt. Typische waarden: 192 B (FS audio), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Wilt u de update nu downloaden?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Wilt u de update nu downloaden?&lt;br /&gt;Dit is een verplichte update; nu afsluiten sluit de applicatie.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Wijzigingslogboek:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>Versie %1 van %2 is uitgebracht!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Er zijn momenteel geen updates beschikbaar</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Gefeliciteerd! U gebruikt de nieuwste versie van %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add Register</source>
        <translation>Register Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="172"/>
        <source>Add register</source>
        <translation>Register toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert Constant</source>
        <translation>Constante Invoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="182"/>
        <source>Insert constant</source>
        <translation>Constante invoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import</source>
        <translation>Importeren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="192"/>
        <source>Import registers from CSV</source>
        <translation>Registers importeren uit CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export</source>
        <translation>Exporteren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="202"/>
        <source>Export registers to CSV</source>
        <translation>Registers exporteren naar CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename</source>
        <translation>Hernoemen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="217"/>
        <source>Rename table</source>
        <translation>Tabel hernoemen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="227"/>
        <source>Delete table</source>
        <translation>Tabel verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="241"/>
        <source>Help</source>
        <translation>Help</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="246"/>
        <source>Open help documentation for shared memory</source>
        <translation>Helpdocumentatie voor gedeeld geheugen openen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="286"/>
        <source>Permissions</source>
        <translation>Machtigingen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="287"/>
        <source>Register Name</source>
        <translation>Registernaam</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="288"/>
        <source>Default Value</source>
        <translation>Standaardwaarde</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="327"/>
        <source>Read-Only</source>
        <translation>Alleen-lezen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="327"/>
        <source>Read/Write</source>
        <translation>Lezen/schrijven</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="465"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Toegangscode %1 naar klembord kopiëren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="498"/>
        <source>Delete register</source>
        <translation>Register Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="515"/>
        <source>No registers.</source>
        <translation>Geen registers.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="565"/>
        <source>Register access code copied</source>
        <translation>Register-toegangscode gekopieerd</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="237"/>
        <source>Show Colorbar</source>
        <translation>Toon Kleurenbalk</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="250"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Toon Assen &amp; Raster</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="263"/>
        <source>Show Crosshair</source>
        <translation>Toon Kruisdraad</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Pause</source>
        <translation>Pauzeren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Resume</source>
        <translation>Hervatten</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">Geschiedenis Wissen</translation>
    </message>
</context>
<context>
    <name>WebView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="70"/>
        <source>Web View Unavailable</source>
        <translation>Webweergave Niet Beschikbaar</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="81"/>
        <source>This build of Serial Studio was compiled without Qt WebEngine, so web pages cannot be displayed.</source>
        <translation>Deze build van Serial Studio is gecompileerd zonder QT WebEngine, waardoor webpagina's niet kunnen worden weergegeven.</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Welkom bij %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio is een krachtige real-time visualisatietool, gebouwd voor ingenieurs, studenten en makers.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>U kunt een volledig functionele proefperiode van 14 dagen starten, activeren met uw licentiesleutel, of de GPLv3-broncode zelf downloaden en compileren.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Aankoop van Pro ondersteunt de auteur rechtstreeks en helpt toekomstige ontwikkeling te financieren.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Zelf de GPLv3-versie bouwen helpt de community te laten groeien en moedigt technische bijdragen aan.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Even geduld…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>Nog %1 dagen in uw proefperiode.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">U gebruikt momenteel de volledig functionele proefversie van %1 Pro. Deze is 14 dagen geldig voor persoonlijk, niet-commercieel gebruik.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>U gebruikt momenteel de volledig functionele proefversie van %1 Pro. Deze is 14 dagen geldig voor persoonlijk, niet-commercieel gebruik.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Upgrade naar een betaald abonnement om Serial Studio Pro te blijven gebruiken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Of compileer de GPLv3-broncode om het gratis te gebruiken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Klik hieronder op "Nu Upgraden" om de beschikbare abonnementen te bekijken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Herinner me niet meer aan de proefperiode.
Ik begrijp dat ik bij het einde een licentie moet kopen of de GPLv3-versie moet bouwen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>Uw %1-proefperiode is verlopen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Uw proefperiode is afgelopen. Upgrade naar een betaald abonnement om %1 met alle Pro-functies te blijven gebruiken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>U kunt ook de open-sourceversie onder de GPLv3-licentie compileren.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Bedankt voor het uitproberen van %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Nu Upgraden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Activeren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>Openen in Beperkte Modus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>Doorgaan</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>Proefperiode Starten</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>Wat is er Nieuw in %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua &amp; Ingebouwde Parsers</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>Parseer frames in Lua 5.4 of met codevrije Ingebouwde sjablonen, naast JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>Ai-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>Een in-app assistent met acht providers die projecten voor u kan bouwen en bewerken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Device Control Loops</source>
        <translation>Apparaatbesturingslussen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Automate your device with an Arduino-style setup() and loop() routine that can read, write, and drive the dashboard.</source>
        <translation>Automatiseer het apparaat met een Arduino-stijl setup() en loop() routine die het dashboard kan lezen, schrijven en aansturen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>Oscilloscoop Sweep &amp; Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>Oscilloscoop-stijl sweep met een geanimeerde trigger-cursor die u op de plot kunt slepen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Output Controls</source>
        <translation>Uitvoerbesturingen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>Verzend terug naar uw apparaat met besturingswidgets en een protocolhelper-engine.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Dashboard Workspaces</source>
        <translation>Dashboard Werkruimten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>Groepeer widgets in uw eigen dashboards en vind ze via de taakbalkzoekfunctie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Session Database &amp; Reports</source>
        <translation>Sessiedatabase &amp; Rapporten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>Neem sessies op naar SQLITE, speel ze af en exporteer HTML- of PDF-rapporten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>Operator Deployments</source>
        <translation>Operator-implementaties</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>Lever een vergrendelde, single-purpose build aan operators met een runtime-only modus.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>New Dashboard Widgets</source>
        <translation>Nieuwe Dashboard Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>Gauge- en Meter-wijzerplaten met live uitlezingen, plus Klok, Stopwatch en Waterfall.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Dataset Transforms</source>
        <translation>Dataset Transformaties</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>Kalibreer, filter en converteer elk kanaal met een functie per dataset, of voeg virtuele datasets toe die nieuwe kanalen berekenen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>Teken volledig aangepaste graphics van binnenkomende gegevens met je eigen tekenscript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Data Tables</source>
        <translation>Gegevenstabellen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>Live register-stijl tabellen met virtuele datasets en bewerkbare cellen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Image Widget</source>
        <translation>Afbeelding Widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>Decodeer en toon afbeeldingsframes die rechtstreeks vanaf je apparaat worden gestreamd.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Notifications &amp; Alarms</source>
        <translation>Meldingen &amp; Alarmen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>Multi-band dataset alarmen met ernstniveaus, doorgestuurd naar het Meldingencentrum.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>Project Lock</source>
        <translation>Projectvergrendeling</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>Vergrendel een project om gebruik door operators te scheiden van bewerken, met een toegangscode.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="193"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf &amp; Bestandsoverdracht</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="194"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>MQTT-invoer en -publicatie, Protobuf-import en XMODEM / YMODEM / ZMODEM-overdrachten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="240"/>
        <source>Welcome to %1!</source>
        <translation>Welkom bij %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="250"/>
        <source>Here's what's new in version %1.</source>
        <translation>Dit is nieuw in versie %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="423"/>
        <source>Show on Startup</source>
        <translation>Tonen bij Opstarten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="430"/>
        <source>Close</source>
        <translation>Sluiten</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Verwijderen uit Werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="345"/>
        <source>Device Disconnected</source>
        <translation>Apparaat Losgekoppeld</translation>
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
        <translation type="vanished">kritiek</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">waarschuwing</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">Waarde %1%2 heeft de %3-band (%4–%5) bereikt.</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Waarde %1%2 heeft het hoge alarm %3%2 bereikt</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Waarde %1%2 heeft het lage alarm %3%2 bereikt</translation>
    </message>
    <message>
        <source>Alarms</source>
        <translation type="vanished">Alarmen</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="166"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="189"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="169"/>
        <source>NE</source>
        <translation>NO</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>E</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>SE</source>
        <translation>ZO</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>S</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="181"/>
        <source>SW</source>
        <translation>ZW</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="184"/>
        <source>W</source>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="187"/>
        <source>NW</source>
        <translation>NW</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>Waarde</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">Wachten op gegevens…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>Satellietbeelden</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Satellietbeelden met Labels</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>Stratenkaart</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>Topografische Kaart</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>Terrein</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>Lichtgrijs Canvas</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>Donkergrijs Canvas</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>Aanvullende kaartlagen zijn alleen beschikbaar voor Pro-gebruikers.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Onbeperkte toegang is niet mogelijk omdat de ArcGIS API-sleutel echte kosten met zich meebrengt.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Time (s)</source>
        <translation>Tijd (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Samples</source>
        <translation>Samples</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Verzendscript time-out na %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>Payload overschrijdt maximale grootte</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="90"/>
        <source>Time (s)</source>
        <translation>Tijd (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="116"/>
        <source>Samples</source>
        <translation>Samples</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="950"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Rasterinterval: %1 eenhe(i)d(en)</translation>
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
        <translation>Grijswaarden</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>Onbekend</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Werkruimte Bewerken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Nieuwe Werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Naam:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>Mijn Werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Selecteer widgets om op te nemen:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Widgets filteren…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Annuleren</translation>
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
        <translation>Werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>Widget Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>Widget toevoegen aan werkruimte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>Hernoemen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>Werkruimte hernoemen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>Verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>Werkruimte verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>Groep</translation>
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
        <translation>Type</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="229"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="267"/>
        <source>(unknown)</source>
        <translation>(onbekend)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(groepswidget)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>Widget uit werkruimte verwijderen</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">Uit werkruimte verwijderen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>Geen widgets in deze werkruimte.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Werkruimten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>Aanpassen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>Werkruimten handmatig bewerken</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>Omhoog Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>Verplaats de geselecteerde werkruimte omhoog</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>Omlaag Verplaatsen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>Verplaats de geselecteerde werkruimte omlaag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>Werkruimte Toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>Werkruimte toevoegen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>Opschonen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>%1 widgetreferentie(s) verwijderen waarvan de doelgroep of dataset niet meer bestaat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>Geen verouderde widgetreferenties in werkruimten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="203"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="204"/>
        <source>Widgets</source>
        <translation>Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="276"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>Geen werkruimten. Voeg er een toe met de werkbalk hierboven, of herstel naar de automatische indeling.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Project heeft geen geschikte groepen -- voeg een groep met widgets toe om werkruimten te vullen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>Herstel naar Automatische Indeling</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">Geen werkruimten.</translation>
    </message>
</context>
</TS>