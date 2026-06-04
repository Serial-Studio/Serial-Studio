<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="167"/>
        <source>Anthropic error</source>
        <translation>Anthropic-Fehler</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="280"/>
        <source>Stream parse error: %1</source>
        <translation>Stream-Parse-Fehler: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="327"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="330"/>
        <source>Invalid API key (%1)</source>
        <translation>Ungültiger API-Schlüssel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="332"/>
        <source>Rate limited: %1</source>
        <translation>Ratenlimit erreicht: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="334"/>
        <source>Anthropic %1: %2</source>
        <translation>Anthropic %1: %2</translation>
    </message>
</context>
<context>
    <name>AI::Assistant</name>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="344"/>
        <source>Switch AI provider?</source>
        <translation>KI-Anbieter wechseln?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="345"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Der Wechsel zu einem anderen Anbieter löscht die aktuelle Konversation. Möchten Sie fortfahren?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="348"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="385"/>
        <source>AI Assistant is not available in this build</source>
        <translation>KI-Assistent ist in diesem Build nicht verfügbar</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">KI-Assistent erfordert eine Pro-Lizenz</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Set an API key first</source>
        <translation>Zuerst einen API-Schlüssel festlegen</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">KI-Assistent erfordert eine Pro-Lizenz</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="165"/>
        <source>AI Assistant is not available in this build</source>
        <translation>KI-Assistent ist in diesem Build nicht verfügbar</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="171"/>
        <source>AI subsystem not initialized</source>
        <translation>KI-Subsystem nicht initialisiert</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="177"/>
        <source>Already busy with a previous request</source>
        <translation>Bereits mit einer vorherigen Anfrage beschäftigt</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="461"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Budget für Tool-Aufrufe in diesem Durchgang erreicht; es werden keine weiteren Tools ausgeführt.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1724"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>Sie haben das Budget für Tool-Aufrufe in diesem Durchgang erreicht. Fordern Sie keine weiteren Tools an. Fassen Sie zusammen, was Sie bisher gefunden haben, und geben Sie an, welche Schritte noch ausstehen, falls die Aufgabe unvollständig ist, damit der Benutzer Ihnen sagen kann, fortzufahren.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Tool-Call-Budget überschritten</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="911"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Das Modell hat eine leere Antwort zurückgegeben. Versuchen Sie, die Anfrage umzuformulieren, zu einem anderen Modell zu wechseln oder zu prüfen, ob die Anfrage von den Sicherheitsfiltern des Anbieters zugelassen wird.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1008"/>
        <source>Sending request to %1...</source>
        <translation>Anfrage wird an %1 gesendet…</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1020"/>
        <source>Provider returned no reply</source>
        <translation>Anbieter hat keine Antwort zurückgegeben</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="147"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Anfrage durch Gemini-Sicherheitsfilter blockiert: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="190"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini gestoppt, ohne eine Antwort zu erzeugen: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="250"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="253"/>
        <source>Invalid API key (%1)</source>
        <translation>Ungültiger API-Schlüssel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="255"/>
        <source>Rate limited: %1</source>
        <translation>Ratenbegrenzung: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="257"/>
        <source>Invalid API key</source>
        <translation>Ungültiger API-Schlüssel</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="259"/>
        <source>Gemini %1: %2</source>
        <translation>Gemini %1: %2</translation>
    </message>
</context>
<context>
    <name>AI::OpenAIReply</name>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="321"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="324"/>
        <source>Invalid API key (%1)</source>
        <translation>Ungültiger API-Schlüssel (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="326"/>
        <source>Rate limited: %1</source>
        <translation>Ratenbegrenzung: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="328"/>
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
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Export Protobuf File</source>
        <translation>Protobuf-datei Exportieren</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="425"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="474"/>
        <source>Unable to start gRPC server</source>
        <translation>GRPC-Server kann nicht gestartet werden</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="475"/>
        <source>Failed to bind to %1</source>
        <translation>Bindung an %1 fehlgeschlagen</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="438"/>
        <source>Unable to start API TCP server</source>
        <translation>API-TCP-Server kann nicht gestartet werden</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="483"/>
        <source>Allow External API Connections?</source>
        <translation>Externe API-Verbindungen Zulassen?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="484"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Durch Freigabe des API-Servers für externe Hosts können andere Geräte im Netzwerk eine Verbindung zu Serial Studio auf Port 7777 herstellen.

Nur in vertrauenswürdigen Netzwerken aktivieren. Nicht vertrauenswürdige Clients können Live-Daten lesen oder Befehle an das Gerät senden.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="519"/>
        <source>Unable to restart API TCP server</source>
        <translation>API-TCP-Server kann nicht neu gestartet werden</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="598"/>
        <source>Allow API device control?</source>
        <translation>API-Gerätesteuerung zulassen?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="599"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Ein Programm, das die lokale API von Serial Studio verwendet, fordert an, Daten an das verbundene Gerät zu senden. API-Clients das Schreiben auf das Gerät erlauben?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="602"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1255"/>
        <source>API server</source>
        <translation>API-Server</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1255"/>
        <source>Invalid pending connection</source>
        <translation>Ungültige ausstehende Verbindung</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>Über</translation>
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
        <translation>Alle Rechte Vorbehalten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 ist freie Software: Sie können sie unter den Bedingungen der GNU General Public License, wie von der Free Software Foundation veröffentlicht, weiterverbreiten und/oder modifizieren; entweder gemäß Version 3 der Lizenz oder (nach Ihrer Wahl) jeder späteren Version.

%1 wird in der Hoffnung verteilt, dass sie nützlich sein wird, aber OHNE JEDE GEWÄHRLEISTUNG; sogar ohne die implizite Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK. Siehe die GNU General Public License für weitere Details.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Diese Konfiguration ist für kommerzielle und proprietäre Nutzung lizenziert. Sie darf in Closed-Source- und kommerziellen Anwendungen verwendet werden, vorbehaltlich der Bedingungen der kommerziellen Lizenz.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Diese Konfiguration ist nur für persönliche und Evaluierungszwecke bestimmt. Kommerzielle Nutzung ist untersagt, sofern keine gültige kommerzielle Lizenz aktiviert ist.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>Diese Software wird „wie besehen" ohne jegliche ausdrückliche oder stillschweigende Gewährleistung bereitgestellt, einschließlich, aber nicht beschränkt auf die Gewährleistung der Marktfähigkeit oder Eignung für einen bestimmten Zweck. In keinem Fall haftet der Autor für Schäden, die aus der Nutzung dieser Software entstehen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Lizenz Verwalten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Spenden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Nach Updates suchen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>License Agreement</source>
        <translation>Lizenzvereinbarung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>Report Bug</source>
        <translation>Fehler Melden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Acknowledgements</source>
        <translation>Danksagungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Benchmark</source>
        <translation>Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="258"/>
        <source>Website</source>
        <translation>Website</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="274"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="170"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="171"/>
        <source>Settings</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="229"/>
        <source>G-FORCE</source>
        <translation>G-KRAFT</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="265"/>
        <source>PITCH ↕</source>
        <translation>NICKEN ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="300"/>
        <source>ROLL ↔</source>
        <translation>ROLLEN ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>Beschleunigungssensor-konfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Anzeigebereich und Eingabeeinheiten des Beschleunigungssensors konfigurieren.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Anzeigebereich</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>Max. G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Maximalen G-Wert eingeben</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Eingangskonfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Eingang bereits in G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>Danksagungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>Über QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Symbol Ändern</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Symbol für diese Aktion ändern</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Diese Aktion mit allen Einstellungen duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Diese Aktion aus dem Projekt löschen</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Widget Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Verfügbare Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Zeile anklicken, um sie zum Arbeitsbereich hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(gesamte Gruppe)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Bereits im Arbeitsbereich</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Zum Arbeitsbereich hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Keine Widgets verfügbar.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Keine Widgets entsprechen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 von %2 Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Alarmbereiche</translation>
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
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Critical</source>
        <translation>Kritisch</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="82"/>
        <source>Tachometer</source>
        <translation>Tachometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="84"/>
        <source>Idle</source>
        <translation>Leerlauf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Operating</source>
        <translation>Betrieb</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Caution</source>
        <translation>Vorsicht</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Redline</source>
        <translation>Drehzahlbegrenzung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="91"/>
        <source>Speedometer</source>
        <translation>Tachometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="93"/>
        <source>Cruise</source>
        <translation>Reisegeschwindigkeit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Fast</source>
        <translation>Schnell</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Top Speed</source>
        <translation>Höchstgeschwindigkeit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="99"/>
        <source>Engine Temperature</source>
        <translation>Motortemperatur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="101"/>
        <source>Cold</source>
        <translation>Kalt</translation>
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
        <translation>Warm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Overheat</source>
        <translation>Überhitzung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="108"/>
        <source>Pressure</source>
        <translation>Druck</translation>
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
        <translation>Hoch</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <source>Burst</source>
        <translation>Burst</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="117"/>
        <source>Battery Voltage</source>
        <translation>Batteriespannung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="119"/>
        <source>Low</source>
        <translation>Niedrig</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Nominal</source>
        <translation>Nominal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="125"/>
        <source>Fuel Level</source>
        <translation>Kraftstoffstand</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="127"/>
        <source>Empty</source>
        <translation>Leer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Reserve</source>
        <translation>Reserve</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="133"/>
        <source>Signal Strength</source>
        <translation>Signalstärke</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="135"/>
        <source>No Signal</source>
        <translation>Kein Signal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>Weak</source>
        <translation>Schwach</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Good</source>
        <translation>Gut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="141"/>
        <source>CPU / System Load</source>
        <translation>CPU / Systemlast</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Busy</source>
        <translation>Ausgelastet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Overload</source>
        <translation>Überlast</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="149"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Warnung / Kritisch</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="270"/>
        <source>Choose Band Color</source>
        <translation>Bandfarbe Wählen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="297"/>
        <source>Presets</source>
        <translation>Voreinstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="320"/>
        <source>Preset</source>
        <translation>Voreinstellung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="335"/>
        <source>Choose preset…</source>
        <translation>Voreinstellung wählen…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="557"/>
        <source>Reset to severity default</source>
        <translation>Auf Schweregrad-Standard zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="571"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Klicken, um Farbe zu wählen. Rechtsklick, um auf Schweregrad-Standard zurückzusetzen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="572"/>
        <source>Click to choose a custom color.</source>
        <translation>Klicken, um benutzerdefinierte Farbe zu wählen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="657"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Keine Bänder definiert. Voreinstellung oben wählen oder Band hinzufügen, um zu beginnen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="781"/>
        <source>Apply</source>
        <translation>Anwenden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="784"/>
        <source>Apply changes to the dataset.</source>
        <translation>Änderungen am Datensatz anwenden.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Voreinstellung Anwenden</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Die aktuellen Bereiche durch die ausgewählte Voreinstellung ersetzen, skaliert auf den Wertebereich dieses Datensatzes.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="346"/>
        <source>Range</source>
        <translation>Bereich</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="374"/>
        <source>Bands</source>
        <translation>Bereiche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="385"/>
        <source>Add Band</source>
        <translation>Bereich Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="389"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Einen neuen Bereich hinzufügen, der an den letzten anschließt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="420"/>
        <source>Min</source>
        <translation>Min.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="426"/>
        <source>Max</source>
        <translation>Max.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="432"/>
        <source>Severity</source>
        <translation>Schweregrad</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="438"/>
        <source>Color</source>
        <translation>Farbe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="445"/>
        <source>Label</source>
        <translation>Beschriftung</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Benutzerdefinierte Farbe wählen.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="594"/>
        <source>(optional)</source>
        <translation>(optional)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="611"/>
        <source>Move up.</source>
        <translation>Nach oben verschieben.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="630"/>
        <source>Move down.</source>
        <translation>Nach unten verschieben.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="643"/>
        <source>Remove this band.</source>
        <translation>Dieses Band entfernen.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Keine Bänder definiert. Voreinstellung anwenden oder Band hinzufügen, um zu beginnen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="674"/>
        <source>Preview</source>
        <translation>Vorschau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="770"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="772"/>
        <source>Discard changes.</source>
        <translation>Änderungen verwerfen.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Speichern</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Änderungen am Datensatz speichern.</translation>
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
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>CSV- vs. MDF4-Export – was ist der Unterschied?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Balken und Anzeige – wann verwende ich welches?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="203"/>
        <source>How can I help with your project?</source>
        <translation>Wie kann ich bei Ihrem Projekt helfen?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>Set up your API key to get started</source>
        <translation>API-Schlüssel einrichten, um zu beginnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="216"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Beschreiben Sie, was Sie erstellen möchten, und ich konfiguriere die Quellen, Gruppen, Datensätze, Frame-Parser und Transformationen für Sie.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="219"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>Um mit dem Chatten zu beginnen, fügen Sie einen API-Schlüssel für den ausgewählten Anbieter ein. Schlüssel werden auf diesem Rechner verschlüsselt und verlassen Ihren Computer nur, um mit dem von Ihnen gewählten Anbieter zu kommunizieren.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="240"/>
        <source>Open API Key Setup</source>
        <translation>API-schlüssel-einrichtung Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Get a key from %1</source>
        <translation>Schlüssel von %1 abrufen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>List the sources in this project</source>
        <translation>Quellen in diesem Projekt auflisten</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="121"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Helfen Sie mir, die Funktionen von Serial Studio zu entdecken</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Was kann diese App für meine Telemetrie tun?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>Walk me through what this project already contains</source>
        <translation>Führen Sie mich durch das, was dieses Projekt bereits enthält</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="127"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Was ist eine Sitzungsdatenbank und warum sollte ich eine verwenden?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">CSV- vs. MDF4-Export – was ist der Unterschied?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="129"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Was ist ein Frame-Parser und wann benötige ich einen?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>Wann sollte ich Lua vs. JavaScript für den Parser verwenden?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Plot, Balken und Anzeige – wann verwende ich welches?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Was ist der Unterschied zwischen einer Transformation und einem Frame-Parser?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="135"/>
        <source>Add a UART source for an Arduino</source>
        <translation>UART-Quelle für einen Arduino hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Ein IMU-Projekt von Grund auf einrichten</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Configure an MQTT subscriber</source>
        <translation>MQTT-Abonnent konfigurieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Add a CAN bus source</source>
        <translation>CAN-Bus-Quelle hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Set up a Modbus poller</source>
        <translation>Modbus-Poller einrichten</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Netzwerkquelle (TCP/UDP) hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Write a CSV frame parser for me</source>
        <translation>CSV-Frame-Parser für mich schreiben</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Help me parse a JSON frame</source>
        <translation>Hilfe beim Parsen eines JSON-Frames</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>EMA-Glättungstransformation zu einem Datensatz hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Decode hexadecimal frames</source>
        <translation>Hexadezimale Frames dekodieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Sensor mit linearer Transformation kalibrieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="148"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Dashboard-Widgets für meine Daten vorschlagen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Build an executive overview workspace</source>
        <translation>Arbeitsbereich für Führungsübersicht erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Painter-Widget für benutzerdefinierte Visualisierung hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Plot, FFT und Wasserfall für einen Datensatz anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Datensätze in nützliche Arbeitsbereiche gruppieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="448"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Fragen Sie Serial Studio beliebig…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="468"/>
        <source>Clear conversation</source>
        <translation>Konversation löschen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="512"/>
        <source>Stop generating</source>
        <translation>Generierung stoppen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="513"/>
        <source>Send message (Enter)</source>
        <translation>Nachricht senden (Eingabe)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="555"/>
        <source>Provider</source>
        <translation>Anbieter</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="588"/>
        <source>Model selection</source>
        <translation>Modellauswahl</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="634"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>Bearbeitungsaktionen ohne Rückfrage ausführen. Blockierte Aktionen bleiben blockiert.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="636"/>
        <source>Auto-approve edits</source>
        <translation>Bearbeitungen automatisch genehmigen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="655"/>
        <source>Manage API keys</source>
        <translation>API-Schlüssel verwalten</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="676"/>
        <source>Working</source>
        <translation>Verarbeitung</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="677"/>
        <source>Ready</source>
        <translation>Bereit</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="678"/>
        <source>  •  cache %1k tok</source>
        <translation>•  Cache %1k Tok</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="679"/>
        <source>  •  cache write %1k tok</source>
        <translation>Cache-Schreiben %1k Tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Kein Mikrofon Erkannt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Mikrofon anschließen oder Einstellungen prüfen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Eingabegerät</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Abtastrate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Abtastformat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Kanäle</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Ausgabegerät</translation>
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
        <translation type="vanished">Bitte Benutzername und Passwort für den Download-Speicherort angeben.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">&amp;Benutzername:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Passwort:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="47"/>
        <source>Axis Range Configuration</source>
        <translation>Konfiguration des Achsenbereichs</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="161"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Sichtbaren Bereich für die Diagrammachsen konfigurieren. Werte werden in Echtzeit während der Eingabe aktualisiert.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="169"/>
        <source>X Axis</source>
        <translation>X-achse</translation>
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
        <translation>Minimalwert eingeben</translation>
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
        <translation>Maximalwert eingeben</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="242"/>
        <source>Y Axis</source>
        <translation>Y-achse</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="317"/>
        <source>Reset</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="327"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Sicherung Wiederherstellen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Unbenannt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Projekt Geladen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Automatisches Speichern</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Vor Wiederherstellung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Vor Löschen des Datensatzes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Vor Löschen der Gruppe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Vor Neuem Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Vor Öffnen des Projekts</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Vor Laden von JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Vor Anwenden der Vorlage</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Vor Löschen der Aktion</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Vor Löschen des Ausgabe-widgets</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Vor Datensatz Verschieben</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Vor Gruppe Verschieben</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Vor Arbeitsbereich Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Vor Alle Arbeitsbereiche Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Vor Widget Entfernen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Vor Arbeitsbereiche Neu Anordnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Vor Stapelverarbeitung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Vor Kachel Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (und %2 weitere)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>Der Frame-Parser-Code unterscheidet sich ebenfalls und wird ersetzt.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Titel ändert sich von „%1" zu „%2". Gruppenstruktur unverändert.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Gleiche Gruppen und Datensätze, aber der Frame-Parser-Code unterscheidet sich – die Wiederherstellung ersetzt ihn.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Gleiche Gruppen und Datensätze wie Ihr aktuelles Projekt. Wiederherstellen kann dennoch Änderungen auf Feldebene zurücksetzen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Wiederherstellen entfernt %1 und stellt %2 wieder her.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Wiederherstellen entfernt %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Wiederherstellen stellt %1 wieder her.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Backup zum Wiederherstellen auswählen. Das aktuelle Projekt wird zuerst automatisch gespeichert, sodass die Wiederherstellung rückgängig gemacht werden kann.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Noch keine Backups für dieses Projekt. Projekt bearbeiten oder speichern, um das rollende Backup zu starten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Ordner Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Wiederherstellen</translation>
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
        <location filename="../../qml/Dialogs/Benchmark.qml" line="62"/>
        <source>%1 frames/s</source>
        <translation>%1 Frames/s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="66"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="71"/>
        <source>n/a</source>
        <translation>k. A.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="73"/>
        <source>Pass</source>
        <translation>Bestanden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="73"/>
        <source>Fail</source>
        <translation>Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="94"/>
        <source>Hotpath Benchmark</source>
        <translation>Hotpath-benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="105"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Misst, wie schnell dieser Computer Frames durch die Datenpipeline von Serial Studio extrahieren, parsen und visualisieren kann.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="143"/>
        <source>The interface will freeze while running</source>
        <translation>Die Oberfläche friert während der Ausführung ein</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="154"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Jede Phase läuft mit voller Geschwindigkeit im Hauptthread, sodass das Fenster nicht mehr reagiert, bis sie abgeschlossen ist. Das aktuelle Projekt wird automatisch neu geladen, wenn der Benchmark endet.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="173"/>
        <source>Frames per phase:</source>
        <translation>Frames pro Phase:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="187"/>
        <source>Minimum duration:</source>
        <translation>Mindestdauer:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="225"/>
        <source>Running %1...</source>
        <translation>%1 Läuft...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="226"/>
        <source>Preparing...</source>
        <translation>Vorbereitung...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="269"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="281"/>
        <source>Throughput</source>
        <translation>Durchsatz</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="293"/>
        <source>Time</source>
        <translation>Zeit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="305"/>
        <source>Result</source>
        <translation>Ergebnis</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="398"/>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation>Bestanden/Nicht bestanden gilt für die Datenpipeline- und Parser-Phasen (Datenpipeline und Integrierter numerischer Parser 1024 K Frames/s; Integrierter gemischter Parser 512 K; Lua numerisch 256 K; JavaScript numerisch und Lua gemischt 128 K; JavaScript gemischt 64 K). Die Export- und Dashboard-Phasen sind informativ.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Bestanden/Nicht bestanden gilt für die Datenpipeline- und Parser-Phasen (Datenpipeline und Native numerisch 1024 K Frames/s; Native gemischt 512 K; Lua numerisch 256 K; JavaScript numerisch und Lua gemischt 128 K; JavaScript gemischt 64 K). Die Export- und Dashboard-Phasen sind informativ.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="413"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Bestanden/Nicht bestanden gilt nur für die Datenpipeline- und Parser-Phasen (Datenpipeline 1024 K Frames/s; numerisch: Lua 256 K, JavaScript 128 K; gemischt: Lua 128 K, JavaScript 64 K). Die Export- und Dashboard-Phasen sind informativ.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Bestanden/Nicht bestanden gilt nur für die Parser-Phasen (Lua-Ziel 256 K Frames/s, JavaScript 128 K). Die Export- und Dashboard-Phasen sind informativ.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="420"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="429"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="439"/>
        <source>Running...</source>
        <translation>Wird Ausgeführt…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="439"/>
        <source>Run Benchmark</source>
        <translation>Benchmark Ausführen</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="187"/>
        <source>Data pipeline</source>
        <translation>Daten-Pipeline</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">Lua-Parser</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">JavaScript-Parser</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + Datenexport</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + Dashboard</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Nativer Parser (numerisch)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Nativer Parser (gemischt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="188"/>
        <source>Built-in parser (numeric)</source>
        <translation>Integrierter Parser (numerisch)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="189"/>
        <source>Built-in parser (mixed)</source>
        <translation>Integrierter Parser (gemischt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="190"/>
        <source>Lua parser (numeric)</source>
        <translation>Lua-Parser (numerisch)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="191"/>
        <source>JavaScript parser (numeric)</source>
        <translation>JavaScript-Parser (numerisch)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="192"/>
        <source>Lua parser (mixed)</source>
        <translation>Lua-Parser (gemischt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="193"/>
        <source>JavaScript parser (mixed)</source>
        <translation>JavaScript-Parser (gemischt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="194"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + Datenexport (gemischt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="195"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + Dashboard (numerisch)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="196"/>
        <source>100 K frames</source>
        <translation>100 K Frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="196"/>
        <source>250 K frames</source>
        <translation>250 K Frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="196"/>
        <source>500 K frames</source>
        <translation>500 K Frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="196"/>
        <source>1 M frames</source>
        <translation>1 M Frames</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="197"/>
        <source>1 second</source>
        <translation>1 Sekunde</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="197"/>
        <source>2 seconds</source>
        <translation>2 Sekunden</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="197"/>
        <source>5 seconds</source>
        <translation>5 Sekunden</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="197"/>
        <source>10 seconds</source>
        <translation>10 Sekunden</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 – Hotpath-benchmark</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="228"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), Arbeitslast: %3 Frames mindestens, %4 s mindestens</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="235"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="235"/>
        <source>Throughput</source>
        <translation>Durchsatz</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="235"/>
        <source>Target</source>
        <translation>Ziel</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="235"/>
        <source>Time</source>
        <translation>Zeit</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="235"/>
        <source>Result</source>
        <translation>Ergebnis</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="246"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="252"/>
        <source>%1 frames/s</source>
        <translation>%1 Frames/s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="246"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="248"/>
        <source>n/a</source>
        <translation>n. v.</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="248"/>
        <source>Pass</source>
        <translation>Bestanden</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="248"/>
        <source>Fail</source>
        <translation>Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="254"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Dashboard wird angezeigt...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Gerät</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Dienst</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Charakteristik</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Scannen…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Kein Bluetooth-adapter Erkannt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Bluetooth-Adapter anschließen oder Systemeinstellungen überprüfen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Dieses Betriebssystem wird noch nicht Unterstützt.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio wird für dieses Betriebssystem aktualisiert, sobald QT es offiziell unterstützt</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Keine CAN-treiber Gefunden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>CAN-Hardware-Treiber für das System installieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>CAN-treiber</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="142"/>
        <source>Interface</source>
        <translation>Schnittstelle</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="174"/>
        <source>Bitrate</source>
        <translation>Bitrate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="238"/>
        <source>Flexible Data-Rate</source>
        <translation>Flexible Data-rate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="270"/>
        <source>DBC Database</source>
        <translation>DBC-datenbank</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="274"/>
        <source>Import DBC File…</source>
        <translation>DBC-datei Importieren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="307"/>
        <source>No CAN Interfaces Found</source>
        <translation>Keine CAN-schnittstellen Gefunden</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="210"/>
        <source>Select CSV file</source>
        <translation>CSV-Datei Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="212"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="383"/>
        <source>Device Connection Active</source>
        <translation>Geräteverbindung Aktiv</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="384"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Um diese Funktion zu nutzen, muss die Verbindung zum Gerät getrennt werden. Fortfahren?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="399"/>
        <source>Check file permissions and location</source>
        <translation>Dateiberechtigungen und Speicherort Prüfen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="432"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Unzureichende Daten in CSV-Datei</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="399"/>
        <source>Cannot read CSV file</source>
        <translation>CSV-Datei kann nicht gelesen werden</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="433"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>Die CSV-Datei muss mindestens eine Datenzeile enthalten, um fortzufahren. Datei prüfen und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="669"/>
        <source>Invalid CSV</source>
        <translation>Ungültige CSV-datei</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="670"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>Die CSV-Datei enthält keine Daten oder Kopfzeilen.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="679"/>
        <source>Select a date/time column</source>
        <translation>Datums-/Zeitspalte auswählen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="679"/>
        <location filename="../../src/CSV/Player.cpp" line="691"/>
        <source>Set interval manually</source>
        <translation>Intervall manuell festlegen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="681"/>
        <source>CSV Date/Time Selection</source>
        <translation>CSV-datums-/zeitauswahl</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="682"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Umgang mit Datums-/Zeitdaten wählen:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="694"/>
        <source>Set Interval</source>
        <translation>Intervall Festlegen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Intervall zwischen Zeilen in Millisekunden eingeben:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="711"/>
        <source>Select Date/Time Column</source>
        <translation>Datum/uhrzeit-spalte Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="712"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Bitte wählen Sie die Spalte aus, die die Datum/Uhrzeit-Daten enthält:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="722"/>
        <source>Invalid Selection</source>
        <translation>Ungültige Auswahl</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="722"/>
        <source>The selected column is not valid.</source>
        <translation>Die ausgewählte Spalte ist nicht gültig.</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>Konsole</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="340"/>
        <source>Console Export is a Pro feature.</source>
        <translation>Konsolen-Export ist eine Pro-Funktion.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="341"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Diese Funktion erfordert eine Lizenz. Bitte erwerben Sie eine, um den Konsolen-Export zu aktivieren.</translation>
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
        <translation>Kein Zeilenende</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="236"/>
        <source>New Line</source>
        <translation>Neue Zeile</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="237"/>
        <source>Carriage Return</source>
        <translation>Wagenrücklauf</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="238"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="248"/>
        <source>Plain Text</source>
        <translation>Klartext</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="249"/>
        <source>Hexadecimal</source>
        <translation>Hexadezimal</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="271"/>
        <source>No Checksum</source>
        <translation>Keine Prüfsumme</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="913"/>
        <source>Device %1</source>
        <translation>Gerät %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Konstante Einfügen</translation>
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
        <translation>Lichtgeschwindigkeit im Vakuum</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Planck-Konstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Elementarladung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Avogadro-Konstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Boltzmann-Konstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Stefan-Boltzmann-Konstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>Mechanik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>Normfallbeschleunigung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Gravitationskonstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Druck</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Standardatmosphäre</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Barometrischer Druck auf Meereshöhe</translation>
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
        <translation>Absoluter Nullpunkt (Celsius)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Gefrierpunkt von Wasser</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Siedepunkt von Wasser (1 atm)</translation>
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
        <translation>Gase &amp; Flüssigkeiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Universelle Gaskonstante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Spezifische Gaskonstante (trockene Luft)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Spezifische Gaskonstante (Wasserdampf)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Luftdichte (Meereshöhe, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Wasserdichte (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Schallgeschwindigkeit in Luft (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Wärmekapazitätsverhältnis (trockene Luft)</translation>
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
        <translation>Vakuumpermittivität</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Vakuumpermeabilität</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Mathematik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Eulersche Zahl</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Goldener Schnitt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Physikalische Konstanten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>SI-Einheit-Vorgabewerte. Klicken Sie auf eine Zeile, um sie in %1 einzufügen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Symbol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Wert</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Kategorie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Keine passenden Konstanten.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 Konstanten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 von %2 Konstanten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Wiederherstellungsoptionen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio wurde unerwartet mehrmals hintereinander geschlossen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>Aufeinanderfolgende Abstürze: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Zuletzt gemeldeter Schritt: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Erkannt am: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Wähle eine Wiederherstellungsaktion. Serial Studio wird nach der Anwendung beenden, damit der nächste Start sauber erfolgt.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Rendering-Backend auf Standard zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Wiederherstellung des zuletzt geöffneten Projekts überspringen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Alle Einstellungen zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Trotzdem Fortfahren</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>CSV-player</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>DBC-dateivorschau</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>DBC-datei: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Überprüfen Sie die CAN-Nachrichten und -Signale für den Import in ein neues Serial Studio-Projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Nachrichten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Nachrichtenname</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>CAN-ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Signale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>Keine Nachrichten in DBC-Datei gefunden.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Gesamt: %1 Nachrichten, %2 Signale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Projekt Erstellen</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="127"/>
        <source>Dashboard %1</source>
        <translation>Dashboard %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>Senden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Keine Sendefunktion definiert</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="56"/>
        <source>Set Wallpaper…</source>
        <translation>Hintergrundbild Festlegen…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="62"/>
        <source>Clear Wallpaper</source>
        <translation>Hintergrundbild Entfernen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="72"/>
        <source>Tile Windows</source>
        <translation>Fenster Kacheln</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="91"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-Funktionen in diesem Projekt erkannt.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="93"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Ersatz-Widgets sind aktiv. Lizenz erwerben für volle Funktionalität.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="208"/>
        <source>Empty Workspace</source>
        <translation>Leerer Arbeitsbereich</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="222"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Suchleiste verwenden, um Widgets zu finden und hinzuzufügen, oder Rechtsklick auf ein Widget in einem anderen Arbeitsbereich, um es hier hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="237"/>
        <source>Search Widgets</source>
        <translation>Widgets Suchen</translation>
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
        <translation>API-server Bereit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="208"/>
        <source>API Server Off</source>
        <translation>API-server Aus</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="123"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="275"/>
        <source>Send</source>
        <translation>Senden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="263"/>
        <source>Enter command…</source>
        <translation>Befehl eingeben…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Keine Übertragungsfunktion definiert</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Befehl eingeben…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Senden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Keine Übertragungsfunktion definiert</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>EIN</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>AUS</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>Keine Übertragungsfunktion definiert</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Resume</source>
        <translation>Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="297"/>
        <source>Awaiting data…</source>
        <translation>Warte auf Daten…</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>Import DBC File</source>
        <translation>DBC-datei Importieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>DBC-Dateien (*.DBC);;Alle Dateien (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="158"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>DBC-Datei konnte nicht geparst werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="159"/>
        <source>Verify the file format and try again.</source>
        <translation>Dateiformat überprüfen und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>DBC Import Error</source>
        <translation>DBC-importfehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="168"/>
        <source>DBC file contains no messages</source>
        <translation>DBC-Datei enthält keine Nachrichten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="169"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Die ausgewählte Datei enthält keine CAN-Nachrichtendefinitionen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC Import Warning</source>
        <translation>DBC-importwarnung</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Temporäre Projektdatei konnte nicht erstellt werden</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">Prüfen, ob die Anwendung Schreibrechte für das temporäre Verzeichnis besitzt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="223"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>DBC-Datei erfolgreich importiert mit %1 Nachrichten und %2 Signalen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="216"/>
        <source>The project editor is now open for customization.</source>
        <translation>Der Projekt-Editor ist jetzt zur Anpassung geöffnet.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Importiertes Projekt konnte nicht geladen werden</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Das generierte Projekt-JSON konnte nicht geladen werden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>%1 Signal(e) mit erweitertem Multiplexing (SG_MUL_VAL_) übersprungen; nur einfaches Multiplexing wird unterstützt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="228"/>
        <source>DBC Import Complete</source>
        <translation>DBC-import Abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="250"/>
        <source>CAN Bus</source>
        <translation>CAN-bus</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="58"/>
        <source>Dataset Value Transform</source>
        <translation>Datensatz-wert-transformation</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="95"/>
        <source>Lua</source>
        <translation>Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="95"/>
        <source>JavaScript</source>
        <translation>Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="119"/>
        <source>Language:</source>
        <translation>Sprache:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="122"/>
        <source>Template:</source>
        <translation>Vorlage:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="101"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Rohwert eingeben (z. B. 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="107"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="133"/>
        <source>Input:</source>
        <translation>Eingabe:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="136"/>
        <source>Output:</source>
        <translation>Ausgabe:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="109"/>
        <source>Apply</source>
        <translation>Anwenden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="110"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="216"/>
        <source>Transform — %1</source>
        <translation>Transformation — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="306"/>
        <source>Enter a value</source>
        <translation>Wert eingeben</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>Invalid number</source>
        <translation>Ungültige Zahl</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="383"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Dokument Formatieren	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="384"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Auswahl Formatieren	ctrl+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="490"/>
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
-- Definieren Sie eine transform(value)-Funktion, die den Live-
-- Datensatzwert empfängt und eine transformierte Zahl zurückgibt.
-- Wenn keine transform()-Funktion definiert ist, wird der Rohwert
-- beibehalten und nichts mit dem Projekt gespeichert.
--
-- Beispiel:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Auf oberster Ebene deklarierte Globals bleiben zwischen Frames
-- bestehen, was für Filter, Akkumulatoren und andere zustandsbehaftete
-- Transformationen nützlich ist, genau wie bei einem Frame-Parser-Skript:
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
-- Verwenden Sie das Vorlagen-Dropdown für fertige Beispiele oder
-- klicken Sie auf Testen, um Ihre Funktion auszuprobieren.
--
</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="518"/>
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
 * Definieren Sie eine transform(value)-Funktion, die den Live-
 * Datensatzwert empfängt und eine transformierte Zahl zurückgibt.
 * Wenn keine transform()-Funktion definiert ist, wird der Rohwert
 * beibehalten und nichts mit dem Projekt gespeichert.
 *
 * Beispiel:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globale Variablen auf oberster Ebene bleiben zwischen Frames
 * erhalten, was für Filter, Akkumulatoren und andere zustandsbehaftete
 * Transformationen nützlich ist -- genau wie bei Frame-Parser-Skripten:
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
 * Verwenden Sie das Vorlagen-Dropdown für fertige Beispiele oder
 * klicken Sie auf Testen, um Ihre Funktion auszuprobieren.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="788"/>
        <source>Select Template…</source>
        <translation>Vorlage Auswählen…</translation>
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
 * Definieren Sie eine transform(value)-Funktion, die den Live-
 * Datensatzwert empfängt und eine transformierte Zahl zurückgibt.
 * Wenn keine transform()-Funktion definiert ist, wird der Rohwert
 * beibehalten und nichts mit dem Projekt gespeichert.
 *
 * Beispiel:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Globale Variablen, die auf oberster Ebene deklariert werden,
 * bleiben zwischen Frames bestehen, was für Filter, Akkumulatoren
 * und andere zustandsbehaftete Transformationen nützlich ist —
 * genau wie bei einem Frame-Parser-Skript:
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
 * Verwenden Sie das Vorlagen-Dropdown für fertige Beispiele oder
 * klicken Sie auf Testen, um Ihre Funktion auszuprobieren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="665"/>
        <source>Engine error</source>
        <translation>Engine-Fehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="706"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="726"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="737"/>
        <source>Error: %1</source>
        <translation>Fehler: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="698"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="730"/>
        <source>Error: transform() not defined</source>
        <translation>Fehler: transform() nicht definiert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="712"/>
        <source>Error: transform() must return a number</source>
        <translation>Fehler: transform() muss eine Zahl zurückgeben</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1289"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1407"/>
        <source>Channel %1</source>
        <translation>Kanal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1419"/>
        <source>Audio Input</source>
        <translation>Audioeingang</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1298"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1424"/>
        <source>Quick Plot</source>
        <translation>Schnelldiagramm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1099"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>JavaScript-Transform überschritt das Budget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1100"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>Eine Datensatz-Transformation dauerte länger als %1 ms; die verbleibenden Datensätze im Frame wurden bis zum nächsten Frame auf Rohwerte zurückgesetzt. Transform-Code optimieren oder vereinfachen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="195"/>
        <source>Frame pool exhausted</source>
        <translation>Frame-Pool erschöpft</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="197"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>Ein nachgelagerter Verbraucher (Dashboard, CSV/MDF4-Export, Sitzungsdatenbank oder API-Abonnent) verarbeitet Frames nicht schnell genug. Serial Studio verwendet vorübergehend Einzelzuweisungen pro Frame, bis der Rückstand abgebaut ist. Einen ressourcenintensiven Verbraucher deaktivieren oder die Datenrate reduzieren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1249"/>
        <source>Device A</source>
        <translation>Gerät A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1305"/>
        <source>Quick Plot Data</source>
        <translation>Schnelldiagramm-daten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1317"/>
        <source>Multiple Plots</source>
        <translation>Mehrere Diagramme</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">Keine</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Ungültige Hex-eingabe</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Bitte gültige hexadezimale Bytes eingeben.

Gültiges Format: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(keine Frames)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">Extraktion hat keinen vollständigen Frame erzeugt. Überprüfen Sie die Start-/End-Trennzeichen und den Erkennungsmodus.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 Frame(s) extrahiert | %2 Byte(s) verarbeitet | %3 Byte(s) gepuffert | %4 verworfen</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Pipeline-konfiguration</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Pipeline-ergebnisse</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Erkennung</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Decoder</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Prüfsumme</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Start-trennzeichen</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">End-trennzeichen</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Hex-trennzeichen</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Nur End-Trennzeichen</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Start- + End-Trennzeichen</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Nur Start-Trennzeichen</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Keine Trennzeichen (gesamter Block)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Klartext (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexadezimal</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binär (rohe Bytes)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Löschen</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Auswerten</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Rohe Stream-Bytes hier eingeben...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Bereitstellen</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Frame-dateneingabe</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Frame-parser-ergebnisse</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Frame-Daten hier eingeben…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Datensatz-index</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Wert</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Frame-Daten oben eingeben, HEX-Modus bei Bedarf aktivieren, dann auf „Auswerten" klicken, um den Frame-Parser auszuführen.

Beispiel (Text): a,b,c,d,e,f
Beispiel (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Frame-parser Testen</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Hex-Bytes eingeben (z. B. 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(leer)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Keine Daten zurückgegeben</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="215"/>
        <source>Change Scripting Language?</source>
        <translation>Skriptsprache Ändern?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="216"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Das Wechseln der Skriptsprache ersetzt den aktuellen Parser-Code durch die entsprechende Vorlage in der neuen Sprache.

Alle nicht gespeicherten Änderungen gehen verloren. Fortfahren?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="384"/>
        <source>Select Javascript file to import</source>
        <translation>Javascript-Datei zum Importieren auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="384"/>
        <source>Select Lua file to import</source>
        <translation>Lua-Datei zum Importieren auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="418"/>
        <source>Code Validation Successful</source>
        <translation>Code-validierung Erfolgreich</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="419"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>Keine Syntaxfehler im Parser-Code erkannt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="532"/>
        <source>Select Frame Parser Template</source>
        <translation>Frame-parser-vorlage Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="533"/>
        <source>Choose a template to load:</source>
        <translation>Zu ladende Vorlage auswählen:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="290"/>
        <source>Import Modbus Register Map</source>
        <translation>Modbus-registermap Importieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="294"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Modbus-Registermaps (*.CSV *.XML *.JSON);;CSV-Dateien (*.CSV);;XML-Dateien (*.XML);;JSON-Dateien (*.JSON);;Alle Dateien (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="332"/>
        <source>No registers found</source>
        <translation>Keine Register gefunden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="333"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Die Datei konnte nicht geparst werden oder enthält keine Registerdefinitionen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="335"/>
        <source>Modbus Import</source>
        <translation>Modbus-import</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Importiertes Projekt konnte nicht geladen werden</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Das generierte Projekt-JSON konnte nicht geladen werden.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Temporäre Projektdatei konnte nicht erstellt werden</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Schreibberechtigungen für das temporäre Verzeichnis prüfen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="381"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>%1 Register in %2 Gruppen erfolgreich importiert.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="383"/>
        <source>The project editor is now open for customization.</source>
        <translation>Der Projekt-Editor ist jetzt zur Anpassung geöffnet.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="385"/>
        <source>Modbus Import Complete</source>
        <translation>Modbus-import Abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="702"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
</context>
<context>
    <name>DataModel::NativeParserEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="254"/>
        <source>Plain text (UTF-8)</source>
        <translation>Klartext (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="254"/>
        <source>Hexadecimal</source>
        <translation>Hexadezimal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="254"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="254"/>
        <source>Binary (raw bytes)</source>
        <translation>Binär (rohe Bytes)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="262"/>
        <source>End delimiter only</source>
        <translation>Nur End-Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="263"/>
        <source>Start + end delimiters</source>
        <translation>Start- + End-Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="264"/>
        <source>Start delimiter only</source>
        <translation>Nur Start-Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="265"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Keine Trennzeichen (gesamter Block)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="276"/>
        <source>No Checksum</source>
        <translation>Keine Prüfsumme</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="472"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Ungültige hexadezimale Eingabe.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="501"/>
        <source>No template selected.</source>
        <translation>Keine Vorlage ausgewählt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="542"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 Frame(s) extrahiert | %2 Byte(s) verarbeitet | %3 Byte(s) gepuffert | %4 verworfen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/NativeParserEditor.cpp" line="703"/>
        <source>Parameters</source>
        <translation>Parameter</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="290"/>
        <source>Select Javascript file to import</source>
        <translation>JavaScript-Datei zum Importieren auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="351"/>
        <source>Select Output Widget Template</source>
        <translation>Output-widget-vorlage Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="352"/>
        <source>Choose a template to load:</source>
        <translation>Vorlage zum Laden auswählen:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="298"/>
        <source>Select Javascript file to import</source>
        <translation>JavaScript-Datei zum Importieren auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="386"/>
        <source>Select Painter Widget Template</source>
        <translation>Painter-widget-vorlage Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Choose a template to load:</source>
        <translation>Vorlage zum Laden auswählen:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>Datensätze für diese Vorlage hinzufügen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>„%1" erwartet %2 Datensatz/Datensätze; die aktuelle Gruppe hat %3.

%4 Datensatz/Datensätze mit den Standardwerten der Vorlage hinzufügen?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2179"/>
        <source>Project Information</source>
        <translation>Projektinformationen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2189"/>
        <source>Project Title</source>
        <translation>Projekttitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2190"/>
        <source>Untitled Project</source>
        <translation>Unbenanntes Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2191"/>
        <source>Name or description of the project</source>
        <translation>Name oder Beschreibung des Projekts</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2416"/>
        <source>Frame Detection</source>
        <translation>Frame-erkennung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2432"/>
        <source>Frame Detection Method</source>
        <translation>Frame-erkennungsmethode</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2433"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Auswahl, wie eingehende Datenframes identifiziert werden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2443"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Hexadezimale Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2444"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Frame-Start-/Endsequenzen als Hexadezimalwerte eingeben</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2460"/>
        <source>Frame Start Delimiter</source>
        <translation>Frame-start-trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2461"/>
        <source>e.g. /*</source>
        <translation>z. B. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2462"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Sequenz, die den Beginn eines Datenframes markiert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2474"/>
        <source>Frame End Delimiter</source>
        <translation>Frame-endebegrenzer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2475"/>
        <source>e.g. */</source>
        <translation>z. B. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2476"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Sequenz, die das Ende eines Daten-Frames markiert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2488"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Nutzdatenverarbeitung &amp; Validierung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2499"/>
        <source>Data Conversion Method</source>
        <translation>Datenkonvertierungsmethode</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2500"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Auswahl, wie eingehende Binärdaten vor dem Parsen dekodiert werden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2516"/>
        <source>Checksum Algorithm</source>
        <translation>Prüfsummenalgorithmus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2517"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Auswahl des Prüfsummenalgorithmus zur Validierung von Frames</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2209"/>
        <source>Group Information</source>
        <translation>Gruppeninformationen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2219"/>
        <source>Group Title</source>
        <translation>Gruppentitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2220"/>
        <source>Untitled Group</source>
        <translation>Unbenannte Gruppe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2221"/>
        <source>Title or description of this dataset group</source>
        <translation>Titel oder Beschreibung dieser Datensatzgruppe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2354"/>
        <source>Composite Widget</source>
        <translation>Zusammengesetztes Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2355"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Auswählen, wie diese Datensatzgruppe visualisiert werden soll (optional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2271"/>
        <source>Image Configuration</source>
        <translation>Bildkonfiguration</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3115"/>
        <source>Virtual Dataset</source>
        <translation>Virtueller Datensatz</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3116"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Virtuelle Datensätze berechnen ihren Wert aus Transformationen und Datentabellen, sie benötigen keinen Frame-Index</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3617"/>
        <source>Auto-detect</source>
        <translation>Automatisch Erkennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3617"/>
        <source>Manual Delimiters</source>
        <translation>Manuelle Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2284"/>
        <source>Detection Mode</source>
        <translation>Erkennungsmodus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1312"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1315"/>
        <source>Frame Parser</source>
        <translation>Frame-parser</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1455"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1456"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1500"/>
        <source>Groups</source>
        <translation>Gruppen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1530"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1543"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1544"/>
        <source>Shared Memory</source>
        <translation>Gemeinsamer Speicher</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1530"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1550"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1551"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4987"/>
        <source>Dataset Values</source>
        <translation>Datensatzwerte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1594"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1607"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1608"/>
        <source>Workspaces</source>
        <translation>Arbeitsbereiche</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1646"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1649"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1650"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1699"/>
        <source>Publishing</source>
        <translation>Veröffentlichung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1710"/>
        <source>Enable Publishing</source>
        <translation>Veröffentlichung Aktivieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1711"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Frames, Rohdaten und Benachrichtigungen an den Broker senden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1722"/>
        <source>Payload</source>
        <translation>Nutzlast</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1723"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Wählt aus, was veröffentlicht wird: geparste Dashboard-Daten oder rohe RX-Bytes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1733"/>
        <source>Publish Rate (Hz)</source>
        <translation>Veröffentlichungsrate (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1734"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>Wie oft pro Sekunde veröffentlicht wird (1-30 Hz). Höhere Raten erhöhen die Broker-Last; Dashboard-Daten sind ratenbegrenzt, sodass ein langsamer Broker das Frame-Parsing nie blockiert.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1746"/>
        <source>Topic Base</source>
        <translation>Thema-basis</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1747"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1748"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Basis-Thema für Frame- und Rohbyte-Veröffentlichung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1759"/>
        <source>Script Topic</source>
        <translation>Skript-thema</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1760"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1784"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>Standardmäßig Basis-Thema, wenn leer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1761"/>
        <source>Topic the user script publishes to</source>
        <translation>Thema, auf das das Benutzerskript veröffentlicht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1771"/>
        <source>Publish Notifications</source>
        <translation>Benachrichtigungen Veröffentlichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1772"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Dashboard-Benachrichtigungen auf ein dediziertes Thema spiegeln</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1783"/>
        <source>Notification Topic</source>
        <translation>Benachrichtigungs-thema</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1785"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Thema, auf das Dashboard-Benachrichtigungen gespiegelt werden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1798"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1808"/>
        <source>Hostname</source>
        <translation>Hostname</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1809"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1810"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>Hostname oder IP-Adresse des MQTT-Brokers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1819"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1820"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>TCP-Port des Brokers (1883 unverschlüsselt, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1830"/>
        <source>Custom Client ID</source>
        <translation>Benutzerdefinierte Client-ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1832"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Aus: Bei jedem Laden des Projekts wird eine neue zufällige ID generiert. Ein: Untenstehende ID verwenden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1843"/>
        <source>Client ID</source>
        <translation>Client-ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1844"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Kennung, die beim CONNECT an den Broker gesendet wird</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1857"/>
        <source>Protocol Version</source>
        <translation>Protokollversion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1858"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>MQTT-Protokollversion für CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1867"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1868"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Sekunden zwischen PINGREQ-Paketen im Leerlauf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1877"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1878"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Verwirft persistenten Sitzungsstatus bei CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1893"/>
        <source>Username</source>
        <translation>Benutzername</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1894"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Benutzername für Broker-Authentifizierung (leer lassen für anonym)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1904"/>
        <source>Password</source>
        <translation>Passwort</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1905"/>
        <source>Password for broker authentication</source>
        <translation>Passwort für Broker-Authentifizierung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1916"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1926"/>
        <source>Use SSL/TLS</source>
        <translation>SSL/TLS Verwenden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1927"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Broker-Verbindung über TLS Tunneln</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1940"/>
        <source>Protocol</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1941"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Ausgehandelte TLS-Protokollfamilie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1951"/>
        <source>Peer Verify</source>
        <translation>Peer Verifizieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1952"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Wie streng die Zertifikatskette des Brokers validiert wird</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1962"/>
        <source>Verify Depth</source>
        <translation>Verify Depth</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1963"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Maximal akzeptierte Zertifikatskettenlänge (0 = unbegrenzt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2236"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2850"/>
        <source>Device %1</source>
        <translation>Gerät %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2254"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Input Device</source>
        <translation>Eingabegerät</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2255"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Auswahl des verbundenen Geräts, das Daten für diese Gruppe bereitstellt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2286"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Automatische Erkennung liest JPEG/PNG-Magic-Bytes; Manuell verwendet explizite Start-/Endsequenzen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2296"/>
        <source>Start Sequence (Hex)</source>
        <translation>Startsequenz (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2297"/>
        <source>e.g. FF D8 FF</source>
        <translation>z. B. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2298"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Hex-Bytes, die den Beginn eines Bild-Frames markieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2307"/>
        <source>End Sequence (Hex)</source>
        <translation>Endsequenz (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2308"/>
        <source>e.g. FF D9</source>
        <translation>z. B. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2309"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Hex-Bytes, die das Ende eines Bild-Frames markieren</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">Identität</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2385"/>
        <source>Device Name</source>
        <translation>Gerätename</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2386"/>
        <source>Device 1</source>
        <translation>Gerät 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2387"/>
        <source>Human-readable name for this input device</source>
        <translation>Lesbarer Name für dieses Eingabegerät</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2396"/>
        <source>Bus Type</source>
        <translation>Bustyp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2397"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Wählen Sie die Hardware-Schnittstelle für dieses Eingabegerät</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2399"/>
        <source>Serial Port</source>
        <translation>Serielle Schnittstelle</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2399"/>
        <source>Network Socket</source>
        <translation>Netzwerk-socket</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2399"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2401"/>
        <source>Audio Input</source>
        <translation>Audio-eingang</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2401"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2401"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2401"/>
        <source>Raw USB</source>
        <translation>Raw USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2402"/>
        <source>HID Device</source>
        <translation>HID-gerät</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2402"/>
        <source>Process</source>
        <translation>Prozess</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2402"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT-abonnent</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2569"/>
        <source>Connection Settings</source>
        <translation>Verbindungseinstellungen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3091"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4678"/>
        <source>General Information</source>
        <translation>Allgemeine Informationen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2826"/>
        <source>Action Title</source>
        <translation>Aktionstitel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2828"/>
        <source>Untitled Action</source>
        <translation>Unbenannte Aktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2829"/>
        <source>Name or description of this action</source>
        <translation>Name oder Beschreibung dieser Aktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2838"/>
        <source>Action Icon</source>
        <translation>Aktionssymbol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2839"/>
        <source>Default Icon</source>
        <translation>Standardsymbol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2840"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Symbol, das für diese Aktion im Dashboard angezeigt wird</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2868"/>
        <source>Target Device</source>
        <translation>Zielgerät</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2869"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Wählen Sie aus, an welches verbundene Gerät diese Aktion Daten sendet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2881"/>
        <source>Data Payload</source>
        <translation>Datennutzlast</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2892"/>
        <source>Send as Binary</source>
        <translation>Als Binär senden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2893"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Rohe Binärdaten senden, wenn diese Aktion ausgelöst wird</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2904"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2916"/>
        <source>Command</source>
        <translation>Befehl</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2905"/>
        <source>Transmit Data (Hex)</source>
        <translation>Daten Übertragen (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2906"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Hexadezimale Nutzlast, die beim Auslösen der Aktion gesendet wird</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2917"/>
        <source>Transmit Data</source>
        <translation>Daten Übertragen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2918"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Textnutzlast, die beim Auslösen der Aktion gesendet wird</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2929"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4737"/>
        <source>Text Encoding</source>
        <translation>Textkodierung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2930"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Zeichenkodierung zur Serialisierung der Textnutzlast</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2954"/>
        <source>End-of-Line Sequence</source>
        <translation>End-of-line-sequenz</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2955"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>EOL-Zeichen, die an die Nachricht angehängt werden (z. B. </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2967"/>
        <source>Execution Behavior</source>
        <translation>Ausführungsverhalten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2978"/>
        <source>Auto-Execute on Connect</source>
        <translation>Automatisch bei Verbindung ausführen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2979"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Diese Aktion automatisch auslösen, wenn das Gerät verbunden wird</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2985"/>
        <source>Timer Behavior</source>
        <translation>Timer-verhalten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2994"/>
        <source>Timer Mode</source>
        <translation>Timer-modus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2997"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Wählen, wann und wie diese Aktion automatisch wiederholt werden soll</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3004"/>
        <source>Interval (ms)</source>
        <translation>Intervall (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3008"/>
        <source>Timer Interval (ms)</source>
        <translation>Timer-Intervall (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3009"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Millisekunden zwischen jeder wiederholten Auslösung dieser Aktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3016"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3020"/>
        <source>Repeat Count</source>
        <translation>Wiederholungsanzahl</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3021"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Anzahl der Sendungen des Befehls bei jeder Auslösung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3101"/>
        <source>Untitled Dataset</source>
        <translation>Unbenannter Datensatz</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3102"/>
        <source>Dataset Title</source>
        <translation>Datensatz-titel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3103"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Name des Datensatzes, verwendet für Beschriftung und Identifikation</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3133"/>
        <source>Hide on Dashboard</source>
        <translation>Im Dashboard ausblenden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3134"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Eigenständige Dashboard-Kachel dieses Datensatzes unterdrücken; das Painter-Widget kann seine Werte weiterhin lesen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3180"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Untere Grenze des Datensatz-Wertebereichs; Widgets und FFT verwenden diesen Wert, wenn ihr eigener Bereich nicht festgelegt ist</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3193"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Obere Grenze des Datensatz-Wertebereichs; Widgets und FFT verwenden diesen Wert, wenn ihr eigener Bereich nicht festgelegt ist</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3253"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Zeit oder einen Datensatz für die X-Achse in Diagrammen wählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3266"/>
        <source>Frequency Analysis</source>
        <translation>Frequenzanalyse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3314"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Wählen Sie Zeit (Standard) oder einen beliebigen Datensatz, dessen Wert die Y-Achse steuert -- erzeugt ein Campbell-Diagramm, wenn z. B. an Drehzahl gebunden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3365"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3454"/>
        <source>Minimum Value (optional)</source>
        <translation>Minimalwert (optional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3366"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Untere Grenze für die Datennormalisierung; verwendet den Wertebereich des Datensatzes, wenn nicht gesetzt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3378"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3467"/>
        <source>Maximum Value (optional)</source>
        <translation>Maximalwert (optional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3379"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Obere Grenze für die Datennormalisierung; verwendet den Wertebereich des Datensatzes, wenn nicht gesetzt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3455"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Untere Grenze des Messgerät- oder Balkenbereichs; verwendet den Wertebereich des Datensatzes, wenn nicht gesetzt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3468"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Obere Grenze des Messgerät- oder Balkenbereichs; verwendet den Wertebereich des Datensatzes, wenn nicht gesetzt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3633"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4988"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Rohe und transformierte Werte für jeden Datensatz (schreibgeschützt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4998"/>
        <source>Shared table defined in this project</source>
        <translation>In diesem Projekt definierte gemeinsame Tabelle</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5350"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>1 Widget-Referenz entfernen, deren Zielgruppe oder Datensatz nicht mehr existiert?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5351"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>%1 Widget-Referenzen entfernen, deren Zielgruppen oder Datensätze nicht mehr existieren?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5356"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Dies betrifft nur die Platzierung der Arbeitsbereich-Kacheln; keine Gruppen, Datensätze oder Daten werden gelöscht.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5359"/>
        <source>Clean Up Workspaces</source>
        <translation>Arbeitsbereiche Bereinigen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3147"/>
        <source>Frame Index</source>
        <translation>Frame-index</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3148"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Frame-Position zur zeitlichen Ausrichtung von Datensätzen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3157"/>
        <source>Measurement Unit</source>
        <translation>Maßeinheit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3158"/>
        <source>Volts, Amps, etc.</source>
        <translation>Volt, Ampere, usw.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3159"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Maßeinheit, z. B. Volt oder Ampere (optional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3207"/>
        <source>Plot Settings</source>
        <translation>Plot-einstellungen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3230"/>
        <source>Enable Plot Widget</source>
        <translation>Plot-widget Aktivieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3232"/>
        <source>Plot data in real-time</source>
        <translation>Daten in Echtzeit plotten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3252"/>
        <source>X-Axis Source</source>
        <translation>X-achsen-quelle</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Datensatz für die X-Achse in Plots auswählen</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Minimaler Plot-Wert (optional)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Untere Grenze für den Plot-Anzeigebereich</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Maximaler Plot-Wert (optional)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Obere Grenze für den Plot-Anzeigebereich</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">FFT-konfiguration</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3277"/>
        <source>Enable FFT Analysis</source>
        <translation>FFT-analyse Aktivieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3278"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Frequenzbereichsanalyse des Datensatzes durchführen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3288"/>
        <source>Enable Waterfall Plot</source>
        <translation>Wasserfall-plot Aktivieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3289"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Zeigt ein scrollendes Spektrogramm des Frequenzinhalts über die Zeit (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3313"/>
        <source>Waterfall Y Axis</source>
        <translation>Wasserfall-y-achse</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="obsolete">Wählen Sie Zeit (Standard) oder einen beliebigen Datensatz, dessen Wert die Y-Achse steuert — erzeugt ein Campbell-Diagramm, wenn z. B. an Drehzahl gebunden</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3342"/>
        <source>FFT Window Size</source>
        <translation>FFT-fenstergröße</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3343"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Anzahl der Samples pro FFT-Berechnungsfenster</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3354"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>FFT-Abtastrate (Hz, erforderlich)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3355"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Abtastfrequenz für FFT (in Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Minimalwert (empfohlen)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Untere Grenze für Datennormalisierung</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Maximalwert (empfohlen)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Obere Grenze für Datennormalisierung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3404"/>
        <source>Widget Settings</source>
        <translation>Widget-einstellungen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3427"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3428"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Wählen Sie das visuelle Widget zur Anzeige dieses Datensatzes</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Minimaler Anzeigewert (erforderlich)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Untere Grenze des Anzeigenbereichs für Messgerät oder Balken</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Maximaler Anzeigewert (erforderlich)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Obere Grenze des Anzeigenbereichs für Messgerät oder Balken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3484"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3646"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3485"/>
        <source>Tick Count</source>
        <translation>Anzahl der Teilstriche</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3489"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Anzahl der Haupt-Teilstriche auf der Skala (0 = automatische Anpassung an Widget-Größe)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3508"/>
        <source>Label Format</source>
        <translation>Beschriftungsformat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3509"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Dezimalstellen oder Notation für Teilstrich-Beschriftungen und Wertanzeige</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Wertanzeige Anzeigen</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Umschalten der numerischen Anzeige im Rahmen, die unter oder neben dem Widget sitzt</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Alarmeinstellungen</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Alarme Aktivieren</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Löst einen visuellen Alarm aus, wenn der Wert die Alarmschwellen überschreitet</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Untere Schwelle</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Löst einen visuellen Alarm aus, wenn der Wert unter diese Schwelle fällt</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Obere Schwelle</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Löst einen visuellen Alarm aus, wenn der Wert diese Schwelle überschreitet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3546"/>
        <source>LED Display Settings</source>
        <translation>LED-anzeigeeinstellungen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3557"/>
        <source>Show in LED Panel</source>
        <translation>In LED-Panel Anzeigen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3558"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Aktiviert visuelle Statusüberwachung mittels LED-Anzeige</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3569"/>
        <source>LED On Threshold (required)</source>
        <translation>LED-Einschaltschwelle (erforderlich)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3570"/>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation>LED leuchtet, wenn der Wert diese Schwelle erreicht oder überschreitet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3591"/>
        <source>Off</source>
        <translation>Aus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3591"/>
        <source>Auto Start</source>
        <translation>Automatischer Start</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3591"/>
        <source>Start on Trigger</source>
        <translation>Start bei Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3591"/>
        <source>Toggle on Trigger</source>
        <translation>Umschalten bei Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3592"/>
        <source>Repeat N Times</source>
        <translation>N-mal Wiederholen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Plain Text (UTF8)</source>
        <translation>Klartext (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Hexadecimal</source>
        <translation>Hexadezimal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3597"/>
        <source>Binary (Direct)</source>
        <translation>Binär (Direkt)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>No Checksum</source>
        <translation>Keine Prüfsumme</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3609"/>
        <source>End Delimiter Only</source>
        <translation>Nur End-trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3609"/>
        <source>Start Delimiter Only</source>
        <translation>Nur Start-trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3610"/>
        <source>Start + End Delimiter</source>
        <translation>Start- + End-trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3610"/>
        <source>No Delimiters</source>
        <translation>Keine Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3620"/>
        <source>Button</source>
        <translation>Schaltfläche</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3620"/>
        <source>Slider</source>
        <translation>Schieberegler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3620"/>
        <source>Toggle</source>
        <translation>Umschalter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3620"/>
        <source>Text Field</source>
        <translation>Textfeld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3621"/>
        <source>Knob</source>
        <translation>Drehregler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3626"/>
        <source>Data Grid</source>
        <translation>Datenraster</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3627"/>
        <source>GPS Map</source>
        <translation>GPS-karte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3628"/>
        <source>Gyroscope</source>
        <translation>Gyroskop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3629"/>
        <source>Multiple Plot</source>
        <translation>Mehrfachdiagramm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3630"/>
        <source>Accelerometer</source>
        <translation>Beschleunigungsmesser</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3631"/>
        <source>3D Plot</source>
        <translation>3D-diagramm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3632"/>
        <source>Image View</source>
        <translation>Bildansicht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3634"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3655"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3639"/>
        <source>Bar</source>
        <translation>Balken</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3640"/>
        <source>Gauge</source>
        <translation>Anzeige</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3641"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3642"/>
        <source>Meter</source>
        <translation>Messgerät</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3647"/>
        <source>Integer (0 decimals)</source>
        <translation>Ganzzahl (0 Dezimalstellen)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3648"/>
        <source>1 decimal</source>
        <translation>1 Dezimalstelle</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3649"/>
        <source>2 decimals</source>
        <translation>2 Dezimalstellen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3650"/>
        <source>3 decimals</source>
        <translation>3 Dezimalstellen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Scientific</source>
        <translation>Wissenschaftlich</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3656"/>
        <source>New Line (\n)</source>
        <translation>Neue Zeile (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3657"/>
        <source>Carriage Return (\r)</source>
        <translation>Wagenrücklauf (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3658"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3662"/>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3663"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4688"/>
        <source>Label</source>
        <translation>Beschriftung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4689"/>
        <source>Display label</source>
        <translation>Beschriftung anzeigen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4699"/>
        <source>Button Icon</source>
        <translation>Schaltflächensymbol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4708"/>
        <source>Colorize Icon</source>
        <translation>Symbol Einfärben</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4709"/>
        <source>Tint the icon with the button color</source>
        <translation>Symbol mit der Schaltflächenfarbe einfärben</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4726"/>
        <source>Initial Value</source>
        <translation>Anfangswert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4738"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Zeichenkodierung, die verwendet wird, wenn transmit() einen String-Wert zurückgibt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4756"/>
        <source>Value Range</source>
        <translation>Wertebereich</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3179"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4766"/>
        <source>Minimum Value</source>
        <translation>Minimalwert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3192"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4775"/>
        <source>Maximum Value</source>
        <translation>Maximalwert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4784"/>
        <source>Step Size</source>
        <translation>Schrittgröße</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="527"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="536"/>
        <source>Lock Project</source>
        <translation>Projekt Sperren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="528"/>
        <source>Choose a password to lock the project:</source>
        <translation>Passwort zum Sperren des Projekts wählen:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="536"/>
        <source>Confirm the password:</source>
        <translation>Passwort bestätigen:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="541"/>
        <source>Passwords do not match</source>
        <translation>Passwörter Stimmen Nicht Überein</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="542"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>Die beiden eingegebenen Passwörter stimmen nicht überein. Das Projekt wurde nicht gesperrt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="576"/>
        <source>Unlock Project</source>
        <translation>Projekt Entsperren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="577"/>
        <source>Enter the project password:</source>
        <translation>Projektpasswort eingeben:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="587"/>
        <source>Incorrect password</source>
        <translation>Falsches Passwort</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="588"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>Das eingegebene Passwort stimmt nicht mit dem in der Projektdatei gespeicherten überein.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="620"/>
        <source>New Project</source>
        <translation>Neues Projekt</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Abtastwerte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1231"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Mehrere Datenquellen erfordern eine Pro-Lizenz</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1232"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro ermöglicht die gleichzeitige Verbindung mit mehreren Geräten. Bitte upgraden Sie, um diese Funktion freizuschalten.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1245"/>
        <source>Device %1</source>
        <translation>Gerät %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Kopie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1528"/>
        <source>Do you want to save your changes?</source>
        <translation>Änderungen speichern?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1529"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Dieses Projekt enthält nicht gespeicherte Änderungen!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="396"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="406"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="420"/>
        <source>Project error</source>
        <translation>Projektfehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="396"/>
        <source>Project title cannot be empty!</source>
        <translation>Projekttitel darf nicht leer sein!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="406"/>
        <source>You need to add at least one group!</source>
        <translation>Mindestens eine Gruppe muss hinzugefügt werden!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="420"/>
        <source>You need to add at least one dataset!</source>
        <translation>Mindestens ein Datensatz muss hinzugefügt werden!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="468"/>
        <source>Your project needs a title</source>
        <translation>Ihr Projekt benötigt einen Titel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="470"/>
        <source>Add a group to get started</source>
        <translation>Gruppe hinzufügen, um zu beginnen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="472"/>
        <source>Add a dataset to a group</source>
        <translation>Datensatz zu einer Gruppe hinzufügen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="486"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Öffnen Sie die Projektansicht oben im Baum und geben Sie einen Namen ein. Sie können das Projekt jederzeit umbenennen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="489"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>Gruppen organisieren Datensätze in Dashboard-Widgets. Verwenden Sie die Gruppe-Schaltfläche in der Symbolleiste oben, um eine zu erstellen, und fügen Sie dann Datensätze hinzu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="493"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>Datensätze sind die Werte, die im Dashboard angezeigt werden. Wählen Sie eine Gruppe im Baum aus und verwenden Sie die Datensatz-Schaltfläche in der Symbolleiste, um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="774"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="822"/>
        <source>Time</source>
        <translation>Zeit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1280"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Möchten Sie die Datenquelle „%1" löschen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1281"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>Gruppen, die diese Quelle verwenden, werden zur Standardquelle verschoben. Diese Aktion kann nicht rückgängig gemacht werden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1568"/>
        <source>Save Serial Studio Project</source>
        <translation>Serial Studio-projekt Speichern</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1570"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2192"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Serial Studio-Projektdateien (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1592"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1828"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2183"/>
        <source>Untitled Project</source>
        <translation>Unbenanntes Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1838"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2344"/>
        <source>Device A</source>
        <translation>Gerät A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2012"/>
        <source>Select Project File</source>
        <translation>Projektdatei Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2014"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>Projektdateien (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2063"/>
        <source>JSON validation error</source>
        <translation>JSON-Validierungsfehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2158"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Projekt aus älterem Dateiformat aktualisiert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2159"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Dieses Projekt wurde mit Schema-Version %1 gespeichert; die aktuelle Version ist %2. Für neue Felder wurden Standardwerte übernommen. Projekt speichern, um das Upgrade abzuschließen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2190"/>
        <source>Save Imported Project</source>
        <translation>Importiertes Projekt Speichern</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2389"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>Projekte mit mehreren Quellen erfordern eine Pro-Lizenz</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2390"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Dieses Projekt enthält mehrere Datenquellen. Nur die erste Quelle wurde geladen. Eine Serial Studio Pro-Lizenz ist erforderlich, um Projekte mit mehreren Quellen zu verwenden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2626"/>
        <source>Workspace IDs remapped on load</source>
        <translation>Arbeitsbereich-IDs beim Laden neu zugeordnet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2627"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 benutzerdefinierte Arbeitsbereichs-IDs überlappten mit dem neuen reservierten automatischen Bereich und wurden in den Benutzerbereich verschoben. Projekt speichern, um die Zuordnung dauerhaft zu machen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2772"/>
        <source>Legacy frame parser function updated</source>
        <translation>Legacy-Frame-Parser-Funktion aktualisiert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2773"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Ihr Projekt verwendete eine veraltete Frame-Parser-Funktion mit einem 'separator'-Argument. Sie wurde automatisch in das neue Format migriert.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2973"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Möchten Sie die Gruppe „%1" löschen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2974"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3025"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3060"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3827"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Diese Aktion kann nicht rückgängig gemacht werden. Möchten Sie fortfahren?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3024"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Möchten Sie die Aktion „%1" löschen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3059"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Möchten Sie den Datensatz „%1" löschen?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Kopie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3735"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3774"/>
        <source>Output Controls</source>
        <translation>Ausgabesteuerungen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3787"/>
        <source>New Button</source>
        <translation>Neue Schaltfläche</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3790"/>
        <source>New Slider</source>
        <translation>Neuer Schieberegler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3793"/>
        <source>New Toggle</source>
        <translation>Neuer Umschalter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3796"/>
        <source>New Text Field</source>
        <translation>Neues Textfeld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3799"/>
        <source>New Knob</source>
        <translation>Neuer Drehregler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3826"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Ausgabe-Widget „%1" löschen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3999"/>
        <source>Group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4018"/>
        <source>New Dataset</source>
        <translation>Neuer Datensatz</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4021"/>
        <source>New Plot</source>
        <translation>Neues Diagramm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4025"/>
        <source>New FFT Plot</source>
        <translation>Neues FFT-diagramm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4029"/>
        <source>New Level Indicator</source>
        <translation>Neue Pegelanzeige</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4033"/>
        <source>New Gauge</source>
        <translation>Neuer Messwertanzeiger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4037"/>
        <source>New Compass</source>
        <translation>Neuer Kompass</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4043"/>
        <source>New Meter</source>
        <translation>Neues Messgerät</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Neues Thermometer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4047"/>
        <source>New LED Indicator</source>
        <translation>Neue LED-anzeige</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4051"/>
        <source>New Waterfall</source>
        <translation>Neuer Wasserfall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4121"/>
        <source>Channel %1</source>
        <translation>Kanal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4194"/>
        <source>New Action</source>
        <translation>Neue Aktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4337"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Möchten Sie das Widget auf Gruppenebene wirklich ändern?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4339"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>Vorhandene Datensätze für diese Gruppe werden gelöscht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4403"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4404"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4405"/>
        <source>Accelerometer %1</source>
        <translation>Beschleunigungssensor %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4420"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4420"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4420"/>
        <source>Gyro %1</source>
        <translation>Gyroskop %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4435"/>
        <source>Latitude</source>
        <translation>Breitengrad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4435"/>
        <source>Longitude</source>
        <translation>Längengrad</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4435"/>
        <source>Altitude</source>
        <translation>Höhe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4450"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4464"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4450"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4464"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4450"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4464"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4739"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5635"/>
        <source>Workspace</source>
        <translation>Arbeitsbereich</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4905"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5112"/>
        <source>Shared Table</source>
        <translation>Gemeinsame Tabelle</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4987"/>
        <source>register</source>
        <translation>Register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5112"/>
        <source>New Shared Table</source>
        <translation>Neue Gemeinsame Tabelle</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5112"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5130"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5149"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5173"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5200"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5219"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5242"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5265"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5635"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5656"/>
        <source>Name:</source>
        <translation>Name:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5130"/>
        <source>Rename Table</source>
        <translation>Tabelle Umbenennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5149"/>
        <source>Rename Group</source>
        <translation>Gruppe Umbenennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5173"/>
        <source>Rename Dataset</source>
        <translation>Datensatz Umbenennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5200"/>
        <source>Rename Data Source</source>
        <translation>Datenquelle Umbenennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5219"/>
        <source>Rename Action</source>
        <translation>Aktion Umbenennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5241"/>
        <source>New Register</source>
        <translation>Neues Register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5265"/>
        <source>Rename Register</source>
        <translation>Register Umbenennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5304"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5329"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6183"/>
        <source>This action cannot be undone.</source>
        <translation>Diese Aktion kann nicht rückgängig gemacht werden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5305"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Dadurch werden %1 Register zusammen mit der Tabelle entfernt. Diese Aktion kann nicht rückgängig gemacht werden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5308"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5328"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6182"/>
        <source>Delete "%1"?</source>
        <translation>„%1" Löschen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5311"/>
        <source>Delete Table</source>
        <translation>Tabelle Löschen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5331"/>
        <source>Delete Register</source>
        <translation>Register Löschen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5355"/>
        <source>Export Table</source>
        <translation>Tabelle Exportieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5357"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5401"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5399"/>
        <source>Import Table</source>
        <translation>Tabelle Importieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5635"/>
        <source>New Workspace</source>
        <translation>Neuer Arbeitsbereich</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5656"/>
        <source>Rename Workspace</source>
        <translation>Arbeitsbereich Umbenennen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5745"/>
        <source>Overview</source>
        <translation>Übersicht</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5755"/>
        <source>All Data</source>
        <translation>Alle Daten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5939"/>
        <source>Discard workspace customisations?</source>
        <translation>Arbeitsbereich-Anpassungen Verwerfen?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5940"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Das Deaktivieren von „Anpassen" verwirft Ihre Änderungen und erstellt die Arbeitsbereichsliste aus den Gruppen des Projekts neu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5943"/>
        <source>Customize Workspaces</source>
        <translation>Arbeitsbereiche Anpassen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6185"/>
        <source>Delete Workspace</source>
        <translation>Arbeitsbereich Löschen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6526"/>
        <source>File save error</source>
        <translation>Fehler beim Speichern der Datei</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2227"/>
        <source>File open error</source>
        <translation>Fehler beim Öffnen der Datei</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="907"/>
        <source>Import Protocol Buffers File</source>
        <translation>Protocol-buffers-datei Importieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="909"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Proto-Dateien (*.proto);;Alle Dateien (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="944"/>
        <source>Failed to open proto file: %1</source>
        <translation>Fehler beim Öffnen der Proto-Datei: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="945"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Dateipfad und Leseberechtigungen überprüfen und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Protobuf Import Error</source>
        <translation>Protobuf-importfehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="962"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Proto-Datei konnte nicht geparst werden in Zeile %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="963"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>Nur proto3-Syntax wird unterstützt. Dateiformat überprüfen und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="970"/>
        <source>Proto file contains no message definitions</source>
        <translation>Proto-Datei enthält keine Message-Definitionen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="971"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Die ausgewählte Datei enthält keine `message`-Blöcke zum Importieren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Protobuf Import Warning</source>
        <translation>Protobuf-importwarnung</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Importiertes Projekt konnte nicht geladen werden</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Das generierte Projekt-JSON konnte nicht geladen werden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1011"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>%1 Message(s) und %2 Feld(er) erfolgreich aus der Proto-Datei importiert.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>The project editor is now open for customization.</source>
        <translation>Der Projekteditor ist jetzt zur Anpassung geöffnet.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1016"/>
        <source>Protobuf Import Complete</source>
        <translation>Protobuf-import Abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1051"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
</context>
<context>
    <name>DataModel::TransmitTestDialog</name>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="158"/>
        <source>Invalid Hex Input</source>
        <translation>Ungültige Hex-eingabe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="159"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Bitte gültige hexadezimale Bytes eingeben.

Gültiges Format: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="165"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Kein Transmit-Funktionscode zur Auswertung vorhanden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="185"/>
        <source>transmit function is not callable</source>
        <translation>Transmit-Funktion ist nicht aufrufbar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="249"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="250"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="251"/>
        <source>Evaluate</source>
        <translation>Auswerten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="252"/>
        <source>Input Value</source>
        <translation>Eingabewert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="253"/>
        <source>Transmit Function Output</source>
        <translation>Ausgabe der Übertragungsfunktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="254"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="278"/>
        <source>Enter value to transmit…</source>
        <translation>Zu übertragenden Wert eingeben…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="255"/>
        <source>Raw string output appears here</source>
        <translation>Rohe Zeichenfolgenausgabe erscheint hier</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="256"/>
        <source>Hex byte output appears here</source>
        <translation>Hex-Byte-Ausgabe erscheint hier</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="259"/>
        <source>Test Transmit Function</source>
        <translation>Übertragungsfunktion Testen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="272"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Hex-Bytes eingeben (z. B. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="383"/>
        <source>(empty) No data returned</source>
        <translation>(leer) Keine Daten zurückgegeben</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="385"/>
        <source>0 bytes</source>
        <translation>0 Bytes</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="426"/>
        <source>%1 byte(s)</source>
        <translation>%1 Byte(s)</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>Gemeinsamer Speicher</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>Gemeinsame Tabelle Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>Gemeinsame Tabelle hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>Hilfedokumentation für gemeinsamen Speicher öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>Beschreibung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>Einträge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="274"/>
        <source>No shared tables.</source>
        <translation>Keine gemeinsamen Tabellen.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Sitzungen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Sitzungsdatei öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Sitzungsdatei schließen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Wiedergabe</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Ausgewählte Sitzung im Dashboard wiedergeben</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Sitzungsdatei entsperren, um Sitzungen zu löschen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Ausgewählte Sitzung löschen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Entsperren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Sperren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Sitzungsdatei entsperren, um Löschungen zu ermöglichen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Passwort festlegen, um Sitzungslöschungen zu verhindern</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>CSV Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Ausgewählte Sitzung nach CSV exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>PDF Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>PDF-Bericht für die ausgewählte Sitzung erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Projekt Wiederherstellen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Projektdatei aus dieser Sitzungsdatei wiederherstellen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Sitzung wird geladen…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>Verarbeitung…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-Funktionen in diesem Projekt erkannt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Ersatz-Widgets werden verwendet. Lizenz erwerben, um volle Funktionalität freizuschalten.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>Diagramme</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>Diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>2D-Diagrammvisualisierung für diesen Datensatz umschalten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>FFT-diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>FFT-Diagramm umschalten, um Frequenzinhalt zu visualisieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>Wasserfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Wasserfalldiagramm (Spektrogramm) umschalten — verwendet die FFT-Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>Widgets</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>Balken/pegel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Balken-/Pegelanzeige für diesen Datensatz umschalten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>Messgerät</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Messgerät-Widget für analoge Anzeige umschalten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Kompass-Widget für Richtungsdaten umschalten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>Messgerät</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Analoges Messgerät-Widget (Halbkreis) umschalten</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Thermometer-Widget für Temperaturdaten umschalten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>LED-Anzeige für binäre oder Schwellenwerte umschalten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>Verhalten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>Alarmbereiche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="304"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation>Definieren Sie farbige Wertebereiche mit Schweregraden für die Anzeige dieses Datensatzes.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="310"/>
        <source>Transform</source>
        <translation>Transformation</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Transformationsausdruck für Kalibrierung, Filterung oder Einheitenumrechnung bearbeiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>Duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="332"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Diesen Datensatz mit derselben Konfiguration duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="337"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="340"/>
        <source>Delete this dataset from the group</source>
        <translation>Diesen Datensatz aus der Gruppe löschen</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>Serial Studio Unterstützen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>Unterstützen Sie die Entwicklung von %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio ist freie &amp; quelloffene Software, die von Freiwilligen unterstützt wird. Erwägen Sie eine Spende oder den Erwerb einer Pro-Lizenz, um die Entwicklung zu unterstützen :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Sie können dieses Projekt auch unterstützen, indem Sie es teilen, Fehler melden und neue Funktionen vorschlagen!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>Spenden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>Serial Studio Pro Erwerben</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="127"/>
        <source>Stop</source>
        <translation>Stoppen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="128"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>Downloading updates</source>
        <translation>Updates Werden Heruntergeladen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="455"/>
        <source>Time remaining</source>
        <translation>Verbleibende Zeit</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <source>unknown</source>
        <translation>unbekannt</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Cannot find downloaded update!</source>
        <translation>Heruntergeladenes Update Kann Nicht Gefunden Werden!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="244"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="245"/>
        <source>Download complete!</source>
        <translation>Download Abgeschlossen!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="253"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Klicken Sie auf „OK", um die Installation des Updates zu starten</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="255"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>Um das Update zu installieren, muss die Anwendung möglicherweise beendet werden.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="246"/>
        <source>The installer opens separately</source>
        <translation>Das Installationsprogramm öffnet sich separat</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>Um das Update zu installieren, muss die Anwendung möglicherweise beendet werden. Dies ist ein obligatorisches Update; ein Abbruch führt zum Schließen der Anwendung.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="275"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Klicken Sie auf „Öffnen", um das Update anzuwenden</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="288"/>
        <source>Updater</source>
        <translation>Updater</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="292"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Soll der Download wirklich abgebrochen werden?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="294"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Soll der Download wirklich abgebrochen werden? Dies ist ein obligatorisches Update; ein Abbruch führt zum Schließen der Anwendung.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="349"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="356"/>
        <source>%1 bytes</source>
        <translation>%1 Bytes</translation>
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
        <translation>von</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="406"/>
        <source>Downloading Updates</source>
        <translation>Updates Werden Heruntergeladen</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Time Remaining</source>
        <translation>Verbleibende Zeit</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>about %1 hours</source>
        <translation>etwa %1 Stunden</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="433"/>
        <source>about one hour</source>
        <translation>etwa eine Stunde</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="441"/>
        <source>%1 minutes</source>
        <translation>%1 Minuten</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="443"/>
        <source>1 minute</source>
        <translation>1 Minute</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="450"/>
        <source>%1 seconds</source>
        <translation>%1 Sekunden</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="452"/>
        <source>1 second</source>
        <translation>1 Sekunde</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Verbleibende Zeit: 0 Minuten</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Öffnen</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>Beispiel-browser</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>Zurück</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Herunterladen &amp;&amp; Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>Auf GitHub Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>In Beispielen Suchen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="244"/>
        <source>Fetching examples…</source>
        <translation>Beispiele werden abgerufen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="567"/>
        <source>Loading...</source>
        <translation>Lädt...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>No README available.</source>
        <translation>Keine README verfügbar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="608"/>
        <source>Copied to Clipboard</source>
        <translation>In Zwischenablage Kopiert</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="671"/>
        <source>No screenshot available</source>
        <translation>Kein Screenshot Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="703"/>
        <source>Details</source>
        <translation>Details</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="732"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="755"/>
        <source>Category:</source>
        <translation>Kategorie:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="768"/>
        <source>Difficulty:</source>
        <translation>Schwierigkeitsgrad:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="786"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="828"/>
        <source>No Results Found</source>
        <translation>Keine Ergebnisse Gefunden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="839"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Rechtschreibung prüfen oder anderen Suchbegriff verwenden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="854"/>
        <source>%1 examples</source>
        <translation>%1 Beispiele</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="863"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>Erweiterungs-manager</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>Aktualisieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>Repos</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>Repository-einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>Zurück</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>Installieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>Deinstallieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>Ausführen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>Stoppen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>Erweiterungen suchen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>Erweiterungen werden abgerufen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>Läuft</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>Aktualisieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>Installiert</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>Nicht Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>Keine Beschreibung verfügbar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>Details</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>Typ:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>Autor:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>Version:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>Lizenz:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>Keine Vorschau</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>PLUGIN-AUSGABE</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Noch keine Ausgabe. Plugin ausführen, um das Protokoll hier anzuzeigen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>Keine Vorschau verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>Repositorys</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>URLs zu Remote-Repositorys oder lokale Ordnerpfade hinzufügen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>LOKAL</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL oder lokaler Pfad…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>Durchsuchen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>Keine Ergebnisse Gefunden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Überprüfen Sie die Schreibweise oder versuchen Sie einen anderen Suchbegriff.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>Keine Erweiterungen Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Fügen Sie eine Repository-URL oder einen lokalen Pfad in den Repo-Einstellungen hinzu und aktualisieren Sie dann.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 Erweiterungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="157"/>
        <source>Interpolation: %1</source>
        <translation>Interpolation: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="185"/>
        <source>Show Area Under Plot</source>
        <translation>Fläche Unter Diagramm Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="203"/>
        <source>Show X Axis Label</source>
        <translation>X-achsenbeschriftung Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="215"/>
        <source>Show Y Axis Label</source>
        <translation>Y-achsenbeschriftung Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="233"/>
        <source>Show Crosshair</source>
        <translation>Fadenkreuz Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Resume</source>
        <translation>Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="259"/>
        <source>Reset View</source>
        <translation>Ansicht Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="265"/>
        <source>Axis Range Settings</source>
        <translation>Achsenbereich-einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="294"/>
        <source>Magnitude (dB)</source>
        <translation>Magnitude (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="295"/>
        <source>Frequency (Hz)</source>
        <translation>Frequenz (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Projekte und CSV-Dateien hier ablegen</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>Dateiübertragung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>Übertragungsprotokoll:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>Dateiauswahl:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>Datei Auswählen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>Übertragungsintervall:</translation>
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
        <translation>Blockgröße:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>Bytes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>Zeitüberschreitung:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>Max. Wiederholungen:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>Fortschritt: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 Bytes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>Fehler: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>Aktivitätsprotokoll</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>Übertragung Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>Übertragung Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>Übertragung Stoppen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>Übertragung Beginnen</translation>
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
        <translation>Gerät %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="395"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1402"/>
        <source>Output Panel</source>
        <translation>Ausgabepanel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="439"/>
        <source>Control</source>
        <translation>Steuerung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="462"/>
        <source>Table</source>
        <translation>Tabelle</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="478"/>
        <source>%1 regs</source>
        <translation>%1 Reg.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="479"/>
        <source>empty</source>
        <translation>leer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="504"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-publisher</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="905"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Öffnet den Transform-Code-Editor für diesen Datensatz.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1184"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1187"/>
        <source>Dataset Container</source>
        <translation>Datensatz-container</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1196"/>
        <source>Multi-Plot</source>
        <translation>Multi-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1199"/>
        <source>Multiple Plot</source>
        <translation>Mehrfachdiagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1208"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1211"/>
        <source>Accelerometer</source>
        <translation>Beschleunigungsmesser</translation>
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
        <translation>GPS-karte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1246"/>
        <source>3D Plot</source>
        <translation>3D-diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1254"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1257"/>
        <source>Image View</source>
        <translation>Bildansicht</translation>
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
        <translation>Datenraster</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1294"/>
        <source>Generic</source>
        <translation>Generisch</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1307"/>
        <source>Plot</source>
        <translation>Diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1320"/>
        <source>FFT Plot</source>
        <translation>FFT-diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1333"/>
        <source>Gauge</source>
        <translation>Anzeige</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1346"/>
        <source>Level Indicator</source>
        <translation>Pegelanzeige</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1359"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1372"/>
        <source>Meter</source>
        <translation>Messgerät</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1385"/>
        <source>LED Indicator</source>
        <translation>LED-anzeige</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1414"/>
        <source>Slider</source>
        <translation>Schieberegler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1427"/>
        <source>Toggle</source>
        <translation>Umschalter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1440"/>
        <source>Knob</source>
        <translation>Drehregler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1453"/>
        <source>Text Field</source>
        <translation>Textfeld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1466"/>
        <source>Button</source>
        <translation>Schaltfläche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1490"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1565"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1652"/>
        <source>Add Group</source>
        <translation>Gruppe Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1505"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1667"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1712"/>
        <source>Add Dataset</source>
        <translation>Datensatz Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1519"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1594"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1681"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1933"/>
        <source>Add Output</source>
        <translation>Ausgabe Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1535"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1607"/>
        <source>Add Action</source>
        <translation>Aktion Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1544"/>
        <source>Add Data Source</source>
        <translation>Datenquelle Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1551"/>
        <source>Add Data Table</source>
        <translation>Datentabelle Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1618"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1753"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1820"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1948"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1982"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2038"/>
        <source>Rename…</source>
        <translation>Umbenennen…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1783"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1853"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1905"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1956"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Duplicate</source>
        <translation>Duplizieren</translation>
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
        <translation>Löschen…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1697"/>
        <source>Edit Frame Parser…</source>
        <translation>Frame-parser Bearbeiten…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1739"/>
        <source>Edit Painter Code…</source>
        <translation>Painter-code Bearbeiten…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1761"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1829"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1881"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1990"/>
        <source>Move Up</source>
        <translation>Nach Oben</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1772"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1841"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1893"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2001"/>
        <source>Move Down</source>
        <translation>Nach Unten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1809"/>
        <source>Edit Transform Code…</source>
        <translation>Transform-code Bearbeiten…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2064"/>
        <source>Edit Code…</source>
        <translation>Code Bearbeiten…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="345"/>
        <source>Action</source>
        <translation>Aktion</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">Noch keine Gruppen definiert</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Frame-parser Testen</translation>
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
        <translation>Zeilen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 Zeile(n)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Zeile %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Pipeline-konfiguration</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Erkennung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Ende</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Prüfsumme</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Hex-trennzeichen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Frame-dateneingabe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Hex-Bytes eingeben (z. B. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Rohe Stream-Bytes hier eingeben...</translation>
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
        <translation>Das Beispiel enthält nicht die konfigurierten Frame-Trennzeichen, daher wird kein Frame extrahiert. Geben Sie diese in das Beispiel ein (z. B. 
 für einen Zeilenumbruch) oder passen Sie den Erkennungsmodus an.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Pipeline-ergebnisse</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Bereitstellen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Wert</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>Extraktion hat keinen vollständigen Frame erzeugt. Überprüfen Sie die Start-/End-Trennzeichen und den Erkennungsmodus.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Geben Sie oben Beispieldaten ein und drücken Sie Auswerten, um eine Vorschau der geparsten Ausgabe anzuzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Auswerten</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Rückgängig</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Wiederholen</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Ausschneiden</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Kopieren</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Einfügen</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Alles Auswählen</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Dokument Formatieren</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Auswahl Formatieren</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Zurücksetzen</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Auf Standard-Parsing-Skript zurücksetzen</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Öffnen</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Skriptdatei für Datenanalyse importieren</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Hilfedokumentation für Datenanalyse öffnen</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Sprache:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Vorlage Auswählen…</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Mit Beispieldaten Testen</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Auswerten</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Letzte Codeänderung rückgängig machen</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Zuvor rückgängig gemachte Änderung wiederherstellen</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Ausgewählten Code in Zwischenablage ausschneiden</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Ausgewählten Code in Zwischenablage kopieren</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Code aus Zwischenablage einfügen</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Hilfe</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Automatisch Zentrieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Trajektorie Zeichnen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Vergrößern</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Verkleinern</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Wetter Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>Nasa-wetter-overlay</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Basiskarte: %1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-Funktionen in diesem Projekt erkannt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Ersatz-Widgets werden verwendet. Lizenz erwerben, um die volle Funktionalität freizuschalten.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>Datensatz Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>Datensatz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Generischen Datensatz zur aktuellen Gruppe hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>Diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>2D-Diagramm zur Visualisierung numerischer Daten hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>FFT-diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>FFT-Diagramm zur Visualisierung im Frequenzbereich hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>Wasserfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Wasserfalldiagramm (Spektrogramm) hinzufügen — verwendet die FFT-Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>Balken/pegel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Balken- oder Pegelanzeige für skalierte Werte hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>Messgerät</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Messgerät-Widget für analoge Visualisierung hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Kompass zur Anzeige von Richtungs- oder Winkeldaten hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>Messgerät</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Analoges Messgerät-Widget (Halbkreis) hinzufügen</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Thermometer-Widget für Temperaturdaten hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>LED-Indikator für binäre Statussignale hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>Steuerung Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>Schaltfläche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>Schaltfläche hinzufügen, die bei Klick einen Befehl sendet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>Schieberegler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Schieberegler zum Senden skalierter numerischer Werte hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>Umschalter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Umschalter für Ein/Aus-Befehle hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>Textfeld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Textfeld zum Eingeben und Senden von Befehlen hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>Drehregler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Drehregler für Sollwertsteuerung hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>Code Bearbeiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Bearbeiten Sie das JavaScript, das dieses Painter-Widget zeichnet</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>Duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Aktuelle Gruppe und deren Inhalte duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Aktuelle Gruppe und alle enthaltenen Datensätze löschen</translation>
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
        <translation>HID-gerät</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Usage Page</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Usage</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Gamepads, Joysticks, Lenkräder, Flight-Controller und andere USB-Geräte der HID-Klasse verbinden.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>HID Usage Tables (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>Hilfecenter</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>Hilfeseiten werden abgerufen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>Suchen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>Lädt…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>Seite aus der Seitenleiste auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>In Zwischenablage kopiert</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>Online Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>%1 Seiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>IO::ConnectionManager</name>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="271"/>
        <source>UART/COM</source>
        <translation>UART/COM</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="272"/>
        <source>Network Socket</source>
        <translation>Netzwerk-socket</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>CAN Bus</source>
        <translation>CAN-bus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>USB Device</source>
        <translation>USB-gerät</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>HID Device</source>
        <translation>HID-gerät</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="280"/>
        <source>Process</source>
        <translation>Prozess</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="281"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT-abonnent</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="631"/>
        <source>Your trial period has ended.</source>
        <translation>Der Testzeitraum ist abgelaufen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="632"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Zur weiteren Nutzung von Serial Studio die Lizenz aktivieren.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="764"/>
        <source>channels</source>
        <translation>Kanäle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="817"/>
        <source> channels</source>
        <translation>Kanäle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="965"/>
        <source>Unsigned 8-bit</source>
        <translation>Unsigned 8-Bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="966"/>
        <source>Signed 16-bit</source>
        <translation>Signed 16-Bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="967"/>
        <source>Signed 24-bit</source>
        <translation>Signed 24-Bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="968"/>
        <source>Signed 32-bit</source>
        <translation>Signed 32-Bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="969"/>
        <source>Float 32-bit</source>
        <translation>Float 32-Bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="972"/>
        <source>Mono</source>
        <translation>Mono</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="973"/>
        <source>Stereo</source>
        <translation>Stereo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1375"/>
        <source>Input Device</source>
        <translation>Eingabegerät</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1383"/>
        <source>Sample Rate</source>
        <translation>Abtastrate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1391"/>
        <source>Sample Format</source>
        <translation>Abtastformat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1399"/>
        <source>Channels</source>
        <translation>Kanäle</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="71"/>
        <source>BLE I/O Module Error</source>
        <translation>BLE-e/a-modul-fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="325"/>
        <source>Select Device</source>
        <translation>Gerät Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="336"/>
        <source>Select Service</source>
        <translation>Dienst Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="347"/>
        <source>Select Characteristic</source>
        <translation>Charakteristik Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="514"/>
        <source>Error while configuring BLE service</source>
        <translation>Fehler beim Konfigurieren des BLE-Dienstes</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="677"/>
        <source>Operation error</source>
        <translation>Operationsfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="680"/>
        <source>Characteristic write error</source>
        <translation>Charakteristik-Schreibfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="683"/>
        <source>Descriptor write error</source>
        <translation>Deskriptor-Schreibfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="686"/>
        <source>Unknown error</source>
        <translation>Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="689"/>
        <source>Characteristic read error</source>
        <translation>Fehler beim Lesen der Characteristic</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="692"/>
        <source>Descriptor read error</source>
        <translation>Fehler beim Lesen des Descriptors</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="942"/>
        <source>BLE Device</source>
        <translation>BLE-gerät</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="950"/>
        <source>Service</source>
        <translation>Dienst</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="969"/>
        <source>Characteristic</source>
        <translation>Characteristic</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::CANBus</name>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="261"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="267"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="273"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="278"/>
        <source>CAN Bus Not Available</source>
        <translation>CAN-bus Nicht Verfügbar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="274"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>Keine CAN-Bus-Plugins auf diesem System gefunden.

CAN-Bus-Unterstützung unter macOS ist eingeschränkt und erfordert möglicherweise Hardware-Treiber von Drittanbietern.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="279"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Auf dieser Plattform sind keine CAN-Bus-Plugins verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="291"/>
        <source>Invalid CAN Configuration</source>
        <translation>Ungültige CAN-konfiguration</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="299"/>
        <source>Invalid Selection</source>
        <translation>Ungültige Auswahl</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="308"/>
        <source>No Devices Available</source>
        <translation>Keine Geräte Verfügbar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="224"/>
        <source>CAN Device Creation Failed</source>
        <translation>CAN-geräteerstellung Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="242"/>
        <source>CAN Connection Failed</source>
        <translation>CAN-verbindung Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="262"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>Keine CAN-Bus-Plugins auf diesem System gefunden.

Unter Linux sicherstellen, dass SOCKETCAN-Kernelmodule geladen sind.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="268"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>Keine CAN-Bus-Plugins auf diesem System gefunden.

Unter Windows CAN-Hardware-Treiber installieren (PEAK, VECTOR, usw.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="292"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>Die CAN-Bus-Konfiguration ist unvollständig. Ein gültiges Plugin und eine Schnittstelle auswählen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="300"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>Das ausgewählte Plugin oder die Schnittstelle ist nicht mehr verfügbar. Listen aktualisieren und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="309"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>Die Plugin- oder Schnittstellenliste ist leer. Listen aktualisieren und sicherstellen, dass die CAN-Hardware angeschlossen ist.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="225"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>CAN-Bus-Gerät kann nicht erstellt werden. Hardware und Treiber überprüfen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="244"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Verbindung zum CAN-Bus-Gerät nicht möglich. Hardwareverbindung und Einstellungen überprüfen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="582"/>
        <source>CAN Bus Error</source>
        <translation>CAN-bus-fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="583"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>Ein Fehler ist aufgetreten, aber das CAN-Gerät ist nicht mehr verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="590"/>
        <source>Error code: %1</source>
        <translation>Fehlercode: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="593"/>
        <source>CAN Bus Communication Error</source>
        <translation>CAN-bus-kommunikationsfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="648"/>
        <source>No CAN driver selected</source>
        <translation>Kein CAN-Treiber ausgewählt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="609"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>SOCKETCAN-Kernelmodule zuerst laden</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="612"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Zuerst eine virtuelle CAN-Schnittstelle einrichten</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="614"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="634"/>
        <source>No interfaces found for %1</source>
        <translation>Keine Schnittstellen für %1 gefunden</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="618"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK-CAN-Treiber&lt;/a&gt; installieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="622"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;VECTOR-CAN-Treiber&lt;/a&gt; installieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="626"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN-Treiber&lt;/a&gt; installieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="629"/>
        <source>Install %1 drivers</source>
        <translation>%1-Treiber installieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="632"/>
        <source>Install %1 drivers for macOS</source>
        <translation>%1-Treiber für macOS installieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="729"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="737"/>
        <source>Interface</source>
        <translation>Schnittstelle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="745"/>
        <source>Bitrate</source>
        <translation>Bitrate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="754"/>
        <source>CAN FD</source>
        <translation>CAN-FD</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="178"/>
        <source>Unknown error</source>
        <translation>Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="181"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Prüfen, ob der Benutzer in der Gruppe 'plugdev' ist oder eine udev-Regel Zugriff auf dieses Gerät gewährt.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="185"/>
        <source>Failed to open "%1"</source>
        <translation>Öffnen von „%1" fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="295"/>
        <source>HID Device Error</source>
        <translation>HID-gerätefehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="296"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>Das HID-Gerät wurde getrennt oder es ist ein schwerwiegender Lesefehler aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="405"/>
        <source>Select Device</source>
        <translation>Gerät Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="549"/>
        <source>HID Device</source>
        <translation>HID-gerät</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::MQTT</name>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="56"/>
        <source>MQTT 3.1</source>
        <translation>MQTT 3.1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="57"/>
        <source>MQTT 3.1.1</source>
        <translation>MQTT 3.1.1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="58"/>
        <source>MQTT 5.0</source>
        <translation>MQTT 5.0</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="61"/>
        <source>TLS 1.2</source>
        <translation>TLS 1.2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>TLS 1.3</source>
        <translation>TLS 1.3</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>TLS 1.3 or Later</source>
        <translation>TLS 1.3 oder Neuer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 oder Neuer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="65"/>
        <source>Any Protocol</source>
        <translation>Beliebiges Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>Secure Protocols Only</source>
        <translation>Nur Sichere Protokolle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="70"/>
        <source>Query Peer</source>
        <translation>Peer Abfragen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="71"/>
        <source>Verify Peer</source>
        <translation>Peer Verifizieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="72"/>
        <source>Auto Verify Peer</source>
        <translation>Peer Automatisch Verifizieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="177"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>MQTT-Funktion Erfordert eine Kommerzielle Lizenz</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">Das Abonnieren eines MQTT-Brokers ist nur mit einer gültigen kommerziellen Serial Studio-Lizenz (Hobbyist-Stufe oder höher) verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="178"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>Das Abonnieren eines MQTT-Brokers ist nur mit einer gültigen Serial Studio-Lizenz oder einer aktiven Testversion verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="408"/>
        <source>Use System Database</source>
        <translation>Systemdatenbank Verwenden</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="409"/>
        <source>Load From Folder…</source>
        <translation>Aus Ordner Laden…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="442"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Pem-zertifikatsverzeichnis Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Hostname</source>
        <translation>Hostname</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="714"/>
        <source>Topic Filter</source>
        <translation>Themenfilter</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="721"/>
        <source>Client ID</source>
        <translation>Client-ID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="728"/>
        <source>Username</source>
        <translation>Benutzername</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="735"/>
        <source>Password</source>
        <translation>Passwort</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="742"/>
        <source>MQTT Version</source>
        <translation>MQTT-version</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="757"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="766"/>
        <source>Auto Keep Alive</source>
        <translation>Auto Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="783"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS Aktiviert</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>SSL Protocol</source>
        <translation>SSL-protokoll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="801"/>
        <source>Peer Verify Mode</source>
        <translation>Peer-verifizierungsmodus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="809"/>
        <source>Peer Verify Depth</source>
        <translation>Peer-verifizierungstiefe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="910"/>
        <source>MQTT Subscription Error</source>
        <translation>MQTT-abonnementfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="911"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Abonnieren des Themas „%1" fehlgeschlagen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Ungültige MQTT-protokollversion</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>Der Broker hat die konfigurierte MQTT-Protokollversion abgelehnt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Client ID Rejected</source>
        <translation>Client-ID Abgelehnt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>Der Broker hat die Client-ID abgelehnt. Versuchen Sie eine andere Kennung.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Server Unavailable</source>
        <translation>MQTT-server Nicht Verfügbar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>Der Broker ist derzeit nicht verfügbar. Später erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>Authentication Error</source>
        <translation>Authentifizierungsfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>Die angegebenen Anmeldedaten wurden vom Broker abgelehnt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>Authorization Error</source>
        <translation>Autorisierungsfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>Account lacks permission for this operation.</source>
        <translation>Konto verfügt nicht über die Berechtigung für diesen Vorgang.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="958"/>
        <source>Network or Transport Error</source>
        <translation>Netzwerk- oder Transportfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="959"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Netzwerk-/Transportschichtproblem beim Verbinden mit dem Broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="962"/>
        <source>MQTT Protocol Violation</source>
        <translation>MQTT-protokollverletzung</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="963"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>Der Broker meldete eine Protokollverletzung und schloss die Verbindung.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="966"/>
        <source>MQTT 5 Error</source>
        <translation>MQTT-5-fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="967"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>Ein MQTT-5-Protokollfehler ist aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="970"/>
        <source>MQTT Error</source>
        <translation>MQTT-fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="971"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>Ein unerwarteter MQTT-Fehler ist aufgetreten.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>Ungültige Serielle Schnittstelle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Modbus-initialisierung Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Modbus-verbindung Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>Die ausgewählte serielle Schnittstelle „%1" ist nicht mehr verfügbar. Aktualisieren Sie die Schnittstellenliste und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Modbus-Gerät kann nicht erstellt werden. Überprüfen Sie die Systemkonfiguration und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Verbindung zu „%1" nicht möglich. Überprüfen Sie die Verbindungseinstellungen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="566"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="567"/>
        <source>Even</source>
        <translation>Gerade</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="568"/>
        <source>Odd</source>
        <translation>Ungerade</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="569"/>
        <source>Space</source>
        <translation>Space</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="570"/>
        <source>Mark</source>
        <translation>Mark</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="622"/>
        <source>Holding Registers (0x03)</source>
        <translation>Holding-Register (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="623"/>
        <source>Input Registers (0x04)</source>
        <translation>Input-Register (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="624"/>
        <source>Coils (0x01)</source>
        <translation>Coils (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="625"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Discrete Inputs (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="811"/>
        <source>No register groups configured</source>
        <translation>Keine Registergruppen konfiguriert</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="812"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Fügen Sie mindestens eine Registergruppe hinzu, bevor Sie ein Projekt generieren.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="814"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="827"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="852"/>
        <source>Modbus Project Generator</source>
        <translation>Modbus-projektgenerator</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Temporäre Projektdatei konnte nicht erstellt werden</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Prüfen Sie die Schreibberechtigungen für das temporäre Verzeichnis.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="824"/>
        <source>Failed to load generated project</source>
        <translation>Generiertes Projekt konnte nicht geladen werden</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="825"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>Das generierte Projekt-JSON konnte nicht geladen werden.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="847"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Projekt mit %1 Gruppen und %2 Datensätzen erfolgreich generiert.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>The project editor is now open for customization.</source>
        <translation>Der Projekteditor ist jetzt zur Anpassung geöffnet.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="866"/>
        <source>Modbus Project</source>
        <translation>Modbus-projekt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="872"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="894"/>
        <source>Holding Registers</source>
        <translation>Halteregister</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="895"/>
        <source>Input Registers</source>
        <translation>Input Register</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="896"/>
        <source>Coils</source>
        <translation>Coils</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="897"/>
        <source>Discrete Inputs</source>
        <translation>Discrete Inputs</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="912"/>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="927"/>
        <source>Register %1</source>
        <translation>Register %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="935"/>
        <source>Coil %1</source>
        <translation>Coil %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="935"/>
        <source>Discrete %1</source>
        <translation>Discrete %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1354"/>
        <source>Error code: %1</source>
        <translation>Fehlercode: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1357"/>
        <source>Modbus Communication Error</source>
        <translation>Modbus-kommunikationsfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1370"/>
        <source>Select Port</source>
        <translation>Port Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1527"/>
        <source>Protocol</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1535"/>
        <source>Slave Address</source>
        <translation>Slave-adresse</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1544"/>
        <source>Poll Interval (ms)</source>
        <translation>Abfrageintervall (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1585"/>
        <source>Host / IP</source>
        <translation>Host / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1592"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1607"/>
        <source>Serial Port</source>
        <translation>Serielle Schnittstelle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1615"/>
        <source>Baud Rate</source>
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1623"/>
        <source>Parity</source>
        <translation>Parität</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1631"/>
        <source>Data Bits</source>
        <translation>Datenbits</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1639"/>
        <source>Stop Bits</source>
        <translation>Stoppbits</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="495"/>
        <source>Network socket error</source>
        <translation>Netzwerk-Socket-Fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="512"/>
        <source>Socket Type</source>
        <translation>Socket-typ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="520"/>
        <source>Remote Address</source>
        <translation>Remote-adresse</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="528"/>
        <source>TCP Port</source>
        <translation>TCP-port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="537"/>
        <source>UDP Local Port</source>
        <translation>UDP-lokalport</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="546"/>
        <source>UDP Remote Port</source>
        <translation>UDP-remote-port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="555"/>
        <source>UDP Multicast</source>
        <translation>UDP-multicast</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="190"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="234"/>
        <source>Failed to start process</source>
        <translation>Prozess konnte nicht gestartet werden</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="191"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Ausführbare Datei „%1" nicht im PATH gefunden.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="379"/>
        <source>Select Executable</source>
        <translation>Ausführbare Datei Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="404"/>
        <source>Select Working Directory</source>
        <translation>Arbeitsverzeichnis Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="430"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Named Pipe / FIFO Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="529"/>
        <source>The process crashed.</source>
        <translation>Der Prozess ist abgestürzt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="530"/>
        <source>Exit code: %1</source>
        <translation>Exit-Code: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="533"/>
        <source>Process "%1" stopped</source>
        <translation>Prozess „%1" gestoppt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="552"/>
        <source>Unknown error</source>
        <translation>Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="553"/>
        <source>Process Error</source>
        <translation>Prozessfehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="567"/>
        <source>Pipe Error</source>
        <translation>Pipe-fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="567"/>
        <source>Could not open named pipe: %1</source>
        <translation>Named Pipe konnte nicht geöffnet werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="781"/>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="784"/>
        <source>Launch Process</source>
        <translation>Prozess Starten</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="784"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="789"/>
        <source>Executable</source>
        <translation>Ausführbare Datei</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="796"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="803"/>
        <source>Working Directory</source>
        <translation>Arbeitsverzeichnis</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="810"/>
        <source>Pipe Path</source>
        <translation>Pipe-pfad</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::UART</name>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="72"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="73"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="395"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="434"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="352"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="752"/>
        <source>Select Port</source>
        <translation>Port Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="396"/>
        <source>Even</source>
        <translation>Gerade</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="397"/>
        <source>Odd</source>
        <translation>Ungerade</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="398"/>
        <source>Space</source>
        <translation>Space</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="399"/>
        <source>Mark</source>
        <translation>Mark</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="435"/>
        <source>RTS/CTS</source>
        <translation>RTS/CTS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="436"/>
        <source>XON/XOFF</source>
        <translation>XON/XOFF</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="572"/>
        <source>"%1" is not a valid path</source>
        <translation>„%1" ist kein gültiger Pfad</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="573"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Bitte anderen Pfad eingeben, um ein benutzerdefiniertes serielles Gerät zu registrieren</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="852"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Das angegebene Gerät wurde nicht gefunden. Verbindung prüfen und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="859"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>Ein unbekannter Fehler ist aufgetreten. Gerät prüfen und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="861"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Das Gerät ist nicht geöffnet. Gerät öffnen, bevor dieser Vorgang ausgeführt wird.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="263"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Verbindung zur seriellen Schnittstelle „%1" fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Kritischer Fehler an serieller Schnittstelle „%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>Unknown error</source>
        <translation>Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="851"/>
        <source>No error occurred.</source>
        <translation>Kein Fehler aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="853"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Zugriff verweigert. Anwendung benötigt die erforderlichen Zugriffsrechte für das Gerät.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="854"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Gerät konnte nicht geöffnet werden. Möglicherweise bereits in Verwendung oder nicht verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="855"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>Fehler beim Schreiben von Daten auf das Gerät.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="856"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>Fehler beim Lesen von Daten vom Gerät.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="857"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>Kritischer Ressourcenfehler aufgetreten. Gerät wurde möglicherweise getrennt oder ist nicht mehr erreichbar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="858"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>Angeforderte Operation wird von diesem Gerät nicht unterstützt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="860"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>Die Operation hat das Zeitlimit überschritten. Das Gerät antwortet möglicherweise nicht.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1029"/>
        <source>Serial Port</source>
        <translation>Serielle Schnittstelle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1037"/>
        <source>Baud Rate</source>
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1045"/>
        <source>Parity</source>
        <translation>Parität</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1053"/>
        <source>Data Bits</source>
        <translation>Datenbits</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1061"/>
        <source>Stop Bits</source>
        <translation>Stoppbits</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1069"/>
        <source>Flow Control</source>
        <translation>Flusskontrolle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1077"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1084"/>
        <source>Auto-Reconnect</source>
        <translation>Automatische Wiederverbindung</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="185"/>
        <source>USB Error</source>
        <translation>USB-fehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Fehler beim Initialisieren des USB-Subsystems. Stellen Sie sicher, dass libusb auf Ihrem System verfügbar ist.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="223"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="240"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="632"/>
        <source>USB Device Error</source>
        <translation>USB-gerätefehler</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>USB-Gerät konnte nicht geöffnet werden: %1.

Unter Linux stellen Sie sicher, dass Sie Lese-/Schreibberechtigung für den Geräteknoten haben (udev-Regel hinzufügen oder als root ausführen). Unter macOS muss möglicherweise zuerst der Kernel-Treiber getrennt werden.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="186"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Kein USB-Gerät ausgewählt. Wählen Sie ein Gerät aus und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="193"/>
        <source>Unknown Device</source>
        <translation>Unbekanntes Gerät</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="198"/>
        <source>Failed to open "%1"</source>
        <translation>Fehler beim Öffnen von „%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="241"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Interface %1 am USB-Gerät konnte nicht beansprucht werden.

Ein anderer Treiber oder eine andere Anwendung hat es möglicherweise bereits geöffnet. Unter Linux versuchen Sie, den Kernel-Treiber zu entladen (z. B. cdc_acm) oder eine udev-Regel hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="404"/>
        <source>Select Device</source>
        <translation>Gerät Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="423"/>
        <source>Select IN Endpoint</source>
        <translation>In-endpunkt Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="434"/>
        <source>None (Read-only)</source>
        <translation>Keine (Nur Lesen)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="509"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Erweiterte USB-Kontrolltransfers Aktivieren?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="510"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Dies aktiviert Kontrolltransfers zusätzlich zu Bulk-Transfers. Das Senden falscher Kontrollanforderungen kann angeschlossene Hardware zum Absturz bringen oder beschädigen. Nur aktivieren, wenn Sie wissen, was Sie tun.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="514"/>
        <source>Advanced USB Mode</source>
        <translation>Erweiterter USB-modus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="633"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>Das USB-Gerät wurde getrennt oder es ist ein schwerwiegender Lesefehler aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="772"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Kein isochroner IN-Endpunkt wurde auf diesem Gerät gefunden, aber Bulk-Endpunkte sind verfügbar.

Übertragungsmodus auf „Bulk-Stream" umschalten und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="777"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Kein Bulk-IN-Endpunkt wurde auf diesem Gerät gefunden, aber isochrone Endpunkte sind verfügbar.

Übertragungsmodus auf „Isochron" umschalten und erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="781"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Kein verwendbarer IN-Endpunkt wurde auf diesem Gerät gefunden.

Das Gerät stellt möglicherweise keine Datenendpunkte in seiner aktiven Konfiguration bereit oder benötigt einen spezifischen Treiber.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1212"/>
        <source>USB Device</source>
        <translation>USB-gerät</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1220"/>
        <source>Transfer Mode</source>
        <translation>Übertragungsmodus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Bulk Stream</source>
        <translation>Bulk-stream</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Advanced Control</source>
        <translation>Erweiterte Kontrolle</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Isochronous</source>
        <translation>Isochron</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1228"/>
        <source>IN Endpoint</source>
        <translation>In-endpunkt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1236"/>
        <source>OUT Endpoint</source>
        <translation>Out-endpunkt</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1244"/>
        <source>ISO Packet Size</source>
        <translation>ISO-paketgröße</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="214"/>
        <source>No file selected…</source>
        <translation>Keine Datei ausgewählt…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="249"/>
        <source>Plain Text</source>
        <translation>Klartext</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="250"/>
        <source>Raw Binary</source>
        <translation>Rohe Binärdaten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="251"/>
        <source>XMODEM</source>
        <translation>XMODEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="252"/>
        <source>XMODEM-1K</source>
        <translation>XMODEM-1K</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="253"/>
        <source>YMODEM</source>
        <translation>YMODEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="254"/>
        <source>ZMODEM</source>
        <translation>ZMODEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="268"/>
        <source>Select file to transmit</source>
        <translation>Datei zum Übertragen auswählen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="295"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>Datei ausgewählt: %1 (%2 Bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="298"/>
        <source>Error opening file: %1</source>
        <translation>Fehler beim Öffnen der Datei: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="389"/>
        <source>Starting %1 transfer…</source>
        <translation>Starte %1-Übertragung…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="624"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="644"/>
        <source>Transmission complete</source>
        <translation>Übertragung abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="626"/>
        <source>Plain text transmission complete</source>
        <translation>Klartextübertragung abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="646"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Binärübertragung abgeschlossen (%1 Bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="670"/>
        <source>Transfer complete</source>
        <translation>Übertragung abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="671"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Übertragung erfolgreich abgeschlossen (%1 Bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="673"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="674"/>
        <source>Transfer failed: %1</source>
        <translation>Übertragung fehlgeschlagen: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="758"/>
        <source>%1 B/s</source>
        <translation>%1 B/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="760"/>
        <source>%1 KB/s</source>
        <translation>%1 Kb/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="762"/>
        <source>%1 MB/s</source>
        <translation>%1 Mb/s</translation>
    </message>
</context>
<context>
    <name>IO::FrameReader</name>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="314"/>
        <source>Frames dropped</source>
        <translation>Frames verworfen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="316"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>Eingehende Daten kommen schneller an, als Serial Studio sie verarbeiten kann; %1 Frame(s) wurden verworfen. Datenrate reduzieren oder einen ressourcenintensiven Verbraucher deaktivieren.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="85"/>
        <source>Cannot open file: %1</source>
        <translation>Datei kann nicht geöffnet werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="114"/>
        <source>Transfer cancelled</source>
        <translation>Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="115"/>
        <source>Transfer cancelled by user</source>
        <translation>Übertragung vom Benutzer abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="95"/>
        <source>Waiting for receiver…</source>
        <translation>Warte auf Empfänger…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="211"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Empfänger bereit (CRC-Modus), sende Daten…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="144"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Zu viele Wiederholungsversuche, Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="145"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximale Anzahl an Wiederholungen überschritten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="149"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>NAK empfangen, Block %1 wird wiederholt (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="158"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="396"/>
        <source>Failed to seek in file</source>
        <translation>Positionierung in Datei fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="168"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Übertragung vom Empfänger abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="169"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Empfänger hat die Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="183"/>
        <source>Transfer complete</source>
        <translation>Übertragung abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="310"/>
        <source>File read returned more data than requested</source>
        <translation>Datei-Lesevorgang hat mehr Daten als angefordert zurückgegeben</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="326"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Block %1 wird gesendet (%2 Bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="339"/>
        <source>Sending EOT…</source>
        <translation>EOT Wird Gesendet…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="386"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Zeitüberschreitung, Wiederholung (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="381"/>
        <source>Transfer timed out</source>
        <translation>Zeitüberschreitung der Übertragung</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="382"/>
        <source>Timeout: no response from receiver</source>
        <translation>Zeitüberschreitung: Keine Antwort vom Empfänger</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="64"/>
        <source>Cannot open file: %1</source>
        <translation>Datei kann nicht geöffnet werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="98"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="176"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Übertragung vom Empfänger abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="99"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="177"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Empfänger hat die Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="76"/>
        <source>Waiting for receiver…</source>
        <translation>Warte auf Empfänger…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="135"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="331"/>
        <source>Sending first EOT…</source>
        <translation>Sende erstes EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="153"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Zu viele Wiederholungen, Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="154"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximale Wiederholungen überschritten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="158"/>
        <source>NAK received, retrying block %1</source>
        <translation>NAK empfangen, wiederhole Block %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="164"/>
        <source>Failed to seek in file</source>
        <translation>Fehler beim Positionieren in Datei</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="228"/>
        <source>Sending second EOT…</source>
        <translation>Sende zweites EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="312"/>
        <source>Sending end-of-batch marker…</source>
        <translation>Sende End-of-Batch-Markierung…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="252"/>
        <source>Transfer complete</source>
        <translation>Übertragung abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="297"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Sende Datei-Header: %1 (%2 Bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="346"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Sende Block %1 (%2/%3 Bytes)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="88"/>
        <source>Cannot open file: %1</source>
        <translation>Datei kann nicht geöffnet werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="106"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Datei ist zu groß für ZMODEM (%1 Bytes, Limit 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="133"/>
        <source>Transfer cancelled</source>
        <translation>Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="134"/>
        <source>Transfer cancelled by user</source>
        <translation>Übertragung vom Benutzer abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="279"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>Hex-Header-CRC-Fehler, Frame wird verworfen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="466"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Sende Dateiinformationen: %1 (%2 Bytes)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="482"/>
        <source>Failed to seek to offset %1</source>
        <translation>Fehler beim Springen zu Offset %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="513"/>
        <source>File read returned more data than requested</source>
        <translation>Datei-Lesevorgang lieferte mehr Daten als angefordert</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="601"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Empfänger fordert Daten ab Offset %1 an</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="610"/>
        <source>Receiver skipped the file</source>
        <translation>Empfänger hat die Datei übersprungen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="623"/>
        <source>Too many errors, transfer aborted</source>
        <translation>Zu viele Fehler, Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="624"/>
        <source>Maximum retries exceeded</source>
        <translation>Maximale Anzahl an Wiederholungen überschritten</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="439"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>Sende ZRQINIT, warte auf Empfänger…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="541"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Dateidaten gesendet, warte auf Bestätigung…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="552"/>
        <source>Sending ZFIN…</source>
        <translation>ZFIN Wird Gesendet…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="589"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Empfänger bereit, Dateiinformationen werden gesendet…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>NAK empfangen, erneuter Versuch (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="650"/>
        <source>Transfer complete</source>
        <translation>Übertragung abgeschlossen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="661"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Übertragung vom Empfänger abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="662"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Empfänger hat die Übertragung abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="671"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="672"/>
        <source>Receiver reported a file error</source>
        <translation>Empfänger hat einen Dateifehler gemeldet</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="879"/>
        <source>Transfer timed out</source>
        <translation>Zeitüberschreitung der Übertragung</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="880"/>
        <source>Timeout: no response from receiver</source>
        <translation>Zeitüberschreitung: keine Antwort vom Empfänger</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="884"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Zeitüberschreitung, erneuter Versuch (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>Symbol Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>Online Suchen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
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
        <translation>Graustufen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Hoher Kontrast</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Lebhaft</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Nachtsicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Infrarot</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Tiefblau</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Bernstein</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Bilder Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Exportordner Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Vergrößern</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Verkleinern</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Fadenkreuz Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>Warte auf Bild…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="24"/>
        <source>API Keys</source>
        <translation>API-schlüssel</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="48"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. Standard ist Claude Haiku 4.5 ($1 Eingabe / $5 Ausgabe pro Million Tokens). Sonnet 4.6 und OPUS 4.7 sind ebenfalls verfügbar. Unterstützt Streaming, Tool-Nutzung, erweitertes Denken und Prompt-Caching.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. Standard ist GPT-4o mini ($0,15 Eingabe / $0,60 Ausgabe pro Million Tokens). GPT-4o, GPT-4 Turbo und o1-mini sind ebenfalls verfügbar.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="58"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. Standard ist Gemini 2.0 Flash mit großzügigem kostenlosen Kontingent (mit Ratenlimits). Gemini 1.5 Pro und Gemini 1.5 Flash sind ebenfalls verfügbar.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="101"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Eigene API-Schlüssel verwenden. Sie werden im Ruhezustand mit einem gerätespezifischen Schlüssel verschlüsselt und verlassen den Computer nur zur Kommunikation mit dem ausgewählten Anbieter.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>Key set</source>
        <translation>Schlüssel Gesetzt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="168"/>
        <source>No key</source>
        <translation>Kein Schlüssel</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">Ein Schlüssel ist hinterlegt – neuen Schlüssel einfügen, um ihn zu ersetzen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="53"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. Standard ist GPT-5 mini für schnelle, kosteneffiziente agentische Arbeit. GPT-5.2 ist die leistungsstärkere Allzweck-Option, und GPT-5.2 Chat verwendet das Modell, das aktuell in ChatGPT genutzt wird.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="62"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. OpenAI-kompatibles API. Standard ist deepseek-chat (V3). deepseek-reasoner (R1) ist ebenfalls verfügbar. Oft die günstigste Cloud-Option für Tool-Nutzung.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="66"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. Ein Schlüssel, ~200 Modelle von Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen und anderen. Kostenlose Modelle (Suffix :free) sind ratenbegrenzt, erfordern aber keine zusätzliche Abrechnung. Pay-as-you-go für die übrigen.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="71"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Hardware-beschleunigte Inferenz (LPUs) für sehr schnelle Llama-, Mixtral-, Gemma-, DeepSeek-R1-Distill- und Qwen-Modelle. Großzügiges kostenloses Kontingent mit täglichen Token-Limits.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="75"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. Standard ist Mistral Large. Codestral ist für Code-Vervollständigung ausgelegt, Pixtral verarbeitet Bilderkennung, und die Ministral-Modelle sind für Edge- / Low-Latency-Anwendungen optimiert.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="79"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Lokaler Modellserver. Funktioniert mit jedem OpenAI-kompatiblen Endpunkt – Ollama, llama.cpp's llama-server, LM Studio oder vLLM. Nichts verlässt den Rechner. Die Modellliste wird live vom Server abgefragt.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>Ein Schlüssel ist hinterlegt – neuen Schlüssel einfügen, um ihn zu ersetzen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="206"/>
        <source>Paste your API key here</source>
        <translation>API-Schlüssel hier einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Hide key</source>
        <translation>Schlüssel verbergen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="214"/>
        <source>Show key while typing</source>
        <translation>Schlüssel während der Eingabe anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Get key</source>
        <translation>Schlüssel abrufen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="226"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Öffnen Sie die Konsole des Anbieters, um einen neuen Schlüssel zu erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="237"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="255"/>
        <source>Remove the stored key for %1</source>
        <translation>Den gespeicherten Schlüssel für %1 entfernen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="279"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Install Ollama</source>
        <translation>Ollama Installieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="284"/>
        <source>Open the Ollama download page</source>
        <translation>Ollama-Downloadseite öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="295"/>
        <source>Apply</source>
        <translation>Anwenden</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="310"/>
        <source>Re-query the model list</source>
        <translation>Modellliste erneut abfragen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="358"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Noch keine API-Schlüssel konfiguriert. Fügen Sie einen Schlüssel hinzu, um zu beginnen.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">Noch keine API-Schlüssel konfiguriert. Fügen Sie oben mindestens einen hinzu, um zu beginnen.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="361"/>
        <source>One provider is ready.</source>
        <translation>Ein Anbieter ist bereit.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="363"/>
        <source>%1 providers are ready.</source>
        <translation>%1 Anbieter sind bereit.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Lizenzierung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Bitte warten…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Serial Studio Pro Aktivieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Lizenzschlüssel unten einfügen, um Pro-Funktionen wie MQTT, 3D-Diagramme und mehr freizuschalten.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>Die Lizenz umfasst 5 Geräteaktivierungen.
Pläne umfassen monatliche, jährliche und lebenslange Optionen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Lizenzschlüssel hier einfügen…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>Alles Auswählen</translation>
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
        <translation>Lizenznehmer</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>Lizenznehmer-e-mail</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>Gerätenutzung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 Geräte in Verwendung (Unbegrenzter Plan)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 von %2 Geräten verwendet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>Geräte-ID</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>Lizenzschlüssel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>Kundenportal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>Lizenz Kaufen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>Aktivieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>Deaktivieren</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="544"/>
        <source>There was an issue validating your license.</source>
        <translation>Bei der Validierung Ihrer Lizenz ist ein Problem aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="562"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="743"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="868"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>Der angegebene Lizenzschlüssel gehört nicht zu Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="563"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Bitte überprüfen Sie, dass Sie Ihre Lizenz im offiziellen Serial Studio Store erworben haben.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="574"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="752"/>
        <source>This license key was activated on a different device.</source>
        <translation>Dieser Lizenzschlüssel wurde auf einem anderen Gerät aktiviert.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="575"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="753"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Deaktivieren Sie ihn dort zuerst oder wenden Sie sich an den Support.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="586"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="763"/>
        <source>This license is not currently active.</source>
        <translation>Diese Lizenz ist derzeit nicht aktiv.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="587"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="764"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Sie ist möglicherweise abgelaufen oder wurde deaktiviert (Status: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="597"/>
        <source>Something went wrong on the server.</source>
        <translation>Auf dem Server ist ein Fehler aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="598"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="774"/>
        <source>No activation ID was returned.</source>
        <translation>Es wurde keine Aktivierungs-ID zurückgegeben.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="608"/>
        <source>Could not validate your license at this time.</source>
        <translation>Die Lizenz konnte zu diesem Zeitpunkt nicht validiert werden.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="609"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="783"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="879"/>
        <source>Try again later.</source>
        <translation>Später erneut versuchen.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="744"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="869"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Überprüfen Sie, dass Sie Ihre Lizenz im offiziellen Serial Studio Store erworben haben.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="773"/>
        <source>Something went wrong on the server…</source>
        <translation>Auf dem Server ist ein Fehler aufgetreten…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="627"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="629"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="680"/>
        <source>Your license has been successfully activated.</source>
        <translation>Ihre Lizenz wurde erfolgreich aktiviert.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="681"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Vielen Dank für die Unterstützung von Serial Studio!
Sie haben jetzt Zugriff auf alle Premium-Funktionen.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="735"/>
        <source>There was an issue activating your license.</source>
        <translation>Bei der Aktivierung Ihrer Lizenz ist ein Problem aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="782"/>
        <source>Could not activate your license at this time.</source>
        <translation>Ihre Lizenz konnte derzeit nicht aktiviert werden.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="859"/>
        <source>There was an issue deactivating your license.</source>
        <translation>Bei der Deaktivierung Ihrer Lizenz ist ein Problem aufgetreten.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="878"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>Ihre Lizenz konnte zu diesem Zeitpunkt nicht deaktiviert werden.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="889"/>
        <source>Your license has been deactivated.</source>
        <translation>Ihre Lizenz wurde deaktiviert.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="890"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Der Zugriff auf Pro-Funktionen wurde entfernt.
Vielen Dank für Ihre Unterstützung von Serial Studio!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="638"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>MDF4-Export ist eine Pro-Funktion.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="639"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Diese Funktion erfordert eine Lizenz. Bitte erwerben Sie eine, um den MDF4-Export zu aktivieren.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="363"/>
        <source>Select MDF4 file</source>
        <translation>MDF4-Datei Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="365"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>MDF4-Dateien (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="389"/>
        <source>Disconnect from device?</source>
        <translation>Verbindung zum Gerät Trennen?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="390"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>Sie müssen die Verbindung zum aktuellen Gerät trennen, bevor Sie eine MDF4-Datei öffnen können.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="407"/>
        <source>Cannot open MDF4 file</source>
        <translation>MDF4-Datei Kann Nicht Geöffnet Werden</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="408"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Die Datei ist möglicherweise beschädigt oder in einem nicht unterstützten Format.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="415"/>
        <source>Invalid MDF4 file</source>
        <translation>Ungültige MDF4-Datei</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="416"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Dateistruktur konnte nicht gelesen werden. Die Datei ist möglicherweise beschädigt.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="433"/>
        <source>No data in file</source>
        <translation>Keine Daten in Datei</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="434"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>Die MDF4-Datei enthält keine Messdaten.</translation>
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
        <translation>z. B. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Themenfilter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>z. B. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>Client-ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Neu Generieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Benutzername</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Passwort</translation>
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
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>SSL/TLS Verwenden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>SSL-protokoll</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Peer Verifizieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Verifizierungstiefe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>Ca-zertifikate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Aus Ordner Laden…</translation>
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
        <translation type="vanished">TLS 1.3 oder Neuer</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 oder Neuer</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">Beliebiges Protokoll</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">Nur Sichere Protokolle</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Keine</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">Peer Abfragen</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">Peer Verifizieren</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">Peer Automatisch Verifizieren</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">Systemdatenbank Verwenden</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">MQTT-abonnent</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">MQTT-publisher</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">MQTT-Funktion erfordert eine kommerzielle Lizenz</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">Aus Ordner Laden…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">Die Verbindung zu MQTT-Brokern ist nur mit einer gültigen kommerziellen Serial Studio-Lizenz (Hobbyist-Stufe oder höher) verfügbar.

Um diese Funktion freizuschalten, aktivieren Sie Ihre Lizenz oder besuchen Sie den Store.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">MQTT-thema Fehlt</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">Vor der Verbindung als Publisher muss ein Thema angegeben werden.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">Konfigurationsfehler</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">MQTT-thema Nicht Festgelegt</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">Es werden keine Nachrichten empfangen, bis ein Thema konfiguriert ist.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">Konfigurationswarnung</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">Ungültiges MQTT-thema</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Das Thema „%1" ist nicht gültig.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">Pem-zertifikatsverzeichnis Auswählen</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">Abonnementfehler</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Abonnieren des Themas „%1" fehlgeschlagen.</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">Ungültige MQTT-protokollversion</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">Der MQTT-Broker hat die Verbindung aufgrund einer nicht unterstützten Protokollversion abgelehnt. Stellen Sie sicher, dass Client und Broker dieselbe Protokollversion unterstützen.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">Client-ID Abgelehnt</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">Der Broker hat die Client-ID abgelehnt. Sie ist möglicherweise fehlerhaft, zu lang oder bereits in Verwendung. Versuchen Sie eine andere Client-ID.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">MQTT-server Nicht Verfügbar</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">Die Netzwerkverbindung wurde hergestellt, aber der Broker ist derzeit nicht verfügbar. Überprüfen Sie den Broker-Status und versuchen Sie es später erneut.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">Authentifizierungsfehler</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">Der angegebene Benutzername oder das Passwort ist falsch oder fehlerhaft. Überprüfen Sie Ihre Anmeldedaten und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">Autorisierungsfehler</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">Der MQTT-Broker hat die Verbindung aufgrund unzureichender Berechtigungen abgelehnt. Stellen Sie sicher, dass Ihr Konto über die erforderlichen Zugriffsrechte verfügt.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">Netzwerk- oder Transportfehler</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">Ein Problem auf Netzwerk- oder Transportebene ist aufgetreten und hat einen unerwarteten Verbindungsabbruch verursacht. Überprüfen Sie Ihre Netzwerkverbindung und Broker-Einstellungen.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">MQTT-protokollverletzung</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">Der Client hat eine Verletzung des MQTT-Protokolls erkannt und die Verbindung geschlossen. Überprüfen Sie Ihre MQTT-Implementierung auf Konformität.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">Unbekannter Fehler</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">Ein unerwarteter Fehler ist aufgetreten. Prüfen Sie die Protokolle für weitere Details oder starten Sie die Anwendung neu.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">MQTT-5-fehler</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">Ein MQTT-Protokollfehler der Stufe 5 ist aufgetreten. Prüfen Sie die Broker-Protokolle oder Reason-Codes für weitere Details.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">MQTT-authentifizierung Fehlgeschlagen</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">Authentifizierung fehlgeschlagen: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">Erweiterte Authentifizierung ist erforderlich, aber MQTT 5.0 ist nicht aktiviert.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">Unbekannt</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">MQTT-authentifizierung Erforderlich</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">Der MQTT-Broker erfordert Authentifizierung mit der Methode: „%1".

Bitte geben Sie die erforderlichen Anmeldedaten ein.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">MQTT-benutzername Eingeben</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">Benutzername:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">MQTT-passwort Eingeben</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">Passwort:</translation>
    </message>
</context>
<context>
    <name>MQTT::Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="815"/>
        <source>MQTT 3.1</source>
        <translation>MQTT 3.1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="816"/>
        <source>MQTT 3.1.1</source>
        <translation>MQTT 3.1.1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="817"/>
        <source>MQTT 5.0</source>
        <translation>MQTT 5.0</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="819"/>
        <source>TLS 1.2</source>
        <translation>TLS 1.2</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="820"/>
        <source>TLS 1.3</source>
        <translation>TLS 1.3</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="821"/>
        <source>TLS 1.3 or Later</source>
        <translation>TLS 1.3 oder Neuer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="822"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 oder Neuer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="823"/>
        <source>Any Protocol</source>
        <translation>Beliebiges Protokoll</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="824"/>
        <source>Secure Protocols Only</source>
        <translation>Nur Sichere Protokolle</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="826"/>
        <source>None</source>
        <translation>Keine</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="827"/>
        <source>Query Peer</source>
        <translation>Peer Abfragen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="828"/>
        <source>Verify Peer</source>
        <translation>Peer Verifizieren</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="829"/>
        <source>Auto Verify Peer</source>
        <translation>Peer Automatisch Verifizieren</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1150"/>
        <source>Raw RX Data</source>
        <translation>Rohe Rx-daten</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1151"/>
        <source>Custom Script</source>
        <translation>Benutzerdefiniertes Skript</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1152"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Dashboard-daten (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1153"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Dashboard-daten (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1313"/>
        <source>MQTT publisher unavailable</source>
        <translation>MQTT-Publisher nicht verfügbar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1314"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>Eine gültige kommerzielle Lizenz ist erforderlich, um MQTT-Publishing zu verwenden.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1316"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1890"/>
        <source>MQTT Test Connection</source>
        <translation>MQTT-testverbindung</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1336"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Pem-zertifikatsverzeichnis Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1887"/>
        <source>MQTT broker reachable</source>
        <translation>MQTT-Broker erreichbar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1887"/>
        <source>MQTT broker unreachable</source>
        <translation>MQTT-Broker nicht erreichbar</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1901"/>
        <source>MQTT broker connection failed</source>
        <translation>MQTT-Broker-Verbindung fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1901"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-publisher</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="49"/>
        <source>MQTT Publisher Script</source>
        <translation>MQTT-publisher-skript</translation>
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
        <translation>Beispiel-Frame-Bytes (Text oder Hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="97"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="101"/>
        <source>Apply</source>
        <translation>Anwenden</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="111"/>
        <source>Language:</source>
        <translation>Sprache:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="114"/>
        <source>Template:</source>
        <translation>Vorlage:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="125"/>
        <source>Frame:</source>
        <translation>Frame:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="129"/>
        <source>Output:</source>
        <translation>Ausgabe:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="269"/>
        <source>Enter a frame</source>
        <translation>Frame eingeben</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="276"/>
        <source>Invalid hex</source>
        <translation>Ungültiges Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="359"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Dokument Formatieren	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Auswahl Formatieren	ctrl+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="488"/>
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
-- Definieren Sie eine mqtt(frame)-Funktion, die die rohen Bytes
-- eines geparsten Frames empfängt und die Nutzlast zurückgibt,
-- die an den Broker veröffentlicht werden soll, oder nil, um
-- diesen Frame zu überspringen.
--
-- Das frame-Argument entspricht dem, was das Frame-Parser-Skript
-- sieht: eine Lua-Zeichenkette mit den Bytes zwischen den
-- FrameReader-Trennzeichen.
--
-- Beispiel:
--   function mqtt(frame)
--     return frame    -- unverändert weiterleiten
--   end
--
-- Verwenden Sie das Vorlagen-Dropdown für fertige Beispiele.
--</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="505"/>
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
 * Definieren Sie eine mqtt(frame)-Funktion, die die rohen Bytes
 * eines geparsten Frames empfängt und die Nutzlast zurückgibt,
 * die an den Broker veröffentlicht werden soll, oder null, um
 * diesen Frame zu überspringen.
 *
 * Das frame-Argument entspricht dem, was das Frame-Parser-Skript
 * sieht: eine Zeichenkette mit den Bytes zwischen den
 * FrameReader-Trennzeichen.
 *
 * Beispiel:
 *   function mqtt(frame) {
 *     return frame;   // unverändert weiterleiten
 *   }
 *
 * Verwenden Sie das Vorlagen-Dropdown für fertige Beispiele.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="602"/>
        <source>Script is empty</source>
        <translation>Skript ist leer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="609"/>
        <source>Lua engine error</source>
        <translation>Lua-Engine-Fehler</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="631"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="645"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="683"/>
        <source>Error: %1</source>
        <translation>Fehler: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="639"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="675"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() ist nicht definiert</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="656"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- Frame übersprungen)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <source>(non-string return)</source>
        <translation>(non-string return)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- Frame übersprungen)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="766"/>
        <source>Select Template…</source>
        <translation>Vorlage Auswählen…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="696"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Broker-Hostname und Port konfigurieren, bevor die Verbindung getestet wird.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="733"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Erfolgreich mit %1:%2 verbunden.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="744"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Zeitüberschreitung nach 5 Sekunden ohne Broker-Erreichbarkeit.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">MQTT-einrichtung</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT ist eine Pro-Funktion</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">Aktivieren Sie Ihre Lizenz oder besuchen Sie den Store, um MQTT-Unterstützung freizuschalten.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">Allgemein</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">Authentifizierung</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">MQTT-optionen</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">SSL-eigenschaften</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">Host</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">Port</translation>
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
        <translation type="vanished">Benutzername</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">MQTT-benutzername</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">Passwort</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">MQTT-passwort</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">Version</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">Modus</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">Thema</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">z. B. sensors/temperature oder home/+/status</translation>
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
        <translation type="vanished">Will-thema</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">z. B. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will-nachricht</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">z. B. Gerät unerwartet getrennt</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">SSL Aktivieren</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">SSL-protokoll</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">Verify Depth</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">Verify Mode</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Schließen</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">Trennen</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">Verbinden</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="184"/>
        <source>Console Only Mode</source>
        <translation>Nur-konsole-modus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="187"/>
        <source>Quick Plot Mode</source>
        <translation>Schnelldiagramm-modus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="194"/>
        <source>Empty Project</source>
        <translation>Leeres Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="683"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="691"/>
        <source>Waiting for data…</source>
        <translation>Warte auf Daten…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="692"/>
        <source>Connecting to device…</source>
        <translation>Verbinde mit Gerät…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Codebeispiel</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Vervollständigung</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Syntaxhervorhebung</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>Leerzeichen</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>In Zwischenablage kopiert</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>MDF4-player</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Sie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Assistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Erkennung</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Ausführung</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>%1 Aktionen in %2 genehmigen?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Diese Aufrufe werden zusammen ausgeführt. Jede Karte unten erweitern, um Argumente zu prüfen.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Alle genehmigen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Alle ablehnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Alles Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Alles Auswählen</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>Sie</translation>
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
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Erkennung</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Ausführung</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>Läuft</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>Wartet auf Genehmigung</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Abgelehnt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Blockiert</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Genehmigen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Ablehnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Alle genehmigen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Alle ablehnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Ergebnis</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>{n} Aktionen in {family} genehmigen?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Diese Aufrufe werden zusammen ausgeführt. Jede Karte erweitern, um Argumente zu prüfen.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="294"/>
        <source>Failed to load README: %1</source>
        <translation>README konnte nicht geladen werden: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Theme</source>
        <translation>Design</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Frame Parser</source>
        <translation>Frame-parser</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Project Template</source>
        <translation>Projektvorlage</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="259"/>
        <source>All Types</source>
        <translation>Alle Typen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="487"/>
        <source>Reset Extensions</source>
        <translation>Erweiterungen Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="488"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Alle Erweiterungen werden deinstalliert, alle benutzerdefinierten Repositorys entfernt und die Standardeinstellungen wiederhergestellt. Fortfahren?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="531"/>
        <source>Select Extension Repository Folder</source>
        <translation>Erweiterungs-repository-ordner Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1064"/>
        <source>Installed (repository no longer available)</source>
        <translation>Installiert (Repository nicht mehr verfügbar)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1388"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1409"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1431"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1475"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1494"/>
        <source>Plugin Error</source>
        <translation>Plugin-fehler</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Plugin „%1" ist nicht installiert.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1389"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>Erweiterung „%1" ist kein Plugin (Typ: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1410"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Plugin-Metadatendatei kann nicht gelesen werden:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Plugin „%1" benötigt GRPC, aber dieser Build enthält keine GRPC-Unterstützung.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Dieses Plugin verwendet GRPC für Hochleistungs-Datenstreaming. Der API-Server muss aktiviert werden.

Jetzt aktivieren?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1447"/>
        <source>API Server Required</source>
        <translation>API-server Erforderlich</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1476"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Plugin "%1" hat kein 'entry'-Feld in info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1486"/>
        <source>Entry point not found:
%1</source>
        <translation>Einstiegspunkt nicht gefunden:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1495"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Plugin "%1" hat einen ungültigen Einstiegspunkt-Pfad.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1538"/>
        <source>Missing Dependency</source>
        <translation>Fehlende Abhängigkeit</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1539"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Dieses Plugin benötigt "%1", wurde aber auf dem System nicht gefunden.

Download-Seite öffnen?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1444"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>Plugins benötigen den API-Server zur Kommunikation mit Serial Studio. Jetzt aktivieren?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Neustart Erforderlich</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>Das neue Rendering-Backend wird nach dem Neustart von Serial Studio wirksam. Jetzt neu starten, um die Änderung anzuwenden?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="320"/>
        <source>Failed to load page: %1</source>
        <translation>Fehler beim Laden der Seite: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="154"/>
        <source>Invalid icon identifier</source>
        <translation>Ungültige Symbol-Kennung</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="228"/>
        <source>Empty SVG data received</source>
        <translation>Leere SVG-Daten empfangen</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Windows-Verknüpfung (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>macOS-Programm (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Desktop-Eintrag (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Verwenden Sie ein .icns-Symbol für das schärfste Ergebnis im Finder und Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Lassen Sie das Symbol leer, um das Symbol der Serial Studio-Anwendung zu übernehmen.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Platzieren Sie die Datei unter ~/.local/share/applications/, um sie im Anwendungsstarter verfügbar zu machen.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Apple Icon Image (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Windows-Symbol (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>Vektor- oder Rasterbild (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="215"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>Eine Pro-Lizenz ist erforderlich, um Verknüpfungen zu erstellen.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="220"/>
        <source>No output path was provided.</source>
        <translation>Kein Ausgabepfad angegeben.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="263"/>
        <source>Failed to write shortcut file.</source>
        <translation>Verknüpfungsdatei konnte nicht geschrieben werden.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Vorhandene Verknüpfung unter %1 konnte nicht ersetzt werden.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Verzeichnisstruktur des .app-Bundles konnte nicht erstellt werden.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Bundle-Launcher konnte nicht geschrieben werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Bundle-Launcher konnte nicht als ausführbar markiert werden.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Info.plist konnte nicht geschrieben werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="141"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="272"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Windows-Verknüpfungsschreiber ist auf dieser Plattform nicht verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="286"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="200"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Linux-Verknüpfungsschreiber ist auf dieser Plattform nicht verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>COM konnte nicht initialisiert werden (erforderlich zum Schreiben von .lnk-Verknüpfungen).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) fehlgeschlagen (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="154"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) fehlgeschlagen (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="164"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Speichern der .lnk-Datei fehlgeschlagen (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="155"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="186"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>MacOS-Verknüpfungsschreiber ist auf dieser Plattform nicht verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Verknüpfungspfad konnte nicht zum Schreiben geöffnet werden: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Serial Studio-Verknüpfung</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Verknüpfung konnte nicht als ausführbar markiert werden.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="426"/>
        <source>System</source>
        <translation>System</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">Automatisch nach Updates suchen?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">Soll %1 automatisch nach Updates suchen? Updates können jederzeit manuell über den „Info"-Dialog gesucht werden</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="170"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="172"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="174"/>
        <source>Save all</source>
        <translation>Alle speichern</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="176"/>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="178"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="180"/>
        <source>Yes to all</source>
        <translation>Ja, alle</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="182"/>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="184"/>
        <source>No to all</source>
        <translation>Nein zu allen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="186"/>
        <source>Abort</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="188"/>
        <source>Retry</source>
        <translation>Wiederholen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="190"/>
        <source>Ignore</source>
        <translation>Ignorieren</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="192"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="194"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="196"/>
        <source>Discard</source>
        <translation>Verwerfen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="198"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="200"/>
        <source>Apply</source>
        <translation>Anwenden</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="202"/>
        <source>Reset</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="204"/>
        <source>Restore defaults</source>
        <translation>Standardwerte Wiederherstellen</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="267"/>
        <source>Select Workspace Location</source>
        <translation>Arbeitsbereich-speicherort Auswählen</translation>
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
        <translation>Serielle Schnittstelle</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="97"/>
        <source>Baud Rate</source>
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="201"/>
        <source>Parity</source>
        <translation>Parität</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="222"/>
        <source>Data Bits</source>
        <translation>Datenbits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="245"/>
        <source>Stop Bits</source>
        <translation>Stoppbits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="268"/>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="278"/>
        <source>IP Address</source>
        <translation>IP-adresse</translation>
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
        <translation>Slave-adresse</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="334"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="382"/>
        <source>Configure Register Groups…</source>
        <translation>Registergruppen Konfigurieren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="392"/>
        <source>Import Register Map…</source>
        <translation>Registermap Importieren…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="407"/>
        <source>%1 group(s) configured</source>
        <translation>%1 Gruppe(n) konfiguriert</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="408"/>
        <source>No groups configured</source>
        <translation>Keine Gruppen konfiguriert</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="349"/>
        <source>Poll Interval (ms)</source>
        <translation>Abfrageintervall (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="354"/>
        <source>Polling interval</source>
        <translation>Abfrageintervall</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Modbus-registergruppen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Konfigurieren Sie mehrere Registergruppen, um verschiedene Registertypen nacheinander abzufragen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Neue Gruppe Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Registertyp:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Startadresse:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Registeranzahl:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Gruppe Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Konfigurierte Gruppen</translation>
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
        <translation>Anzahl</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Aktion</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Entfernen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Keine Gruppen konfiguriert.
Fügen Sie oben Gruppen hinzu, um mehrere Registertypen abzufragen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Gruppen insgesamt: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Projekt Generieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Alle Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Modbus-registerübersicht Vorschau</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>Datei: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Register prüfen, die in ein neues Serial Studio-Projekt importiert werden sollen.</translation>
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
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Adresse</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Datentyp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Einheiten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>Keine Register in Datei gefunden.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Gesamt: %1 Register in %2 Gruppen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Projekt Erstellen</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-publisher</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>Mit Broker verbunden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Nicht verbunden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Verbindung Testen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Broker mit den aktuellen Einstellungen testen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Veröffentlichung zuerst aktivieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Skript Bearbeiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Publisher-Skript bearbeiten (Lua oder JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Ca-zertifikate Laden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>PEM-Zertifikate aus einem Ordner hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>SSL/TLS zuerst aktivieren</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="249"/>
        <source>Interpolation: %1</source>
        <translation>Interpolation: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Legenden Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="271"/>
        <source>Show X Axis Label</source>
        <translation>X-achsenbeschriftung Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="282"/>
        <source>Show Y Axis Label</source>
        <translation>Y-achsenbeschriftung Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="294"/>
        <source>Show Crosshair</source>
        <translation>Fadenkreuz Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="301"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="301"/>
        <source>Resume</source>
        <translation>Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="318"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Sweep- / Trigger-modus</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="330"/>
        <source>Trigger Settings</source>
        <translation>Trigger-einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="354"/>
        <source>Reset View</source>
        <translation>Ansicht Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="360"/>
        <source>Axis Range Settings</source>
        <translation>Achsenbereich-einstellungen</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Samples</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="365"/>
        <source>Bytes per value</source>
        <translation>Bytes pro Wert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="366"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Anzahl der Bytes, die zu jedem Kanalwert kombiniert werden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="374"/>
        <source>Endianness</source>
        <translation>Byte-reihenfolge</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="375"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>Byte-Reihenfolge beim Kombinieren von Mehrbytwerten.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="383"/>
        <source>Signed values</source>
        <translation>Vorzeichenbehaftete Werte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="384"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Interpretiert jeden Wert als vorzeichenbehaftetes Zweierkomplement.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="588"/>
        <source>Tag routing table</source>
        <translation>Tag-Routing-Tabelle</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="589"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Kommagetrennte Tag:Index-Einträge, z. B. 1:0,2:1,3:2. Tags können dezimal oder mit 0x-Präfix hexadezimal sein.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1033"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1238"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1166"/>
        <source>Validate checksum</source>
        <translation>Prüfsumme validieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1034"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Verwirft Nachrichten mit ungültiger Fletcher-Prüfsumme.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1239"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Verwirft Nachrichten mit ungültiger additiver Prüfsumme.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1392"/>
        <source>Protocol version</source>
        <translation>Protokollversion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1393"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Wählt die erwartete Startmarkierung (0xFE für v1, 0xFD für v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1821"/>
        <source>Validate CRC</source>
        <translation>CRC Validieren</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1822"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Verwirft Frames mit ungültiger CRC-24Q-Prüfsumme.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1997"/>
        <source>Channel count</source>
        <translation>Kanalanzahl</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1998"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Anzahl der Ausgabekanäle (Register oder Coils).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2006"/>
        <source>Register offset</source>
        <translation>Register-Offset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2007"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Adress-Offset, der von Single-Write-Echos abgezogen wird (40001 für Legacy-Modicon-Maps).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2017"/>
        <source>Signed registers</source>
        <translation>Vorzeichenbehaftete Register</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2018"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Interpretiert 16-Bit-Register als vorzeichenbehaftete Zweierkomplement-Werte.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2325"/>
        <source>Payload layout</source>
        <translation>Nutzlast-Layout</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2326"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>Array gibt jedes Element der Reihe nach aus; Map leitet Schlüssel durch die Schlüsselliste.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2336"/>
        <source>Keys (map mode)</source>
        <translation>Schlüssel (Map-Modus)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2337"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Kommagetrennte Map-Schlüssel in Kanalreihenfolge. Nur für das Map-Layout verwendet.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Skalare Felder</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Kommagetrennte JSON-Felder, die in jedem generierten Frame wiederholt werden.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Sample-Array-Feld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>JSON-Feld, das das gebündelte Sample-Array enthält.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Records-Array-Feld</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>JSON-Feld, das das Array der Record-Objekte enthält.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Record-Felder (in Kanalreihenfolge)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Kommagetrennte Record-Felder. Die Position jedes Feldes legt seinen Kanalindex fest.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="526"/>
        <source>Column widths</source>
        <translation>Spaltenbreiten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="527"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Kommagetrennte Zeichenanzahl pro Feld. Leer lassen, um bei Leerzeichen zu trennen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="535"/>
        <source>Trim whitespace</source>
        <translation>Leerzeichen entfernen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="536"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Entfernt Auffüllung um jedes geschnittene Feld.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="665"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="814"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1282"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1710"/>
        <source>Keys (in channel order)</source>
        <translation>Schlüssel (in Kanalreihenfolge)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="666"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="815"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1711"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Kommagetrennte Schlüsselnamen. Die Position jedes Schlüssels legt seinen Kanalindex fest.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="674"/>
        <source>Pair separator</source>
        <translation>Paar-Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="675"/>
        <source>Character between key=value pairs.</source>
        <translation>Zeichen zwischen Schlüssel=Wert-Paaren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="681"/>
        <source>Key-value separator</source>
        <translation>Schlüssel-Wert-Trennzeichen</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="682"/>
        <source>Character between a key and its value.</source>
        <translation>Zeichen zwischen einem Schlüssel und seinem Wert.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="688"/>
        <source>Numeric values only</source>
        <translation>Nur numerische Werte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="689"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Ignoriert Paare, deren Wert keine Zahl ist.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="931"/>
        <source>Command routing table</source>
        <translation>Befehlsroutingtabelle</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="932"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Semikolongetrennte Einträge der NAME:Index-Liste, z. B. CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1157"/>
        <source>Talker prefix</source>
        <translation>Talker-Präfix</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1158"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>Zweistellige Talker-ID, z. B. GP für GPS oder GN für Mehrkonstellationsempfänger.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1167"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Lehnt Sätze ab, deren *hh-Prüfsumme nicht übereinstimmt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1283"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Kommagetrennte Parameternamen. Die Position jedes Schlüssels legt seinen Kanalindex fest.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1422"/>
        <source>Fields (in channel order)</source>
        <translation>Felder (in Kanalreihenfolge)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1423"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Kommagetrennte Feldnamen. Die Position jedes Feldes legt seinen Kanalindex fest.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1543"/>
        <source>Tags (in channel order)</source>
        <translation>Tags (in Kanalreihenfolge)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1544"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Kommagetrennte Tag-Namen. Die Position jedes Tags legt seinen Kanalindex fest.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Socket-typ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Remote-adresse</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Lokaler Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>0 eingeben für automatischen Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Remote-port</translation>
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
        <translation>Nach Kanal filtern…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Alle Benachrichtigungen löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(kein Titel)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Noch keine Benachrichtigungen</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">Datensatz-Transformationen und Output-Widget-Skripte können hier Ereignisse über notifyInfo / notifyWarning / notifyCritical veröffentlichen.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>Online-symbole Durchsuchen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>Download fehlgeschlagen: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Symbole suchen (z. B. Temperatur, Pfeil, Abspielen)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>Suchen Sie oben nach Symbolen, um zu beginnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Ausgabe-Widgets erfordern eine Pro-Lizenz.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Schaltfläche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Befehl bei Klick senden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Schieberegler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Skalierte numerische Werte senden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Umschalter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Ein/Aus-Befehle senden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Textfeld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Beliebige Befehle eingeben und senden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Drehknopf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Dreheingabe für Sollwerte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Sie können Ausgabe-Widgets konfigurieren, aber diese erscheinen nur mit einer Pro-Lizenz im Dashboard.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Dieses Ausgabe-Widget duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Dieses Ausgabe-Widget löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Übertragungsfunktion</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Übertragungsfunktion aus einer .js-Datei importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Vorlage</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Eine vorgefertigte Übertragungsfunktionsvorlage auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Die Übertragungsfunktion mit Beispieleingabe testen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Rückgängig</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Wiederholen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Ausschneiden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Alles Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Dokument Formatieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Auswahl Formatieren</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Painter-widget-fehler</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Painter-widget-code-editor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Painter-Code aus einer .js-Datei importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Vorlage</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Integrierte Painter-Vorlage auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Formatieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Painter-Code neu formatieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Testen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Live-Vorschau mit simulierten Datensatzwerten öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Vorschau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Ausschneiden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Alles Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Rückgängig</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Wiederholen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Dokument Formatieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Auswahl Formatieren</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Painter Live-vorschau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Vorschau</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Simulierte Datensätze</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Laufzeit OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>Warte auf ersten Frame...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Konsole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Konsole leeren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="245"/>
        <source>Interpolation: %1</source>
        <translation>Interpolation: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="258"/>
        <source>Show Area Under Plot</source>
        <translation>Fläche Unter Diagramm Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="276"/>
        <source>Show X Axis Label</source>
        <translation>X-achsenbeschriftung Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="287"/>
        <source>Show Y Axis Label</source>
        <translation>Y-achsenbeschriftung Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="299"/>
        <source>Show Crosshair</source>
        <translation>Fadenkreuz Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="306"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="306"/>
        <source>Resume</source>
        <translation>Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="323"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Sweep- / Trigger-modus</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="335"/>
        <source>Trigger Settings</source>
        <translation>Trigger-einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="359"/>
        <source>Reset View</source>
        <translation>Ansicht Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="365"/>
        <source>Axis Range Settings</source>
        <translation>Achsenbereich-einstellungen</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="203"/>
        <source>Interpolate</source>
        <translation>Interpolieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="221"/>
        <source>Orbit Navigation</source>
        <translation>Orbit-navigation</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="231"/>
        <source>Pan Navigation</source>
        <translation>Schwenk-navigation</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="242"/>
        <source>Orthogonal View</source>
        <translation>Orthogonale Ansicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="248"/>
        <source>Top View</source>
        <translation>Draufsicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="254"/>
        <source>Left View</source>
        <translation>Linke Ansicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="260"/>
        <source>Front View</source>
        <translation>Vorderansicht</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="277"/>
        <source>Auto Center</source>
        <translation>Auto-zentrierung</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="293"/>
        <source>Anaglyph 3D</source>
        <translation>Anaglyph 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="307"/>
        <source>Invert Eye Positions</source>
        <translation>Augenpositionen Invertieren</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="59"/>
        <source>None</source>
        <translation>Keine</translation>
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
        <translation>Linear</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1326"/>
        <source>Time</source>
        <translation>Zeit</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1349"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Ziehen zum Verschieben, Rechtsklick zum Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1352"/>
        <source>Click to place cursor</source>
        <translation>Klicken zum Platzieren des Cursors</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1354"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Klicken zum Platzieren des zweiten Cursors — Ziehen zum Verschieben</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Website Besuchen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Lizenz Kaufen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Aktivieren</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Assistent – Pro-Funktion</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>Der Assistent ist eine Serial Studio Pro-Funktion. Aktivieren Sie Ihre Lizenz, um ihn freizuschalten.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Aktivieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Schließen</translation>
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
        <translation>Prozess Starten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Ausführbare Datei</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/pfad/zur/ausführbaren/datei</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Durchsuchen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 wert1 --arg2 wert2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Arbeitsverzeichnis</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(optional) /arbeits/verzeichnis</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Pipe-pfad</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Laufenden Prozess Auswählen…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Starten Sie einen Kindprozess und erfassen Sie dessen stdout, oder verbinden Sie sich mit einer Named Pipe, die von einem bestehenden Prozess geschrieben wird.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Mehr über Named Pipes erfahren</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Laufenden Prozess Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Wählen Sie einen laufenden Prozess aus, um einen Named-Pipe-Pfadvorschlag abzuleiten.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Prozesse Filtern</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Zum Filtern nach Name eingeben…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Aktualisieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Laufende Prozesse</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>Prozess</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>Keine Prozesse entsprechen dem Filter.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Keine laufenden Prozesse gefunden.
Aktualisieren klicken, um die Liste zu aktualisieren.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 Prozess(e)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="42"/>
        <source>modified</source>
        <translation>geändert</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="342"/>
        <source>This project is password protected</source>
        <translation>Dieses Projekt ist passwortgeschützt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="343"/>
        <source>Editing is available in Project mode</source>
        <translation>Bearbeitung ist im Projektmodus verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="354"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Geben Sie das Passwort ein, um Änderungen vorzunehmen, oder öffnen Sie ein anderes Projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="355"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Wechseln Sie in den Projektmodus, um ein Projekt zu laden und zu bearbeiten.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="377"/>
        <source>Unlock</source>
        <translation>Entsperren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="378"/>
        <source>Switch to Project Mode</source>
        <translation>In Projektmodus Wechseln</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="397"/>
        <source>Open Other Project</source>
        <translation>Anderes Projekt Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="398"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="414"/>
        <source>Create New Project</source>
        <translation>Neues Projekt Erstellen</translation>
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
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>Nach Oben</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>Nach Unten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Ausgewählte Duplizieren (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>Duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>Ausgewählte Löschen (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Neu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Neues JSON-Projekt erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Bestehendes JSON-Projekt öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Aktuelles Projekt speichern</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Speichern Unter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Aktuelles Projekt unter neuem Namen speichern</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">Entsperren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Sperren</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">Projekt-Editor mit dem Projektpasswort entsperren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Projekt aus einem Protocol Buffers (.proto) Schema generieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Passwort Festlegen und Projekt-Editor Sperren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Gerät Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Neue Datenquelle (Gerät) zum Projekt hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Aktion</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Neue Aktion zum Projekt hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Ausgabe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Wiederherstellen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Einen kürzlich erstellten automatischen Snapshot des aktuellen Projekts wiederherstellen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Neues Ausgabe-Bedienfeld mit einer Schaltfläche hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Schieberegler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Ausgabe-Schieberegler hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Umschalter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Ausgabe-Umschalter hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Drehregler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Ausgabe-Drehregler hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Textfeld</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Ausgabe-Textfeld hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Schaltfläche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Ausgabe-Schaltflächen-Steuerelement hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>Datensatz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>Generischen Datensatz hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>2D-Diagramm-Datensatz hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>FFT-diagramm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Fast-Fourier-Transformation-Diagramm hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Anzeige</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Anzeige-Widget für numerische Daten hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Pegelanzeige</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Fügt eine vertikale Balken-Pegelanzeige hinzu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>Kompass</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Fügt ein Kompass-Widget für Richtungsdaten hinzu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>LED-anzeige</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Fügt eine LED-Statusanzeige hinzu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Fügt eine Datensatz-Container-Gruppe hinzu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Datensatz-container</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Bild</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Bild-/Videostream-Viewer hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Bildansicht</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="458"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Ein benutzerdefiniertes JavaScript-gerendertes Painter-Widget hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="459"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Painter-Widgets erfordern eine Pro-Lizenz – das Hinzufügen eines solchen fällt auf ein Datenraster zurück</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="460"/>
        <source>Painter Widget</source>
        <translation>Painter-widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="472"/>
        <source>Table</source>
        <translation>Tabelle</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="475"/>
        <source>Add a data table view</source>
        <translation>Datentabellenansicht hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="477"/>
        <source>Data Grid</source>
        <translation>Datenraster</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Multi-Plot</source>
        <translation>Multi-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>2D-Plot mit mehreren Signalen hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="487"/>
        <source>Multiple Plot</source>
        <translation>Mehrfach-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="492"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="497"/>
        <source>3D Plot</source>
        <translation>3D-plot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Add a 3D plot visualization</source>
        <translation>3D-Plot-Visualisierung hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="507"/>
        <source>Accelerometer</source>
        <translation>Beschleunigungssensor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>Gruppe für 3-Achsen-Beschleunigungssensordaten hinzufügen</translation>
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
        <translation>Gruppe für 3-Achsen-Gyroskop-Daten hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="522"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="527"/>
        <source>GPS Map</source>
        <translation>GPS-karte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a map widget for GPS data</source>
        <translation>Karten-Widget für GPS-Daten hinzufügen</translation>
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
        <translation>Assistenten öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="552"/>
        <source>Help Center</source>
        <translation>Hilfecenter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="556"/>
        <source>Open the Project Editor documentation</source>
        <translation>Dokumentation des Projekt-Editors öffnen</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Projektübersicht</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>Pro-Funktionen in diesem Projekt erkannt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Ersatz-Widgets werden verwendet. Lizenz erwerben, um volle Funktionalität freizuschalten.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Projekttitel:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Unbenanntes Projekt</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Punkte:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>Zeitbereich:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="235"/>
        <source>Source</source>
        <translation>Quelle</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="236"/>
        <source>Sources</source>
        <translation>Quellen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="241"/>
        <source>Group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="242"/>
        <source>Groups</source>
        <translation>Gruppen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="247"/>
        <source>Dataset</source>
        <translation>Datensatz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="248"/>
        <source>Datasets</source>
        <translation>Datensätze</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Action</source>
        <translation>Aktion</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="254"/>
        <source>Actions</source>
        <translation>Aktionen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="342"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Doppelklicken Sie auf einen Block, um ihn zu bearbeiten. Rechtsklicken Sie an eine beliebige Stelle, um eine Gruppe, einen Datensatz, eine Aktion, eine Datentabelle oder ein Gerät hinzuzufügen.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Vorschau der Protocol Buffers-datei</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Proto-datei: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Durchsuchen Sie die Nachrichten unten und erstellen Sie dann das Projekt. Jede Nachricht wird zu einer Dashboard-Gruppe; Kanalblöcke mit übereinstimmendem Typ erhalten ein MultiPlot und Nachrichten mit gemischten Typen erhalten ein DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Felder anzeigen für</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Felder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Feldname</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Keine Felder in der ausgewählten Nachricht.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Gesamt: %1 Nachrichten, %2 Felder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Projekt Erstellen</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Kein Fehler</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Der Broker hat die Verbindung aufgrund einer nicht unterstützten Protokollversion abgelehnt. Passen Sie die MQTT-Version des Brokers an und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Der Broker hat die Client-ID abgelehnt. Sie ist möglicherweise fehlerhaft, zu lang oder bereits in Verwendung. Generieren Sie sie neu und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>Das Netzwerk hat den Broker erreicht, aber der Broker ist derzeit nicht verfügbar. Überprüfen Sie seinen Status und versuchen Sie es später erneut.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>Der Benutzername oder das Passwort ist falsch oder fehlerhaft. Überprüfen Sie die Anmeldedaten und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Der Broker hat die Verbindung aufgrund unzureichender Berechtigungen abgelehnt. Stellen Sie sicher, dass das Konto über die erforderlichen ACLs verfügt.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>Ein Netzwerk- oder Transportschichtproblem hat die Verbindung verhindert. Prüfen Sie Konnektivität, Ports und TLS-Konfiguration.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>Der Client hat eine Verletzung des MQTT-Protokolls erkannt und die Verbindung geschlossen. Überprüfen Sie die Kompatibilität von Broker und Client.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>Ein unerwarteter Fehler ist aufgetreten. Prüfen Sie die Broker-Protokolle und die Anwendungskonsole für weitere Details.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>Ein MQTT-5-Protokollfehler ist aufgetreten. Prüfen Sie den Reason-Code des Brokers für weitere Details.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Nicht spezifizierter MQTT-Fehler (Code %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="234"/>
        <source>Failed to load welcome text :(</source>
        <translation>Willkommenstext konnte nicht geladen werden :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="264"/>
        <source>Network error</source>
        <translation>Netzwerkfehler</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <location filename="../../src/Licensing/Trial.cpp" line="284"/>
        <location filename="../../src/Licensing/Trial.cpp" line="317"/>
        <source>Trial Activation Error</source>
        <translation>Fehler Bei Testversionsaktivierung</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="281"/>
        <source>Invalid server response</source>
        <translation>Ungültige Serverantwort</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="282"/>
        <source>The server returned malformed data: %1</source>
        <translation>Der Server hat fehlerhafte Daten zurückgegeben: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="314"/>
        <source>Unexpected server response</source>
        <translation>Unerwartete Serverantwort</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="315"/>
        <source>The server response is missing required fields.</source>
        <translation>In der Serverantwort fehlen erforderliche Felder.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="168"/>
        <source>Console Output File Error</source>
        <translation>Fehler Bei Konsolenausgabedatei</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="169"/>
        <source>Cannot open file for writing!</source>
        <translation>Datei kann nicht zum Schreiben geöffnet werden!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="805"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Ungültiger Bluetooth-Adapter!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="808"/>
        <source>Unsuported platform or operating system</source>
        <translation>Nicht unterstützte Plattform oder Betriebssystem</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="811"/>
        <source>Unsupported discovery method</source>
        <translation>Nicht unterstützte Erkennungsmethode</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="814"/>
        <source>General I/O error</source>
        <translation>Allgemeiner E/A-Fehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="256"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="284"/>
        <source>Frame Parser Disabled</source>
        <translation>Frame-parser Deaktiviert</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="285"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Der Lua-Frame-Parser für Quelle %1 hat %2 Frames hintereinander eine Zeitüberschreitung verursacht und wurde deaktiviert, um Serial Studio reaktionsfähig zu halten.

Wahrscheinlichste Ursache: eine Endlosschleife oder extrem langsame Operation im Skript. Skript korrigieren und Projekt neu laden, um das Parsen wieder zu aktivieren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="333"/>
        <source>Lua Syntax Error</source>
        <translation>Lua-syntaxfehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="334"/>
        <source>The parser code contains an error:

%1</source>
        <translation>Der Parser-Code enthält einen Fehler:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="384"/>
        <source>Lua Runtime Error</source>
        <translation>Lua-laufzeitfehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="385"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>Der Parser-Code hat einen Fehler ausgelöst:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="488"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="406"/>
        <source>Missing Parse Function</source>
        <translation>Fehlende Parse-funktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="407"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>Die Funktion 'parse' ist im Skript nicht definiert.

Bitte stellen Sie sicher, dass Ihr Code Folgendes enthält:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="542"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="469"/>
        <source>Parse Function Runtime Error</source>
        <translation>Laufzeitfehler der Parse-funktion</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="470"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>Die Parse-Funktion enthält einen Fehler:

%1

Bitte beheben Sie den Fehler im Funktionskörper.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="257"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Der JavaScript-Frame-Parser für Quelle %1 hat %2 Frames hintereinander eine Zeitüberschreitung verursacht und wurde deaktiviert, um Serial Studio reaktionsfähig zu halten.

Wahrscheinlichste Ursache: eine Endlosschleife oder extrem langsame Operation im Skript. Skript korrigieren und Projekt neu laden, um das Parsen wieder zu aktivieren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="427"/>
        <source>JavaScript Timed Out</source>
        <translation>Javascript-zeitüberschreitung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="428"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>Der Parser-Code wurde innerhalb von %1 ms nicht vollständig ausgewertet und wurde unterbrochen.

Wahrscheinlichste Ursache: eine Endlosschleife auf oberster Ebene des Skripts.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="446"/>
        <source>JavaScript Syntax Error</source>
        <translation>Javascript-syntaxfehler</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="447"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>Der Parser-Code enthält einen Syntaxfehler in Zeile %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="462"/>
        <source>JavaScript Exception Occurred</source>
        <translation>Javascript-ausnahme Aufgetreten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="463"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>Der Parser-Code hat folgende Ausnahmen ausgelöst:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="489"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>Die Funktion 'parse' ist im Skript nicht definiert.

Stellen Sie sicher, dass Ihr Code Folgendes enthält:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="543"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>Die Parse-Funktion enthält einen Fehler in Zeile %1:

%2

Bitte beheben Sie den Fehler im Funktionskörper.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="646"/>
        <source>Invalid Function Declaration</source>
        <translation>Ungültige Funktionsdeklaration</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Kein aufrufbarer 'parse'-Export gefunden.

Definieren Sie eine der folgenden Varianten:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="663"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>Die Funktion 'parse' muss mindestens einen Parameter akzeptieren (die Frame-Nutzlast).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Keine gültige Deklaration der Funktion 'parse' gefunden.

Erwartetes Format:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="662"/>
        <source>Invalid Function Parameter</source>
        <translation>Ungültiger Funktionsparameter</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Die Funktion 'parse' muss mindestens einen Parameter haben.

Erwartetes Format:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="627"/>
        <source>Deprecated Function Signature</source>
        <translation>Veraltete Funktionssignatur</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="628"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>Die Funktion 'parse' verwendet das alte Zwei-Parameter-Format: parse(%1, %2)

Dieses Format wird nicht mehr unterstützt. Bitte auf das neue Ein-Parameter-Format aktualisieren:
function parse(%1) { ... }

Der Trennzeichen-Parameter wird nicht mehr benötigt.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="188"/>
        <source>Critical</source>
        <translation>Kritisch</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="188"/>
        <source>Warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="523"/>
        <source>Project file not found</source>
        <translation>Projektdatei nicht gefunden</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="524"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Die von dieser Verknüpfung referenzierte Projektdatei konnte nicht gefunden werden:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="527"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Soll diese Verknüpfung gelöscht werden?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="531"/>
        <source>Delete Shortcut</source>
        <translation>Verknüpfung Löschen</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="533"/>
        <source>Quit</source>
        <translation>Beenden</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1070"/>
        <source>Time (s)</source>
        <translation>Zeit (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1132"/>
        <source>Freq: %1</source>
        <translation>Freq: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1136"/>
        <source>Time: −%1</source>
        <translation>Zeit: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="327"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Kein OpenAI-API-Schlüssel gesetzt. Öffnen Sie Schlüssel verwalten, um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="199"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Kein Anthropic-API-Schlüssel gesetzt. Öffnen Sie Schlüssel verwalten, um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="282"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Kein Gemini-API-Schlüssel gesetzt. Öffnen Sie Schlüssel verwalten, um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="321"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Keine URL für lokalen Modellserver konfiguriert. Öffnen Sie „Schlüssel verwalten", um eine festzulegen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="141"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Kein DeepSeek-API-Schlüssel festgelegt. Öffnen Sie „Schlüssel verwalten", um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="164"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Kein Mistral-API-Schlüssel festgelegt. Öffnen Sie „Schlüssel verwalten", um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="177"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Kein OpenRouter-API-Schlüssel festgelegt. Öffnen Sie „Schlüssel verwalten", um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="148"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Kein Groq-API-Schlüssel festgelegt. Öffnen Sie „Schlüssel verwalten", um einen hinzuzufügen.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="863"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Der Frame-Parser verwendet mehr als %1% der CPU-Zeit.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="865"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio verwirft Frames, um die Anwendung reaktionsfähig zu halten. Bitte Frame-Parser-Skript vereinfachen oder optimieren, um die Arbeitslast zu reduzieren.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="388"/>
        <source>Expected %1, got '%2'</source>
        <translation>%1 erwartet, '%2' erhalten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="437"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Enum-Name nach 'enum' erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="451"/>
        <source>Expected oneof name</source>
        <translation>Oneof-Name erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="479"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Feld-Tag '%1' außerhalb des Bereichs (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="497"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Schlüsseltyp in map&lt;&gt; erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="505"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Werttyp in map&lt;&gt; erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="513"/>
        <source>Expected map field name</source>
        <translation>Map-Feldname erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="525"/>
        <source>Expected map field tag</source>
        <translation>Map-Feld-Tag erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="557"/>
        <source>Expected field type, got '%1'</source>
        <translation>Feldtyp erwartet, '%1' erhalten</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="577"/>
        <source>Expected field name after type</source>
        <translation>Feldname nach Typ erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="587"/>
        <source>Expected field tag number</source>
        <translation>Feld-Tag-Nummer erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Nachrichtenverschachtelung zu tief (Limit %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="640"/>
        <source>Expected message name</source>
        <translation>Nachrichtenname erwartet</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="722"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Unerwartetes Token '%1' im Datei-Gültigkeitsbereich</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="768"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Nicht unterstütztes Schlüsselwort der obersten Ebene '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Automatisch (Plattformstandard)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Software (Fallback)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Zeile %1</translation>
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
        <translation type="vanished">Zeilen</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 Zeile(n)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="185"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>Die native Parser-Konfiguration ist kein gültiges JSON-Objekt.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="195"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Unbekannte native Parser-Vorlage: „%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="329"/>
        <source>Built-In Parser Error</source>
        <translation>Integrierter Parser-fehler</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Nativer Parser-fehler</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append wird für GZIP nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Öffnen von gzip zum Lesen und Schreiben gleichzeitig wird nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>Eine gzip-Datei kann entweder zum Lesen oder zum Schreiben geöffnet werden. Was soll es sein?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Gzopen() der Datei fehlgeschlagen</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append wird für QuaZIODevice nicht unterstützt</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite wird für QuaZIODevice nicht unterstützt</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>ZIP/UNZIP API-Fehler %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>PDF-bericht Generieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Bericht Generieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>Durchgezogen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>Gestrichelt</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>Gepunktet</translation>
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
        <translation>Letter (8,5 × 11 Zoll)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="90"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8,5 × 14 Zoll)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="91"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7,25 × 10,5 Zoll)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="92"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 Zoll)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="93"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 Zoll)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>%1 — Session Report</source>
        <translation>%1 — Sitzungsbericht</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="279"/>
        <source>Session Report</source>
        <translation>Sitzungsbericht</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="187"/>
        <source>Branding</source>
        <translation>Branding</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="193"/>
        <source>Page</source>
        <translation>Seite</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="199"/>
        <source>Sections</source>
        <translation>Abschnitte</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="247"/>
        <source>Identity</source>
        <translation>Identität</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="261"/>
        <source>Company</source>
        <translation>Unternehmen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="268"/>
        <source>e.g. Acme Test Systems</source>
        <translation>z. B. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="272"/>
        <source>Document title</source>
        <translation>Dokumenttitel</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="283"/>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="290"/>
        <source>Prepared by (optional)</source>
        <translation>Erstellt von (optional)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="299"/>
        <source>Logo</source>
        <translation>Logo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="312"/>
        <source>File</source>
        <translation>Datei</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="323"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG oder SVG (optional)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="325"/>
        <source>Browse…</source>
        <translation>Durchsuchen…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="328"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="369"/>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="381"/>
        <source>Page size</source>
        <translation>Seitengröße</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="509"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Min-, Max- und Mittelwerte in Diagrammen annotieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export HTML</source>
        <translation>HTML Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="497"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Deckblatt (Logo, Dokumenttitel, Test-Untertitel)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="500"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Testinformationen (Projekt, Zeitstempel, Klassifizierung und Notizen)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="503"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Messzusammenfassung (Min., Max., Mittelwert, Standardabweichung pro Parameter)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="506"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Parametertrends (Zeitreihendarstellung pro numerischem Parameter)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="396"/>
        <source>Plot appearance</source>
        <translation>Diagrammdarstellung</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="410"/>
        <source>Line width</source>
        <translation>Linienbreite</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="442"/>
        <source>Line style</source>
        <translation>Linienstil</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="482"/>
        <source>Include</source>
        <translation>Einschließen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="532"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export PDF</source>
        <translation>PDF Exportieren</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Bericht Wird Erstellt</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>Verarbeitung…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Dies kann bei Sitzungen mit vielen Parametern einige Sekunden dauern. Das Fenster schließt sich automatisch, sobald der Bericht fertig ist.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Connection Lost</source>
        <translation>Verbindung Verloren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="42"/>
        <source>Device Unavailable</source>
        <translation>Gerät Nicht Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>The connection to your device was lost.</source>
        <translation>Die Verbindung zu Ihrem Gerät wurde unterbrochen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="97"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio konnte das Gerät nicht erreichen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Überprüfen Sie das Kabel, die Stromversorgung und ob eine andere Anwendung das Gerät übernommen hat. Sie können versuchen, die Verbindung wiederherzustellen, zu einem anderen Gerät wechseln oder beenden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Sicherstellen, dass es angeschlossen, eingeschaltet und nicht bereits von einer anderen Anwendung verwendet wird. Erneut versuchen, ein anderes Gerät auswählen oder beenden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>Beenden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>Anderes Gerät Wählen</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">Neu Konfigurieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>Erneut Versuchen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>Erneut Verbinden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Wählen Sie das richtige Gerät aus und drücken Sie dann auf Verbinden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>E/a-schnittstelle: %1</translation>
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
        <location filename="../../src/SerialStudio.cpp" line="336"/>
        <source>Data Grids</source>
        <translation>Datenraster</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="339"/>
        <source>Multiple Data Plots</source>
        <translation>Mehrfache Datendiagramme</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="342"/>
        <source>Accelerometers</source>
        <translation>Beschleunigungsmesser</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="345"/>
        <source>Gyroscopes</source>
        <translation>Gyroskope</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="348"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="351"/>
        <source>FFT Plots</source>
        <translation>FFT-diagramme</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="354"/>
        <source>LED Panels</source>
        <translation>LED-panels</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="357"/>
        <source>Data Plots</source>
        <translation>Datendiagramme</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="360"/>
        <source>Bars</source>
        <translation>Balken</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="363"/>
        <source>Gauges</source>
        <translation>Messgeräte</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="366"/>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="369"/>
        <source>Clock</source>
        <translation>Uhr</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="372"/>
        <source>Stopwatch</source>
        <translation>Stoppuhr</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="375"/>
        <source>Compasses</source>
        <translation>Kompasse</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="378"/>
        <source>Meters</source>
        <translation>Messgeräte</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Thermometer</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="381"/>
        <source>3D Plots</source>
        <translation>3D-diagramme</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="385"/>
        <source>Image Views</source>
        <translation>Bildansichten</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="388"/>
        <source>Output Panels</source>
        <translation>Ausgabepanels</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="391"/>
        <source>Notifications</source>
        <translation>Benachrichtigungen</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="394"/>
        <source>Waterfalls</source>
        <translation>Wasserfalldiagramme</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="397"/>
        <source>Painter Widgets</source>
        <translation>Painter-widgets</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="922"/>
        <source>UTF-8</source>
        <translation>UTF-8</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="923"/>
        <source>UTF-16 LE</source>
        <translation>UTF-16 LE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="924"/>
        <source>UTF-16 BE</source>
        <translation>UTF-16 BE</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="925"/>
        <source>Latin-1</source>
        <translation>Latin-1</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="926"/>
        <source>System</source>
        <translation>System</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="927"/>
        <source>GBK</source>
        <translation>GBK</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="928"/>
        <source>GB18030</source>
        <translation>GB18030</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="929"/>
        <source>Big5</source>
        <translation>Big5</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="930"/>
        <source>Shift-JIS</source>
        <translation>Shift-jis</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="931"/>
        <source>EUC-JP</source>
        <translation>EUC-JP</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="932"/>
        <source>EUC-KR</source>
        <translation>EUC-KR</translation>
    </message>
</context>
<context>
    <name>SessionDetail</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="19"/>
        <source>Session Details</source>
        <translation>Sitzungsdetails</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Wählen Sie eine Sitzung aus, um Details anzuzeigen.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Gestartet:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Beendet:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(läuft)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Frames:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Notizen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Sitzungsnotizen hinzufügen…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Notizen sind für abgeschlossene Sitzungen schreibgeschützt.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Neues Tag…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Sitzungsdatei entsperren, um Sitzungen zu löschen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Tags</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Wiedergabe</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>CSV Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Bericht Erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Sitzungen</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Suchen</translation>
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
        <translation>Keine Sitzungen gefunden.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Keine Sitzungsdatei geöffnet.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Select logo image</source>
        <translation>Logo-Bild auswählen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1000"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Bilder (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="418"/>
        <source>Open Session File</source>
        <translation>Sitzungsdatei Öffnen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="420"/>
        <source>Session files (*.db)</source>
        <translation>Sitzungsdateien (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1198"/>
        <source>Cannot open session file</source>
        <translation>Sitzungsdatei kann nicht geöffnet werden</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="647"/>
        <source>Delete session from %1?</source>
        <translation>Sitzung aus %1 löschen?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="650"/>
        <source>Delete Session</source>
        <translation>Sitzung Löschen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1059"/>
        <source>No project data</source>
        <translation>Keine Projektdaten</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="648"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Alle Messwerte und Rohdaten für diese Sitzung werden dauerhaft entfernt.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="477"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <source>Lock Session File</source>
        <translation>Sitzungsdatei Sperren</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="478"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Passwort zum Sperren der Sitzungsdatei wählen:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="487"/>
        <source>Confirm the password:</source>
        <translation>Passwort bestätigen:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>Passwords do not match</source>
        <translation>Passwörter Stimmen Nicht Überein</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="496"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>Die beiden eingegebenen Passwörter stimmen nicht überein. Die Sitzungsdatei wurde nicht gesperrt.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="532"/>
        <source>Unlock Session File</source>
        <translation>Sitzungsdatei Entsperren</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="533"/>
        <source>Enter the session file password:</source>
        <translation>Passwort der Sitzungsdatei eingeben:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="543"/>
        <source>Incorrect password</source>
        <translation>Falsches Passwort</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="544"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>Das eingegebene Passwort stimmt nicht mit dem in der Sitzungsdatei gespeicherten überein.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="637"/>
        <source>Session file locked</source>
        <translation>Sitzungsdatei gesperrt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="638"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Sitzungsdatei entsperren, bevor aufgezeichnete Sitzungen gelöscht werden.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">Diese Sitzung enthält keine eingebettete Projektdatei – das Dashboard verwendet ein Quick-Plot-Layout.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="772"/>
        <source>Export Session to CSV</source>
        <translation>Sitzung als CSV Exportieren</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="772"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV-Dateien (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">Export Abgeschlossen</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">Sitzung exportiert nach:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">Export wird vorbereitet…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <source>Done</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="934"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <source>Failed</source>
        <translation>Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="940"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="980"/>
        <source>Report Failed</source>
        <translation>Bericht Fehlgeschlagen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="942"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="981"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Der Bericht konnte nicht erstellt werden. Überprüfen Sie den Ausgabepfad und versuchen Sie es erneut.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="868"/>
        <source>Save PDF Report</source>
        <translation>PDF-bericht Speichern</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="848"/>
        <source>Loading session data…</source>
        <translation>Sitzungsdaten werden geladen…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="868"/>
        <source>Save HTML Report</source>
        <translation>HTML-bericht Speichern</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="869"/>
        <source>PDF files (*.pdf)</source>
        <translation>PDF-Dateien (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="869"/>
        <source>HTML files (*.html)</source>
        <translation>HTML-Dateien (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1060"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Diese Sitzungsdatei enthält kein eingebettetes Projekt.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1069"/>
        <source>Invalid project data</source>
        <translation>Ungültige Projektdaten</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1070"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>Die eingebetteten Projekt-JSON-Daten sind fehlerhaft und können nicht wiederhergestellt werden.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1080"/>
        <source>Restore Project</source>
        <translation>Projekt Wiederherstellen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1080"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Serial Studio-Projekte (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1088"/>
        <source>Cannot write file</source>
        <translation>Datei kann nicht geschrieben werden</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1088"/>
        <source>Check file permissions and try again.</source>
        <translation>Dateiberechtigungen prüfen und erneut versuchen.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="77"/>
        <source>Empty file path</source>
        <translation>Leerer Dateipfad</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="174"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="229"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="290"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="361"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="386"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="414"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="454"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="644"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="709"/>
        <source>Database not open</source>
        <translation>Datenbank nicht geöffnet</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="267"/>
        <source>Database not open or empty label</source>
        <translation>Datenbank nicht geöffnet oder Bezeichnung leer</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="335"/>
        <source>Invalid label</source>
        <translation>Ungültige Bezeichnung</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="603"/>
        <source>Cancelled</source>
        <translation>Abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="716"/>
        <source>Could not load session data</source>
        <translation>Sitzungsdaten konnten nicht geladen werden</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="212"/>
        <source>Assembling report…</source>
        <translation>Bericht wird zusammengestellt…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="221"/>
        <source>Writing output…</source>
        <translation>Ausgabe wird geschrieben…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="293"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="357"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="731"/>
        <source>Session Report</source>
        <translation>Sitzungsbericht</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="360"/>
        <source>Untitled project</source>
        <translation>Unbenanntes Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="368"/>
        <source>Prepared by</source>
        <translation>Erstellt von</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="371"/>
        <source>Generated on %1</source>
        <translation>Generiert am %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="394"/>
        <source>Test ID</source>
        <translation>Test-ID</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="396"/>
        <source>Duration</source>
        <translation>Dauer</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="398"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="515"/>
        <source>Samples</source>
        <translation>Samples</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="400"/>
        <source>Parameters</source>
        <translation>Parameter</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="402"/>
        <source>Started</source>
        <translation>Gestartet</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="404"/>
        <source>Ended</source>
        <translation>Beendet</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="440"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="442"/>
        <source>Test identifier</source>
        <translation>Testkennung</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="443"/>
        <source>Start time</source>
        <translation>Startzeit</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="444"/>
        <source>End time</source>
        <translation>Endzeit</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="445"/>
        <source>Total duration</source>
        <translation>Gesamtdauer</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Samples acquired</source>
        <translation>Erfasste Samples</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="447"/>
        <source>Parameters logged</source>
        <translation>Protokollierte Parameter</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="465"/>
        <source>Classification</source>
        <translation>Klassifizierung</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="473"/>
        <source>Notes</source>
        <translation>Notizen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="481"/>
        <source>Test Information</source>
        <translation>Testinformationen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="504"/>
        <source>Parameter</source>
        <translation>Parameter</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="507"/>
        <source>Units</source>
        <translation>Einheiten</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="516"/>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="517"/>
        <source>Maximum</source>
        <translation>Maximum</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="518"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="680"/>
        <source>Mean</source>
        <translation>Mittelwert</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="519"/>
        <source>Std. Deviation</source>
        <translation>Standardabweichung</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="565"/>
        <source>Measurement Summary</source>
        <translation>Messzusammenfassung</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="566"/>
        <source>click a column to sort</source>
        <translation>Spalte zum Sortieren anklicken</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="592"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 Samples über %2 Sekunden</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="611"/>
        <source>Combined Parameter View</source>
        <translation>Kombinierte Parameteransicht</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="612"/>
        <source>click legend items to toggle signals</source>
        <translation>Legendenelemente zum Umschalten von Signalen anklicken</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="620"/>
        <source>Parameter Trends</source>
        <translation>Parametertrends</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="678"/>
        <source>Min</source>
        <translation>Min.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="679"/>
        <source>Max</source>
        <translation>Max.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="758"/>
        <source>Page %1 of %2</source>
        <translation>Seite %1 von %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="828"/>
        <source>Loading rendering engine…</source>
        <translation>Rendering-Engine wird geladen…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="850"/>
        <source>Rendering charts…</source>
        <translation>Diagramme werden gerendert…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="895"/>
        <source>Generating PDF…</source>
        <translation>PDF Wird Generiert…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="259"/>
        <source>Open Session File</source>
        <translation>Sitzungsdatei Öffnen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Session files (*.db)</source>
        <translation>Sitzungsdateien (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="337"/>
        <source>Device Connection Active</source>
        <translation>Geräteverbindung Aktiv</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="338"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Um diese Funktion zu nutzen, muss die Verbindung zum Gerät getrennt werden. Fortfahren?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="388"/>
        <location filename="../../src/Sessions/Player.cpp" line="470"/>
        <source>Cannot open session file</source>
        <translation>Sitzungsdatei kann nicht geöffnet werden</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="389"/>
        <source>Unknown error</source>
        <translation>Unbekannter Fehler</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="407"/>
        <source>No project data</source>
        <translation>Keine Projektdaten</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="408"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Diese Sitzung enthält keine eingebettete Projektdatei – das Dashboard verwendet ein Quick-Plot-Layout.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="471"/>
        <source>Check file permissions and try again.</source>
        <translation>Dateiberechtigungen prüfen und erneut versuchen.</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">Keine Sitzungen gefunden</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">Diese Datei enthält keine aufgezeichneten Sitzungen.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">Sitzung hat keine Spalten</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">Der ausgewählten Sitzung fehlen die Spaltendefinitionen.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">Sitzung hat keine Messwerte</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">Die ausgewählte Sitzung enthält keine Frames.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Leerer Dateipfad</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Diese Datei enthält keine aufgezeichneten Sitzungen.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Abgebrochen</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>Der ausgewählten Sitzung fehlen die Spaltendefinitionen.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>Die ausgewählte Sitzung enthält keine Frames.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="159"/>
        <source>Language</source>
        <translation>Sprache</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="175"/>
        <source>Theme</source>
        <translation>Design</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="210"/>
        <source>Workspace Folder</source>
        <translation>Arbeitsbereich-ordner</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Automatically Check for Updates</source>
        <translation>Automatisch nach Updates suchen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="386"/>
        <source>Export…</source>
        <translation>Exportieren…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="435"/>
        <source>Data Plotting</source>
        <translation>Datenvisualisierung</translation>
    </message>
    <message>
        <source>Point Count</source>
        <translation type="vanished">Punktanzahl</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="500"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Ui-aktualisierungsrate (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="737"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Taskleisten-schaltflächen Immer Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="624"/>
        <source>Show Actions Panel</source>
        <translation>Aktionsbereich Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>API-server Aktivieren (Port 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="849"/>
        <source>Console</source>
        <translation>Konsole</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="144"/>
        <source>Appearance</source>
        <translation>Erscheinungsbild</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="194"/>
        <source>Files &amp; Updates</source>
        <translation>Dateien &amp; Updates</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="255"/>
        <source>Advanced</source>
        <translation>Erweitert</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="338"/>
        <source>Allow External API Connections</source>
        <translation>Externe API-verbindungen Zulassen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="303"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Symbolleiste Automatisch Ausblenden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Taskbar</source>
        <translation>Taskleiste</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="270"/>
        <source>Rendering Backend</source>
        <translation>Rendering-backend</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>API Access Token</source>
        <translation>API-zugriffstoken</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="450"/>
        <source>Time Range</source>
        <translation>Zeitbereich</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Small</source>
        <translation>Klein</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Large</source>
        <translation>Groß</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Extra Large</source>
        <translation>Sehr Groß</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Custom</source>
        <translation>Benutzerdefiniert</translation>
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
        <translation>Videos Standardmäßig Speichern</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Behavior</source>
        <translation>Verhalten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="752"/>
        <source>Show Search Field</source>
        <translation>Suchfeld Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="767"/>
        <source>Auto-hide Taskbar</source>
        <translation>Taskleiste Automatisch Ausblenden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="785"/>
        <source>Hide Delay (ms)</source>
        <translation>Ausblendverzögerung (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="809"/>
        <source>Pinned Buttons</source>
        <translation>Angeheftete Schaltflächen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="827"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Ziehen Sie eine angeheftete Schaltfläche in der Taskleiste, um sie neu anzuordnen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="848"/>
        <source>Settings</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="851"/>
        <source>Clock</source>
        <translation>Uhr</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="852"/>
        <source>Stopwatch</source>
        <translation>Stoppuhr</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="853"/>
        <source>Pause / Resume</source>
        <translation>Pausieren / Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="854"/>
        <source>File Transmission</source>
        <translation>Dateiübertragung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="855"/>
        <source>AI Assistant</source>
        <translation>Ki-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="984"/>
        <source>Display</source>
        <translation>Anzeige</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="999"/>
        <source>Display Mode</source>
        <translation>Anzeigemodus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="537"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1012"/>
        <source>Font Family</source>
        <translation>Schriftfamilie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="86"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="850"/>
        <source>Notifications</source>
        <translation>Benachrichtigungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="384"/>
        <source>Export Protobuf File</source>
        <translation>Protobuf-datei Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="522"/>
        <source>Dashboard Font</source>
        <translation>Dashboard-schriftart</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="552"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1027"/>
        <source>Font Size</source>
        <translation>Schriftgröße</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">Bildexport</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">Bilder standardmäßig speichern</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1044"/>
        <source>Show Timestamps</source>
        <translation>Zeitstempel Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1063"/>
        <source>Data Transmission</source>
        <translation>Datenübertragung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1078"/>
        <source>Line Ending</source>
        <translation>Zeilenende</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1091"/>
        <source>Input Mode</source>
        <translation>Eingabemodus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1104"/>
        <source>Text Encoding</source>
        <translation>Textkodierung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1117"/>
        <source>Checksum</source>
        <translation>Prüfsumme</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1130"/>
        <source>Echo Sent Data</source>
        <translation>Gesendete Daten Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1149"/>
        <source>Escape Codes</source>
        <translation>Escape-codes</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>VT100 Emulation</source>
        <translation>Vt100-emulation</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1183"/>
        <source>ANSI Colors</source>
        <translation>Ansi-farben</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1241"/>
        <source>Delivery</source>
        <translation>Zustellung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1256"/>
        <source>System Notifications</source>
        <translation>Systembenachrichtigungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1277"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Warnungs-/Kritische Ereignisse als Desktop-Benachrichtigungen anzeigen, wenn Serial Studio nicht im Vordergrund ist.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1287"/>
        <source>Application Logs</source>
        <translation>Anwendungsprotokolle</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1302"/>
        <source>Route Warnings to Notifications</source>
        <translation>Warnungen an Benachrichtigungen Weiterleiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1323"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Standardmäßig deaktiviert – QT und QML geben häufig Warnungen aus, und die Aktivierung kann echte Alarme übertönen. Kritische Meldungen werden unabhängig von dieser Einstellung immer weitergeleitet.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1342"/>
        <source>Reset</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Close</source>
        <translation>Schließen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1385"/>
        <source>Apply</source>
        <translation>Anwenden</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Geräteeinrichtung</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>API-server Aktiv (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>API-server Bereit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>API-server Aus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Frame-analyse</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Nur Konsole (Keine Analyse)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Schnelldiagramm (Kommagetrennte Werte)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Über Projektdatei Parsen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Projektdatei Ändern (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Projektdatei Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Datenexport</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>CSV-tabelle</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Sitzungsaufzeichnung</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>MDF4-aufzeichnung</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Konsolenprotokoll</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">CSV-datei</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">Sitzungsdatenbank</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">MDF4-datei</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">Konsolen-dump</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">CSV-datei Erstellen</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">MDF4-datei Erstellen</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">Sitzungsprotokoll Erstellen</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">Konsolendaten Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>E/a-schnittstelle: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Multi-geräte-projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Dieses Projekt streamt Daten von %1 unabhängigen Geräten.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Jedes Gerät hat eigene Verbindungseinstellungen. Konfigurieren Sie diese im Projekt-Editor unter dem Reiter Quellen.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Projekt-editor Öffnen</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">Neue Verknüpfung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="95"/>
        <source>Choose an Icon</source>
        <translation>Symbol Auswählen</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">Verknüpfung Speichern</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="24"/>
        <source>New Deployment</source>
        <translation>Neue Bereitstellung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="108"/>
        <source>Save Deployment</source>
        <translation>Bereitstellung Speichern</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="148"/>
        <source>General</source>
        <translation>Allgemein</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="154"/>
        <source>Taskbar</source>
        <translation>Taskleiste</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="160"/>
        <source>Logging</source>
        <translation>Protokollierung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="217"/>
        <source>Identity</source>
        <translation>Identität</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="273"/>
        <source>Click to choose an icon</source>
        <translation>Klicken, um ein Symbol auszuwählen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="282"/>
        <source>Name:</source>
        <translation>Name:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="291"/>
        <source>Deployment Name</source>
        <translation>Bereitstellungsname</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="382"/>
        <source>Theme</source>
        <translation>Design</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="392"/>
        <source>Same as Serial Studio</source>
        <translation>Wie Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="414"/>
        <source>Actions Panel</source>
        <translation>Aktionsbereich</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="425"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="634"/>
        <source>File Transmission</source>
        <translation>Dateiübertragung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="441"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Ein Doppelklick auf diese Bereitstellung führt direkt zum Live-Dashboard für dieses Projekt. Es gibt keine Symbolleiste oder Einrichtungsbereich, nur die Daten, und Serial Studio wird beendet, sobald das Gerät getrennt wird.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="487"/>
        <source>Visibility</source>
        <translation>Sichtbarkeit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="502"/>
        <source>Mode</source>
        <translation>Modus</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="511"/>
        <source>Always shown</source>
        <translation>Immer Angezeigt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="512"/>
        <source>Auto-hide</source>
        <translation>Automatisch Ausblenden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="513"/>
        <source>Hidden</source>
        <translation>Ausgeblendet</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="528"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Das Ausblenden der Taskleiste entfernt die Schaltflächen zum Minimieren/Maximieren/Schließen des Fensters und erzwingt automatisches Layout, sodass das Dashboard immer den verfügbaren Bereich ausfüllt.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>Die Taskleiste wird eingeblendet, wenn der Cursor in die Nähe des unteren Bildschirmrands bewegt wird.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="534"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>Die Taskleiste ist dauerhaft am unteren Rand des Dashboards sichtbar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="547"/>
        <source>Pinned Buttons</source>
        <translation>Angeheftete Schaltflächen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="564"/>
        <source>Console</source>
        <translation>Konsole</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="578"/>
        <source>Notifications</source>
        <translation>Benachrichtigungen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="592"/>
        <source>Clock</source>
        <translation>Uhr</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="606"/>
        <source>Stopwatch</source>
        <translation>Stoppuhr</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="620"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="745"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Aufzeichnungen werden im Serial Studio-Arbeitsbereichsordner gespeichert</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">Name der Verknüpfung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="300"/>
        <source>Change Icon…</source>
        <translation>Symbol Ändern…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="317"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="335"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="345"/>
        <source>Choose a project file to begin</source>
        <translation>Projektdatei auswählen, um zu beginnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="367"/>
        <source>Behavior</source>
        <translation>Verhalten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="404"/>
        <source>Fullscreen</source>
        <translation>Vollbild</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">Ein Doppelklick auf diese Verknüpfung führt direkt zum Live-Dashboard für dieses Projekt. Es gibt keine Symbolleiste oder Einrichtungsbereich, nur die Daten, und Serial Studio wird beendet, sobald das Gerät getrennt wird.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">Projekt Einbetten</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">Ein Doppelklick auf diese Verknüpfung führt direkt zum Live-Dashboard für dieses Projekt. Es gibt keine Symbolleiste oder Einrichtungsbereich, nur die Daten, und Serial Studio wird beendet, sobald das Gerät getrennt wird.

Projekt Einbetten aktivieren, um das Projekt in die Verknüpfung einzubetten, sodass es auch funktioniert, wenn die ursprüngliche Datei verschoben oder gelöscht wird.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="684"/>
        <source>Recorders</source>
        <translation>Recorder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="699"/>
        <source>CSV File</source>
        <translation>CSV-datei</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="709"/>
        <source>MDF4 File</source>
        <translation>MDF4-datei</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="719"/>
        <source>Session Database</source>
        <translation>Sitzungsdatenbank</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="729"/>
        <source>Console Log</source>
        <translation>Konsolenprotokoll</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">Aufzeichnungen werden im Standardspeicherort des jeweiligen Moduls gespeichert.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="774"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="783"/>
        <source>Save</source>
        <translation>Speichern</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="116"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="255"/>
        <source>Undo</source>
        <translation>Rückgängig</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="267"/>
        <source>Redo</source>
        <translation>Wiederholen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="132"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="288"/>
        <source>Cut</source>
        <translation>Ausschneiden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="137"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="299"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="142"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="310"/>
        <source>Paste</source>
        <translation>Einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="149"/>
        <source>Select All</source>
        <translation>Alles Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="159"/>
        <source>Format Document</source>
        <translation>Dokument Formatieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="166"/>
        <source>Format Selection</source>
        <translation>Auswahl Formatieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="228"/>
        <source>Reset</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="232"/>
        <source>Reset template parameters to their defaults</source>
        <translation>Vorlagenparameter auf Standardwerte zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="233"/>
        <source>Reset to the default parsing script</source>
        <translation>Auf das Standard-Parsing-Skript zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="244"/>
        <source>Open</source>
        <translation>Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="250"/>
        <source>Import a script file for data parsing</source>
        <translation>Eine Skriptdatei für das Daten-Parsing importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Open help documentation for data parsing</source>
        <translation>Hilfedokumentation für das Daten-Parsing öffnen</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Sprache:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Nativ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="410"/>
        <source>Select Template…</source>
        <translation>Vorlage Auswählen…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="261"/>
        <source>Undo the last code edit</source>
        <translation>Letzte Code-Bearbeitung rückgängig machen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="274"/>
        <source>Redo the previously undone edit</source>
        <translation>Zuvor rückgängig gemachte Bearbeitung wiederherstellen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="294"/>
        <source>Cut selected code to clipboard</source>
        <translation>Ausgewählten Code in die Zwischenablage ausschneiden</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="305"/>
        <source>Copy selected code to clipboard</source>
        <translation>Ausgewählten Code in Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="315"/>
        <source>Paste code from clipboard</source>
        <translation>Code aus Zwischenablage einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="329"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="383"/>
        <source>Platform:</source>
        <translation>Plattform:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="391"/>
        <source>Built-In</source>
        <translation>Integriert</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="419"/>
        <source>Template:</source>
        <translation>Vorlage:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="447"/>
        <source>Test With Sample Data</source>
        <translation>Mit Beispieldaten Testen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="457"/>
        <source>Evaluate</source>
        <translation>Auswerten</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Duplizieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Kopie dieser Datenquelle erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Diese Datenquelle entfernen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>Die primäre Datenquelle kann nicht entfernt werden</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>Sitzungs-player</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>Sitzung wird geladen…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="276"/>
        <source>Workspaces</source>
        <translation>Arbeitsbereiche</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="406"/>
        <source>Actions</source>
        <translation>Aktionen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="429"/>
        <source>No Actions Available</source>
        <translation>Keine Aktionen Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="459"/>
        <source>Plugins</source>
        <translation>Plugins</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="507"/>
        <source>No Plugins Installed</source>
        <translation>Keine Plugins Installiert</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="99"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="543"/>
        <source>Auto Layout</source>
        <translation>Automatisches Layout</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="107"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="555"/>
        <source>Full Screen</source>
        <translation>Vollbild</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="113"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="568"/>
        <source>Add External Window</source>
        <translation>Externes Fenster Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="155"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="853"/>
        <source>Help Center</source>
        <translation>Hilfecenter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="684"/>
        <source>Tools</source>
        <translation>Werkzeuge</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="813"/>
        <source>No Tools Available</source>
        <translation>Keine Werkzeuge Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="881"/>
        <source>Reset</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Quit</source>
        <translation>Beenden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="939"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Hide</source>
        <translation>Ausblenden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>New Workspace…</source>
        <translation>Neuer Arbeitsbereich…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Clock</source>
        <translation>Uhr</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="141"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="764"/>
        <source>Stopwatch</source>
        <translation>Stoppuhr</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="781"/>
        <source>Sessions</source>
        <translation>Sitzungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="168"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="790"/>
        <source>File Transmission</source>
        <translation>Dateiübertragung</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="175"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>AI Assistant</source>
        <translation>Ki-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="343"/>
        <source>Show "%1"</source>
        <translation>„%1" Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="348"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Alle Ausgeblendeten Arbeitsbereiche Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="372"/>
        <source>No Workspaces Available</source>
        <translation>Keine Arbeitsbereiche Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="497"/>
        <source>Manage Plugins…</source>
        <translation>Plugins Verwalten…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="588"/>
        <source>Export</source>
        <translation>Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="619"/>
        <source>CSV File</source>
        <translation>CSV-datei</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="625"/>
        <source>MDF4 File</source>
        <translation>MDF4-datei</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="631"/>
        <source>Console Transcript</source>
        <translation>Konsolenprotokoll</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="640"/>
        <source>Session Database</source>
        <translation>Sitzungsdatenbank</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="654"/>
        <source>No Export Formats Available</source>
        <translation>Keine Exportformate Verfügbar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="119"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="740"/>
        <source>Console</source>
        <translation>Konsole</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="125"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="749"/>
        <source>Notifications</source>
        <translation>Benachrichtigungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="149"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Preferences</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="928"/>
        <source>Edit…</source>
        <translation>Bearbeiten…</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="874"/>
        <source>Resume</source>
        <translation>Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Disconnect</source>
        <translation>Verbindung Trennen</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Stoppen</translation>
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
        <translation>Runde</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Zurücksetzen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Gesamt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Keine Runden aufgezeichnet</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Runde drücken, während die Stoppuhr läuft</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>Keine Daten Verfügbar</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Datensatz-werte</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Suchen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Gruppe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Datensatz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Einheiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(virtuell)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Zugriffscode %1 in Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Datensatz-Zugriffscode kopiert</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>Keine Datensätze in diesem Projekt definiert.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Keine Datensätze entsprechen Ihrer Suche.</translation>
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
        <translation>Wert</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Benutzerdefiniertes Symbol)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="806"/>
        <source>No</source>
        <translation>Nein</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="806"/>
        <source>Yes</source>
        <translation>Ja</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Startmenü</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Menü</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Suchen…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Konsole</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Benachrichtigungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Uhr</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Stoppuhr</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>Ki-assistent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Fortsetzen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Verbunden mit %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Nicht verbunden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>MQTT-publisher</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>Verbunden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>Getrennt</translation>
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
        <translation>Gesendete Nachrichten:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>MQTT-einstellungen Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Dateiübertragung</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">Widgets suchen…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Aus Arbeitsbereich entfernen</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Alles auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Daten an Gerät senden</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Zeitstempel Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Echo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>VT-100 Emulieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Ansi-farben</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Anzeige: %1</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>Wartet auf Genehmigung</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Fehler</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Abgelehnt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Blockiert</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>Läuft</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Genehmigen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Ablehnen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Alles Kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Alles Auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Ergebnis</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Projekt-editor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Projekt-Editor öffnen, um JSON-Layout zu erstellen oder zu ändern</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>CSV-Datei abspielen, als wären es Live-Sensordaten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Anwendungseinstellungen und -präferenzen öffnen</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>CSV Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>MDF4 Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Sitzungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Aufgezeichnete Sitzungen durchsuchen, wiedergeben und exportieren</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">Tastenkombinationen</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">Tastenkombination für das aktuelle Projekt erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Erweiterungen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Erweiterungen durchsuchen und installieren</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">MQTT-Verbindung konfigurieren (Veröffentlichen oder Abonnieren)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Bereitstellen</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">Operator-Bereitstellung für das aktuelle Projekt erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Serielle Port-Kommunikation (UART) auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Audio-Eingangsgerät auswählen (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Raw-USB-Kommunikation auswählen (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Netzwerk</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>TCP/UDP-Netzwerkkommunikation auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>MODBUS-Kommunikation auswählen (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>HID-Gerätekommunikation auswählen (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Bluetooth Low Energy-Kommunikation auswählen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>CAN Bus-Kommunikation auswählen (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Prozess</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Prozesspipe-Kommunikation auswählen (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Beispiele</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Beispielprojekte durchsuchen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Hilfecenter</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Dokumentation, FAQ und Wiki durchsuchen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Detaillierte Dokumentation ansehen und Fragen auf DeepWiki stellen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Verbindung zum konfigurierten Gerät herstellen oder trennen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>Über</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Projekt Öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Ein vorhandenes JSON-Projekt öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Eine MDF4-Datei abspielen, als wären es Live-Sensordaten (Pro)</translation>
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
        <translation>Mit einer KI chatten, um Ihr Projekt zu erstellen und zu bearbeiten</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Operator-App für das aktuelle Projekt erstellen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Anwendungsinformationen und Lizenzdetails anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>Ki-wiki &amp; Chat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Lizenz verwalten und Serial Studio Pro aktivieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Trennen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Verbinden</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">Verbindung zu Gerät oder MQTT-Broker herstellen oder trennen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Aktivieren</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Trigger-einstellungen</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Hält die Wellenform stationär, indem jeder Sweep an einem Trigger-Ereignis ausgerichtet wird.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Fixiert ein sich wiederholendes Signal an Ort und Stelle, wie die Auto-Taste eines Oszilloskops. Jeder Durchlauf beginnt am selben Punkt der Wellenform, sodass sie stillsteht, anstatt vorbeizuscrollen.</translation>
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
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Einzel</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto läuft frei, wenn nichts den Pegel kreuzt.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal wartet auf eine Kreuzung.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single erfasst einen Durchlauf und stoppt dann.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="238"/>
        <source>Slope:</source>
        <translation>Flanke:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="270"/>
        <source>Trigger on a downward crossing</source>
        <translation>Trigger bei einer abfallenden Flanke</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="316"/>
        <source>Timebase:</source>
        <translation>Zeitbasis:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="381"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Zeitbasis leer lassen, um den Zeitbereich des Plots zu verwenden; verringern, um ein schnelles Signal zu vergrößern. Holdoff ignoriert neue Trigger für einen Moment nach jedem Trigger.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Signal:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Zu kreuzender Wert</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Flanke:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="251"/>
        <source>Rising</source>
        <translation>Steigend</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="255"/>
        <source>Trigger on an upward crossing</source>
        <translation>Trigger bei einer ansteigenden Flanke</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="266"/>
        <source>Falling</source>
        <translation>Fallend</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Ein neuer Durchlauf beginnt jedes Mal, wenn das Signal den Pegel in der gewählten Richtung kreuzt. Auto läuft auch frei, wenn keine Kreuzung gefunden wird.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="289"/>
        <source>Timing</source>
        <translation>Timing</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Zeitbasis (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="329"/>
        <source>Match time range</source>
        <translation>Zeitbereich anpassen</translation>
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
        <translation>Haltezeit:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="360"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">Die Zeitbasis legt fest, wie viel Zeit ein Durchlauf zeigt; leer lassen, um den Zeitbereich des Plots zu verwenden. Verringern, um ein schnelles Signal zu vergrößern. Holdoff ignoriert neue Trigger für einen Moment nach jedem Trigger.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="396"/>
        <source>Capture Next</source>
        <translation>Nächsten Erfassen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="398"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Für eine weitere Einzelaufnahme scharf schalten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Pegel:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Triggerpegel</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Haltezeit (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Haltezeit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Quelle:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Neu Scharf Schalten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="411"/>
        <source>Close</source>
        <translation>Schließen</translation>
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
        <translation>Baudrate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="174"/>
        <source>Data Bits</source>
        <translation>Datenbits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="195"/>
        <source>Parity</source>
        <translation>Parität</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="216"/>
        <source>Stop Bits</source>
        <translation>Stoppbits</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="237"/>
        <source>Flow Control</source>
        <translation>Flusskontrolle</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="269"/>
        <source>Auto Reconnect</source>
        <translation>Automatisch Wiederverbinden</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="287"/>
        <source>Send DTR Signal</source>
        <translation>Dtr-signal Senden</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1246"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1989"/>
        <source>Console</source>
        <translation>Konsole</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1333"/>
        <location filename="../../src/UI/Dashboard.cpp" line="2001"/>
        <source>Notifications</source>
        <translation>Benachrichtigungen</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1419"/>
        <location filename="../../src/UI/Dashboard.cpp" line="2013"/>
        <source>Clock</source>
        <translation>Uhr</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1504"/>
        <location filename="../../src/UI/Dashboard.cpp" line="2024"/>
        <source>Stopwatch</source>
        <translation>Stoppuhr</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2081"/>
        <location filename="../../src/UI/Dashboard.cpp" line="2097"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Fallback)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2123"/>
        <source>LED Panel (%1)</source>
        <translation>LED-panel (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="148"/>
        <source>Invalid</source>
        <translation>Ungültig</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1098"/>
        <source>Select Background Image</source>
        <translation>Hintergrundbild Auswählen</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1100"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Bilder (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>USB-gerät</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Übertragungsmodus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Bulk-stream</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Erweitert (Bulk + Control)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Isochron</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Verbindung zu USB-Geräten über Bulk-, Control- oder isochrone Übertragungen. Geeignet für Datenlogger, Geräte mit benutzerdefinierter Firmware und USB-Instrumente.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>USB-Spezifikationen (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>In-endpunkt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>Out-endpunkt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>Maximale Paketgröße</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Control-transfers Aktiviert</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>Das Senden falscher Control-Anfragen kann angeschlossene Hardware zum Absturz bringen oder beschädigen. Mit Vorsicht verwenden.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Über USB-Control-Transfers Informieren</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>Die Paketgröße sollte der vom Endpunkt gemeldeten maximalen Transfergröße entsprechen. Typische Werte: 192 B (FS Audio), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Update jetzt herunterladen?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Update jetzt herunterladen?&lt;br /&gt;Dies ist ein obligatorisches Update. Ein Abbruch schließt die Anwendung.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Änderungsprotokoll:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>Version %1 von %2 wurde veröffentlicht!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Momentan sind keine Updates verfügbar</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Glückwunsch! Die neueste Version von %1 wird ausgeführt</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="166"/>
        <source>Add Register</source>
        <translation>Register Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add register</source>
        <translation>Register hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="176"/>
        <source>Insert Constant</source>
        <translation>Konstante Einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert constant</source>
        <translation>Konstante einfügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="186"/>
        <source>Import</source>
        <translation>Importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import registers from CSV</source>
        <translation>Register aus CSV importieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="196"/>
        <source>Export</source>
        <translation>Exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export registers to CSV</source>
        <translation>Register nach CSV exportieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="211"/>
        <source>Rename</source>
        <translation>Umbenennen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename table</source>
        <translation>Tabelle umbenennen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="221"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete table</source>
        <translation>Tabelle löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="238"/>
        <source>Help</source>
        <translation>Hilfe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="243"/>
        <source>Open help documentation for shared memory</source>
        <translation>Hilfedokumentation für Shared Memory öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="283"/>
        <source>Permissions</source>
        <translation>Berechtigungen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="284"/>
        <source>Register Name</source>
        <translation>Registername</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="285"/>
        <source>Default Value</source>
        <translation>Standardwert</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read-Only</source>
        <translation>Nur Lesen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read/Write</source>
        <translation>Lesen/schreiben</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="462"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Zugriffscode %1 in Zwischenablage kopieren</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="495"/>
        <source>Delete register</source>
        <translation>Register löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="512"/>
        <source>No registers.</source>
        <translation>Keine Register.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="562"/>
        <source>Register access code copied</source>
        <translation>Register-Zugriffscode kopiert</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="232"/>
        <source>Show Colorbar</source>
        <translation>Farbskala Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="245"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Achsen &amp; Raster Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="258"/>
        <source>Show Crosshair</source>
        <translation>Fadenkreuz Anzeigen</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Pause</source>
        <translation>Pausieren</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Resume</source>
        <translation>Fortsetzen</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">Verlauf Löschen</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Willkommen bei %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio ist ein leistungsstarkes Echtzeit-Visualisierungswerkzeug für Ingenieure, Studenten und Maker.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>Sie können eine voll funktionsfähige 14-Tage-Testversion starten, diese mit Ihrem Lizenzschlüssel aktivieren oder den GPLv3-Quellcode selbst herunterladen und kompilieren.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Der Kauf von Pro unterstützt den Autor direkt und hilft bei der Finanzierung zukünftiger Entwicklung.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Das selbstständige Kompilieren der GPLv3-Version hilft beim Wachstum der Community und fördert technische Beiträge.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Bitte warten…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>Noch %1 Tage in Ihrer Testversion verbleibend.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">Derzeit wird die voll funktionsfähige Testversion von %1 Pro verwendet. Sie ist 14 Tage lang für persönliche, nicht-kommerzielle Nutzung gültig.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>Derzeit wird die voll funktionsfähige Testversion von %1 Pro verwendet. Sie ist 14 Tage lang für persönliche, nicht-kommerzielle Nutzung gültig.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Auf einen kostenpflichtigen Plan upgraden, um Serial Studio Pro weiterhin zu nutzen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Oder den GPLv3-Quellcode kompilieren, um es kostenlos zu verwenden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Um verfügbare Abonnementpläne anzuzeigen, unten auf „Jetzt Upgraden" klicken.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Nicht mehr über die Testversion informieren.
Es ist klar, dass nach Ablauf eine Lizenz erworben oder die GPLv3-Version erstellt werden muss.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>Die %1-Testversion ist abgelaufen.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Der Testzeitraum ist beendet. Um %1 mit allen Pro-Funktionen weiterhin zu nutzen, bitte auf einen kostenpflichtigen Plan upgraden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>Alternativ kann auch die Open-Source-Version unter der GPLv3-Lizenz kompiliert werden.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Vielen Dank für das Testen von %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Jetzt Upgraden</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Aktivieren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>Im eingeschränkten Modus öffnen</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>Fortfahren</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>Testversion Starten</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Aus Arbeitsbereich entfernen</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="335"/>
        <source>Device Disconnected</source>
        <translation>Gerät Getrennt</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="332"/>
        <source>Alarm</source>
        <translation>Alarm</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="333"/>
        <source>critical</source>
        <translation>kritisch</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="333"/>
        <source>warning</source>
        <translation>Warnung</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="337"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Wert %1%2 hat den %3-Bereich betreten (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Wert %1%2 hat den oberen Alarm %3%2 erreicht</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Wert %1%2 hat den unteren Alarm %3%2 erreicht</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="343"/>
        <source>Alarms</source>
        <translation>Alarme</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="158"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="181"/>
        <source>N</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="161"/>
        <source>NE</source>
        <translation>NO</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="164"/>
        <source>E</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="167"/>
        <source>SE</source>
        <translation>SO</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="170"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="173"/>
        <source>SW</source>
        <translation>SW</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="176"/>
        <source>W</source>
        <translation>W</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="179"/>
        <source>NW</source>
        <translation>NW</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="128"/>
        <source>Title</source>
        <translation>Titel</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="129"/>
        <source>Value</source>
        <translation>Wert</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">Warte auf Daten…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Satellite Imagery</source>
        <translation>Satellitenbilder</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Satellitenbilder mit Beschriftung</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Street Map</source>
        <translation>Straßenkarte</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Topographic Map</source>
        <translation>Topografische Karte</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Terrain</source>
        <translation>Gelände</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Light Gray Canvas</source>
        <translation>Hellgraue Leinwand</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="97"/>
        <source>Dark Gray Canvas</source>
        <translation>Dunkelgraue Leinwand</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="97"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="410"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>Zusätzliche Kartenebenen sind nur für Pro-Benutzer verfügbar.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="411"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Uneingeschränkter Zugriff ist nicht möglich, da der ArcGIS-API-Schlüssel echte Kosten verursacht.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="102"/>
        <source>Time (s)</source>
        <translation>Zeit (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="102"/>
        <source>Samples</source>
        <translation>Samples</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="175"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Übertragungsskript nach %1 ms abgelaufen</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="194"/>
        <source>Payload exceeds maximum size</source>
        <translation>Nutzdaten überschreiten maximale Größe</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="87"/>
        <source>Time (s)</source>
        <translation>Zeit (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="115"/>
        <source>Samples</source>
        <translation>Abtastwerte</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="1046"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Rasterintervall: %1 Einheit(en)</translation>
    </message>
</context>
<context>
    <name>Widgets::Waterfall</name>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="412"/>
        <source>Viridis</source>
        <translation>Viridis</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="414"/>
        <source>Inferno</source>
        <translation>Inferno</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="416"/>
        <source>Magma</source>
        <translation>Magma</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="418"/>
        <source>Plasma</source>
        <translation>Plasma</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="420"/>
        <source>Turbo</source>
        <translation>Turbo</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="422"/>
        <source>Jet</source>
        <translation>Jet</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="424"/>
        <source>Hot</source>
        <translation>Hot</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="426"/>
        <source>Grayscale</source>
        <translation>Graustufen</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="428"/>
        <source>Unknown</source>
        <translation>Unbekannt</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Arbeitsbereich Bearbeiten</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Neuer Arbeitsbereich</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Name:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>Mein Arbeitsbereich</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Einzuschließende Widgets auswählen:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Widgets filtern…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
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
        <translation>Arbeitsbereich</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>Widget Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>Widget zum Arbeitsbereich hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>Umbenennen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>Arbeitsbereich umbenennen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>Arbeitsbereich löschen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>Gruppe</translation>
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
        <translation>(unbekannt)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(Gruppen-Widget)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>Widget aus Arbeitsbereich entfernen</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">Aus Arbeitsbereich entfernen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>Keine Widgets in diesem Arbeitsbereich.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Arbeitsbereiche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>Anpassen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>Arbeitsbereiche manuell bearbeiten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>Nach Oben</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>Ausgewählten Arbeitsbereich nach oben verschieben</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>Nach Unten</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>Ausgewählten Arbeitsbereich nach unten verschieben</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>Arbeitsbereich Hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>Arbeitsbereich hinzufügen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>Bereinigen</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>%1 Widget-Referenz(en) entfernen, deren Zielgruppe oder Datensatz nicht mehr existiert</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>Keine veralteten Widget-Referenzen in den Arbeitsbereichen</translation>
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
        <translation>Keine Arbeitsbereiche. Fügen Sie einen über die Symbolleiste hinzu oder setzen Sie auf das automatische Layout zurück.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Projekt hat keine geeigneten Gruppen – fügen Sie eine Gruppe mit Widgets hinzu, um Arbeitsbereiche zu füllen.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>Auf Automatisches Layout Zurücksetzen</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">Keine Arbeitsbereiche.</translation>
    </message>
</context>
</TS>