<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="it_IT" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Errore Anthropic</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>Errore di parsing dello stream: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>Chiave API non valida (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>Limite di frequenza raggiunto: %1</translation>
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
        <translation>Consentire il Controllo del Dispositivo tramite AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="219"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>Questo consente all'assistente AI di configurare i dispositivi, aprire e chiudere le connessioni e inviare dati all'hardware.

Ogni azione sul dispositivo richiede comunque l'approvazione esplicita per ogni chiamata nella chat, anche quando l'approvazione automatica è abilitata. Abilitare solo se si considera affidabile il provider AI configurato per l'accesso all'hardware.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="417"/>
        <source>Switch AI provider?</source>
        <translation>Cambiare provider AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="418"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Il passaggio a un provider diverso cancella la conversazione corrente. Continuare?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="421"/>
        <source>Assistant</source>
        <translation>Assistente</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="458"/>
        <source>AI Assistant is not available in this build</source>
        <translation>L'Assistente AI non è disponibile in questa build</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">L'Assistente AI richiede una licenza Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="463"/>
        <source>Set an API key first</source>
        <translation>Impostare prima una chiave API</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">L'Assistente AI richiede una licenza Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="169"/>
        <source>AI Assistant is not available in this build</source>
        <translation>L'Assistente AI non è disponibile in questa build</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="175"/>
        <source>AI subsystem not initialized</source>
        <translation>Sottosistema AI non inizializzato</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="181"/>
        <source>Already busy with a previous request</source>
        <translation>Già occupato con una richiesta precedente</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="495"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Budget per le chiamate agli strumenti raggiunto per questo turno; nessun altro strumento verrà eseguito.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1093"/>
        <source>Waiting for %1 to respond. Loading the model and processing the prompt can take a while on local hardware...</source>
        <translation>In attesa di risposta da %1. Il caricamento del modello e l'elaborazione del prompt possono richiedere tempo su hardware locale...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1952"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>Hai raggiunto il budget per le chiamate agli strumenti per questo turno. Non richiedere altri strumenti. Riassumi ciò che hai trovato finora e, se l'attività è incompleta, indica quali passaggi rimangono in modo che l'utente possa dirti di continuare.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Budget di chiamate strumento superato</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="940"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Il modello ha restituito una risposta vuota. Provare a riformulare, passare a un modello diverso o verificare che la richiesta sia consentita dai filtri di sicurezza del provider.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1097"/>
        <source>Sending request to %1...</source>
        <translation>Invio richiesta a %1...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1110"/>
        <source>Provider returned no reply</source>
        <translation>Il provider non ha restituito risposta</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="146"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Prompt bloccato dal filtro di sicurezza Gemini: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="200"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini interrotto senza produrre una risposta: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="262"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="265"/>
        <source>Invalid API key (%1)</source>
        <translation>Chiave API non valida (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="267"/>
        <source>Rate limited: %1</source>
        <translation>Limite di frequenza raggiunto: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="269"/>
        <source>Invalid API key</source>
        <translation>Chiave API non valida</translation>
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
        <translation>Chiave API non valida (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="431"/>
        <source>Rate limited: %1</source>
        <translation>Limite di frequenza raggiunto: %1</translation>
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
        <translation>Esporta File Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>Impossibile avviare il server GRPC</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>Impossibile associare a %1</translation>
    </message>
</context>
<context>
    <name>API::ProcessLauncher</name>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="82"/>
        <source>No program specified</source>
        <translation>Nessun programma specificato</translation>
    </message>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="88"/>
        <source>Program "%1" not found in PATH</source>
        <translation>Programma "%1" non trovato in PATH</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>Impossibile avviare il server TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>Consentire Connessioni API Esterne?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Esporre il server API a host esterni consente ad altri dispositivi sulla rete di connettersi a Serial Studio sulla porta 7777.

Abilitare solo su reti attendibili. Client non attendibili potrebbero leggere dati in tempo reale o inviare comandi al dispositivo.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>Impossibile riavviare il server TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>Consentire il controllo del dispositivo tramite API?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Un programma che utilizza l'API locale di Serial Studio sta richiedendo di inviare dati al dispositivo connesso. Consentire ai client API di scrivere sul dispositivo?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>API server</source>
        <translation>Server API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>Invalid pending connection</source>
        <translation>Connessione in sospeso non valida</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>Informazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>Versione %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Copyright © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>Tutti i Diritti Riservati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 è software libero: puoi ridistribuirlo e/o modificarlo secondo i termini della GNU General Public License pubblicata dalla Free Software Foundation; versione 3 della Licenza o (a tua scelta) qualsiasi versione successiva.

%1 è distribuito nella speranza che sia utile, ma SENZA ALCUNA GARANZIA; senza nemmeno la garanzia implicita di COMMERCIABILITÀ o IDONEITÀ PER UN PARTICOLARE SCOPO. Consulta la GNU General Public License per maggiori dettagli.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Questa configurazione è concessa in licenza per uso commerciale e proprietario. Può essere utilizzata in applicazioni closed-source e commerciali, secondo i termini della licenza commerciale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Questa configurazione è solo per uso personale e di valutazione. L'uso commerciale è vietato a meno che non sia attivata una licenza commerciale valida.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>Questo software è fornito 'così com'è' senza garanzie di alcun tipo, esplicite o implicite, incluse ma non limitate alle garanzie di commerciabilità o idoneità per un particolare scopo. In nessun caso l'autore sarà responsabile per danni derivanti dall'uso di questo software.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Gestisci Licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Dona</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Verifica Aggiornamenti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>Novità</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">Suggerimenti e Trucchi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>Contratto di Licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>Segnala Bug</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>Ringraziamenti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>Sito Web</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="186"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="187"/>
        <source>Settings</source>
        <translation>Impostazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="245"/>
        <source>G-FORCE</source>
        <translation>FORZA-G</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="283"/>
        <source>PITCH ↕</source>
        <translation>BECCHEGGIO ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="320"/>
        <source>ROLL ↔</source>
        <translation>ROLLIO ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>Configurazione Accelerometro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Configura l'intervallo di visualizzazione e le unità di misura dell'accelerometro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Intervallo di Visualizzazione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>G Massimi:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Inserire valore G massimo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Configurazione Ingresso</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Ingresso già in G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="34"/>
        <source>Acknowledgements</source>
        <translation>Riconoscimenti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="75"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="86"/>
        <source>About Qt…</source>
        <translation>Informazioni su QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Cambia Icona</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Cambia l'icona utilizzata per questa azione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Duplica questa azione con tutte le sue impostazioni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Elimina questa azione dal progetto</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Aggiungi Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Widget Disponibili</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Fare clic su una riga per aggiungerla all'area di lavoro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Cerca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(intero gruppo)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Già nell'area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Aggiungi all'area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Nessun widget disponibile.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Nessun widget corrispondente.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 di %2 widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Bande Allarme</translation>
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
        <translation>Avviso</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>Critico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>Tachimetro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>Minimo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>Funzionamento</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>Attenzione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>Zona Rossa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>Tachimetro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>Crociera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>Veloce</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>Velocità Massima</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>Temperatura del Motore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>Freddo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>Normale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>Caldo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>Surriscaldamento</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>Pressione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>Vuoto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>Alto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>Scoppio</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>Tensione Batteria</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>Basso</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>Nominale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>Livello Carburante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>Vuoto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>Riserva</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>Intensità Segnale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>Nessun Segnale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>Debole</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>Buono</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>Carico CPU / Sistema</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>Occupato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>Sovraccarico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Avviso / Critico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>Indicatore (On / Off)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>On</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>Indicatore di Guasto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>Guasto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>Scegli Colore Banda</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>Preset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>Preset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>Scegli preset…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>Lampeggio</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>Ripristina al predefinito di gravità</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Clicca per scegliere un colore. Clicca col tasto destro per ripristinare al predefinito di gravità.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>Clicca per scegliere un colore personalizzato.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>Fa lampeggiare il LED mentre il valore si trova in questa banda.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Nessuna banda definita. Scegli un preset sopra o aggiungi una banda per iniziare.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>Applica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>Applica le modifiche al dataset.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Applica Preset</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Sostituisce le bande correnti con il preset selezionato, scalato all'intervallo di questo dataset.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>Intervallo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>Bande</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>Aggiungi Banda</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Aggiunge una nuova banda in continuazione dall'ultima.</translation>
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
        <translation>Gravità</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>Colore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>Etichetta</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Scegli un colore personalizzato.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">automatico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(facoltativo)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>Sposta su.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>Sposta giù.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>Rimuovi questa banda.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Nessuna banda definita. Applica un preset o aggiungi una banda per iniziare.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>Anteprima</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>Scarta modifiche.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Salva</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Salva le modifiche al dataset.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>Assistente</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="79"/>
        <source>New chat</source>
        <translation>Nuova chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>Toggle chat list</source>
        <translation>Mostra/nascondi elenco chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="238"/>
        <source>Settings</source>
        <translation>Impostazioni</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="274"/>
        <source>Manage API keys…</source>
        <translation>Gestisci chiavi API…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="341"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>Esportazione CSV vs MDF4 - qual è la differenza?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="344"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Bar e Gauge - quando usare ciascuno?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="416"/>
        <source>How can I help with your project?</source>
        <translation>Come posso aiutarti con il tuo progetto?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="417"/>
        <source>Set up your API key to get started</source>
        <translation>Configura la tua chiave API per iniziare</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="429"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Descrivi cosa vorresti costruire e configurerò sorgenti, gruppi, dataset, frame parser e trasformazioni per te.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>Per iniziare a chattare, incolla una chiave API per il provider selezionato. Le chiavi sono crittografate su questa macchina e non lasciano mai il tuo computer se non per comunicare con il provider che scegli.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="453"/>
        <source>Open API Key Setup</source>
        <translation>Apri Configurazione Chiave API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="463"/>
        <source>Get a key from %1</source>
        <translation>Ottieni una chiave da %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="337"/>
        <source>List the sources in this project</source>
        <translation>Elenca le sorgenti in questo progetto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="334"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Aiutami a scoprire le funzionalità di Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="335"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Cosa può fare questa app per la mia telemetria?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="336"/>
        <source>Walk me through what this project already contains</source>
        <translation>Guidami attraverso ciò che questo progetto contiene già</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="340"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Cos'è un database di sessione e perché dovrei usarne uno?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">Esportazione CSV vs MDF4 — qual è la differenza?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="342"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Cos'è un parser di frame e quando ne ho bisogno?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="343"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>Quando dovrei usare Lua vs JavaScript per il parser?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Plot, Bar e Gauge — quando usare ciascuno?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="345"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Qual è la differenza tra una trasformazione e un parser di frame?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="348"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Aggiungi una sorgente UART per un Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="349"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Configura un progetto IMU da zero</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="350"/>
        <source>Configure an MQTT subscriber</source>
        <translation>Configura un sottoscrittore MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="351"/>
        <source>Add a CAN bus source</source>
        <translation>Aggiungi una sorgente bus CAN</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="352"/>
        <source>Set up a Modbus poller</source>
        <translation>Configura un poller Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="353"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Aggiungi una sorgente di rete (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="354"/>
        <source>Write a CSV frame parser for me</source>
        <translation>Scrivi un frame parser CSV per me</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="355"/>
        <source>Help me parse a JSON frame</source>
        <translation>Aiutami ad analizzare una trama JSON</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="356"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>Aggiungi una trasformata di smoothing EMA a un dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="357"/>
        <source>Decode hexadecimal frames</source>
        <translation>Decodifica trame esadecimali</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="358"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Calibra un sensore con una trasformata lineare</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="361"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Suggerisci widget per la dashboard per i miei dati</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="362"/>
        <source>Build an executive overview workspace</source>
        <translation>Costruisci uno spazio di lavoro panoramica esecutiva</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="363"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Aggiungi un widget painter per una visualizzazione personalizzata</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="364"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Mostra Grafico, FFT e Waterfall per un Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="365"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Raggruppa i Miei Dataset in Workspace Utili</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="644"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>Trascina file o cartelle per consentire all'assistente di leggerli</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="689"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>Cartella "%1" aggiunta - leggibile in questa sessione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="690"/>
        <source>Added "%1" - readable this session</source>
        <translation>"%1" aggiunto - leggibile in questa sessione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="774"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Chiedi qualsiasi cosa a Serial Studio…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Clear conversation</source>
        <translation>Cancella conversazione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="838"/>
        <source>Stop generating</source>
        <translation>Interrompi generazione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="839"/>
        <source>Send message (Enter)</source>
        <translation>Invia messaggio (Invio)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="159"/>
        <source>Provider</source>
        <translation>Provider</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="192"/>
        <source>Model selection</source>
        <translation>Selezione modello</translation>
    </message>
    <message>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation type="vanished">Esegui azioni di modifica senza chiedere ogni volta. Le azioni bloccate restano bloccate.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Auto-approve edits</source>
        <translation>Approva automaticamente le modifiche</translation>
    </message>
    <message>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation type="vanished">Consente all'AI di configurare i dispositivi, connettere/disconnettere e inviare dati. Ogni azione richiede comunque l'approvazione.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="259"/>
        <source>Allow device control</source>
        <translation>Consenti controllo dispositivo</translation>
    </message>
    <message>
        <source>Manage API keys</source>
        <translation type="vanished">Gestisci chiavi API</translation>
    </message>
    <message>
        <source>Working</source>
        <translation type="vanished">In Elaborazione</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation type="vanished">Pronto</translation>
    </message>
    <message>
        <source>  •  cache %1k tok</source>
        <translation type="vanished">•  cache %1k tok</translation>
    </message>
    <message>
        <source>  •  cache write %1k tok</source>
        <translation type="vanished">scrittura cache %1k tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Nessun Microfono Rilevato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Collega un microfono o controlla le impostazioni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Dispositivo di Ingresso</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Frequenza di Campionamento</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Formato Campione</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Canali</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Dispositivo di Uscita</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">Dialogo</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">Fornire nome utente e password per la posizione di download.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">&amp;Nome utente:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Password:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="65"/>
        <source>Axis Range Configuration</source>
        <translation>Configurazione Intervallo Assi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="179"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Configura l'intervallo visibile per gli assi del grafico. I valori si aggiornano in tempo reale durante la digitazione.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="187"/>
        <source>X Axis</source>
        <translation>Asse X</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="212"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="283"/>
        <source>Minimum:</source>
        <translation>Minimo:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="224"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="295"/>
        <source>Enter min value</source>
        <translation>Inserire valore minimo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="233"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="304"/>
        <source>Maximum:</source>
        <translation>Massimo:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="245"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="316"/>
        <source>Enter max value</source>
        <translation>Inserire valore massimo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="260"/>
        <source>Y Axis</source>
        <translation>Asse Y</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="335"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="345"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Recupera Backup</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Senza Titolo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Progetto Caricato</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Salvataggio Automatico</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Prima del Ripristino</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Prima di Eliminare il Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Prima di Eliminare il Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Prima di Nuovo Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Prima di Aprire Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Prima di Caricare JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Prima di Applicare Template</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Prima di Eliminare Azione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Prima di Eliminare Widget di Output</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Prima di Spostare Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Prima di Spostare Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Prima di Eliminare Spazio di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Prima di Cancellare Tutti gli Spazi di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Prima di Rimuovere Widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Prima di Riordinare Spazi di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Prima di Operazione Batch</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Prima di Aggiungere Riquadro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (e altri %2)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>Anche il codice del frame parser differisce e verrà sostituito.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Titolo cambia da "%1" a "%2". Struttura del gruppo invariata.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Stessi gruppi e dataset, ma il codice del frame parser differisce — il ripristino lo sostituirà.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Stessi gruppi e dataset del progetto corrente. Il ripristino potrebbe comunque annullare modifiche a livello di campo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Il ripristino rimuove %1 e ripristina %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Il ripristino rimuove %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Il ripristino ripristina %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Scegli un backup da ripristinare. Il progetto corrente viene salvato automaticamente prima, quindi il ripristino è reversibile.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Nessun backup per questo progetto. Modifica o salva il progetto per avviare il backup incrementale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Apri Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Ripristina</translation>
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
        <translation>%1 frame/s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="83"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="88"/>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>Superato</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>Fallito</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>Benchmark Hotpath</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Misura la velocità con cui questo computer può estrarre, analizzare e visualizzare i frame attraverso la pipeline dati di Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>L'interfaccia si bloccherà durante l'esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Ogni fase viene eseguita a pieno regime sul thread principale, quindi la finestra smette di rispondere fino al termine. Il progetto corrente viene ricaricato automaticamente al termine del benchmark.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>Frame per fase:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>Durata minima:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>Fasi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>Parser</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>Esportazione dati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>Dati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>Solo numerici</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>Misto (numerico + testo)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>Selezionare almeno una fase e un tipo di dati per eseguire un benchmark.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>Esecuzione Di %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>Preparazione...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>Throughput</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>Tempo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>Risultato</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>Eseguire un test per visualizzare i risultati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>Pass/Fail si applica alle fasi di pipeline dati e parser (pipeline dati e Parser integrato numerico 1024 K frame/s; Parser integrato misto 512 K; Lua numerico 256 K; JavaScript numerico e Lua misto 128 K; JavaScript misto 64 K). Le fasi di esportazione e dashboard sono informative.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail si applica solo alle fasi di data-pipeline e parser (data pipeline e numerico integrato 1024 K frame/s; misto integrato 512 K; Lua numerico 256 K; JavaScript numerico e Lua misto 128 K; JavaScript misto 64 K). Le fasi di esportazione e dashboard sono informative.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail si applica alle fasi di pipeline dati e parser (pipeline dati e Native numerico 1024 K frame/s; Native misto 512 K; Lua numerico 256 K; JavaScript numerico e Lua misto 128 K; JavaScript misto 64 K). Le fasi di esportazione e dashboard sono informative.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail si applica solo alle fasi di data-pipeline e parser (data pipeline 1024 K frame/s; numerico: Lua 256 K, JavaScript 128 K; misto: Lua 128 K, JavaScript 64 K). Le fasi di esportazione e dashboard sono informative.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail si applica solo alle fasi del parser (obiettivo Lua 256 K frame/s, JavaScript 128 K). Le fasi di esportazione e dashboard sono informative.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>In Esecuzione...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>Esegui Benchmark</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>Pipeline dati</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">Parser Lua</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">Parser JavaScript</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + esportazione dati</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + dashboard</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Parser nativo (numerico)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Parser nativo (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Parser integrato (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Parser integrato (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Parser Lua (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>Parser JavaScript (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Parser Lua (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>Parser JavaScript (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Parser integrato + esportazione dati (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + esportazione dati (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + esportazione dati (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Parser integrato + esportazione dati (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + esportazione dati (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + esportazione dati (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Parser integrato + dashboard (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + dashboard (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 K frame</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 K frame</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 K frame</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 M frame</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>1 secondo</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>2 secondi</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 secondi</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 secondi</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - Benchmark Hotpath</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), carico di lavoro: %3 frame minimo, %4 s minimo</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>Throughput</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>Obiettivo</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>Tempo</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>Risultato</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 frame/s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>n/d</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>Superato</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>Fallito</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + dashboard (numerico)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>Integrato + dashboard (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + dashboard (misto)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + dashboard (misto)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Visualizzazione dashboard...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Servizio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Caratteristica</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Scansione…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Nessun Adattatore Bluetooth Rilevato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Collega un adattatore Bluetooth o verifica le impostazioni di sistema</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Questo Sistema Operativo non è Ancora Supportato.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio verrà aggiornato per funzionare con questo sistema operativo non appena QT lo supporterà ufficialmente</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Nessun Driver CAN Trovato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>Installa i driver hardware CAN per il tuo sistema</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>Driver CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="140"/>
        <source>Interface</source>
        <translation>Interfaccia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="169"/>
        <source>Bitrate</source>
        <translation>Bitrate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="223"/>
        <source>Flexible Data-Rate</source>
        <translation>Flexible Data-rate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="243"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="263"/>
        <source>Listen-Only</source>
        <translation>Solo Ascolto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="283"/>
        <source>DBC Database</source>
        <translation>Database DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="287"/>
        <source>Import DBC File…</source>
        <translation>Importa File DBC…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="320"/>
        <source>No CAN Interfaces Found</source>
        <translation>Nessuna Interfaccia CAN Trovata</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>Seleziona file CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>File CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>Connessione Dispositivo Attiva</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Per utilizzare questa funzione, è necessario disconnettersi dal dispositivo. Procedere?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>Verificare i permessi e la posizione del file</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Dati Insufficienti nel File CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>Impossibile leggere il file CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>Il file CSV deve contenere almeno una riga di dati per procedere. Verificare il file e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>CSV Non Valido</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>Il file CSV non contiene dati o intestazioni.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>Selezionare una colonna data/ora</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>Imposta intervallo manualmente</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>Selezione Data/ora CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Scegliere come gestire i dati data/ora:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>Imposta Intervallo</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Inserire l'intervallo tra le righe in millisecondi:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>Seleziona Colonna Data/ora</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Selezionare la colonna che contiene i dati di data/ora:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>Selezione Non Valida</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>La colonna selezionata non è valida.</translation>
    </message>
</context>
<context>
    <name>ChatSidebar</name>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="44"/>
        <source>Chats</source>
        <translation>Chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="57"/>
        <location filename="../../qml/AI/ChatSidebar.qml" line="115"/>
        <source>New chat</source>
        <translation>Nuova chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="125"/>
        <source>%1 messages</source>
        <translation>%1 messaggi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="147"/>
        <source>Rename...</source>
        <translation>Rinomina…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="158"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="197"/>
        <source>Rename chat</source>
        <translation>Rinomina chat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="217"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="222"/>
        <source>Rename</source>
        <translation>Rinomina</translation>
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
        <translation>L'Esportazione Console è una funzionalità Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Questa funzionalità richiede una licenza. Acquistarne una per abilitare l'esportazione console.</translation>
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
        <translation>Nessuna Terminazione di Riga</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>Nuova Riga</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>Ritorno Carrello</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>Testo Semplice</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>Esadecimale</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>Nessun Checksum</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>Dispositivo %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Inserisci Costante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>Fondamentale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>Velocità della luce nel vuoto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Costante di Planck</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Carica elementare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Costante di Avogadro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Costante di Boltzmann</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Costante di Stefan-Boltzmann</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>Meccanica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>Accelerazione di gravità standard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Costante gravitazionale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Pressione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Atmosfera standard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Pressione barometrica al livello del mare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>Temperatura</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>Zero assoluto (Celsius)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Punto di congelamento dell'acqua</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Punto di ebollizione dell'acqua (1 atm)</translation>
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
        <translation>Gas e Fluidi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Costante universale dei gas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Costante specifica dei gas (aria secca)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Costante specifica dei gas (vapore acqueo)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Densità dell'aria (livello del mare, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Densità dell'acqua (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Velocità del suono nell'aria (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Rapporto di capacità termica (aria secca)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>Elettromagnetismo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>Permittività del vuoto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Permeabilità del vuoto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Matematica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pi Greco</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Numero di Eulero</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Rapporto aureo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Costanti Fisiche</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>Valori predefiniti in unità SI. Fare clic su una riga per inserirla in %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Cerca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Simbolo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Valore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Categoria</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Nessuna costante corrispondente.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 costanti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 di %2 costanti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script di Controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Loop</source>
        <translation>Ciclo di Controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>Ripeti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>Taglia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>Incolla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>Seleziona Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>Formatta Documento</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>Formatta Selezione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control loop</source>
        <translation>Ripristina il ciclo di controllo predefinito</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control loop file</source>
        <translation>Importa un file di ciclo di controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control loop documentation</source>
        <translation>Apri la documentazione del ciclo di controllo</translation>
    </message>
    <message>
        <source>Reset to the default control script</source>
        <translation type="vanished">Ripristina lo script di controllo predefinito</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>Apri</translation>
    </message>
    <message>
        <source>Import a control script file</source>
        <translation type="vanished">Importa un file di script di controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>Annulla l'ultima modifica del codice</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>Ripristina la modifica precedentemente annullata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>Taglia il codice selezionato negli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>Copia il codice selezionato negli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>Incolla il codice dagli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>Aiuto</translation>
    </message>
    <message>
        <source>Open the control script documentation</source>
        <translation type="vanished">Apri la documentazione dello script di controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>Valida</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Verifica che lo script compili correttamente</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Opzioni di Ripristino</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio si è chiuso inaspettatamente più volte di seguito.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>Arresti anomali consecutivi: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Ultima fase segnalata: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Rilevato a: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Scegli un'azione di ripristino. Serial Studio si chiuderà dopo averla applicata così il prossimo avvio sarà pulito.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Ripristina Backend di Rendering ai Valori Predefiniti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Salta il Ripristino dell'Ultimo Progetto Aperto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Ripristina tutte le Preferenze</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Continua Comunque</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="35"/>
        <source>CSV Player</source>
        <translation>Lettore CSV</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>Anteprima File DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>File DBC: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Esamina i messaggi e i segnali CAN da importare in un nuovo progetto Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Messaggi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Nome Messaggio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>ID CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Segnali</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>Nessun messaggio trovato nel file DBC.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Totale: %1 messaggi, %2 segnali</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Crea Progetto</translation>
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
        <translation>Invia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Nessuna funzione di trasmissione definita</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="326"/>
        <source>Set Wallpaper…</source>
        <translation>Imposta Sfondo…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="332"/>
        <source>Clear Wallpaper</source>
        <translation>Rimuovi Sfondo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="342"/>
        <source>Tile Windows</source>
        <translation>Disponi Finestre</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="361"/>
        <source>Pro features detected in this project.</source>
        <translation>Funzionalità Pro rilevate in questo progetto.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="363"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Widget di fallback attivi. Acquista una licenza per la funzionalità completa.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="504"/>
        <source>Empty Workspace</source>
        <translation>Area di Lavoro Vuota</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="518"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Usa la barra di ricerca per trovare e aggiungere widget, oppure fai clic destro su un widget in un'altra area di lavoro per aggiungerlo qui.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="533"/>
        <source>Search Widgets</source>
        <translation>Cerca Widget</translation>
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
        <translation>Server API Attivo (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="216"/>
        <source>API Server Ready</source>
        <translation>Server API Pronto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="217"/>
        <source>API Server Off</source>
        <translation>Server API Spento</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="155"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="307"/>
        <source>Send</source>
        <translation>Invia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="295"/>
        <source>Enter command…</source>
        <translation>Inserire comando…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Nessuna funzione di trasmissione definita</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Inserire comando…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Invia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Nessuna funzione di trasmissione definita</translation>
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
        <translation>Nessuna funzione di trasmissione definita</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Resume</source>
        <translation>Riprendi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="313"/>
        <source>Awaiting data…</source>
        <translation>In attesa di dati…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="368"/>
        <source>Open %1 in a separate window</source>
        <translation>Apri %1 in una finestra separata</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="289"/>
        <source>Select Javascript file to import</source>
        <translation>Seleziona file Javascript da importare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="368"/>
        <source>Code Validation Failed</source>
        <translation>Validazione Codice Fallita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="358"/>
        <source>Line %1: %2</source>
        <translation>Riga %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="369"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>Lo script deve definire una funzione setup() e/o loop().</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="374"/>
        <source>Code Validation Successful</source>
        <translation>Validazione Codice Riuscita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="375"/>
        <source>No syntax errors detected in the control loop.</source>
        <translation>Nessun errore di sintassi rilevato nel ciclo di controllo.</translation>
    </message>
    <message>
        <source>No syntax errors detected in the control script.</source>
        <translation type="vanished">Nessun errore di sintassi rilevato nello script di controllo.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>Importa File DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>File DBC (*.DBC);;Tutti i File (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>Impossibile analizzare il file DBC: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>Verificare il formato del file e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>Errore Importazione DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>Il file DBC non contiene messaggi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Il file selezionato non contiene definizioni di messaggi CAN.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>Avviso Importazione DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>Panoramica</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>Attivo</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Impossibile creare il file di progetto temporaneo</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">Verificare che l'applicazione disponga dei permessi di scrittura nella directory temporanea.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>File DBC importato con successo con %1 messaggi e %2 segnali.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>L'editor del progetto è ora aperto per la personalizzazione.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Impossibile caricare il progetto importato</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Il JSON del progetto generato non può essere caricato.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>Ignorati %1 segnale/i che utilizzano multiplexing esteso (SG_MUL_VAL_); è supportato solo il multiplexing semplice.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>Importazione DBC Completata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="254"/>
        <source>CAN Bus</source>
        <translation>Bus CAN</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="63"/>
        <source>Dataset Value Transform</source>
        <translation>Trasformazione Valore Dataset</translation>
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
        <translation>Linguaggio:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="132"/>
        <source>Template:</source>
        <translation>Modello:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Inserire valore grezzo (es. 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Test</source>
        <translation>Prova</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="112"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="143"/>
        <source>Input:</source>
        <translation>Ingresso:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="146"/>
        <source>Output:</source>
        <translation>Uscita:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="114"/>
        <source>Apply</source>
        <translation>Applica</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="115"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="224"/>
        <source>Transform — %1</source>
        <translation>Trasformazione — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="304"/>
        <source>The value transform has a syntax error and was not applied.</source>
        <translation>La trasformazione del valore contiene un errore di sintassi e non è stata applicata.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>The value transform must define a transform(value) function.</source>
        <translation>La trasformazione del valore deve definire una funzione transform(value).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="314"/>
        <source>Define a transform(value) function that returns a number, or use Clear to remove the transform.</source>
        <translation>Definire una funzione transform(value) che restituisca un numero, oppure utilizzare Cancella per rimuovere la trasformazione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="332"/>
        <source>Enter a value</source>
        <translation>Inserire un valore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="339"/>
        <source>Invalid number</source>
        <translation>Numero non valido</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="408"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatta Documento	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="409"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatta Selezione	ctrl+i</translation>
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
-- Definire una funzione transform(value) che riceve la lettura
-- del dataset in tempo reale e restituisce un numero trasformato.
-- Se non è definita alcuna funzione transform(), il valore grezzo
-- viene mantenuto e nulla viene salvato con il progetto.
--
-- Esempio:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Le variabili globali dichiarate al livello superiore persistono
-- tra i frame, utile per filtri, accumulatori e qualsiasi altra
-- trasformazione con stato, proprio come uno script parser di frame:
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
-- Usare il menu Template per esempi pronti all'uso, oppure
-- fare clic su Prova per testare la funzione.
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
 * Definisci una funzione transform(value) che riceve la lettura
 * del dataset in tempo reale e restituisce un numero trasformato.
 * Se non è definita alcuna funzione transform(), il valore grezzo
 * viene mantenuto e nulla viene salvato con il progetto.
 *
 * Esempio:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Le variabili globali dichiarate al livello superiore persistono
 * tra i frame, il che è utile per filtri, accumulatori e qualsiasi
 * altra trasformazione con stato -- proprio come uno script di
 * analisi dei frame:
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
 * Usa il menu a tendina Template per esempi pronti all'uso, oppure
 * fai clic su Test per provare la tua funzione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="608"/>
        <source>Failed to create the Lua engine.</source>
        <translation>Impossibile creare il motore Lua.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="646"/>
        <source>Line %1: %2</source>
        <translation>Riga %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="800"/>
        <source>Select Template…</source>
        <translation>Seleziona Modello…</translation>
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
 * Definire una funzione transform(value) che riceve la lettura
 * in tempo reale del dataset e restituisce un numero trasformato.
 * Se non viene definita alcuna funzione transform(), il valore
 * grezzo viene mantenuto e nulla viene salvato con il progetto.
 *
 * Esempio:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Le variabili globali dichiarate al livello superiore persistono
 * tra i frame, utile per filtri, accumulatori e qualsiasi altra
 * trasformazione con stato — proprio come uno script parser di frame:
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
 * Utilizzare il menu a discesa Modello per esempi pronti all'uso,
 * oppure fare clic su Test per provare la funzione.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <source>Engine error</source>
        <translation>Errore del motore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="714"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="727"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="744"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="755"/>
        <source>Error: %1</source>
        <translation>Errore: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="720"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="748"/>
        <source>Error: transform() not defined</source>
        <translation>Errore: transform() non definita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="732"/>
        <source>Error: transform() must return a number</source>
        <translation>Errore: transform() deve restituire un numero</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1705"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1819"/>
        <source>Channel %1</source>
        <translation>Canale %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1830"/>
        <source>Audio Input</source>
        <translation>Ingresso Audio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1714"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1835"/>
        <source>Quick Plot</source>
        <translation>Grafico Rapido</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1429"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>Trasformazione JavaScript ha superato il budget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1430"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>Una trasformazione di dataset ha impiegato più di %1 ms; i dataset rimanenti nella frame sono stati riportati ai valori grezzi fino alla frame successiva. Profila o semplifica il codice di trasformazione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="230"/>
        <source>Frame pool exhausted</source>
        <translation>Pool di Frame esaurito</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="232"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>Un consumatore a valle (dashboard, esportazione CSV/MDF4, session DB o sottoscrittore API) non sta svuotando le frame abbastanza velocemente. Serial Studio sta passando ad allocazioni per frame fino a quando l’arretrato non si libera. Disabilita un consumatore pesante o riduci la velocità dei dati.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1666"/>
        <source>Device A</source>
        <translation>Dispositivo A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1721"/>
        <source>Quick Plot Data</source>
        <translation>Dati Grafico Rapido</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1733"/>
        <source>Multiple Plots</source>
        <translation>Grafici Multipli</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>Testo semplice (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>Esadecimale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>Binario (byte raw)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>Solo delimitatore finale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>Delimitatore iniziale + finale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>Solo delimitatore iniziale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Nessun delimitatore (blocco intero)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>Nessun Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>Seleziona Modello Frame Parser</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>Scegli un modello da caricare:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Input esadecimale non valido.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>Nessun modello selezionato.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 trama/e estratta/e | %2 byte consumati | %3 byte nel buffer | %4 scartati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>JSON Non Valido: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>Parametri</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">Nessuno</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Input Hex Non Valido</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Inserire byte esadecimali validi.

Formato valido: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(nessuna trama)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">L'estrazione non ha prodotto una trama completa. Verificare i delimitatori di inizio/fine e la modalità di rilevamento.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 trama/e estratta/e | %2 byte consumati | %3 byte nel buffer | %4 scartati</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Configurazione Pipeline</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Risultati Pipeline</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Rilevamento</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Decodificatore</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Checksum</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Delimitatore di Inizio</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">Delimitatore di Fine</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Delimitatori Esadecimali</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Solo delimitatore finale</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Delimitatore iniziale + finale</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Solo delimitatore iniziale</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Nessun delimitatore (blocco intero)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Testo semplice (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Esadecimale</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binario (byte raw)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Cancella</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Valuta</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Inserisci qui i byte del flusso raw...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Prepara</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Input Dati Frame</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Risultati Frame Parser</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Inserire dati frame qui…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Indice Dataset</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Valore</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Inserire i dati del frame sopra, abilitare la modalità HEX se necessario, quindi fare clic su "Valuta" per eseguire il parser del frame.

Esempio (Testo): a,b,c,d,e,f
Esempio (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Testa Parser del Frame</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Inserire byte esadecimali (es. 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(vuoto)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Nessun dato restituito</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Change Scripting Language?</source>
        <translation>Cambiare Linguaggio di Scripting?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="220"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Il cambio del linguaggio di scripting sostituisce il codice parser corrente con il template equivalente nel nuovo linguaggio.

Qualsiasi modifica non salvata andrà persa. Continuare?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Javascript file to import</source>
        <translation>Seleziona file Javascript da importare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Lua file to import</source>
        <translation>Seleziona file Lua da importare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>Code Validation Successful</source>
        <translation>Validazione Codice Riuscita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="415"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>Nessun errore di sintassi rilevato nel codice del parser.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Select Frame Parser Template</source>
        <translation>Seleziona Modello Frame Parser</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="526"/>
        <source>Choose a template to load:</source>
        <translation>Scegli un modello da caricare:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>Importa Mappa Registri Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Mappe Registri Modbus (*.CSV *.XML *.JSON);;File CSV (*.CSV);;File XML (*.XML);;File JSON (*.JSON);;Tutti i File (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>Nessun registro trovato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Il file non può essere analizzato o non contiene definizioni di registri.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>Importazione Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>Panoramica</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>Acceso</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Impossibile caricare il progetto importato</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Il JSON del progetto generato non può essere caricato.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Impossibile creare il file di progetto temporaneo</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Verificare i permessi di scrittura nella directory temporanea.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>Importati con successo %1 registri in %2 gruppi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>L'editor del progetto è ora aperto per la personalizzazione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>Importazione Modbus Completata</translation>
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
        <translation type="vanished">Testo semplice (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Esadecimale</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binario (byte grezzi)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Solo delimitatore finale</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Delimitatore iniziale + finale</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Solo delimitatore iniziale</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Nessun delimitatore (blocco intero)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">Nessun Checksum</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">Input esadecimale non valido.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">Nessun modello selezionato.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 trama/e estratta/e | %2 byte consumati | %3 byte nel buffer | %4 scartati</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">Parametri</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="288"/>
        <source>Select Javascript file to import</source>
        <translation>Seleziona file Javascript da importare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>Seleziona Modello Widget di Output</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>Scegli un modello da caricare:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>Seleziona file Javascript da importare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>Seleziona Modello Widget Painter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>Scegli un modello da caricare:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>Aggiungere dataset per questo modello?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" richiede %2 dataset; il gruppo corrente ne ha %3.

Aggiungere %4 dataset usando i valori predefiniti del modello?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2550"/>
        <source>Project Information</source>
        <translation>Informazioni Progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2560"/>
        <source>Project Title</source>
        <translation>Titolo Progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2561"/>
        <source>Untitled Project</source>
        <translation>Progetto Senza Titolo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2562"/>
        <source>Name or description of the project</source>
        <translation>Nome o descrizione del progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2693"/>
        <source>Time</source>
        <translation>Tempo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2693"/>
        <source>Samples</source>
        <translation>Campioni</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2705"/>
        <source>Plot every curve against time or against the sample number</source>
        <translation>Traccia ogni curva rispetto al tempo o al numero di campione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2721"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2723"/>
        <source>Web address to load in this widget</source>
        <translation>Indirizzo web da caricare in questo widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2831"/>
        <source>Frame Detection</source>
        <translation>Rilevamento Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2845"/>
        <source>Frame Detection Method</source>
        <translation>Metodo di Rilevamento Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2846"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Seleziona come vengono identificati i frame di dati in arrivo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2856"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Delimitatori Esadecimali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Inserisci le sequenze di inizio/fine frame come valori esadecimali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2873"/>
        <source>Frame Start Delimiter</source>
        <translation>Delimitatore di Inizio Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2874"/>
        <source>e.g. /*</source>
        <translation>es. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2875"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Sequenza che marca l'inizio di un frame di dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2887"/>
        <source>Frame End Delimiter</source>
        <translation>Delimitatore Fine Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2888"/>
        <source>e.g. */</source>
        <translation>es. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2889"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Sequenza che marca la fine di un frame di dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2895"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Elaborazione e Validazione Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2906"/>
        <source>Data Conversion Method</source>
        <translation>Metodo di Conversione Dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2907"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Selezionare come i dati binari in ingresso vengono decodificati prima del parsing</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2923"/>
        <source>Checksum Algorithm</source>
        <translation>Algoritmo Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2924"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Selezionare l'algoritmo checksum utilizzato per validare i frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2580"/>
        <source>Group Information</source>
        <translation>Informazioni Gruppo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2590"/>
        <source>Group Title</source>
        <translation>Titolo Gruppo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2591"/>
        <source>Untitled Group</source>
        <translation>Gruppo Senza Titolo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2592"/>
        <source>Title or description of this dataset group</source>
        <translation>Titolo o descrizione di questo gruppo di dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2763"/>
        <source>Composite Widget</source>
        <translation>Widget Composito</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2764"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Seleziona come questo gruppo di dataset deve essere visualizzato (facoltativo)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2642"/>
        <source>Image Configuration</source>
        <translation>Configurazione Immagine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3503"/>
        <source>Virtual Dataset</source>
        <translation>Dataset Virtuale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3504"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>I dataset virtuali calcolano il loro valore da trasformazioni e tabelle dati, non richiedono un indice frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3907"/>
        <source>Fixed decimal places for the value display; overrides the format (-1 = auto)</source>
        <translation>Cifre decimali fisse per il display del valore; sovrascrive il formato (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4036"/>
        <source>Auto-detect</source>
        <translation>Rilevamento Automatico</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4036"/>
        <source>Manual Delimiters</source>
        <translation>Delimitatori Manuali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2655"/>
        <source>Detection Mode</source>
        <translation>Modalità di Rilevamento</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1471"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1474"/>
        <source>Frame Parser</source>
        <translation>Frame Parser</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation type="vanished">Gruppi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1768"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1769"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1786"/>
        <source>Shared Memory</source>
        <translation>Memoria Condivisa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1774"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1775"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5835"/>
        <source>Dataset Values</source>
        <translation>Valori Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1935"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1948"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1949"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1964"/>
        <source>Workspaces</source>
        <translation>Aree di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1981"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1984"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1985"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script di Controllo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2058"/>
        <source>Publishing</source>
        <translation>Pubblicazione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2068"/>
        <source>Enable Publishing</source>
        <translation>Abilita Pubblicazione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2069"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Trasmette frame, byte grezzi e notifiche al broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2080"/>
        <source>Payload</source>
        <translation>Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2081"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Seleziona cosa pubblicare: dati della dashboard analizzati o byte RX grezzi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2091"/>
        <source>Publish Rate (Hz)</source>
        <translation>Frequenza di Pubblicazione (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2092"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>Quante volte al secondo pubblicare (1-30 Hz). Frequenze più alte aumentano il carico sul broker; i dati della dashboard sono limitati in frequenza così un broker lento non blocca mai l'analisi dei frame.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2104"/>
        <source>Topic Base</source>
        <translation>Base Topic</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2105"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2106"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Topic base utilizzato per la pubblicazione di frame e byte grezzi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2116"/>
        <source>Script Topic</source>
        <translation>Topic Script</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2117"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2141"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>Predefinito a Topic Base quando vuoto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2118"/>
        <source>Topic the user script publishes to</source>
        <translation>Topic a cui lo script utente pubblica</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2128"/>
        <source>Publish Notifications</source>
        <translation>Pubblica Notifiche</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2129"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Replica le notifiche del dashboard su un topic dedicato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2140"/>
        <source>Notification Topic</source>
        <translation>Topic Notifiche</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2142"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Topic dove vengono replicate le notifiche del dashboard</translation>
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
        <translation>Hostname o indirizzo IP del broker MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2176"/>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2177"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>Porta TCP esposta dal broker (1883 plain, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2187"/>
        <source>Custom Client ID</source>
        <translation>ID Client Personalizzato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2189"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Off: viene generato un ID casuale nuovo ad ogni caricamento del progetto. On: utilizza l'ID sottostante.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2200"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2201"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Identificatore inviato al broker durante la CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2214"/>
        <source>Protocol Version</source>
        <translation>Versione Protocollo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2215"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>Revisione del protocollo MQTT utilizzata su CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2224"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2225"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Secondi tra pacchetti PINGREQ quando inattivo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2234"/>
        <source>Clean Session</source>
        <translation>Sessione Pulita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2235"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Scarta qualsiasi stato di sessione persistente su CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2250"/>
        <source>Username</source>
        <translation>Nome Utente</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2251"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Nome utente per l'autenticazione al broker (lasciare vuoto per anonimo)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Password</source>
        <translation>Password</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2262"/>
        <source>Password for broker authentication</source>
        <translation>Password per l'autenticazione al broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2273"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2283"/>
        <source>Use SSL/TLS</source>
        <translation>Usa SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2284"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Effettua il tunneling della connessione al broker tramite TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2297"/>
        <source>Protocol</source>
        <translation>Protocollo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2298"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Famiglia di protocolli TLS negoziata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2308"/>
        <source>Peer Verify</source>
        <translation>Verifica Peer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2309"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Quanto rigorosamente viene validata la catena di certificati del broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2319"/>
        <source>Verify Depth</source>
        <translation>Profondità di Verifica</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2320"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Lunghezza massima della catena di certificati accettata (0 = illimitata)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2607"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3241"/>
        <source>Device %1</source>
        <translation>Dispositivo %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2625"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2790"/>
        <source>Input Device</source>
        <translation>Dispositivo di Ingresso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2626"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Seleziona quale dispositivo connesso fornisce dati per questo gruppo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2657"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Il rilevamento automatico legge i byte magici JPEG/PNG; Manuale usa sequenze di inizio/fine esplicite</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2667"/>
        <source>Start Sequence (Hex)</source>
        <translation>Sequenza di Inizio (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2668"/>
        <source>e.g. FF D8 FF</source>
        <translation>es. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2669"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Byte esadecimali che marcano l'inizio di un frame immagine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2678"/>
        <source>End Sequence (Hex)</source>
        <translation>Sequenza di Fine (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2679"/>
        <source>e.g. FF D9</source>
        <translation>es. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2680"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Byte esadecimali che marcano la fine di un frame immagine</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">Identità</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2800"/>
        <source>Device Name</source>
        <translation>Nome Dispositivo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2801"/>
        <source>Device 1</source>
        <translation>Dispositivo 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2802"/>
        <source>Human-readable name for this input device</source>
        <translation>Nome leggibile per questo dispositivo di ingresso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2811"/>
        <source>Bus Type</source>
        <translation>Tipo di Bus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2812"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Seleziona l'interfaccia hardware per questo dispositivo di ingresso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Serial Port</source>
        <translation>Porta Seriale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Network Socket</source>
        <translation>Socket di Rete</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Audio Input</source>
        <translation>Ingresso Audio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>CAN Bus</source>
        <translation>Bus CAN</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Raw USB</source>
        <translation>USB Raw</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>HID Device</source>
        <translation>Dispositivo HID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>Process</source>
        <translation>Processo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>MQTT Subscriber</source>
        <translation>Sottoscrittore MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2972"/>
        <source>Connection Settings</source>
        <translation>Impostazioni di Connessione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3209"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3479"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5502"/>
        <source>General Information</source>
        <translation>Informazioni Generali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3218"/>
        <source>Action Title</source>
        <translation>Titolo Azione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3220"/>
        <source>Untitled Action</source>
        <translation>Azione Senza Titolo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3221"/>
        <source>Name or description of this action</source>
        <translation>Nome o descrizione di questa azione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3230"/>
        <source>Action Icon</source>
        <translation>Icona Azione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3231"/>
        <source>Default Icon</source>
        <translation>Icona Predefinita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3232"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Icona visualizzata per questa azione nel dashboard</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3259"/>
        <source>Target Device</source>
        <translation>Dispositivo di Destinazione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3260"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Seleziona a quale dispositivo connesso questa azione invia dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3272"/>
        <source>Data Payload</source>
        <translation>Payload Dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3283"/>
        <source>Send as Binary</source>
        <translation>Invia come Binario</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3284"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Invia dati binari grezzi quando questa azione viene attivata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3295"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3307"/>
        <source>Command</source>
        <translation>Comando</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3296"/>
        <source>Transmit Data (Hex)</source>
        <translation>Trasmetti Dati (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3297"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Payload esadecimale da inviare quando l'azione viene attivata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3308"/>
        <source>Transmit Data</source>
        <translation>Trasmetti Dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3309"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Payload testuale da inviare quando l'azione viene attivata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3320"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5561"/>
        <source>Text Encoding</source>
        <translation>Codifica Testo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3321"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Codifica caratteri utilizzata per serializzare il payload testuale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3345"/>
        <source>End-of-Line Sequence</source>
        <translation>Sequenza di Fine Riga</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3346"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>Caratteri EOL da aggiungere al messaggio (es. </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3358"/>
        <source>Execution Behavior</source>
        <translation>Comportamento di Esecuzione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3369"/>
        <source>Auto-Execute on Connect</source>
        <translation>Esecuzione Automatica alla Connessione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3370"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Attiva automaticamente questa azione quando il dispositivo si connette</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3376"/>
        <source>Timer Behavior</source>
        <translation>Comportamento del Timer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3385"/>
        <source>Timer Mode</source>
        <translation>Modalità Timer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3388"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Scegli quando e come questa azione deve ripetersi automaticamente</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3395"/>
        <source>Interval (ms)</source>
        <translation>Intervallo (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3399"/>
        <source>Timer Interval (ms)</source>
        <translation>Intervallo Timer (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3400"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Millisecondi tra ogni attivazione ripetuta di questa azione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3407"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3411"/>
        <source>Repeat Count</source>
        <translation>Conteggio Ripetizioni</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3412"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Numero di volte in cui inviare il comando ad ogni attivazione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3489"/>
        <source>Untitled Dataset</source>
        <translation>Dataset Senza Titolo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3490"/>
        <source>Dataset Title</source>
        <translation>Titolo Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3491"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Nome del dataset, utilizzato per etichettatura e identificazione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3520"/>
        <source>Hide on Dashboard</source>
        <translation>Nascondi nel Dashboard</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Sopprime il riquadro autonomo di questo dataset nel dashboard; il widget painter può comunque leggerne i valori</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Limite inferiore dell'intervallo di valori del dataset; i widget e FFT utilizzano questo valore quando il proprio intervallo non è impostato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3580"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Limite superiore dell'intervallo di valori del dataset; i widget e FFT utilizzano questo valore quando il proprio intervallo non è impostato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Scegli Tempo o un dataset per guidare l'asse X nei grafici</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Frequency Analysis</source>
        <translation>Analisi di Frequenza</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3698"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Scegli Tempo (predefinito) o qualsiasi dataset il cui valore guida l'asse Y -- produce un diagramma di Campbell quando associato ad es. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3748"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3837"/>
        <source>Minimum Value (optional)</source>
        <translation>Valore Massimo (opzionale)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3749"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Limite inferiore per la normalizzazione dei dati; utilizza l'intervallo di valori del dataset quando non impostato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3761"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3850"/>
        <source>Maximum Value (optional)</source>
        <translation>Valore Massimo (opzionale)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3762"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Limite superiore per la normalizzazione dei dati; utilizza l'intervallo di valori del dataset quando non impostato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3838"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Limite inferiore dell'intervallo del misuratore o della barra; utilizza l'intervallo di valori del dataset quando non impostato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3851"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Limite superiore dell'intervallo del misuratore o della barra; utilizza l'intervallo di valori del dataset quando non impostato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3903"/>
        <source>Decimal Points</source>
        <translation>Punti Decimali</translation>
    </message>
    <message>
        <source>Decimal places shown in the data grid value column (-1 = auto)</source>
        <translation type="vanished">Cifre decimali mostrate nella colonna dei valori della griglia dati (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3950"/>
        <source>On</source>
        <translation>On</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3994"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>Il LED si accende quando il valore raggiunge o supera questa soglia; definisci bande di allarme per colori multi-stato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4051"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4052"/>
        <source>Web View</source>
        <translation>Visualizzatore Web</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5836"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Valori grezzi e trasformati per ogni dataset (sola lettura)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5845"/>
        <source>Shared table defined in this project</source>
        <translation>Tabella condivisa definita in questo progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6468"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>Rimuovere 1 riferimento widget il cui gruppo o dataset di destinazione non esiste più?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6469"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>Rimuovere %1 riferimenti widget i cui gruppi o dataset di destinazione non esistono più?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6474"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Questo influenzerà solo il posizionamento dei riquadri nell'area di lavoro; nessun gruppo, dataset o dato verrà eliminato.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6477"/>
        <source>Clean Up Workspaces</source>
        <translation>Pulisci Aree di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3534"/>
        <source>Frame Index</source>
        <translation>Indice Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1638"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1639"/>
        <source>Dashboard Widgets</source>
        <translation>Widget della Dashboard</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2005"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2008"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2009"/>
        <source>Control Loop</source>
        <translation>Ciclo di Controllo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3535"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Posizione del frame utilizzata per allineare i dataset nel tempo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Measurement Unit</source>
        <translation>Unità di Misura</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3545"/>
        <source>Volts, Amps, etc.</source>
        <translation>Volt, Ampere, ecc.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3546"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Unità di misura, come volt o ampere (opzionale)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Plot Settings</source>
        <translation>Impostazioni Grafico</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Enable Plot Widget</source>
        <translation>Abilita Widget Grafico</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3618"/>
        <source>Plot data in real-time</source>
        <translation>Traccia dati in tempo reale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2704"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3637"/>
        <source>X-Axis Source</source>
        <translation>Sorgente Asse X</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Scegli quale dataset utilizzare per l'Asse X nei grafici</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Valore Minimo Grafico (opzionale)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Limite inferiore per l'intervallo di visualizzazione del grafico</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Valore Massimo del Grafico (opzionale)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Limite superiore per l'intervallo di visualizzazione del grafico</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">Configurazione FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3662"/>
        <source>Enable FFT Analysis</source>
        <translation>Abilita Analisi FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3663"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Esegue l'analisi nel dominio della frequenza del dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3673"/>
        <source>Enable Waterfall Plot</source>
        <translation>Abilita Grafico Waterfall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3674"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Mostra uno spettrogramma scorrevole del contenuto di frequenza nel tempo (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3697"/>
        <source>Waterfall Y Axis</source>
        <translation>Asse Y Waterfall</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="vanished">Scegli Tempo (predefinito) o qualsiasi dataset il cui valore guida l'asse Y — produce un diagramma di Campbell quando associato ad es. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3725"/>
        <source>FFT Window Size</source>
        <translation>Dimensione Finestra FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3726"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Numero di campioni utilizzati per ogni finestra di calcolo FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3737"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>Frequenza di Campionamento FFT (Hz, richiesta)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3738"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Frequenza di campionamento utilizzata per FFT (in Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Valore Minimo (consigliato)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Limite inferiore per la normalizzazione dei dati</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Valore Massimo (consigliato)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Limite superiore per la normalizzazione dei dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3787"/>
        <source>Widget Settings</source>
        <translation>Impostazioni Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3810"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3811"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Seleziona il widget visivo utilizzato per visualizzare questo dataset</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Valore Minimo di Visualizzazione (obbligatorio)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Limite inferiore dell'intervallo di visualizzazione del misuratore o della barra</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Valore Massimo di Visualizzazione (obbligatorio)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Limite superiore dell'intervallo di visualizzazione del misuratore o della barra</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3867"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3902"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4063"/>
        <source>Auto</source>
        <translation>Automatico</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3868"/>
        <source>Tick Count</source>
        <translation>Conteggio Tacche</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3872"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Numero di tacche principali sulla scala del quadrante (0 = adatta automaticamente alla dimensione del widget)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3891"/>
        <source>Label Format</source>
        <translation>Formato Etichetta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3892"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Cifre decimali o notazione utilizzata sulle etichette delle tacche e sul display del valore</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Mostra Indicatore Valore</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Attiva o disattiva la lettura numerica incorniciata che si trova sotto o accanto al widget</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Impostazioni Allarme</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Abilita Allarmi</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Attiva un allarme visivo quando il valore supera le soglie di allarme</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Soglia Inferiore</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Attiva un allarme visivo quando il valore scende sotto questa soglia</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Soglia Superiore</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Attiva un allarme visivo quando il valore supera questa soglia</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3967"/>
        <source>LED Display Settings</source>
        <translation>Impostazioni Display LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3978"/>
        <source>Show in LED Panel</source>
        <translation>Mostra nel Pannello LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3979"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Abilita il monitoraggio visivo dello stato tramite un display LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3993"/>
        <source>LED On Threshold (required)</source>
        <translation>Soglia LED Acceso (obbligatorio)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">Il LED si accende quando il valore raggiunge o supera questa soglia</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Off</source>
        <translation>Off</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Auto Start</source>
        <translation>Avvio Automatico</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Start on Trigger</source>
        <translation>Avvia su Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Toggle on Trigger</source>
        <translation>Commuta su Trigger</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4015"/>
        <source>Repeat N Times</source>
        <translation>Ripeti N Volte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Plain Text (UTF8)</source>
        <translation>Testo Semplice (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Hexadecimal</source>
        <translation>Esadecimale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4019"/>
        <source>Binary (Direct)</source>
        <translation>Binario (Diretto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4024"/>
        <source>No Checksum</source>
        <translation>Nessun Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4028"/>
        <source>End Delimiter Only</source>
        <translation>Solo Delimitatore Finale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4028"/>
        <source>Start Delimiter Only</source>
        <translation>Solo Delimitatore Iniziale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4029"/>
        <source>Start + End Delimiter</source>
        <translation>Delimitatore Iniziale + Finale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4029"/>
        <source>No Delimiters</source>
        <translation>Nessun Delimitatore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Button</source>
        <translation>Pulsante</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Slider</source>
        <translation>Cursore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Toggle</source>
        <translation>Interruttore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Text Field</source>
        <translation>Campo di Testo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4040"/>
        <source>Knob</source>
        <translation>Manopola</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4044"/>
        <source>Data Grid</source>
        <translation>Griglia Dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4045"/>
        <source>GPS Map</source>
        <translation>Mappa GPS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4046"/>
        <source>Gyroscope</source>
        <translation>Giroscopio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4047"/>
        <source>Multiple Plot</source>
        <translation>Grafico Multiplo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4048"/>
        <source>Accelerometer</source>
        <translation>Accelerometro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4049"/>
        <source>3D Plot</source>
        <translation>Grafico 3D</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4050"/>
        <source>Image View</source>
        <translation>Visualizzatore Immagini</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4053"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4056"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4071"/>
        <source>None</source>
        <translation>Nessuno</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4057"/>
        <source>Bar</source>
        <translation>Barra</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4058"/>
        <source>Gauge</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4059"/>
        <source>Compass</source>
        <translation>Bussola</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4060"/>
        <source>Meter</source>
        <translation>Misuratore</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4064"/>
        <source>Integer (0 decimals)</source>
        <translation>Intero (0 decimali)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4065"/>
        <source>1 decimal</source>
        <translation>1 decimale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4066"/>
        <source>2 decimals</source>
        <translation>2 decimali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4067"/>
        <source>3 decimals</source>
        <translation>3 decimali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4068"/>
        <source>Scientific</source>
        <translation>Scientifica</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4072"/>
        <source>New Line (\n)</source>
        <translation>Nuova Riga (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4073"/>
        <source>Carriage Return (\r)</source>
        <translation>Ritorno Carrello (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4074"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4077"/>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4078"/>
        <source>Yes</source>
        <translation>Sì</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4908"/>
        <source>(multiple)</source>
        <translation>(multipli)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4922"/>
        <source>Mixed</source>
        <translation>Misto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5512"/>
        <source>Label</source>
        <translation>Etichetta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5513"/>
        <source>Display label</source>
        <translation>Etichetta di Visualizzazione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5523"/>
        <source>Button Icon</source>
        <translation>Icona Pulsante</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5532"/>
        <source>Colorize Icon</source>
        <translation>Colora Icona</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5533"/>
        <source>Tint the icon with the button color</source>
        <translation>Tinta l'icona con il colore del pulsante</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5550"/>
        <source>Initial Value</source>
        <translation>Valore Iniziale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5562"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Codifica dei caratteri utilizzata quando transmit() restituisce un valore stringa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5580"/>
        <source>Value Range</source>
        <translation>Intervallo di Valore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3566"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5590"/>
        <source>Minimum Value</source>
        <translation>Valore Minimo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3579"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5599"/>
        <source>Maximum Value</source>
        <translation>Valore Massimo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5608"/>
        <source>Step Size</source>
        <translation>Dimensione del Passo</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="644"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="653"/>
        <source>Lock Project</source>
        <translation>Blocca Progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="645"/>
        <source>Choose a password to lock the project:</source>
        <translation>Scegli una password per bloccare il progetto:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="653"/>
        <source>Confirm the password:</source>
        <translation>Conferma la password:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="658"/>
        <source>Passwords do not match</source>
        <translation>Le password non corrispondono</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="659"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>Le due password inserite non corrispondono. Il progetto non è stato bloccato.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="693"/>
        <source>Unlock Project</source>
        <translation>Sblocca Progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="694"/>
        <source>Enter the project password:</source>
        <translation>Inserire la password del progetto:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="704"/>
        <source>Incorrect password</source>
        <translation>Password errata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="705"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>La password inserita non corrisponde a quella memorizzata nel file di progetto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="736"/>
        <source>New Project</source>
        <translation>Nuovo Progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="936"/>
        <source>Samples</source>
        <translation>Campioni</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1523"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Più sorgenti dati richiedono una licenza Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1524"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro consente di connettersi a più dispositivi contemporaneamente. Effettua l'upgrade per sbloccare questa funzionalità.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1536"/>
        <source>Device %1</source>
        <translation>Dispositivo %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Copia)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1806"/>
        <source>Do you want to save your changes?</source>
        <translation>Salvare le modifiche?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1807"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Sono presenti modifiche non salvate in questo progetto!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="516"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="538"/>
        <source>Project error</source>
        <translation>Errore progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="516"/>
        <source>Project title cannot be empty!</source>
        <translation>Il titolo del progetto non può essere vuoto!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <source>You need to add at least one group!</source>
        <translation>È necessario aggiungere almeno un gruppo!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="538"/>
        <source>You need to add at least one dataset!</source>
        <translation>È necessario aggiungere almeno un dataset!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="585"/>
        <source>Your project needs a title</source>
        <translation>Il progetto necessita di un titolo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="587"/>
        <source>Add a group to get started</source>
        <translation>Aggiungi un gruppo per iniziare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="589"/>
        <source>Add a dataset to a group</source>
        <translation>Aggiungi un dataset a un gruppo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="603"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Apri la vista Progetto in cima all'albero e inserisci un nome. Puoi rinominare il progetto in qualsiasi momento.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="606"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>I gruppi organizzano i dataset in widget della dashboard. Usa il pulsante Gruppo nella barra degli strumenti sopra per crearne uno, quindi aggiungi dataset ad esso.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="610"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>I dataset sono i valori che appaiono sul dashboard. Selezionare un gruppo nell'albero e utilizzare il pulsante Dataset nella barra degli strumenti per aggiungerne uno.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="935"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="984"/>
        <source>Time</source>
        <translation>Tempo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1568"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Eliminare la sorgente dati "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1569"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>I gruppi che utilizzano questa sorgente verranno spostati alla sorgente predefinita. Questa azione non può essere annullata.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1846"/>
        <source>Save Serial Studio Project</source>
        <translation>Salva Progetto Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1848"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2512"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>File Progetto Serial Studio (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1869"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2110"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2503"/>
        <source>Untitled Project</source>
        <translation>Progetto Senza Titolo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2125"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2658"/>
        <source>Device A</source>
        <translation>Dispositivo A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2337"/>
        <source>Select Project File</source>
        <translation>Seleziona File Progetto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2339"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>File Progetto (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2383"/>
        <source>JSON validation error</source>
        <translation>Errore di validazione JSON</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2477"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Progetto aggiornato da un formato file precedente</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2478"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Questo progetto è stato salvato con la versione schema %1; la versione attuale è %2. I valori predefiniti sono stati applicati a tutti i nuovi campi. Salva il progetto per confermare l’aggiornamento.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2510"/>
        <source>Save Imported Project</source>
        <translation>Salva Progetto Importato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2702"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>I progetti multi-sorgente richiedono una licenza Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2703"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Questo progetto contiene più sorgenti dati. È stata caricata solo la prima sorgente. È richiesta una licenza Serial Studio Pro per utilizzare progetti multi-sorgente.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2941"/>
        <source>Workspace IDs remapped on load</source>
        <translation>ID degli spazi di lavoro rimappati al caricamento</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2942"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 ID area di lavoro personalizzate si sovrapponevano al nuovo intervallo automatico riservato e sono state spostate nell'intervallo utente. Salva il progetto per rendere permanente la rimappatura.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3125"/>
        <source>Legacy frame parser function updated</source>
        <translation>Funzione parser frame legacy aggiornata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3126"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Il progetto utilizzava una funzione di analisi frame legacy con un argomento 'separator'. È stata migrata automaticamente al nuovo formato.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3370"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Eliminare il gruppo "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3371"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3416"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3448"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4186"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Questa azione non può essere annullata. Procedere?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3415"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Eliminare l'azione "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3447"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Eliminare il dataset "%1"?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Copia)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4098"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4134"/>
        <source>Output Controls</source>
        <translation>Controlli di Uscita</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4146"/>
        <source>New Button</source>
        <translation>Nuovo Pulsante</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4149"/>
        <source>New Slider</source>
        <translation>Nuovo Cursore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4152"/>
        <source>New Toggle</source>
        <translation>Nuovo Interruttore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4155"/>
        <source>New Text Field</source>
        <translation>Nuovo Campo di Testo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4158"/>
        <source>New Knob</source>
        <translation>Nuova Manopola</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4185"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Eliminare il widget di output "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4350"/>
        <source>Group</source>
        <translation>Gruppo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4368"/>
        <source>New Dataset</source>
        <translation>Nuovo Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4371"/>
        <source>New Plot</source>
        <translation>Nuovo Grafico</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4375"/>
        <source>New FFT Plot</source>
        <translation>Nuovo Grafico FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4379"/>
        <source>New Level Indicator</source>
        <translation>Nuovo Indicatore di Livello</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4383"/>
        <source>New Gauge</source>
        <translation>Nuovo Indicatore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4387"/>
        <source>New Compass</source>
        <translation>Nuova Bussola</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4393"/>
        <source>New Meter</source>
        <translation>Nuovo Indicatore</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Nuovo Termometro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4397"/>
        <source>New LED Indicator</source>
        <translation>Nuovo Indicatore LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4401"/>
        <source>New Waterfall</source>
        <translation>Nuovo Waterfall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4469"/>
        <source>Channel %1</source>
        <translation>Canale %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4540"/>
        <source>New Action</source>
        <translation>Nuova Azione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4681"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Cambiare il widget a livello di gruppo?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4683"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>I dataset esistenti per questo gruppo vengono eliminati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4751"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4752"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4753"/>
        <source>Accelerometer %1</source>
        <translation>Accelerometro %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <source>Gyro %1</source>
        <translation>Giroscopio %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Latitude</source>
        <translation>Latitudine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Longitude</source>
        <translation>Longitudine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Altitude</source>
        <translation>Altitudine</translation>
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
        <translation>Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5259"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5465"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6727"/>
        <source>Shared Table</source>
        <translation>Tabella Condivisa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5336"/>
        <source>register</source>
        <translation>registro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5465"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6727"/>
        <source>New Shared Table</source>
        <translation>Nuova Tabella Condivisa</translation>
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
        <translation>Nome:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5488"/>
        <source>Rename Table</source>
        <translation>Rinomina Tabella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5507"/>
        <source>Rename Group</source>
        <translation>Rinomina Gruppo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5531"/>
        <source>Rename Dataset</source>
        <translation>Rinomina Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5558"/>
        <source>Rename Data Source</source>
        <translation>Rinomina Sorgente Dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5577"/>
        <source>Rename Action</source>
        <translation>Rinomina Azione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5598"/>
        <source>New Register</source>
        <translation>Nuovo Registro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5621"/>
        <source>Rename Register</source>
        <translation>Rinomina Registro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5655"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5680"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7389"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8173"/>
        <source>This action cannot be undone.</source>
        <translation>Questa azione non può essere annullata.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5656"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Questo rimuove %1 registro/i insieme alla tabella. Questa azione non può essere annullata.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5659"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5679"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7388"/>
        <source>Delete "%1"?</source>
        <translation>Eliminare "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5662"/>
        <source>Delete Table</source>
        <translation>Elimina Tabella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5682"/>
        <source>Delete Register</source>
        <translation>Elimina Registro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5704"/>
        <source>Export Table</source>
        <translation>Esporta Tabella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5706"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5747"/>
        <source>CSV files (*.csv)</source>
        <translation>File CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5745"/>
        <source>Import Table</source>
        <translation>Importa Tabella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5996"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6268"/>
        <source>New Workspace</source>
        <translation>Nuova Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6017"/>
        <source>Rename Workspace</source>
        <translation>Rinomina Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6063"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6251"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6351"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6485"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6569"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6710"/>
        <source>Folder</source>
        <translation>Cartella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6251"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6485"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6710"/>
        <source>New Folder</source>
        <translation>Nuova Cartella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6290"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6506"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6748"/>
        <source>Rename Folder</source>
        <translation>Rinomina Cartella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6308"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6524"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6766"/>
        <source>Delete folder "%1"?</source>
        <translation>Eliminare la cartella "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6309"/>
        <source>The folder is removed; its workspaces and sub-folders move up to the parent.</source>
        <translation>La cartella viene rimossa; i suoi spazi di lavoro e le sottocartelle vengono spostati al livello superiore.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6311"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6527"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6770"/>
        <source>Delete Folder</source>
        <translation>Elimina Cartella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6525"/>
        <source>The folder is removed; its groups and sub-folders move up to the parent.</source>
        <translation>La cartella viene rimossa; i suoi gruppi e le sottocartelle vengono spostati al livello superiore.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6767"/>
        <source>The folder is removed; its tables and sub-folders move up to the parent. The accessor path of those tables changes accordingly.</source>
        <translation>La cartella viene rimossa; le sue tabelle e le sottocartelle vengono spostate al livello superiore. Il percorso di accesso di tali tabelle cambia di conseguenza.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6855"/>
        <source>Overview</source>
        <translation>Panoramica</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6864"/>
        <source>All Data</source>
        <translation>Tutti i Dati</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7157"/>
        <source>Discard workspace customisations?</source>
        <translation>Scartare le personalizzazioni dell'area di lavoro?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7158"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Disattivando Personalizza si scartano le modifiche e si ricostruisce l'elenco delle aree di lavoro dai gruppi del progetto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7161"/>
        <source>Customize Workspaces</source>
        <translation>Personalizza Aree di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7391"/>
        <source>Delete Workspace</source>
        <translation>Elimina Spazio di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7787"/>
        <source>Project file removed from disk</source>
        <translation>File di progetto rimosso dal disco</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7788"/>
        <source>%1 was deleted or renamed by another program. Save the project to recreate it.</source>
        <translation>%1 è stato eliminato o rinominato da un altro programma. Salva il progetto per ricrearlo.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7809"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7830"/>
        <source>Project file changed on disk</source>
        <translation>File di progetto modificato sul disco</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7810"/>
        <source>%1 was modified by another program. The in-memory project was kept; reopen the file to load the external changes.</source>
        <translation>%1 è stato modificato da un altro programma. Il progetto in memoria è stato mantenuto; riapri il file per caricare le modifiche esterne.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7826"/>
        <source>The project file was modified by another program.

Reload it and discard your unsaved changes?</source>
        <translation>Il file di progetto è stato modificato da un altro programma.

Ricaricarlo e scartare le modifiche non salvate?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7828"/>
        <source>The project file was modified by another program.

Reload it?</source>
        <translation>Il file di progetto è stato modificato da un altro programma.

Ricaricarlo?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7859"/>
        <source>File save error</source>
        <translation>Errore salvataggio file</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8172"/>
        <source>Delete %1 selected items?</source>
        <translation>Eliminare %1 elementi selezionati?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8175"/>
        <source>Delete Items</source>
        <translation>Elimina Elementi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2546"/>
        <source>File open error</source>
        <translation>Errore apertura file</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Importa File Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>File Proto (*.proto);;Tutti i File (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Impossibile aprire il file proto: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Verificare il percorso del file e i permessi di lettura, quindi riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Errore Importazione Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Il file proto è troppo grande (il limite è 10 MB).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>Verificare di aver selezionato il file di definizione .proto corretto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Impossibile analizzare il file proto alla riga %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>È supportata solo la sintassi proto3. Verificare il formato del file e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Il file proto non contiene definizioni di messaggi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Il file selezionato non ha blocchi `message` da importare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Avviso Importazione Protobuf</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Impossibile caricare il progetto importato</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Impossibile caricare il JSON del progetto generato.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>Importati con successo %1 messaggio/i e %2 campo/i dal file proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>L'editor del progetto è ora aperto per la personalizzazione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Importazione Protobuf Completata</translation>
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
        <translation>Input Hex Non Valido</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Inserire byte esadecimali validi.

Formato valido: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Nessun codice funzione di trasmissione da valutare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>la funzione transmit non è richiamabile</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>Valuta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>Valore di Input</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>Output della Funzione di Trasmissione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>Inserire valore da trasmettere…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>L'output della stringa grezza appare qui</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>L'output dei byte esadecimali appare qui</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>Testa Funzione di Trasmissione</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Inserire byte esadecimali (es. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(vuoto) Nessun dato restituito</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 byte</translation>
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
        <translation>Memoria Condivisa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="167"/>
        <source>Add Folder</source>
        <translation>Aggiungi Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="169"/>
        <source>Add a top-level folder</source>
        <translation>Aggiungi una cartella di primo livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="177"/>
        <source>Add Shared Table</source>
        <translation>Aggiungi Tabella Condivisa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="179"/>
        <source>Add shared table</source>
        <translation>Aggiungi tabella condivisa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="188"/>
        <source>Help</source>
        <translation>Aiuto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="193"/>
        <source>Open help documentation for shared memory</source>
        <translation>Apri la documentazione di aiuto per la memoria condivisa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="202"/>
        <source>Title</source>
        <translation>Titolo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="203"/>
        <source>Registers</source>
        <translation>Registri</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="vanished">Nome</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="vanished">Descrizione</translation>
    </message>
    <message>
        <source>Entries</source>
        <translation type="vanished">Voci</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="297"/>
        <source>No shared tables.</source>
        <translation>Nessuna tabella condivisa.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Apri</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Apri un file di sessione</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Chiudi file di sessione</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Riproduci</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Riproduci la sessione selezionata sul dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Sblocca il file di sessione per eliminare le sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Elimina la sessione selezionata</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Sblocca</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Blocca</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Sblocca il file di sessione per consentire le eliminazioni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Imposta una password per impedire l'eliminazione delle sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>Esporta CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Esporta la sessione selezionata in CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>Esporta PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>Genera un report PDF per la sessione selezionata</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Ripristina Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Ripristina il file di progetto da questo file di sessione</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Caricamento sessione…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>In Elaborazione…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="112"/>
        <source>Pro features detected in this project.</source>
        <translation>Funzionalità Pro rilevate in questo progetto.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="114"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Widget di fallback in uso. Acquista una licenza per sbloccare la funzionalità completa.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="142"/>
        <source>Plots</source>
        <translation>Grafici</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="147"/>
        <source>Plot</source>
        <translation>Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="151"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>Attiva/disattiva la visualizzazione del grafico 2D per questo dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>FFT Plot</source>
        <translation>Grafico FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="166"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>Attiva il grafico FFT per visualizzare il contenuto in frequenza</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="178"/>
        <source>Waterfall</source>
        <translation>Waterfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="182"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Attiva/disattiva grafico a cascata (spettrogramma) — utilizza le impostazioni FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="199"/>
        <source>Widgets</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="205"/>
        <source>Bar/Level</source>
        <translation>Barra/livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="209"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Attiva l'indicatore barra/livello per questo dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="220"/>
        <source>Gauge</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="225"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Attiva il widget indicatore per la visualizzazione in stile analogico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="237"/>
        <source>Compass</source>
        <translation>Bussola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="241"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Attiva il widget bussola per i dati direzionali</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="252"/>
        <source>Meter</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="257"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Attiva il widget indicatore analogico (semicerchio)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="308"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>Definisci intervalli di valori colorati con livelli di gravità per l'indicatore o il LED di questo dataset.</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometro</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Attiva il widget termometro per i dati di temperatura</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="268"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="273"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>Attiva l'indicatore LED per valori binari o con soglia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="293"/>
        <source>Behavior</source>
        <translation>Comportamento</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="298"/>
        <source>Alarm Bands</source>
        <translation>Bande Allarme</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">Definisci intervalli di valori colorati con livelli di gravità per l'indicatore di questo dataset.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Transform</source>
        <translation>Trasformazione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="318"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Modifica un'espressione di trasformazione del valore per calibrazione, filtraggio o conversione di unità</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="331"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="336"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Duplica questo dataset con la stessa configurazione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="344"/>
        <source>Delete this dataset from the group</source>
        <translation>Elimina questo dataset dal gruppo</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="36"/>
        <source>Support Serial Studio</source>
        <translation>Supporta Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="85"/>
        <source>Support the development of %1!</source>
        <translation>Supporta lo sviluppo di %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="96"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio è un software libero e open-source supportato da volontari. Considera di fare una donazione o di ottenere una licenza Pro per supportare gli sforzi di sviluppo :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="109"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Puoi anche supportare questo progetto condividendolo, segnalando bug e proponendo nuove funzionalità!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="123"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="134"/>
        <source>Donate</source>
        <translation>Dona</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="149"/>
        <source>Get Serial Studio Pro</source>
        <translation>Ottieni Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="142"/>
        <source>Stop</source>
        <translation>Interrompi</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="143"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="377"/>
        <source>Downloading updates</source>
        <translation>Download aggiornamenti in corso</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="470"/>
        <source>Time remaining</source>
        <translation>Tempo rimanente</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <source>unknown</source>
        <translation>sconosciuto</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Error</source>
        <translation>Errore</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Cannot find downloaded update!</source>
        <translation>Impossibile trovare l'aggiornamento scaricato!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="260"/>
        <source>Download complete!</source>
        <translation>Download completato!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="268"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Fare clic su "OK" per iniziare l'installazione dell'aggiornamento</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="270"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>Per installare l'aggiornamento, potrebbe essere necessario chiudere l'applicazione.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="261"/>
        <source>The installer opens separately</source>
        <translation>Il programma di installazione si apre separatamente</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="274"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>Per installare l'aggiornamento, potrebbe essere necessario chiudere l'applicazione. Questo è un aggiornamento obbligatorio; uscire ora chiuderà l'applicazione.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="290"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Fare clic sul pulsante "Apri" per applicare l'aggiornamento</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="303"/>
        <source>Updater</source>
        <translation>Aggiornamento</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="307"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Annullare il download?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="309"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Annullare il download? Questo è un aggiornamento obbligatorio; uscire ora chiuderà l'applicazione</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="364"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="371"/>
        <source>%1 bytes</source>
        <translation>%1 byte</translation>
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
        <translation>di</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="421"/>
        <source>Downloading Updates</source>
        <translation>Download Aggiornamenti in Corso</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Time Remaining</source>
        <translation>Tempo Rimanente</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Unknown</source>
        <translation>Sconosciuto</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="446"/>
        <source>about %1 hours</source>
        <translation>circa %1 ore</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="448"/>
        <source>about one hour</source>
        <translation>circa un'ora</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="456"/>
        <source>%1 minutes</source>
        <translation>%1 minuti</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="458"/>
        <source>1 minute</source>
        <translation>1 minuto</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="465"/>
        <source>%1 seconds</source>
        <translation>%1 secondi</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="467"/>
        <source>1 second</source>
        <translation>1 secondo</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Tempo rimanente: 0 minuti</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Apri</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="33"/>
        <source>Examples Browser</source>
        <translation>Browser Esempi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="119"/>
        <source>Back</source>
        <translation>Indietro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="151"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="172"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Scarica e Apri</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="187"/>
        <source>View on GitHub</source>
        <translation>Visualizza su GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="90"/>
        <source>Search in Examples…</source>
        <translation>Cerca negli Esempi…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="244"/>
        <source>Fetching examples…</source>
        <translation>Recupero esempi…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="567"/>
        <source>Loading...</source>
        <translation>Caricamento...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>No README available.</source>
        <translation>Nessun README disponibile.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="608"/>
        <source>Copied to Clipboard</source>
        <translation>Copiato negli Appunti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="671"/>
        <source>No screenshot available</source>
        <translation>Nessuna schermata disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="703"/>
        <source>Details</source>
        <translation>Dettagli</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="732"/>
        <source>Info</source>
        <translation>Info</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="755"/>
        <source>Category:</source>
        <translation>Categoria:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="768"/>
        <source>Difficulty:</source>
        <translation>Difficoltà:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="786"/>
        <source>Project:</source>
        <translation>Progetto:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="828"/>
        <source>No Results Found</source>
        <translation>Nessun Risultato Trovato</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="839"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Controlla l'ortografia o prova un termine di ricerca diverso.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="854"/>
        <source>%1 examples</source>
        <translation>%1 esempi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="863"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="31"/>
        <source>Extension Manager</source>
        <translation>Gestione Estensioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="118"/>
        <source>Refresh</source>
        <translation>Aggiorna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="132"/>
        <source>Repos</source>
        <translation>Repository</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="162"/>
        <source>Repository Settings</source>
        <translation>Impostazioni Repository</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="174"/>
        <source>Back</source>
        <translation>Indietro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="215"/>
        <source>Install</source>
        <translation>Installa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="232"/>
        <source>Uninstall</source>
        <translation>Disinstalla</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="259"/>
        <source>Run</source>
        <translation>Esegui</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="283"/>
        <source>Stop</source>
        <translation>Interrompi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="317"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="77"/>
        <source>Search extensions…</source>
        <translation>Cerca estensioni…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="367"/>
        <source>Fetching extensions…</source>
        <translation>Recupero estensioni…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="604"/>
        <source>Running</source>
        <translation>In Esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="626"/>
        <source>Update</source>
        <translation>Aggiorna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="626"/>
        <source>Installed</source>
        <translation>Installata</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="643"/>
        <source>Unavailable</source>
        <translation>Non Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="822"/>
        <source>No description available.</source>
        <translation>Nessuna descrizione disponibile.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="863"/>
        <source>Details</source>
        <translation>Dettagli</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="884"/>
        <source>Type:</source>
        <translation>Tipo:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="897"/>
        <source>Author:</source>
        <translation>Autore:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="909"/>
        <source>Version:</source>
        <translation>Versione:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="921"/>
        <source>License:</source>
        <translation>Licenza:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="982"/>
        <source>No preview</source>
        <translation>Nessuna anteprima</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1011"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>USCITA PLUGIN</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1041"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Nessuna uscita ancora. Esegui il plugin per vedere il suo log qui.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1076"/>
        <source>No preview available</source>
        <translation>Nessuna anteprima disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1120"/>
        <source>Repositories</source>
        <translation>Repository</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1133"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>Aggiungi URL a repository remoti o percorsi di cartelle locali.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1170"/>
        <source>LOCAL</source>
        <translation>LOCALE</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1227"/>
        <source>URL or local path…</source>
        <translation>URL o percorso locale…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1258"/>
        <source>Browse…</source>
        <translation>Sfoglia…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1235"/>
        <source>Add</source>
        <translation>Aggiungi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1295"/>
        <source>No Results Found</source>
        <translation>Nessun Risultato Trovato</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1306"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Controlla l'ortografia o prova un termine di ricerca diverso.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1330"/>
        <source>No Extensions Available</source>
        <translation>Nessuna Estensione Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1341"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Aggiungi un URL di repository o un percorso locale nelle impostazioni Repos, quindi aggiorna.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1356"/>
        <source>%1 extensions</source>
        <translation>%1 estensioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1365"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="167"/>
        <source>Interpolation: %1</source>
        <translation>Interpolazione: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="195"/>
        <source>Show Area Under Plot</source>
        <translation>Mostra Area Sotto il Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="213"/>
        <source>Show X Axis Label</source>
        <translation>Mostra Etichetta Asse X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="225"/>
        <source>Show Y Axis Label</source>
        <translation>Mostra Etichetta Asse Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="243"/>
        <source>Show Crosshair</source>
        <translation>Mostra Mirino</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Resume</source>
        <translation>Riprendi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="269"/>
        <source>Reset View</source>
        <translation>Ripristina Vista</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="275"/>
        <source>Axis Range Settings</source>
        <translation>Impostazioni Intervallo Assi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="304"/>
        <source>Magnitude (dB)</source>
        <translation>Magnitudine (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="305"/>
        <source>Frequency (Hz)</source>
        <translation>Frequenza (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Rilascia qui Progetti e file CSV</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="33"/>
        <source>File Transmission</source>
        <translation>Trasmissione File</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="101"/>
        <source>Transfer Protocol:</source>
        <translation>Protocollo di Trasferimento:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="134"/>
        <source>File Selection:</source>
        <translation>Selezione File:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="151"/>
        <source>Select File…</source>
        <translation>Seleziona File…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="169"/>
        <source>Transmission Interval:</source>
        <translation>Intervallo di Trasmissione:</translation>
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
        <translation>Dimensione Blocco:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="233"/>
        <source>bytes</source>
        <translation>byte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="243"/>
        <source>Timeout:</source>
        <translation>Timeout:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="281"/>
        <source>Max Retries:</source>
        <translation>Tentativi Massimi:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="339"/>
        <source>Progress: %1%</source>
        <translation>Progresso: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="372"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 byte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="380"/>
        <source>Errors: %1</source>
        <translation>Errori: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="460"/>
        <source>Activity Log</source>
        <translation>Registro Attività</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="464"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="418"/>
        <source>Pause Transmission</source>
        <translation>Sospendi Trasmissione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Resume Transmission</source>
        <translation>Riprendi Trasmissione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="422"/>
        <source>Stop Transmission</source>
        <translation>Interrompi Trasmissione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Begin Transmission</source>
        <translation>Avvia Trasmissione</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="448"/>
        <source>Frame Parser</source>
        <translation>Frame Parser</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="455"/>
        <source>Device %1</source>
        <translation>Dispositivo %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="575"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1862"/>
        <source>Output Panel</source>
        <translation>Pannello di Output</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="619"/>
        <source>Control</source>
        <translation>Controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="694"/>
        <source>Table</source>
        <translation>Tabella</translation>
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
        <translation>vuoto</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script di Controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="843"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1251"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Apri l'editor del codice di trasformazione per questo dataset.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1632"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1635"/>
        <source>Dataset Container</source>
        <translation>Contenitore di Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1644"/>
        <source>Multi-Plot</source>
        <translation>Grafico Multiplo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1647"/>
        <source>Multiple Plot</source>
        <translation>Grafico Multiplo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1656"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1659"/>
        <source>Accelerometer</source>
        <translation>Accelerometro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1668"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1671"/>
        <source>Gyroscope</source>
        <translation>Giroscopio</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1680"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1683"/>
        <source>GPS Map</source>
        <translation>Mappa GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1691"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1694"/>
        <source>3D Plot</source>
        <translation>Grafico 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1702"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1705"/>
        <source>Image View</source>
        <translation>Visualizzatore Immagini</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1714"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1717"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1729"/>
        <source>Web View</source>
        <translation>Visualizzatore Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1738"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1741"/>
        <source>Data Grid</source>
        <translation>Griglia Dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1754"/>
        <source>Generic</source>
        <translation>Generico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1767"/>
        <source>Plot</source>
        <translation>Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1780"/>
        <source>FFT Plot</source>
        <translation>Grafico FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1793"/>
        <source>Gauge</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1806"/>
        <source>Level Indicator</source>
        <translation>Indicatore di Livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1819"/>
        <source>Compass</source>
        <translation>Bussola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1832"/>
        <source>Meter</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <source>Edit Control Script…</source>
        <translation type="vanished">Modifica Script di Controllo…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="404"/>
        <source>Control Loop</source>
        <translation>Ciclo di Controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="799"/>
        <source>Shared Memory</source>
        <translation>Memoria Condivisa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1845"/>
        <source>LED Indicator</source>
        <translation>Indicatore LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1874"/>
        <source>Slider</source>
        <translation>Cursore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1887"/>
        <source>Toggle</source>
        <translation>Interruttore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1900"/>
        <source>Knob</source>
        <translation>Manopola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1913"/>
        <source>Text Field</source>
        <translation>Campo di Testo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1926"/>
        <source>Button</source>
        <translation>Pulsante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1950"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2026"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2114"/>
        <source>Add Group</source>
        <translation>Aggiungi Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1966"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2042"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2130"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2175"/>
        <source>Add Dataset</source>
        <translation>Aggiungi Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1980"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2056"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2144"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2189"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2396"/>
        <source>Add Output</source>
        <translation>Aggiungi Output</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1996"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2069"/>
        <source>Add Action</source>
        <translation>Aggiungi Azione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2005"/>
        <source>Add Data Source</source>
        <translation>Aggiungi Sorgente Dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Add Data Table</source>
        <translation>Aggiungi Tabella Dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2080"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2216"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2283"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2411"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2445"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2501"/>
        <source>Rename…</source>
        <translation>Rinomina…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2088"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2246"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2316"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2368"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2419"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2475"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
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
        <translation>Elimina…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2160"/>
        <source>Edit Frame Parser…</source>
        <translation>Modifica Frame Parser…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2202"/>
        <source>Edit Painter Code…</source>
        <translation>Modifica Codice Painter…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2224"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2292"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2344"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2453"/>
        <source>Move Up</source>
        <translation>Sposta Su</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2235"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2304"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2356"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2464"/>
        <source>Move Down</source>
        <translation>Sposta Giù</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2272"/>
        <source>Edit Transform Code…</source>
        <translation>Modifica Codice Transform…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2527"/>
        <source>Edit Code…</source>
        <translation>Modifica Codice…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2542"/>
        <source>Edit Control Loop…</source>
        <translation>Modifica Ciclo di Controllo…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="245"/>
        <source>Group</source>
        <translation>Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="372"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="756"/>
        <source>Folder</source>
        <translation>Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="517"/>
        <source>Action</source>
        <translation>Azione</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">Nessun gruppo ancora definito</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Testa Parser del Frame</translation>
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
        <translation>Decodificatore</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>Righe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 riga/righe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Riga %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Configurazione Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Rilevamento</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Inizio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Fine</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Delimitatori Esadecimali</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Input Dati Frame</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Inserisci byte esadecimali (es. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Inserisci qui i byte del flusso raw...</translation>
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
        <translation>Il campione non contiene i delimitatori di frame configurati, quindi nessun frame verrà estratto. Digitali nel campione (es. 
 per una nuova riga) o modifica la modalità di rilevamento.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Risultati Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Prepara</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Valore</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>L'estrazione non ha prodotto una trama completa. Verificare i delimitatori di inizio/fine e la modalità di rilevamento.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Inserire i dati di esempio sopra e premere Valuta per visualizzare l'anteprima dell'output analizzato</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Valuta</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Annulla</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Ripeti</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Taglia</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Copia</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Incolla</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Seleziona Tutto</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Formatta Documento</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Formatta Selezione</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Ripristina</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Ripristina allo script di analisi predefinito</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Apri</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Importa un file di script per l'analisi dei dati</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Apri la documentazione di aiuto per l'analisi dei dati</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Linguaggio:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Seleziona Modello…</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Testa con Dati di Esempio</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Valuta</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Annulla l'ultima modifica del codice</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Ripristina la modifica precedentemente annullata</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Taglia il codice selezionato negli appunti</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Copia il codice selezionato negli appunti</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Incolla il codice dagli appunti</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Aiuto</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Centra Automaticamente</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Traccia Traiettoria</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Ingrandisci</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Riduci</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Mostra Meteo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>Sovrapposizione Meteo NASA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Mappa Base: %1</translation>
    </message>
</context>
<context>
    <name>GroupFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="132"/>
        <source>Add Sub-folder</source>
        <translation>Aggiungi Sotto-cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="134"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Aggiungi una sotto-cartella all'interno di questa cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="142"/>
        <source>Add Group</source>
        <translation>Aggiungi Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="144"/>
        <source>Add a group inside this folder</source>
        <translation>Aggiungi un gruppo all'interno di questa cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="154"/>
        <source>Rename</source>
        <translation>Rinomina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="156"/>
        <source>Rename folder</source>
        <translation>Rinomina cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="164"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="166"/>
        <source>Delete folder</source>
        <translation>Elimina cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="179"/>
        <source>Title</source>
        <translation>Titolo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="180"/>
        <source>Contents</source>
        <translation>Contenuti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="270"/>
        <source>This folder is empty. Use the toolbar to add a group or sub-folder.</source>
        <translation>Questa cartella è vuota. Usa la barra degli strumenti per aggiungere un gruppo o una sotto-cartella.</translation>
    </message>
</context>
<context>
    <name>GroupTemplateMenu</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="43"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="45"/>
        <source>Dataset Container</source>
        <translation>Contenitore di Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="51"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="53"/>
        <source>Data Grid</source>
        <translation>Griglia Dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="59"/>
        <source>Multi-Plot</source>
        <translation>Grafico Multiplo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="61"/>
        <source>Multiple Plot</source>
        <translation>Grafico Multiplo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="67"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="69"/>
        <source>3D Plot</source>
        <translation>Grafico 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="75"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="77"/>
        <source>Accelerometer</source>
        <translation>Accelerometro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="83"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="85"/>
        <source>Gyroscope</source>
        <translation>Giroscopio</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="91"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="93"/>
        <source>GPS Map</source>
        <translation>Mappa GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="99"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="101"/>
        <source>Image View</source>
        <translation>Visualizzatore Immagini</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="107"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="109"/>
        <source>Web View</source>
        <translation>Visualizzatore Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="115"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="117"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="100"/>
        <source>Pro features detected in this project.</source>
        <translation>Funzionalità Pro rilevate in questo progetto.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="102"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Widget di fallback in uso. Acquista una licenza per sbloccare la funzionalità completa.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="132"/>
        <source>Add Dataset</source>
        <translation>Aggiungi Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="142"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Aggiungi un dataset generico al gruppo corrente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="149"/>
        <source>Plot</source>
        <translation>Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="153"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>Aggiungi un grafico 2D per visualizzare dati numerici</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="161"/>
        <source>FFT Plot</source>
        <translation>Grafico FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="166"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>Aggiungi un grafico FFT per la visualizzazione nel dominio delle frequenze</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="172"/>
        <source>Waterfall</source>
        <translation>Waterfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="177"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Aggiungi un grafico a cascata (spettrogramma) — utilizza le impostazioni FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="183"/>
        <source>Bar/Level</source>
        <translation>Barra/livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="187"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Aggiungi un indicatore a barra o livello per valori scalati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="198"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Aggiungi un widget indicatore per la visualizzazione in stile analogico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="205"/>
        <source>Compass</source>
        <translation>Bussola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="209"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Aggiungi una bussola per visualizzare dati direzionali o angolari</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="215"/>
        <source>Meter</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="219"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Aggiungi un widget indicatore analogico (arco semicircolare)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometro</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Aggiungi un widget termometro per i dati di temperatura</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="226"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="231"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>Aggiungi un indicatore LED per segnali di stato binari</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="244"/>
        <source>Add Control</source>
        <translation>Aggiungi Controllo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="250"/>
        <source>Button</source>
        <translation>Pulsante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="254"/>
        <source>Add a button that sends a command on click</source>
        <translation>Aggiungi un pulsante che invia un comando al clic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="260"/>
        <source>Slider</source>
        <translation>Cursore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="264"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Aggiungi un cursore per inviare valori numerici scalati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="270"/>
        <source>Toggle</source>
        <translation>Interruttore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="274"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Aggiungi un interruttore per comandi on/off</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Text Field</source>
        <translation>Campo di Testo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="284"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Aggiungi un campo di testo per digitare e inviare comandi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="290"/>
        <source>Knob</source>
        <translation>Manopola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="294"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Aggiungi una manopola rotativa per il controllo del setpoint</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="313"/>
        <source>Edit Code</source>
        <translation>Modifica Codice</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="317"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Modifica il JavaScript che disegna questo widget painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="330"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="334"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Duplica il gruppo corrente e il suo contenuto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="339"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="344"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Elimina il gruppo corrente e tutti i dataset contenuti</translation>
    </message>
</context>
<context>
    <name>GroupsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="33"/>
        <source>Dashboard Widgets</source>
        <translation>Widget Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="127"/>
        <source>Add Folder</source>
        <translation>Aggiungi Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="129"/>
        <source>Add a top-level folder</source>
        <translation>Aggiungi una cartella di primo livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="137"/>
        <source>Add Group</source>
        <translation>Aggiungi Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="139"/>
        <source>Add a group from a template</source>
        <translation>Aggiungi un gruppo da un modello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="154"/>
        <source>Title</source>
        <translation>Titolo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="155"/>
        <source>Contents</source>
        <translation>Contenuti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="245"/>
        <source>No groups yet. Use the toolbar to add a group or a folder.</source>
        <translation>Nessun gruppo ancora. Usa la barra degli strumenti per aggiungere un gruppo o una cartella.</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="410"/>
        <source>ROLL ↔</source>
        <translation>ROLLIO ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="440"/>
        <source>YAW ↻</source>
        <translation>IMBARDATA ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="470"/>
        <source>PITCH ↕</source>
        <translation>BECCHEGGIO ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>Dispositivo HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Pagina di Utilizzo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Utilizzo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Collega gamepad, joystick, volanti, controller di volo e altri dispositivi USB di classe HID.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>Tabelle di Utilizzo HID (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="32"/>
        <source>Help Center</source>
        <translation>Centro Assistenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="94"/>
        <source>Fetching help pages…</source>
        <translation>Recupero pagine di assistenza…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="128"/>
        <source>Search…</source>
        <translation>Cerca…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="246"/>
        <source>Loading…</source>
        <translation>Caricamento…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="290"/>
        <source>Select a page from the sidebar</source>
        <translation>Seleziona una pagina dalla barra laterale</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="320"/>
        <source>Copied to Clipboard</source>
        <translation>Copiato negli Appunti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="352"/>
        <source>View Online</source>
        <translation>Visualizza Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="371"/>
        <source>%1 pages</source>
        <translation>%1 pagine</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="380"/>
        <source>Close</source>
        <translation>Chiudi</translation>
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
        <translation>Socket di Rete</translation>
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
        <translation>Bus CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>USB Device</source>
        <translation>Dispositivo USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>HID Device</source>
        <translation>Dispositivo HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>Process</source>
        <translation>Processo</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>MQTT Subscriber</source>
        <translation>Sottoscrittore MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="685"/>
        <source>Your trial period has ended.</source>
        <translation>Il periodo di prova è terminato.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="686"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Per continuare a usare Serial Studio, attivare la licenza.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="752"/>
        <source>channels</source>
        <translation>canali</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="805"/>
        <source> channels</source>
        <translation>canali</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="953"/>
        <source>Unsigned 8-bit</source>
        <translation>Senza Segno 8-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="954"/>
        <source>Signed 16-bit</source>
        <translation>Con Segno 16-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="955"/>
        <source>Signed 24-bit</source>
        <translation>Con Segno 24-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="956"/>
        <source>Signed 32-bit</source>
        <translation>Con Segno 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="957"/>
        <source>Float 32-bit</source>
        <translation>Virgola Mobile 32-bit</translation>
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
        <translation>Dispositivo di Ingresso</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1350"/>
        <source>Sample Rate</source>
        <translation>Frequenza di Campionamento</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1358"/>
        <source>Sample Format</source>
        <translation>Formato Campione</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1366"/>
        <source>Channels</source>
        <translation>Canali</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>Errore Modulo I/O BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>Seleziona Dispositivo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>Seleziona Servizio</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>Seleziona Caratteristica</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>Errore durante la configurazione del servizio BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>Errore operazione</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>Errore scrittura caratteristica</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>Errore scrittura descrittore</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>Errore sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>Errore di lettura della caratteristica</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>Errore di lettura del descrittore</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>Dispositivo BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>Servizio</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>Notifica Caratteristica</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>Caratteristica</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::CANBus</name>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="318"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="324"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="330"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="335"/>
        <source>CAN Bus Not Available</source>
        <translation>Bus CAN Non Disponibile</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="331"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>Nessun plugin del bus CAN trovato su questo sistema.

Il supporto del bus CAN su macOS è limitato e potrebbe richiedere driver hardware di terze parti.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="336"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Nessun plugin del bus CAN disponibile su questa piattaforma.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="348"/>
        <source>Invalid CAN Configuration</source>
        <translation>Configurazione CAN Non Valida</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="356"/>
        <source>Invalid Selection</source>
        <translation>Selezione Non Valida</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="365"/>
        <source>No Devices Available</source>
        <translation>Nessun Dispositivo Disponibile</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="274"/>
        <source>CAN Device Creation Failed</source>
        <translation>Creazione Dispositivo CAN Fallita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="301"/>
        <source>CAN Connection Failed</source>
        <translation>Connessione CAN Fallita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="319"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>Nessun plugin CAN bus trovato su questo sistema.

Su Linux, assicurarsi che i moduli kernel SOCKETCAN siano caricati.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="325"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>Nessun plugin CAN bus trovato su questo sistema.

Su Windows, installa i driver hardware CAN (PEAK, VECTOR, ecc.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="349"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>La configurazione del CAN bus è incompleta. Selezionare un plugin e un'interfaccia validi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="357"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>Il plugin o l'interfaccia selezionati non sono più disponibili. Aggiornare gli elenchi e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="366"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>L'elenco dei plugin o delle interfacce è vuoto. Aggiornare gli elenchi e verificare che l'hardware CAN sia collegato.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="275"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>Impossibile creare il dispositivo CAN bus. Verificare l'hardware e i driver.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Impossibile connettersi al dispositivo CAN bus. Verificare la connessione hardware e le impostazioni.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="695"/>
        <source>CAN Bus Error</source>
        <translation>Errore Bus CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="696"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>Si è verificato un errore ma il dispositivo CAN non è più disponibile.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="703"/>
        <source>Error code: %1</source>
        <translation>Codice errore: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="706"/>
        <source>CAN Bus Communication Error</source>
        <translation>Errore di Comunicazione Bus CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="785"/>
        <source>No CAN driver selected</source>
        <translation>Nessun driver CAN selezionato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="725"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>Caricare prima i moduli kernel SOCKETCAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="721"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>Collegare un adattatore %1, quindi aggiornare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="728"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Configurare prima un'interfaccia CAN virtuale</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="730"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="750"/>
        <source>No interfaces found for %1</source>
        <translation>Nessuna interfaccia trovata per %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="734"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>Installare i &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;driver PEAK CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="738"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>Installare i &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;driver VECTOR CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="742"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>Installa i &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;driver CAN SysTec&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="745"/>
        <source>Install %1 drivers</source>
        <translation>Installa i driver %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="748"/>
        <source>Install %1 drivers for macOS</source>
        <translation>Installa i driver %1 per macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="762"/>
        <source>

If the interface is down, bring it up first:
sudo ip link set %1 up type can bitrate %2</source>
        <translation>Se l'interfaccia è inattiva, attivarla prima:
sudo ip link set %1 up type can bitrate %2

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="869"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="877"/>
        <source>Interface</source>
        <translation>Interfaccia</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="885"/>
        <source>Bitrate</source>
        <translation>Bitrate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="894"/>
        <source>CAN FD</source>
        <translation>CAN FD</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="901"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="908"/>
        <source>Listen-Only</source>
        <translation>Solo Ascolto</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="382"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>Inizializzazione di libusb per l'adattatore CANable non riuscita.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="410"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>Impossibile enumerare i dispositivi USB.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="430"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>L'adattatore CANable selezionato non è più connesso, oppure un'altra applicazione lo sta utilizzando. Su Windows il dispositivo deve utilizzare il driver WinUSB (candleLight lo installa automaticamente).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="443"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>Impossibile acquisire l'interfaccia USB CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="510"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>L'adattatore CANable non è aperto in scrittura.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="545"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>Impossibile trasmettere la trama CAN all'adattatore.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="561"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>Errore Bus CAN segnalato dall'adattatore CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="616"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>Una trama CAN non è stata riconosciuta sul bus.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="715"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>L'adattatore CANable ha rifiutato l'handshake del formato host.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="722"/>
        <source>Could not read CANable timing constants.</source>
        <translation>Impossibile leggere le costanti di temporizzazione CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="728"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>Il bitrate %1 bps non è supportato da questo adattatore CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="734"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>L'adattatore CANable ha rifiutato il bitrate richiesto.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="748"/>
        <source>Could not start the CANable channel.</source>
        <translation>Impossibile avviare il canale CANable.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>Errore sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Verifica che l'utente sia nel gruppo 'plugdev' o che una regola udev conceda l'accesso a questo dispositivo.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>Impossibile aprire "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>Errore Dispositivo HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>Il dispositivo HID è stato disconnesso o ha riscontrato un errore di lettura fatale.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>Seleziona Dispositivo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>Dispositivo HID</translation>
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
        <translation>TLS 1.3 o Successivo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 o Successivo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>Qualsiasi Protocollo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>Solo Protocolli Sicuri</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>Nessuno</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>Interroga Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>Verifica Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>Verifica Automatica Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>La Funzionalità MQTT Richiede una Licenza Commerciale</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">La sottoscrizione a un broker MQTT è disponibile solo con una licenza commerciale Serial Studio valida (livello Hobbyist o superiore).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>La sottoscrizione a un broker MQTT è disponibile solo con una licenza Serial Studio valida o una prova attiva.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>Usa Database di Sistema</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>Carica da Cartella…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Seleziona Directory Certificati PEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>Hostname</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>Filtro Topic</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>Nome Utente</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>Password</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>Versione MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Sessione Pulita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Keep Alive Automatico</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS Abilitato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>Protocollo SSL</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>Modalità Verifica Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>Profondità Verifica Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>Errore Sottoscrizione MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Impossibile sottoscrivere il topic "%1".</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Versione Protocollo MQTT Non Valida</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>Il broker ha rifiutato la versione del protocollo MQTT configurata.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>ID Client Rifiutato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>Il broker ha rifiutato l'ID client. Provare un identificatore diverso.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>Server MQTT Non Disponibile</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>Il broker non è attualmente disponibile. Riprovare più tardi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>Errore di Autenticazione</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>Le credenziali fornite sono state rifiutate dal broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>Errore di Autorizzazione</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>L'account non dispone dei permessi per questa operazione.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>Errore di Rete o Trasporto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Problema a livello di rete/trasporto durante la connessione al broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>Violazione del Protocollo MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>Il broker ha segnalato una violazione del protocollo e ha chiuso la connessione.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>Errore MQTT 5</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>Si è verificato un errore a livello di protocollo MQTT 5.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>Errore MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>Si è verificato un errore MQTT imprevisto.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>Porta Seriale Non Valida</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Inizializzazione Modbus Fallita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Connessione Modbus Fallita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>La porta seriale selezionata "%1" non è più disponibile. Aggiornare l'elenco delle porte e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Impossibile creare il dispositivo Modbus. Verificare la configurazione di sistema e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Impossibile connettersi a "%1". Verificare le impostazioni di connessione.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="609"/>
        <source>None</source>
        <translation>Nessuno</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="610"/>
        <source>Even</source>
        <translation>Pari</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="611"/>
        <source>Odd</source>
        <translation>Dispari</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="612"/>
        <source>Space</source>
        <translation>Spazio</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="613"/>
        <source>Mark</source>
        <translation>Marcatura</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="665"/>
        <source>Holding Registers (0x03)</source>
        <translation>Holding Register (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="666"/>
        <source>Input Registers (0x04)</source>
        <translation>Input Register (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="667"/>
        <source>Coils (0x01)</source>
        <translation>Coil (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="668"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Discrete Input (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>No register groups configured</source>
        <translation>Nessun gruppo di registri configurato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="851"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Aggiungere almeno un gruppo di registri prima di generare un progetto.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="865"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="890"/>
        <source>Modbus Project Generator</source>
        <translation>Generatore di Progetti Modbus</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Impossibile creare il file di progetto temporaneo</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Verificare i permessi di scrittura nella directory temporanea.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="862"/>
        <source>Failed to load generated project</source>
        <translation>Impossibile caricare il progetto generato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="863"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>Il JSON del progetto generato non può essere caricato.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="885"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Progetto generato con successo con %1 gruppi e %2 dataset.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="888"/>
        <source>The project editor is now open for customization.</source>
        <translation>L'editor del progetto è ora aperto per la personalizzazione.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="903"/>
        <source>Modbus Project</source>
        <translation>Progetto Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="928"/>
        <source>Holding Registers</source>
        <translation>Holding Register</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="929"/>
        <source>Input Registers</source>
        <translation>Registri di Ingresso</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="930"/>
        <source>Coils</source>
        <translation>Bobine</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="931"/>
        <source>Discrete Inputs</source>
        <translation>Ingressi Discreti</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="945"/>
        <source>Unknown</source>
        <translation>Sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="958"/>
        <source>Register %1</source>
        <translation>Registro %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Coil %1</source>
        <translation>Bobina %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Discrete %1</source>
        <translation>Discreto %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1372"/>
        <source>Error code: %1</source>
        <translation>Codice errore: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1375"/>
        <source>Modbus Communication Error</source>
        <translation>Errore di Comunicazione Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1387"/>
        <source>Select Port</source>
        <translation>Seleziona Porta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1540"/>
        <source>Protocol</source>
        <translation>Protocollo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1548"/>
        <source>Slave Address</source>
        <translation>Indirizzo Slave</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1557"/>
        <source>Poll Interval (ms)</source>
        <translation>Intervallo di Polling (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1597"/>
        <source>Host / IP</source>
        <translation>Host / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1604"/>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1619"/>
        <source>Serial Port</source>
        <translation>Porta Seriale</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1627"/>
        <source>Baud Rate</source>
        <translation>Baud Rate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1635"/>
        <source>Parity</source>
        <translation>Parità</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1643"/>
        <source>Data Bits</source>
        <translation>Bit di Dati</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1651"/>
        <source>Stop Bits</source>
        <translation>Bit di Stop</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="547"/>
        <source>Network socket error</source>
        <translation>Errore socket di rete</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="563"/>
        <source>Socket Type</source>
        <translation>Tipo di Socket</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="571"/>
        <source>Remote Address</source>
        <translation>Indirizzo Remoto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="579"/>
        <source>TCP Port</source>
        <translation>Porta TCP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="588"/>
        <source>UDP Local Port</source>
        <translation>Porta Locale UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="597"/>
        <source>UDP Remote Port</source>
        <translation>Porta Remota UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="606"/>
        <source>UDP Multicast</source>
        <translation>Multicast UDP</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="183"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="224"/>
        <source>Failed to start process</source>
        <translation>Impossibile avviare il processo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Eseguibile "%1" non trovato in PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>Seleziona Eseguibile</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>Seleziona Directory di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Seleziona Named Pipe / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>Il processo è andato in crash.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>Codice di uscita: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>Processo "%1" arrestato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>Errore sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>Errore Processo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>Errore Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>Impossibile aprire named pipe: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>Modalità</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>Avvia Processo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>Eseguibile</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>Argomenti</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>Directory di Lavoro</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>Percorso Pipe</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="206"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>Il bitrate %1 bps non è supportato dall'Analizzatore USB-CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="216"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Impossibile aprire la porta seriale %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="227"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>Inizializzazione dell'Analizzatore USB-CAN non riuscita.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="264"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>L'Analizzatore USB-CAN non è aperto in scrittura.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="310"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>Errore bus CAN segnalato dall'Analizzatore USB-CAN.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="167"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>Il bitrate %1 bps non è una velocità slcan standard.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="177"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Impossibile aprire la porta seriale %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="190"/>
        <source>Failed to open the slcan channel.</source>
        <translation>Apertura del canale slcan non riuscita.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="229"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>L'adattatore slcan non è aperto in scrittura.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="267"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>Errore bus CAN segnalato dall'adattatore slcan.</translation>
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
        <translation>Nessuno</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="349"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="733"/>
        <source>Select Port</source>
        <translation>Seleziona Porta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="392"/>
        <source>Even</source>
        <translation>Pari</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="393"/>
        <source>Odd</source>
        <translation>Dispari</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="394"/>
        <source>Space</source>
        <translation>Spazio</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="395"/>
        <source>Mark</source>
        <translation>Mark</translation>
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
        <translation>"%1" non è un percorso valido</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="563"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Digitare un altro percorso per registrare un dispositivo seriale personalizzato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="823"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Il dispositivo specificato non è stato trovato. Verificare la connessione e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="830"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>Si è verificato un errore sconosciuto. Verificare il dispositivo e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="832"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Il dispositivo non è aperto. Aprire il dispositivo prima di tentare questa operazione.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="261"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Impossibile connettersi alla porta seriale "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="798"/>
        <source>Unknown</source>
        <translation>Sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="799"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Errore critico sulla porta seriale "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="800"/>
        <source>Unknown error</source>
        <translation>Errore sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="822"/>
        <source>No error occurred.</source>
        <translation>Nessun errore rilevato.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="824"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Permesso negato. Verificare che l'applicazione disponga dei diritti di accesso necessari al dispositivo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="825"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Impossibile aprire il dispositivo. Potrebbe essere già in uso o non disponibile.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="826"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>Errore durante la scrittura dei dati sul dispositivo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>Errore durante la lettura dei dati dal dispositivo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>Errore critico di risorsa. Il dispositivo potrebbe essere stato disconnesso o non è più accessibile.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>L'operazione richiesta non è supportata su questo dispositivo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="831"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>L'operazione è scaduta. Il dispositivo potrebbe non rispondere.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Serial Port</source>
        <translation>Porta Seriale</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Baud Rate</source>
        <translation>Velocità in Baud</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Parity</source>
        <translation>Parità</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Data Bits</source>
        <translation>Bit di Dati</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>Stop Bits</source>
        <translation>Bit di Stop</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1034"/>
        <source>Flow Control</source>
        <translation>Controllo di Flusso</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1042"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1049"/>
        <source>Auto-Reconnect</source>
        <translation>Riconnessione Automatica</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="156"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="164"/>
        <source>USB Error</source>
        <translation>Errore USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="157"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Inizializzazione del sottosistema USB non riuscita. Verificare che libusb sia disponibile sul sistema.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="214"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="572"/>
        <source>USB Device Error</source>
        <translation>Errore Dispositivo USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>Impossibile aprire il dispositivo USB: %1.

Su Linux, assicurarsi di avere i permessi di lettura/scrittura sul nodo del dispositivo (aggiungere una regola udev o eseguire come root). Su macOS, potrebbe essere necessario scollegare prima il driver del kernel.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="165"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Nessun dispositivo USB selezionato. Selezionare un dispositivo e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="171"/>
        <source>Unknown Device</source>
        <translation>Dispositivo Sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <source>Failed to open "%1"</source>
        <translation>Apertura di "%1" non riuscita</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="215"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Impossibile rivendicare l'interfaccia %1 sul dispositivo USB.

Un altro driver o applicazione potrebbe averla già aperta. Su Linux, provare a scaricare il driver del kernel (es. cdc_acm) o aggiungere una regola udev.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="360"/>
        <source>Select Device</source>
        <translation>Seleziona Dispositivo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="379"/>
        <source>Select IN Endpoint</source>
        <translation>Seleziona Endpoint IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="390"/>
        <source>None (Read-only)</source>
        <translation>Nessuno (Sola Lettura)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="464"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Abilitare i Trasferimenti di Controllo USB Avanzati?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="465"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Questo abilita i trasferimenti di controllo oltre ai trasferimenti bulk. L'invio di richieste di controllo errate può bloccare o danneggiare l'hardware connesso. Abilitare solo se si sa cosa si sta facendo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="469"/>
        <source>Advanced USB Mode</source>
        <translation>Modalità USB Avanzata</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="573"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>Il dispositivo USB è stato disconnesso o ha riscontrato un errore di lettura fatale.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="710"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Nessun endpoint IN isocrono è stato trovato su questo dispositivo, ma sono disponibili endpoint bulk.

Cambiare la Modalità di Trasferimento in "Flusso Bulk" e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="715"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Nessun endpoint IN bulk è stato trovato su questo dispositivo, ma sono disponibili endpoint isocroni.

Cambiare la Modalità di Trasferimento in "Isocrono" e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="719"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Nessun endpoint IN utilizzabile è stato trovato su questo dispositivo.

Il dispositivo potrebbe non esporre endpoint dati nella sua configurazione attiva, oppure potrebbe richiedere un driver specifico.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1211"/>
        <source>USB Device</source>
        <translation>Dispositivo USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1219"/>
        <source>Transfer Mode</source>
        <translation>Modalità di Trasferimento</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Bulk Stream</source>
        <translation>Flusso Bulk</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Advanced Control</source>
        <translation>Controllo Avanzato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Isochronous</source>
        <translation>Isocrono</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1227"/>
        <source>IN Endpoint</source>
        <translation>Endpoint IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1235"/>
        <source>OUT Endpoint</source>
        <translation>Endpoint OUT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1243"/>
        <source>ISO Packet Size</source>
        <translation>Dimensione Pacchetto ISO</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>Nessun file selezionato…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>Testo Semplice</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Binario Raw</translation>
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
        <translation>Seleziona file da trasmettere</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>File selezionato: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>Errore apertura file: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>Avvio trasferimento %1…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>Trasmissione completata</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>Trasmissione testo semplice completata</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Trasmissione binaria raw completata (%1 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>Trasferimento completato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Trasferimento completato con successo (%1 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>Trasferimento fallito: %1</translation>
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
        <translation>Frame scartati</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>I dati in arrivo sono più veloci di quanto Serial Studio possa elaborarli; %1 frame sono stati scartati. Ridurre la velocità dei dati o disabilitare un consumatore pesante.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>Impossibile aprire il file: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>Trasferimento annullato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>Trasferimento annullato dall'utente</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>In attesa del ricevitore…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Ricevitore pronto (modalità CRC), invio dati…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Troppi tentativi, trasferimento interrotto</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>Numero massimo di tentativi superato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>NAK ricevuto, nuovo tentativo blocco %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>Impossibile eseguire seek nel file</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Trasferimento annullato dal ricevitore</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Il ricevitore ha annullato il trasferimento</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>Trasferimento completato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>La lettura del file ha restituito più dati del richiesto</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Invio blocco %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>Invio EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Timeout, nuovo tentativo (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>Trasferimento scaduto</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>Timeout: nessuna risposta dal ricevitore</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>Impossibile aprire il file: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Trasferimento annullato dal ricevitore</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Il ricevitore ha annullato il trasferimento</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>In attesa del ricevitore…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>Invio primo EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Troppi tentativi, trasferimento interrotto</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>Numero massimo di tentativi superato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>NAK ricevuto, nuovo tentativo blocco %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>Impossibile posizionarsi nel file</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>Invio secondo EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>Invio marcatore fine batch…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>Trasferimento completato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Invio intestazione file: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Invio blocco %1 (%2/%3 byte)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>Impossibile aprire il file: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Il file è troppo grande per ZMODEM (%1 byte, limite 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>Trasferimento annullato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>Trasferimento annullato dall'utente</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>Mancata corrispondenza CRC intestazione esadecimale, frame scartato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Invio informazioni file: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>Impossibile posizionarsi all'offset %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>La lettura del file ha restituito più dati del richiesto</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Il ricevitore richiede dati dall'offset %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>Il ricevitore ha saltato il file</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>Troppi errori, trasferimento interrotto</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>Numero massimo di tentativi superato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>Invio ZRQINIT, in attesa del ricevitore…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Dati file inviati, in attesa di conferma…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>Invio ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Ricevitore pronto, invio informazioni file…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>NAK ricevuto, nuovo tentativo (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>Trasferimento completato</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Trasferimento annullato dal ricevitore</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Il ricevitore ha annullato il trasferimento</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>Il ricevitore ha segnalato un errore file</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>Timeout trasferimento</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>Timeout: nessuna risposta dal ricevitore</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Timeout, nuovo tentativo (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="41"/>
        <source>Select Icon</source>
        <translation>Seleziona Icona</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="118"/>
        <source>Search Online…</source>
        <translation>Cerca Online…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="131"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="143"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>Normale</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>Scala di Grigi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Alto Contrasto</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Vivido</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Visione Notturna</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Infrarosso</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Blu Profondo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Ambra</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Esporta Immagini</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Apri Cartella di Esportazione</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Ingrandisci</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Riduci</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Mostra Mirino</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>In Attesa di Immagine…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>Chiavi API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. Il modello predefinito è Claude Haiku 4.5 ($1 input / $5 output per milione di token). Sono disponibili anche Sonnet 4.6 e OPUS 4.7. Supporta streaming, uso di strumenti, ragionamento esteso e caching dei prompt.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. Il modello predefinito è GPT-4o mini ($0,15 input / $0,60 output per milione di token). Sono disponibili anche GPT-4o, GPT-4 Turbo e o1-mini.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. Il modello predefinito è Gemini 2.0 Flash, che offre un livello gratuito generoso (soggetto a limiti di frequenza). Sono disponibili anche Gemini 1.5 Pro e Gemini 1.5 Flash.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Utilizza le tue chiavi API. Vengono crittografate a riposo con una chiave specifica per la macchina e non lasciano mai il tuo computer se non per comunicare con il provider selezionato.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>Chiave impostata</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>Nessuna chiave</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">Una chiave è registrata — incollane una nuova per sostituirla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. L'impostazione predefinita è GPT-5 mini per lavoro agente veloce ed economico. GPT-5.2 è l'opzione generale più potente, e GPT-5.2 Chat segue il modello attualmente usato in ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. API compatibile con OpenAI. L'impostazione predefinita è deepseek-chat (V3). È disponibile anche deepseek-reasoner (R1). Spesso l'opzione cloud più economica per l'uso di strumenti.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. Una chiave, ~200 modelli da Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen e altri. I modelli gratuiti (con suffisso :free) hanno limiti di velocità ma non richiedono fatturazione aggiuntiva. Pagamento a consumo per gli altri.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Inferenza accelerata via hardware (LPU) per modelli Llama, Mixtral, Gemma, DeepSeek-R1 distill e Qwen molto veloci. Piano gratuito generoso con limiti giornalieri di token.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. Il predefinito è Mistral Large. Codestral è destinato al completamento del codice, Pixtral gestisce la visione e i modelli Ministral sono ottimizzati per uso edge / bassa latenza.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Server di modelli locale. Funziona con qualsiasi endpoint compatibile con OpenAI -- Ollama, llama-server di llama.cpp, LM Studio o vLLM. Nulla esce dalla tua macchina. L'elenco dei modelli viene interrogato in tempo reale dal server.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>Una chiave è registrata -- incollane una nuova per sostituirla</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>Incolla la tua chiave API qui</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>Nascondi chiave</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>Mostra chiave durante la digitazione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>Ottieni chiave</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Apri la console del provider per creare una nuova chiave</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>Salva</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>Rimuovi la chiave memorizzata per %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Installa Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Apri la pagina di download di Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>Applica</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>Richiedi nuovamente l'elenco dei modelli</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Nessuna chiave API configurata. Aggiungi una chiave per iniziare.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">Nessuna chiave API configurata. Aggiungine almeno una sopra per iniziare.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>Un provider è pronto.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 provider sono pronti.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Licenze</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Attendere…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Attiva Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Incolla la tua chiave di licenza qui sotto per sbloccare le funzionalità Pro come MQTT, grafici 3D e altro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>La tua licenza include 5 attivazioni dispositivo.
I piani includono opzioni Mensile, Annuale e Lifetime.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Incolla la tua chiave di licenza qui…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="333"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="382"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Incolla</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="339"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="388"/>
        <source>Select All</source>
        <translation>Seleziona Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="235"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="426"/>
        <source>Product</source>
        <translation>Prodotto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="242"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="253"/>
        <source>Licensee</source>
        <translation>Licenziatario</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="272"/>
        <source>Licensee E-Mail</source>
        <translation>E-mail Licenziatario</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="289"/>
        <source>Device Usage</source>
        <translation>Utilizzo Dispositivi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 dispositivi in uso (Piano Illimitato)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="298"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 di %2 dispositivi utilizzati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="308"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="443"/>
        <source>Device ID</source>
        <translation>ID Dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="355"/>
        <source>License Key</source>
        <translation>Chiave di Licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="433"/>
        <source>Serial Studio %1 (Offline)</source>
        <translation>Serial Studio %1 (Offline)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="460"/>
        <source>Expires</source>
        <translation>Scade</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="467"/>
        <source>%1 (%2 days left)</source>
        <translation>%1 (%2 giorni rimanenti)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="480"/>
        <source>Your offline license expires soon. Request a new certificate to stay activated.</source>
        <translation>La licenza offline scade a breve. Richiedere un nuovo certificato per rimanere attivati.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="497"/>
        <source>Customer Portal</source>
        <translation>Portale Cliente</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="508"/>
        <source>Buy License</source>
        <translation>Acquista Licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="515"/>
        <source>Activate</source>
        <translation>Attiva</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="526"/>
        <source>Activate Offline…</source>
        <translation>Attiva Offline…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="535"/>
        <source>Deactivate</source>
        <translation>Disattiva</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="512"/>
        <source>There was an issue validating your license.</source>
        <translation>Si è verificato un problema durante la convalida della licenza.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="530"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="711"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="830"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>La chiave di licenza fornita non appartiene a Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="531"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Verificare di aver acquistato la licenza dallo store ufficiale di Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="542"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="720"/>
        <source>This license key was activated on a different device.</source>
        <translation>Questa chiave di licenza è stata attivata su un dispositivo diverso.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="543"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="721"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Disattivarla prima su quel dispositivo o contattare il supporto per assistenza.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="554"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="731"/>
        <source>This license is not currently active.</source>
        <translation>Questa licenza non è attualmente attiva.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="555"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="732"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Potrebbe essere scaduta o disattivata (stato: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="565"/>
        <source>Something went wrong on the server.</source>
        <translation>Si è verificato un errore sul server.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="566"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="742"/>
        <source>No activation ID was returned.</source>
        <translation>Nessun ID di attivazione restituito.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="576"/>
        <source>Could not validate your license at this time.</source>
        <translation>Impossibile convalidare la licenza in questo momento.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="577"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="751"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="840"/>
        <source>Try again later.</source>
        <translation>Riprovare più tardi.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="712"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="831"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Verificare di aver acquistato la licenza dallo store ufficiale di Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="741"/>
        <source>Something went wrong on the server…</source>
        <translation>Si è verificato un problema sul server…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="595"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="597"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="648"/>
        <source>Your license has been successfully activated.</source>
        <translation>La licenza è stata attivata con successo.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="649"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Grazie per il supporto a Serial Studio!
Ora si ha accesso a tutte le funzionalità premium.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="703"/>
        <source>There was an issue activating your license.</source>
        <translation>Si è verificato un problema durante l'attivazione della licenza.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="750"/>
        <source>Could not activate your license at this time.</source>
        <translation>Impossibile attivare la licenza in questo momento.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="822"/>
        <source>There was an issue deactivating your license.</source>
        <translation>Si è verificato un problema durante la disattivazione della licenza.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="839"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>Impossibile disattivare la licenza in questo momento.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="849"/>
        <source>Your license has been deactivated.</source>
        <translation>La licenza è stata disattivata.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="850"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>L'accesso alle funzionalità Pro è stato rimosso.
Grazie ancora per aver supportato Serial Studio!</translation>
    </message>
</context>
<context>
    <name>Licensing::OfflineLicense</name>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="183"/>
        <source>Could not open the certificate file.</source>
        <translation>Impossibile aprire il file del certificato.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="190"/>
        <source>The certificate file is too large to be valid.</source>
        <translation>Il file del certificato è troppo grande per essere valido.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="217"/>
        <source>Could not write the device file.</source>
        <translation>Impossibile scrivere il file del dispositivo.</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>L'Esportazione MDF4 è una funzionalità Pro.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Questa funzionalità richiede una licenza. Acquistarne una per abilitare l'esportazione MDF4.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>Seleziona file MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>File MDF4 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>Disconnettere dal dispositivo?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>È necessario disconnettersi dal dispositivo corrente prima di aprire un file MDF4.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>Impossibile aprire il file MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Il file potrebbe essere danneggiato o in un formato non supportato.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>File MDF4 Non Valido</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Impossibile leggere la struttura del file. Il file potrebbe essere danneggiato.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>Nessun Dato nel File</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>Il file MDF4 non contiene dati di misurazione.</translation>
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
        <translation>es. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Filtro Topic</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>es. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Rigenera</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Nome Utente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Password</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>Versione</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Sessione Pulita</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>Usa SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>Protocollo SSL</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Verifica Peer</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Profondità Verifica</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>Certificati CA</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Carica da Cartella…</translation>
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
        <translation type="vanished">TLS 1.3 o Successivo</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 o Successivo</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">Qualsiasi Protocollo</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">Solo Protocolli Sicuri</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Nessuno</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">Interroga Peer</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">Verifica Peer</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">Verifica Automatica Peer</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">Usa Database di Sistema</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">Sottoscrittore MQTT</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">Publisher MQTT</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">La Funzionalità MQTT Richiede una Licenza Commerciale</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">Carica da Cartella…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">La connessione ai broker MQTT è disponibile solo con una licenza commerciale valida di Serial Studio (livello Hobbyist o superiore).

Per sbloccare questa funzionalità, attiva la tua licenza o visita lo store.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">Topic MQTT Mancante</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">È necessario specificare un topic prima di connettersi come publisher.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">Errore di Configurazione</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">Topic MQTT Non Impostato</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">Non riceverai alcun messaggio finché non viene configurato un topic.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">Avviso di Configurazione</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">Topic MQTT Non Valido</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Il topic "%1" non è valido.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">Seleziona Directory Certificati PEM</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">Errore Sottoscrizione</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Impossibile sottoscrivere il topic "%1".</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">Versione Protocollo MQTT Non Valida</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">Il broker MQTT ha rifiutato la connessione a causa di una versione del protocollo non supportata. Assicurarsi che client e broker supportino la stessa versione del protocollo.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">ID Client Rifiutato</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">Il broker ha rifiutato l'ID client. Potrebbe essere malformato, troppo lungo o già in uso. Provare a utilizzare un ID client diverso.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">Server MQTT Non Disponibile</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">La connessione di rete è stata stabilita, ma il broker non è attualmente disponibile. Verificare lo stato del broker e riprovare più tardi.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">Errore di Autenticazione</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">Il nome utente o la password forniti sono errati o non validi. Verificare le credenziali e riprovare.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">Errore di Autorizzazione</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">Il broker MQTT ha negato la connessione a causa di permessi insufficienti. Assicurarsi che l'account disponga dei diritti di accesso necessari.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">Errore di Rete o Trasporto</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">Si è verificato un problema a livello di rete o trasporto, causando un errore di connessione imprevisto. Verificare la connessione di rete e le impostazioni del broker.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">Violazione del Protocollo MQTT</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">Il client ha rilevato una violazione del protocollo MQTT e ha chiuso la connessione. Verificare la conformità dell'implementazione MQTT.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">Errore Sconosciuto</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">Si è verificato un errore imprevisto. Controllare i log per maggiori dettagli o riavviare l'applicazione.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">Errore MQTT 5</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">Si è verificato un errore di protocollo MQTT livello 5. Controllare i log del broker o i codici di ragione per maggiori dettagli.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">Autenticazione MQTT Fallita</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">Autenticazione fallita: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">È richiesta l'autenticazione estesa, ma MQTT 5.0 non è abilitato.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">Sconosciuto</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">Autenticazione MQTT Richiesta</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">Il broker MQTT richiede l'autenticazione utilizzando il metodo: "%1".

Fornire le credenziali necessarie.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">Inserire Nome Utente MQTT</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">Nome Utente:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">Inserire Password MQTT</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">Password:</translation>
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
        <translation>TLS 1.3 o Successivo</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 o Successivo</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>Qualsiasi Protocollo</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>Solo Protocolli Sicuri</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>Nessuno</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>Interroga Peer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>Verifica Peer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>Verifica Automatica Peer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>Dati RX Grezzi</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>Script Personalizzato</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Dati Dashboard (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Dati Dashboard (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>Publisher MQTT non disponibile</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>È richiesta una licenza commerciale valida per utilizzare la pubblicazione MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>Test Connessione MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Seleziona Directory Certificati PEM</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>Broker MQTT raggiungibile</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>Broker MQTT non raggiungibile</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>Connessione al broker MQTT fallita</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="50"/>
        <source>MQTT Publisher Script</source>
        <translation>Script Publisher MQTT</translation>
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
        <translation>Byte frame di esempio (testo o hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Test</source>
        <translation>Prova</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="100"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Apply</source>
        <translation>Applica</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="103"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="112"/>
        <source>Language:</source>
        <translation>Linguaggio:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="115"/>
        <source>Template:</source>
        <translation>Modello:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="126"/>
        <source>Frame:</source>
        <translation>Frame:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="130"/>
        <source>Output:</source>
        <translation>Uscita:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="270"/>
        <source>Enter a frame</source>
        <translation>Inserisci un frame</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="277"/>
        <source>Invalid hex</source>
        <translation>Esadecimale non valido</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatta Documento	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="361"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatta Selezione	ctrl+i</translation>
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
-- Definire una funzione mqtt(frame) che riceve i byte grezzi
-- di un frame analizzato e restituisce il payload da pubblicare
-- al broker, oppure nil per saltare questo frame.
--
-- L'argomento frame corrisponde a ciò che vede lo script
-- Frame Parser: una stringa Lua contenente i byte tra i
-- delimitatori FrameReader.
--
-- Esempio:
--   function mqtt(frame)
--     return frame    -- inoltra così com'è
--   end
--
-- Usare il menu Template per esempi pronti all'uso.
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
 * Definire una funzione mqtt(frame) che riceve i byte grezzi
 * di un frame analizzato e restituisce il payload da pubblicare
 * al broker, oppure null per saltare questo frame.
 *
 * L'argomento frame corrisponde a ciò che vede lo script
 * Frame Parser: una stringa contenente i byte tra i
 * delimitatori FrameReader.
 *
 * Esempio:
 *   function mqtt(frame) {
 *     return frame;   // inoltra così com'è
 *   }
 *
 * Usare il menu Template per esempi pronti all'uso.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="615"/>
        <source>Script is empty</source>
        <translation>Lo script è vuoto</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="622"/>
        <source>Lua engine error</source>
        <translation>Errore del motore Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="696"/>
        <source>Error: %1</source>
        <translation>Errore: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="652"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() non è definita</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- frame saltato)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="671"/>
        <source>(non-string return)</source>
        <translation>(return non-stringa)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="701"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- frame saltato)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="779"/>
        <source>Select Template…</source>
        <translation>Seleziona Modello…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Configurare hostname e porta del broker prima di testare la connessione.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Connesso con successo a %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Timeout dopo 5 secondi senza raggiungere il broker.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">Configurazione MQTT</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT è una Funzionalità Pro</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">Attivare la licenza o visitare lo store per sbloccare il supporto MQTT.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">Generale</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">Autenticazione</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">Opzioni MQTT</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">Proprietà SSL</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">Host</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">Porta</translation>
    </message>
    <message>
        <source>Client ID</source>
        <translation type="vanished">ID Client</translation>
    </message>
    <message>
        <source>Keep Alive (s)</source>
        <translation type="vanished">Keep Alive (s)</translation>
    </message>
    <message>
        <source>Clean Session</source>
        <translation type="vanished">Sessione Pulita</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">Nome Utente</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">Nome Utente MQTT</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">Password</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">Password MQTT</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">Versione</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">Modalità</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">Topic</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">es. sensors/temperature o home/+/status</translation>
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
        <translation type="vanished">es. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will Message</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">es. Dispositivo disconnesso inaspettatamente</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">Abilita SSL</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">Protocollo SSL</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">Profondità di Verifica</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">Modalità di Verifica</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Chiudi</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">Disconnetti</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">Connetti</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="205"/>
        <source>Console Only Mode</source>
        <translation>Modalità Solo Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="208"/>
        <source>Quick Plot Mode</source>
        <translation>Modalità Grafico Rapido</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="215"/>
        <source>Empty Project</source>
        <translation>Progetto Vuoto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="696"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="704"/>
        <source>Waiting for data…</source>
        <translation>In attesa di dati…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="705"/>
        <source>Connecting to device…</source>
        <translation>Connessione al dispositivo…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Esempio di codice</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Completatore</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Evidenziatore</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Stile</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>spazi</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>Copiato negli Appunti</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="23"/>
        <source>MDF4 Player</source>
        <translation>Lettore MDF4</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Errore</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Tu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Assistente</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Rilevamento</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>Approvare %1 azioni in %2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Queste chiamate verranno eseguite insieme. Espandere ogni scheda per ispezionare gli argomenti.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Approva tutto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Nega tutto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Copia Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Seleziona Tutto</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>Tu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>Assistente</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>Errore</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Scoperta</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>In Esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>In attesa di approvazione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Completato</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Negato</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Bloccato</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Approva</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Nega</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Approva tutto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Nega tutto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Argomenti</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Risultato</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>Approvare {n} azioni in {family}?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Queste chiamate verranno eseguite insieme. Espandere ogni scheda per ispezionare gli argomenti.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>Impossibile caricare README: %1</translation>
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
        <translation>Frame Parser</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>Modello di Progetto</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>Tutti i Tipi</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>Ripristina Estensioni</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Questa operazione disinstalla tutte le estensioni, rimuove tutti i repository personalizzati e ripristina le impostazioni predefinite. Continuare?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>Seleziona Cartella Repository Estensioni</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1017"/>
        <source>Installed (repository no longer available)</source>
        <translation>Installato (repository non più disponibile)</translation>
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
        <translation>Errore Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Il plugin "%1" non è installato.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1335"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>L'estensione "%1" non è un plugin (tipo: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1356"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Impossibile leggere il file di metadati del plugin:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Il plugin "%1" richiede GRPC ma questa build non include il supporto GRPC.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Questo plugin utilizza GRPC per lo streaming di dati ad alte prestazioni. Il server API deve essere abilitato.

Abilitarlo ora?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1393"/>
        <source>API Server Required</source>
        <translation>Server API Richiesto</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1422"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Il plugin "%1" non ha il campo 'entry' in info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Entry point not found:
%1</source>
        <translation>Punto di ingresso non trovato:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Il plugin "%1" ha un percorso del punto di ingresso non valido.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1484"/>
        <source>Missing Dependency</source>
        <translation>Dipendenza Mancante</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Questo plugin richiede "%1" ma non è stato trovato nel sistema.

Aprire la pagina di download?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>I plugin necessitano del server API per comunicare con Serial Studio. Abilitarlo ora?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Riavvio Richiesto</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>Il nuovo backend di rendering avrà effetto dopo il riavvio di Serial Studio. Riavviare ora per applicare la modifica?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>Impossibile caricare la pagina: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="152"/>
        <source>Invalid icon identifier</source>
        <translation>Identificatore icona non valido</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="222"/>
        <source>Empty SVG data received</source>
        <translation>Ricevuti dati SVG vuoti</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Collegamento Windows (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>Applicazione macOS (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Desktop Entry (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Usa un'icona .icns per il risultato più nitido in Finder e nel Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Lascia l'icona vuota per ereditare l'icona dell'eseguibile Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Posiziona il file in ~/.local/share/applications/ per esporlo nel launcher delle applicazioni.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Apple Icon Image (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Icona Windows (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>Immagine Vettoriale o Raster (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>È richiesta una licenza Pro per generare collegamenti.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>Nessun percorso di output fornito.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>Impossibile scrivere il file di collegamento.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Impossibile sostituire il collegamento esistente in %1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Impossibile creare la struttura di directory del bundle .app.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Impossibile scrivere il launcher del bundle: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Impossibile contrassegnare il launcher del bundle come eseguibile.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Impossibile scrivere Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Il writer di scorciatoie Windows non è disponibile su questa piattaforma.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Il writer di scorciatoie Linux non è disponibile su questa piattaforma.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>Impossibile inizializzare COM (richiesto per scrivere scorciatoie .lnk).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) fallito (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) fallito (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Salvataggio del file .lnk fallito (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>Il writer di scorciatoie macOS non è disponibile su questa piattaforma.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Impossibile aprire il percorso della scorciatoia in scrittura: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Scorciatoia Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Impossibile contrassegnare la scorciatoia come eseguibile.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>Sistema</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">Verificare automaticamente gli aggiornamenti?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">%1 deve verificare automaticamente gli aggiornamenti? Puoi sempre verificare gli aggiornamenti manualmente dalla finestra "Informazioni"</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>Salva</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>Salva tutto</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>Apri</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>Sì</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>Sì a tutto</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>No a tutti</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>Interrompi</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>Riprova</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>Ignora</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>Scarta</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>Aiuto</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>Applica</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>Ripristina Predefiniti</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>Seleziona Posizione Workspace</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>Protocollo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="70"/>
        <source>Serial Port</source>
        <translation>Porta Seriale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="93"/>
        <source>Baud Rate</source>
        <translation>Baud Rate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="184"/>
        <source>Parity</source>
        <translation>Parità</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="205"/>
        <source>Data Bits</source>
        <translation>Bit di Dati</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="226"/>
        <source>Stop Bits</source>
        <translation>Bit di Stop</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="247"/>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="257"/>
        <source>IP Address</source>
        <translation>Indirizzo IP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="271"/>
        <source>Port</source>
        <translation>Porta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="280"/>
        <source>TCP Port</source>
        <translation>Porta TCP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="308"/>
        <source>Slave Address</source>
        <translation>Indirizzo Slave</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="313"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="361"/>
        <source>Configure Register Groups…</source>
        <translation>Configura Gruppi di Registri…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="371"/>
        <source>Import Register Map…</source>
        <translation>Importa Mappa Registri…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="386"/>
        <source>%1 group(s) configured</source>
        <translation>%1 gruppo/i configurato/i</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="387"/>
        <source>No groups configured</source>
        <translation>Nessun gruppo configurato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="328"/>
        <source>Poll Interval (ms)</source>
        <translation>Intervallo di Polling (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="333"/>
        <source>Polling interval</source>
        <translation>Intervallo di polling</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Gruppi di Registri Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Configura più gruppi di registri per interrogare diversi tipi di registro in sequenza.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Aggiungi Nuovo Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Tipo di Registro:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Indirizzo Iniziale:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Numero di Registri:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Aggiungi Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Gruppi Configurati</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>Inizio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>Conteggio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Azione</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Rimuovi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Nessun gruppo configurato.
Aggiungi gruppi sopra per interrogare più tipi di registro.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Gruppi totali: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Genera Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Cancella Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Anteprima Mappa Registri Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>File: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Rivedi i registri da importare in un nuovo progetto Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>Registri</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>Nome</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Indirizzo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Tipo di Dato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Unità</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>Nessun registro trovato nel file.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Totale: %1 registri in %2 gruppi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Crea Progetto</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>Connesso al broker</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Non connesso</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Testa Connessione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Verifica il broker con le impostazioni correnti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Abilitare prima la pubblicazione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Modifica Script</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Modifica lo script di pubblicazione (Lua o JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Carica Certificati CA</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>Aggiungi certificati PEM da una cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>Abilitare prima SSL/TLS</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpola</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="305"/>
        <source>Interpolation: %1</source>
        <translation>Interpolazione: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Mostra Legende</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="327"/>
        <source>Show X Axis Label</source>
        <translation>Mostra Etichetta Asse X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="338"/>
        <source>Show Y Axis Label</source>
        <translation>Mostra Etichetta Asse Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="350"/>
        <source>Show Crosshair</source>
        <translation>Mostra Mirino</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Resume</source>
        <translation>Riprendi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="374"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Modalità Sweep / Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="386"/>
        <source>Trigger Settings</source>
        <translation>Impostazioni Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="410"/>
        <source>Reset View</source>
        <translation>Ripristina Vista</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="416"/>
        <source>Axis Range Settings</source>
        <translation>Impostazioni Intervallo Assi</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Campioni</translation>
    </message>
</context>
<context>
    <name>MultiSelectionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="35"/>
        <source>%1 items selected</source>
        <translation>%1 elementi selezionati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="118"/>
        <source>Plots</source>
        <translation>Grafici</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="123"/>
        <source>Plot</source>
        <translation>Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="127"/>
        <source>Toggle 2D plot for every selected dataset</source>
        <translation>Attiva/disattiva grafico 2D per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="139"/>
        <source>FFT Plot</source>
        <translation>Grafico FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="142"/>
        <source>Toggle FFT plot for every selected dataset</source>
        <translation>Attiva/disattiva grafico FFT per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="154"/>
        <source>Waterfall</source>
        <translation>Waterfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="157"/>
        <source>Toggle waterfall for every selected dataset</source>
        <translation>Attiva/disattiva waterfall per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="173"/>
        <source>Widgets</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="179"/>
        <source>Bar/Level</source>
        <translation>Barra/livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="182"/>
        <source>Set bar/level for every selected dataset</source>
        <translation>Imposta barra/livello per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="197"/>
        <source>Set gauge for every selected dataset</source>
        <translation>Imposta indicatore per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="209"/>
        <source>Compass</source>
        <translation>Bussola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="212"/>
        <source>Set compass for every selected dataset</source>
        <translation>Imposta bussola per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="223"/>
        <source>Meter</source>
        <translation>Misuratore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="227"/>
        <source>Set meter for every selected dataset</source>
        <translation>Imposta misuratore per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="238"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="242"/>
        <source>Toggle LED for every selected dataset</source>
        <translation>Attiva/disattiva LED per ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="266"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="268"/>
        <source>Duplicate every selected dataset</source>
        <translation>Duplica ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="276"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="279"/>
        <source>Delete every selected dataset</source>
        <translation>Elimina ogni dataset selezionato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="319"/>
        <source>Editing %1 items. Shared fields apply to all; per-item fields are locked.</source>
        <translation>Modifica di %1 elementi. I campi condivisi si applicano a tutti; i campi per elemento sono bloccati.</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>Byte per valore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Numero di byte combinati in ciascun valore di canale.</translation>
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
        <translation>Ordine dei byte utilizzato quando si combinano valori multi-byte.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>Valori con segno</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Interpreta ogni valore come intero con segno in complemento a due.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>Tabella di routing dei tag</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Voci tag:indice separate da virgola, ad es. 1:0,2:1,3:2. I tag possono essere decimali o esadecimali con prefisso 0x.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>Valida Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Rifiuta i messaggi con un checksum Fletcher non valido.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Rifiuta i messaggi con un checksum additivo non valido.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>Versione Protocollo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Seleziona il marcatore di inizio previsto (0xFE per v1, 0xFD per v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>Valida CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Rifiuta i frame con un checksum CRC-24Q non valido.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>Conteggio canali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Numero di canali di uscita (registri o coil).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>Offset registro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Offset di indirizzo sottratto dagli echo di scrittura singola (40001 per mappe Modicon legacy).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>Registri con segno</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Interpreta i registri a 16 bit come valori con segno in complemento a due.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>Layout payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>L'array emette ogni elemento in ordine; la mappa instrada le chiavi attraverso l'elenco delle chiavi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>Chiavi (modalità mappa)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Chiavi mappa separate da virgola nell'ordine dei canali. Utilizzato solo per il layout mappa.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Campi scalari</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Campi JSON separati da virgola ripetuti in ogni frame generato.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Campo array di campioni</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>Campo JSON contenente l'array di campioni raggruppati.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Campo array di record</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>Campo JSON contenente l'array di oggetti record.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Campi record (nell'ordine dei canali)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Campi record separati da virgola. La posizione di ogni campo imposta il suo indice di canale.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>Larghezze Colonne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Conteggi di caratteri separati da virgola per campo. Lasciare vuoto per dividere sugli spazi bianchi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>Rimuovi Spazi Bianchi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Rimuove la spaziatura attorno a ogni campo suddiviso.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>Chiavi (in Ordine di Canale)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Nomi di chiavi separati da virgola. La posizione di ogni chiave imposta il suo indice di canale.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>Separatore di Coppie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>Carattere tra coppie chiave=valore.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>Separatore Chiave-Valore</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>Carattere tra una chiave e il suo valore.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>Solo valori numerici</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Ignora le coppie il cui valore non è un numero.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>Tabella di routing dei comandi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Voci separate da punto e virgola in formato NOME:indice, ad es. CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>Prefisso talker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>Identificativo talker di due lettere, ad es. GP per GPS o GN per ricevitori multi-costellazione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Rifiuta le frasi il cui checksum *hh non corrisponde.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Nomi dei parametri separati da virgola. La posizione di ciascuna chiave imposta il suo indice di canale.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>Campi (in ordine di canale)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Nomi dei campi separati da virgola. La posizione di ciascun campo imposta il suo indice di canale.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>Tag (in ordine di canale)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Nomi di tag separati da virgola. La posizione di ciascun tag imposta il suo indice di canale.</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">Blocchi di Registri</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">Blocchi di registri interrogati con voci tipizzate e scalate. Scritto dall'importatore di mappe di registri Modbus.</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">Mappa dei Segnali</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">Messaggi CAN con i loro layout di bit dei segnali e scalatura. Scritto dall'importatore DBC.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Tipo di Socket</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Indirizzo Remoto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Porta Locale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>Digita 0 per porta automatica</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Porta Remota</translation>
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
        <translation>Filtra per canale…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Cancella tutte le notifiche</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(nessun titolo)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Nessuna notifica ancora</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">Le trasformazioni dei dataset e gli script dei widget di output possono pubblicare eventi qui tramite notifyInfo / notifyWarning / notifyCritical.</translation>
    </message>
</context>
<context>
    <name>OfflineActivation</name>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="37"/>
        <source>Activate Offline</source>
        <translation>Attiva Offline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="55"/>
        <source>Activate Serial Studio Pro on a machine with no internet access. No account or connection is needed on this computer.</source>
        <translation>Attiva Serial Studio Pro su una macchina senza accesso a internet. Non sono necessari account o connessione su questo computer.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="86"/>
        <source>Save your device file</source>
        <translation>Salva il file dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="95"/>
        <source>Save this computer's device file. It identifies this machine and contains no personal information.</source>
        <translation>Salva il file dispositivo di questo computer. Identifica questa macchina e non contiene informazioni personali.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="102"/>
        <source>Save Device File…</source>
        <translation>Salva File Dispositivo…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="136"/>
        <source>Get your license file</source>
        <translation>Ottieni il file di licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="145"/>
        <source>On another computer with internet access, go to the address below, upload the device file, and enter your email and license key.</source>
        <translation>Su un altro computer con accesso a internet, vai all'indirizzo sottostante, carica il file dispositivo e inserisci la tua email e chiave di licenza.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="157"/>
        <source>Open in Browser</source>
        <translation>Apri nel Browser</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="193"/>
        <source>Import your license</source>
        <translation>Importa la licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="202"/>
        <source>Copy the license file to this computer and import it. Pro features unlock immediately.</source>
        <translation>Copia il file di licenza su questo computer e importalo. Le funzionalità Pro si sbloccano immediatamente.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="209"/>
        <source>Import License File…</source>
        <translation>Importa File Licenza…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="224"/>
        <source>Save Device File</source>
        <translation>Salva File Dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="226"/>
        <source>Serial Studio device file (*.ssmachine)</source>
        <translation>File dispositivo Serial Studio (*.ssmachine)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="235"/>
        <source>Import License File</source>
        <translation>Importa File Licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="236"/>
        <source>Serial Studio license (*.sslic)</source>
        <translation>Licenza Serial Studio (*.sslic)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="236"/>
        <source>All files (*)</source>
        <translation>Tutti i file (*)</translation>
    </message>
</context>
<context>
    <name>OfflineLicense</name>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="192"/>
        <source>License certificate is valid.</source>
        <translation>Il certificato di licenza è valido.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="194"/>
        <source>The certificate signature is invalid or corrupted.</source>
        <translation>La firma del certificato non è valida o è corrotta.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="197"/>
        <source>This certificate was issued for a different device.</source>
        <translation>Questo certificato è stato emesso per un dispositivo diverso.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="200"/>
        <source>This certificate has expired.</source>
        <translation>Questo certificato è scaduto.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="202"/>
        <source>This certificate does not grant a valid license tier.</source>
        <translation>Questo certificato non concede un livello di licenza valido.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="206"/>
        <source>The certificate file is malformed or unreadable.</source>
        <translation>Il file del certificato è malformato o illeggibile.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="41"/>
        <source>Search Online Icons</source>
        <translation>Cerca Icone Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="71"/>
        <source>Download failed: %1</source>
        <translation>Download fallito: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="96"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Cerca icone (es. temperatura, freccia, play)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="108"/>
        <source>Search</source>
        <translation>Cerca</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="147"/>
        <source>Search for icons above to get started</source>
        <translation>Cerca icone sopra per iniziare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="248"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="258"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>I widget di output richiedono una licenza Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Pulsante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Invia un comando al clic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Cursore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Invia valori numerici scalati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Interruttore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Invia comandi on/off</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Campo di Testo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Digita e invia comandi arbitrari</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Manopola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Ingresso rotativo per setpoint</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Puoi configurare i widget di output, ma appaiono sulla dashboard solo con una licenza Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Duplica questo widget di output</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Elimina questo widget di output</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Funzione di Trasmissione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Importa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Importa funzione di trasmissione da un file .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Modello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Seleziona un modello di funzione di trasmissione predefinito</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Prova</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Prova la funzione di trasmissione con input di esempio</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Ripeti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Taglia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Incolla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Seleziona Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Formatta Documento</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Formatta Selezione</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Errore Widget Painter</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Editor Codice Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Importa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Importa codice painter da un file .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Modello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Seleziona un modello di painter integrato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Formatta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Riformatta il codice del painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Prova</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Apri un'anteprima live con valori di dataset simulati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Anteprima</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Taglia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Incolla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Seleziona Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Ripeti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Formatta Documento</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Formatta Selezione</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Anteprima Live Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Anteprima</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Dataset simulati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>In attesa del primo frame...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Cancella console</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpola</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="294"/>
        <source>Interpolation: %1</source>
        <translation>Interpolazione: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="307"/>
        <source>Show Area Under Plot</source>
        <translation>Mostra Area Sotto il Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="326"/>
        <source>Show X Axis Label</source>
        <translation>Mostra Etichetta Asse X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="337"/>
        <source>Show Y Axis Label</source>
        <translation>Mostra Etichetta Asse Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="349"/>
        <source>Show Crosshair</source>
        <translation>Mostra Mirino</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Resume</source>
        <translation>Riprendi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="373"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Modalità Sweep / Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="385"/>
        <source>Trigger Settings</source>
        <translation>Impostazioni Trigger</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="409"/>
        <source>Reset View</source>
        <translation>Ripristina Vista</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="415"/>
        <source>Axis Range Settings</source>
        <translation>Impostazioni Intervallo Assi</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="202"/>
        <source>Interpolate</source>
        <translation>Interpola</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="220"/>
        <source>Orbit Navigation</source>
        <translation>Navigazione Orbita</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="230"/>
        <source>Pan Navigation</source>
        <translation>Navigazione Pan</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="241"/>
        <source>Orthogonal View</source>
        <translation>Vista Ortogonale</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="247"/>
        <source>Top View</source>
        <translation>Vista Dall'alto</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="253"/>
        <source>Left View</source>
        <translation>Vista da Sinistra</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="259"/>
        <source>Front View</source>
        <translation>Vista Frontale</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="276"/>
        <source>Auto Center</source>
        <translation>Centra Automaticamente</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="292"/>
        <source>Anaglyph 3D</source>
        <translation>Anaglifo 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="306"/>
        <source>Invert Eye Positions</source>
        <translation>Inverti Posizioni Occhi</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="71"/>
        <source>None</source>
        <translation>Nessuno</translation>
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
        <translation>Lineare</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1392"/>
        <source>Time</source>
        <translation>Tempo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1415"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Trascina per spostare, clic destro per cancellare</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1418"/>
        <source>Click to place cursor</source>
        <translation>Clicca per posizionare il cursore</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1420"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Clicca per posizionare il secondo cursore — Trascina per spostare</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Visita il Sito Web</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Acquista Licenza</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Attiva</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Assistente — Funzionalità Pro</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>L'Assistente è una funzionalità di Serial Studio Pro. Attiva la tua licenza per sbloccarlo.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Attiva</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>Modalità</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>Avvia Processo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Eseguibile</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/percorso/eseguibile</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Sfoglia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Argomenti</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 valore1 --arg2 valore2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Directory di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(opzionale) /directory/di/lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Percorso Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Seleziona Processo in Esecuzione…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Avvia un processo figlio e cattura il suo stdout, oppure connettiti a una named pipe scritta da un processo esistente.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Scopri di più sulle named pipe</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Seleziona Processo in Esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Seleziona un processo in esecuzione per derivare un suggerimento di percorso named-pipe.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Filtra Processi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Digita per filtrare per nome…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Aggiorna</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Processi in Esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>Processo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>Nessun processo corrisponde al filtro.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Nessun processo in esecuzione trovato.
Fai clic su Aggiorna per aggiornare l'elenco.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 processo/i</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Seleziona</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>modificato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="367"/>
        <source>This project is password protected</source>
        <translation>Questo progetto è protetto da password</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="368"/>
        <source>Editing is available in Project mode</source>
        <translation>La modifica è disponibile in modalità Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="379"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Inserisci la password per apportare modifiche o apri un progetto diverso.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="380"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Passa alla modalità Progetto per caricare e modificare un progetto.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="402"/>
        <source>Unlock</source>
        <translation>Sblocca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="403"/>
        <source>Switch to Project Mode</source>
        <translation>Passa alla Modalità Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="422"/>
        <source>Open Other Project</source>
        <translation>Apri Altro Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="423"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="439"/>
        <source>Create New Project</source>
        <translation>Crea Nuovo Progetto</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>Struttura Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>Cerca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="357"/>
        <source>Move Up</source>
        <translation>Sposta Su</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="365"/>
        <source>Move Down</source>
        <translation>Sposta Giù</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="386"/>
        <source>Rename</source>
        <translation>Rinomina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="404"/>
        <source>Hide Selected (%1)</source>
        <translation>Nascondi Selezionati (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="405"/>
        <source>Show Selected (%1)</source>
        <translation>Mostra Selezionati (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="406"/>
        <source>Hide</source>
        <translation>Nascondi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="406"/>
        <source>Show</source>
        <translation>Mostra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="429"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Duplica Selezionato (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="430"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="452"/>
        <source>Delete Selected (%1)</source>
        <translation>Elimina Selezionato (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="453"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="504"/>
        <source>New Folder</source>
        <translation>Nuova Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="517"/>
        <source>New Sub-Folder</source>
        <translation>Nuova Sotto-cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="533"/>
        <source>Move to Folder</source>
        <translation>Sposta in Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="540"/>
        <source>Top Level</source>
        <translation>Livello Superiore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="573"/>
        <source>Move Here</source>
        <translation>Sposta Qui</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Nuovo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Crea un nuovo progetto JSON</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Apri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Apri un progetto JSON esistente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Salva</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Salva il progetto corrente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Salva Come</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Salva il progetto corrente con un nuovo nome</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">Sblocca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Blocca</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">Sblocca l'Editor Progetto con la password del progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Importa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Genera un progetto da uno schema Protocol Buffers (.proto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Imposta una Password e Blocca l'Editor di Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Aggiungi Dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Aggiungi una nuova sorgente dati (dispositivo) al progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Azione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Aggiungi una nuova azione al progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Uscita</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Ripristina un'istantanea automatica recente del progetto corrente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Aggiungi un nuovo pannello di controllo uscita con un pulsante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Cursore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Aggiungi un controllo cursore di uscita</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Interruttore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Aggiungi un controllo interruttore di uscita</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Manopola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Aggiungi un controllo manopola di uscita</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Campo di Testo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Aggiungi un controllo campo di testo di uscita</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Pulsante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Aggiungi un controllo pulsante di output</translation>
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
        <translation>Aggiungi un dataset generico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>Aggiungi un dataset grafico 2D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>Grafico FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Aggiungi un grafico Fast Fourier Transform</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Indicatore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Aggiungi un widget indicatore per dati numerici</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Indicatore di Livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Aggiungi un indicatore di livello a barra verticale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>Bussola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Aggiungi un widget bussola per i dati direzionali</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>Indicatore LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Aggiungi un indicatore di stato in stile LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Aggiungi un gruppo contenitore di dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Contenitore di Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Immagine</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Aggiungi un visualizzatore di flussi immagini/video</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Visualizzatore Immagini</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="450"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Web View</source>
        <translation>Visualizzatore Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="452"/>
        <source>Add an web viewer</source>
        <translation>Aggiungi un visualizzatore web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="462"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="466"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Aggiungi un widget painter personalizzato renderizzato in JavaScript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="467"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>I widget painter richiedono una licenza Pro — aggiungendone uno verrà utilizzata una griglia dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="468"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="480"/>
        <source>Table</source>
        <translation>Tabella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Add a data table view</source>
        <translation>Aggiungi una vista tabella dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Data Grid</source>
        <translation>Griglia Dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="491"/>
        <source>Multi-Plot</source>
        <translation>Grafico Multiplo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="493"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>Aggiungi un grafico 2D con segnali multipli</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Multiple Plot</source>
        <translation>Grafico Multiplo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="500"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>3D Plot</source>
        <translation>Grafico 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <source>Add a 3D plot visualization</source>
        <translation>Aggiungi una visualizzazione grafico 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="511"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="515"/>
        <source>Accelerometer</source>
        <translation>Accelerometro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>Aggiungi un gruppo per dati accelerometrici a 3 assi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="521"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="524"/>
        <source>Gyroscope</source>
        <translation>Giroscopio</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>Aggiungi un gruppo per dati giroscopici a 3 assi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="530"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="535"/>
        <source>GPS Map</source>
        <translation>Mappa GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="533"/>
        <source>Add a map widget for GPS data</source>
        <translation>Aggiungi un widget mappa per dati GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="547"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="551"/>
        <source>Assistant</source>
        <translation>Assistente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="554"/>
        <source>Open the Assistant</source>
        <translation>Apri l'Assistente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="560"/>
        <source>Help Center</source>
        <translation>Centro Assistenza</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="564"/>
        <source>Open the Project Editor documentation</source>
        <translation>Apri la documentazione dell'Editor Progetto</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Riepilogo Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>Funzionalità Pro rilevate in questo progetto.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Widget di fallback in uso. Acquista una licenza per sbloccare la funzionalità completa.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Titolo Progetto:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Progetto Senza Titolo</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Punti:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="147"/>
        <source>Settings</source>
        <translation>Impostazioni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="173"/>
        <source>Time Range:</source>
        <translation>Intervallo Temporale:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="221"/>
        <source>Point Count:</source>
        <translation>Numero di Punti:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="240"/>
        <source>Change-Driven Transforms:</source>
        <translation>Trasformazioni Guidate da Cambiamenti:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="250"/>
        <source>Run a dataset's transform only when one of its inputs changes. Speeds up large table-driven projects; off by default.</source>
        <translation>Esegue la trasformazione di un dataset solo quando uno dei suoi input cambia. Accelera progetti di grandi dimensioni basati su tabelle; disattivato per impostazione predefinita.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="297"/>
        <source>Source</source>
        <translation>Sorgente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="298"/>
        <source>Sources</source>
        <translation>Sorgenti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="303"/>
        <source>Group</source>
        <translation>Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="304"/>
        <source>Groups</source>
        <translation>Gruppi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="309"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="310"/>
        <source>Datasets</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="315"/>
        <source>Action</source>
        <translation>Azione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="316"/>
        <source>Actions</source>
        <translation>Azioni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="404"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Fare doppio clic su un blocco per modificarlo. Fare clic destro ovunque per aggiungere un gruppo, dataset, azione, tabella dati o dispositivo.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Anteprima File Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>File Proto: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Esplorare i messaggi qui sotto, quindi creare il progetto. Ogni messaggio diventa un gruppo della dashboard; i blocchi di canale con tipo corrispondente ottengono un MultiPlot e i messaggi con tipo misto ottengono una DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Mostra campi per</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Campi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Nome Campo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Nessun campo nel messaggio selezionato.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Totale: %1 messaggi, %2 campi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Crea Progetto</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Nessun errore</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Il broker ha rifiutato la connessione a causa di una versione del protocollo non supportata. Abbinare la versione MQTT del broker e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Il broker ha rifiutato l'ID client. Potrebbe essere malformato, troppo lungo o già in uso. Rigenerarlo e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>La rete ha raggiunto il broker, ma il broker non è attualmente disponibile. Verificarne lo stato e riprovare più tardi.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>Il nome utente o la password sono errati o non validi. Verificare le credenziali e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Il broker ha negato la connessione a causa di permessi insufficienti. Verificare che l'account disponga delle ACL necessarie.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>Un problema di rete o a livello di trasporto ha impedito la connessione. Controllare connettività, porte e configurazione TLS.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>Il client ha rilevato una violazione del protocollo MQTT e ha chiuso la connessione. Verificare la compatibilità tra broker e client.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>Si è verificato un errore imprevisto. Controllare i log del broker e la console dell'applicazione per maggiori dettagli.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>Si è verificato un errore di protocollo MQTT 5. Esaminare il codice di ragione del broker per maggiori dettagli.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Errore MQTT non specificato (codice %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>Impossibile caricare il testo di benvenuto :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="250"/>
        <source>Network error</source>
        <translation>Errore di rete</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="253"/>
        <location filename="../../src/Licensing/Trial.cpp" line="269"/>
        <location filename="../../src/Licensing/Trial.cpp" line="301"/>
        <source>Trial Activation Error</source>
        <translation>Errore Attivazione Trial</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="266"/>
        <source>Invalid server response</source>
        <translation>Risposta del server non valida</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>The server returned malformed data: %1</source>
        <translation>Il server ha restituito dati malformati: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="298"/>
        <source>Unexpected server response</source>
        <translation>Risposta del server imprevista</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>The server response is missing required fields.</source>
        <translation>La risposta del server non contiene i campi richiesti.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Errore File Output Console</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>Impossibile aprire il file in scrittura!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Adattatore Bluetooth non valido!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>Piattaforma o sistema operativo non supportato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>Metodo di rilevamento non supportato</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>Errore generale di I/O</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>Parser di Frame Disabilitato</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Il parser di frame Lua per la sorgente %1 è andato in timeout per %2 frame consecutivi ed è stato disabilitato per mantenere Serial Studio reattivo.

Causa più probabile: un ciclo infinito o un'operazione estremamente lenta nel corpo dello script. Correggere lo script e ricaricare il progetto per riabilitare il parsing.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>Errore di Sintassi Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>Il codice del parser contiene un errore:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>Errore di Runtime Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>Il codice del parser ha generato un errore:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>Funzione Parse Mancante</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>La funzione 'parse' non è definita nello script.

Assicurarsi che il codice includa:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>Errore di Runtime della Funzione Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>La funzione parse contiene un errore:

%1

Correggere l'errore nel corpo della funzione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Il parser di frame JavaScript per la sorgente %1 è andato in timeout per %2 frame consecutivi ed è stato disabilitato per mantenere Serial Studio reattivo.

Causa più probabile: un ciclo infinito o un'operazione estremamente lenta nel corpo dello script. Correggere lo script e ricaricare il progetto per riabilitare il parsing.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>Timeout Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>Il codice del parser non ha terminato la valutazione entro %1 ms ed è stato interrotto.

Causa più probabile: un ciclo infinito al livello superiore dello script.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>Errore di Sintassi Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>Il codice del parser contiene un errore di sintassi alla riga %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>Si È Verificata Un'eccezione Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>Il codice del parser ha generato le seguenti eccezioni:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>La funzione 'parse' non è definita nello script.

Assicurarsi che il codice includa:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>La funzione parse contiene un errore alla riga %1:

%2

Correggere l'errore nel corpo della funzione.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>Dichiarazione di Funzione Non Valida</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Nessuna esportazione 'parse' richiamabile trovata.

Definire una delle seguenti:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>La funzione 'parse' deve accettare almeno un parametro (il payload della frame).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Nessuna dichiarazione valida della funzione 'parse' trovata.

Formato atteso:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>Parametro Funzione Non Valido</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">La funzione 'parse' deve avere almeno un parametro.

Formato atteso:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>Firma Funzione Deprecata</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>La funzione 'parse' utilizza il vecchio formato a due parametri: parse(%1, %2)

Questo formato non è più supportato. Aggiornare al nuovo formato a parametro singolo:
function parse(%1) { ... }

Il parametro separatore non è più necessario.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="192"/>
        <source>Critical</source>
        <translation>Critico</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="192"/>
        <source>Warning</source>
        <translation>Avviso</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="585"/>
        <source>Project file not found</source>
        <translation>File di progetto non trovato</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="586"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Il file di progetto a cui fa riferimento questo collegamento non è stato trovato:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="589"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Eliminare questo collegamento?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="593"/>
        <source>Delete Shortcut</source>
        <translation>Elimina Collegamento</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="595"/>
        <source>Quit</source>
        <translation>Esci</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>Tempo (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>Freq: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>Tempo: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Nessuna chiave API OpenAI impostata. Apri Gestisci Chiavi per aggiungerne una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="240"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Nessuna chiave API Anthropic impostata. Apri Gestisci Chiavi per aggiungerne una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Nessuna chiave API Gemini impostata. Apri Gestisci Chiavi per aggiungerne una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="351"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Nessun URL del server del modello locale configurato. Aprire Gestisci Chiavi per impostarne uno.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Nessuna chiave API DeepSeek impostata. Aprire Gestisci Chiavi per aggiungerne una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Nessuna chiave API Mistral impostata. Apri Gestisci Chiavi per aggiungerne una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Nessuna chiave API OpenRouter impostata. Apri Gestisci Chiavi per aggiungerne una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Nessuna chiave API Groq impostata. Apri Gestisci Chiavi per aggiungerne una.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1149"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Il parser di frame sta utilizzando più del %1% del tempo CPU.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1151"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio sta scartando frame per mantenere l'applicazione reattiva. Semplificare o ottimizzare lo script del parser di frame per ridurne il carico di lavoro.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>Atteso %1, ricevuto '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Nome enum atteso dopo 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>Nome oneof atteso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Tag campo '%1' fuori intervallo (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Tipo chiave atteso in map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Tipo valore atteso in map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>Nome campo map atteso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>Tag campo map atteso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>Tipo campo atteso, ricevuto '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>Nome campo atteso dopo il tipo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>Numero tag campo atteso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Annidamento messaggi troppo profondo (limite %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>Nome messaggio atteso</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Token imprevisto '%1' nell'ambito del file</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Parola chiave di livello superiore non supportata '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Automatico (Predefinito della Piattaforma)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Software (Alternativo)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Riga %1</translation>
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
        <translation type="vanished">Decodificatore</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">Righe</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 riga/righe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>La configurazione del parser nativo non è un oggetto JSON valido.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Modello di parser nativo sconosciuto: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Errore Parser Integrato</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Errore Parser Nativo</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="51"/>
        <source>Offline Activation</source>
        <translation>Attivazione Offline</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append non è supportato per GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Apertura gzip sia in lettura che in scrittura non supportata</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>È possibile aprire un gzip in lettura o in scrittura. Quale delle due?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Impossibile eseguire gzopen() sul file</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append non è supportato per QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite non è supportato per QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>Errore API ZIP/UNZIP %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>Genera Report PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Genera Report</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="71"/>
        <source>Solid</source>
        <translation>Solido</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="72"/>
        <source>Dashed</source>
        <translation>Tratteggiato</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="73"/>
        <source>Dotted</source>
        <translation>Punteggiato</translation>
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
        <translation>Letter (8,5 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8,5 × 14 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="106"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7,25 × 10,5 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="107"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="108"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="123"/>
        <source>%1 — Session Report</source>
        <translation>%1 — Report Sessione</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="125"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="661"/>
        <source>Session Report</source>
        <translation>Report Sessione</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="563"/>
        <source>Branding</source>
        <translation>Branding</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="569"/>
        <source>Page</source>
        <translation>Pagina</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="575"/>
        <source>Sections</source>
        <translation>Sezioni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="581"/>
        <source>Datasets</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="629"/>
        <source>Identity</source>
        <translation>Identità</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="643"/>
        <source>Company</source>
        <translation>Azienda</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="650"/>
        <source>e.g. Acme Test Systems</source>
        <translation>es. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="654"/>
        <source>Document title</source>
        <translation>Titolo documento</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="665"/>
        <source>Author</source>
        <translation>Autore</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="672"/>
        <source>Prepared by (optional)</source>
        <translation>Preparato da (facoltativo)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="681"/>
        <source>Logo</source>
        <translation>Logo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="694"/>
        <source>File</source>
        <translation>File</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="705"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG o SVG (facoltativo)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="707"/>
        <source>Browse…</source>
        <translation>Sfoglia…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="710"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="751"/>
        <source>Paper</source>
        <translation>Carta</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="763"/>
        <source>Page size</source>
        <translation>Dimensione pagina</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="891"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Annota i valori minimi, massimi e medi sui grafici</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="927"/>
        <source>Include datasets</source>
        <translation>Includi dataset</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="941"/>
        <source>Expand All</source>
        <translation>Espandi Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="950"/>
        <source>Collapse All</source>
        <translation>Comprimi Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="962"/>
        <source>Search datasets</source>
        <translation>Cerca dataset</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1086"/>
        <source>Loading datasets...</source>
        <translation>Caricamento dataset...</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1087"/>
        <source>No datasets match your search.</source>
        <translation>Nessun dataset corrisponde alla ricerca.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1107"/>
        <source>Select at least one dataset to include.</source>
        <translation>Selezionare almeno un dataset da includere.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1124"/>
        <source>Export HTML</source>
        <translation>Esporta HTML</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="879"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Copertina (logo, titolo documento, sottotitolo test)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="882"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Informazioni del test (progetto, timestamp, classificazione e note)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="885"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Riepilogo delle misurazioni (min, max, media, deviazione std. per parametro)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="888"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Tendenze dei parametri (grafico temporale per parametro numerico)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="778"/>
        <source>Plot appearance</source>
        <translation>Aspetto del Grafico</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="792"/>
        <source>Line width</source>
        <translation>Spessore Linea</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="824"/>
        <source>Line style</source>
        <translation>Stile Linea</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="864"/>
        <source>Include</source>
        <translation>Includi</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1116"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="1124"/>
        <source>Export PDF</source>
        <translation>Esporta PDF</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Generazione Report</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>In Elaborazione…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Questa operazione può richiedere alcuni secondi per sessioni con molti parametri. La finestra si chiude automaticamente quando il report è pronto.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>Connessione Persa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>Dispositivo Non Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>La connessione al dispositivo è stata persa.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio non è riuscito a raggiungere il dispositivo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Verificare il cavo, l'alimentazione e che nessun'altra applicazione abbia preso il controllo del dispositivo. È possibile provare a riconnettersi, passare a un dispositivo diverso o uscire.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Verificare che sia collegato, acceso e non già in uso da un'altra applicazione. È possibile riprovare, selezionare un dispositivo diverso o uscire.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>Esci</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>Scegli Dispositivo Diverso</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">Riconfigura</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>Riprova</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>Riconnetti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Seleziona il dispositivo corretto, quindi premi Connetti.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>Interfaccia I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>Connetti</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="338"/>
        <source>Data Grids</source>
        <translation>Griglie Dati</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="341"/>
        <source>Multiple Data Plots</source>
        <translation>Grafici Dati Multipli</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="344"/>
        <source>Accelerometers</source>
        <translation>Accelerometri</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="347"/>
        <source>Gyroscopes</source>
        <translation>Giroscopi</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="350"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="353"/>
        <source>FFT Plots</source>
        <translation>Grafici FFT</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="356"/>
        <source>LED Panels</source>
        <translation>Pannelli LED</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="359"/>
        <source>Data Plots</source>
        <translation>Grafici Dati</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="362"/>
        <source>Bars</source>
        <translation>Barre</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="365"/>
        <source>Gauges</source>
        <translation>Indicatori</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="368"/>
        <source>Terminal</source>
        <translation>Terminale</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="371"/>
        <source>Clock</source>
        <translation>Orologio</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="374"/>
        <source>Stopwatch</source>
        <translation>Cronometro</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="377"/>
        <source>Compasses</source>
        <translation>Bussole</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="380"/>
        <source>Meters</source>
        <translation>Indicatori</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Termometri</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>3D Plots</source>
        <translation>Grafici 3D</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Web Views</source>
        <translation>Visualizzatori Web</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="390"/>
        <source>Image Views</source>
        <translation>Visualizzatori Immagini</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="393"/>
        <source>Output Panels</source>
        <translation>Pannelli di Uscita</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="396"/>
        <source>Notifications</source>
        <translation>Notifiche</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="399"/>
        <source>Waterfalls</source>
        <translation>Cascate</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="402"/>
        <source>Painter Widgets</source>
        <translation>Widget Painter</translation>
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
        <translation>Sistema</translation>
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
        <translation>Dettagli Sessione</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Seleziona una sessione per visualizzare i dettagli.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Progetto:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Avviata:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Terminata:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(in corso)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Frame:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Note</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Aggiungi note sessione…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Le note sono di sola lettura per le sessioni completate.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Nuovo tag…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Sblocca il file di sessione per eliminare le sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Aggiungi</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Riproduci</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>Esporta CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Genera Report</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Cerca</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>Data</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>Frame</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>Nessuna sessione trovata.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Nessun file di sessione aperto.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1026"/>
        <source>Select logo image</source>
        <translation>Seleziona immagine logo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1028"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Immagini (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Open Session File</source>
        <translation>Apri File di Sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="430"/>
        <source>Session files (*.db)</source>
        <translation>File di sessione (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1225"/>
        <source>Cannot open session file</source>
        <translation>Impossibile aprire il file di sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="655"/>
        <source>Delete session from %1?</source>
        <translation>Eliminare la sessione da %1?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="658"/>
        <source>Delete Session</source>
        <translation>Elimina Sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1086"/>
        <source>No project data</source>
        <translation>Nessun dato di progetto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Tutte le letture e i dati grezzi per questa sessione verranno rimossi permanentemente.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>Lock Session File</source>
        <translation>Blocca File di Sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="487"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Scegli una password per bloccare il file di sessione:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="496"/>
        <source>Confirm the password:</source>
        <translation>Conferma la password:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="504"/>
        <source>Passwords do not match</source>
        <translation>Le password non corrispondono</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="505"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>Le due password inserite non corrispondono. Il file di sessione non è stato bloccato.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="541"/>
        <source>Unlock Session File</source>
        <translation>Sblocca File di Sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="542"/>
        <source>Enter the session file password:</source>
        <translation>Inserire la password del file di sessione:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="552"/>
        <source>Incorrect password</source>
        <translation>Password errata</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="553"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>La password inserita non corrisponde a quella memorizzata nel file di sessione.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="645"/>
        <source>Session file locked</source>
        <translation>File di sessione bloccato</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="646"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Sbloccare il file di sessione prima di eliminare le sessioni registrate.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">Questa sessione non contiene un file di progetto incorporato — il dashboard utilizza un layout di visualizzazione rapida.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>Export Session to CSV</source>
        <translation>Esporta Sessione in CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>CSV files (*.csv)</source>
        <translation>File CSV (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">Esportazione Completata</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">Sessione esportata in:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">Preparazione esportazione…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Done</source>
        <translation>Completato</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="962"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Failed</source>
        <translation>Non Riuscito</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="968"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1008"/>
        <source>Report Failed</source>
        <translation>Generazione Report Non Riuscita</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1009"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Impossibile generare il report. Verificare il percorso di output e riprovare.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save PDF Report</source>
        <translation>Salva Report PDF</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="860"/>
        <source>Loading session data…</source>
        <translation>Caricamento dati sessione…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save HTML Report</source>
        <translation>Salva Report HTML</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>PDF files (*.pdf)</source>
        <translation>File PDF (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>HTML files (*.html)</source>
        <translation>File HTML (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1087"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Questo file di sessione non contiene un progetto incorporato.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1096"/>
        <source>Invalid project data</source>
        <translation>Dati progetto non validi</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1097"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>Il JSON del progetto incorporato è malformato e non può essere ripristinato.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Restore Project</source>
        <translation>Ripristina Progetto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Progetti Serial Studio (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Cannot write file</source>
        <translation>Impossibile scrivere il file</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Check file permissions and try again.</source>
        <translation>Verificare i permessi del file e riprovare.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="77"/>
        <source>Empty file path</source>
        <translation>Percorso file vuoto</translation>
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
        <translation>Database non aperto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="263"/>
        <source>Database not open or empty label</source>
        <translation>Database non aperto o etichetta vuota</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="331"/>
        <source>Invalid label</source>
        <translation>Etichetta non valida</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="598"/>
        <source>Cancelled</source>
        <translation>Annullato</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="717"/>
        <source>Could not load session data</source>
        <translation>Impossibile caricare i dati della sessione</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>Assemblaggio report…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>Scrittura output…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>Report Sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>Progetto senza titolo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>Preparato da</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>Generato il %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>ID Prova</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>Durata</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>Campioni</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>Parametri</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>Avviato</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>Terminato</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>Progetto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>Identificatore Prova</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>Ora di Inizio</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>Ora di Fine</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>Durata Totale</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>Campioni Acquisiti</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>Parametri Registrati</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>Classificazione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>Note</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>Informazioni Prova</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>Parametro</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>Unità</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>Minimo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>Massimo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>Media</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>Deviazione Std.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>Riepilogo Misurazioni</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>clicca su una colonna per ordinare</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 campioni in %2 secondi</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>Vista Parametri Combinata</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>clicca sugli elementi della legenda per attivare/disattivare i segnali</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>Andamenti Parametri</translation>
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
        <translation>Pagina %1 di %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>Caricamento motore di rendering…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>Rendering grafici…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>Generazione PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>Apri File di Sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>File di sessione (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>Connessione Dispositivo Attiva</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Per utilizzare questa funzione, è necessario disconnettersi dal dispositivo. Procedere?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>Impossibile aprire il file di sessione</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>Errore sconosciuto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>Nessun dato di progetto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Questa sessione non contiene un file di progetto incorporato — il dashboard utilizza un layout di visualizzazione rapida.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>Verificare i permessi del file e riprovare.</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">Nessuna sessione trovata</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">Questo file non contiene sessioni di registrazione.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">La sessione non ha colonne</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">La sessione selezionata non ha le definizioni delle colonne.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">La sessione non ha letture</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">La sessione selezionata non contiene frame.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Percorso file vuoto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Questo file non contiene sessioni di registrazione.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Annullato</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>La sessione selezionata non contiene le definizioni delle colonne.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>La sessione selezionata non contiene alcun frame.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Preferenze</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>Generale</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="166"/>
        <source>Language</source>
        <translation>Linguaggio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="182"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="289"/>
        <source>Workspace Folder</source>
        <translation>Cartella Workspace</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="585"/>
        <source>Automatically Check for Updates</source>
        <translation>Verifica Automaticamente gli Aggiornamenti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Dashboard</source>
        <translation>Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="402"/>
        <source>Export…</source>
        <translation>Esporta…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="649"/>
        <source>Data Plotting</source>
        <translation>Tracciamento Dati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Point Count</source>
        <translation>Numero di Punti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="741"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Frequenza di Aggiornamento UI (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1051"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Mostra Sempre i Pulsanti della Barra delle Applicazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="872"/>
        <source>Show Actions Panel</source>
        <translation>Mostra Pannello Azioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="336"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>Abilita Server API (Porta 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="85"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1163"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="151"/>
        <source>Appearance</source>
        <translation>Aspetto</translation>
    </message>
    <message>
        <source>Files &amp; Updates</source>
        <translation type="vanished">File e Aggiornamenti</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="vanished">Avanzate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>Allow External API Connections</source>
        <translation>Consenti Connessioni API Esterne</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="888"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Nascondi Automaticamente la Barra Degli Strumenti</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <source>Taskbar</source>
        <translation>Barra delle Applicazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="470"/>
        <source>Rendering Backend</source>
        <translation>Backend di Rendering</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="370"/>
        <source>API Access Token</source>
        <translation>Token di Accesso API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Startup</source>
        <translation>Avvio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="202"/>
        <source>Window</source>
        <translation>Finestra</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="221"/>
        <source>Custom Window Decorations</source>
        <translation>Decorazioni Finestra Personalizzate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Window Shadow</source>
        <translation>Ombra Finestra</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="264"/>
        <source>Window decoration changes apply after restarting %1.</source>
        <translation>Le modifiche alle decorazioni della finestra si applicano dopo il riavvio di %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="273"/>
        <source>Files</source>
        <translation>File</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>API &amp; Plugins</source>
        <translation>API e Plugin</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="452"/>
        <source>Graphics</source>
        <translation>Grafica</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="508"/>
        <source>System</source>
        <translation>Sistema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="524"/>
        <source>Apply Performance Hints</source>
        <translation>Applica Suggerimenti Prestazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="538"/>
        <source>Keep Display Awake</source>
        <translation>Mantieni Display Attivo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="558"/>
        <source>Performance hints raise process priority and opt out of OS power throttling. Changes take effect the next time Serial Studio starts.</source>
        <translation>I suggerimenti prestazioni aumentano la priorità del processo e disattivano la limitazione energetica del sistema operativo. Le modifiche hanno effetto al prossimo avvio di Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="569"/>
        <source>Updates &amp; News</source>
        <translation>Aggiornamenti e Notizie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="599"/>
        <source>Show What's New on Startup</source>
        <translation>Mostra Novità all'Avvio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="664"/>
        <source>Time Range</source>
        <translation>Intervallo Temporale</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Small</source>
        <translation>Piccolo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Normal</source>
        <translation>Normale</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Large</source>
        <translation>Grande</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Extra Large</source>
        <translation>Extra Grande</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Custom</source>
        <translation>Personalizzato</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="857"/>
        <source>Layout</source>
        <translation>Layout</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="904"/>
        <source>Auto-Layout Margin</source>
        <translation>Margine Layout Automatico</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="929"/>
        <source>Auto-Layout Spacing</source>
        <translation>Spaziatura Layout Automatico</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="961"/>
        <source>Video Export</source>
        <translation>Esportazione Video</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="979"/>
        <source>Save Videos by Default</source>
        <translation>Salva Video per Impostazione Predefinita</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1030"/>
        <source>Behavior</source>
        <translation>Comportamento</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1066"/>
        <source>Show Search Field</source>
        <translation>Mostra Campo di Ricerca</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1081"/>
        <source>Auto-hide Taskbar</source>
        <translation>Nascondi Automaticamente la Barra delle Applicazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1099"/>
        <source>Hide Delay (ms)</source>
        <translation>Ritardo Nascondimento (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1123"/>
        <source>Pinned Buttons</source>
        <translation>Pulsanti Bloccati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1141"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Trascina un pulsante bloccato sulla barra delle applicazioni per riordinarlo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1162"/>
        <source>Settings</source>
        <translation>Impostazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1165"/>
        <source>Clock</source>
        <translation>Orologio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1166"/>
        <source>Stopwatch</source>
        <translation>Cronometro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1167"/>
        <source>Pause / Resume</source>
        <translation>Pausa / Riprendi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1168"/>
        <source>File Transmission</source>
        <translation>Trasmissione File</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1169"/>
        <source>AI Assistant</source>
        <translation>Assistente AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1298"/>
        <source>Display</source>
        <translation>Visualizzazione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1313"/>
        <source>Display Mode</source>
        <translation>Modalità di Visualizzazione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="778"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1326"/>
        <source>Font Family</source>
        <translation>Famiglia di Caratteri</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="92"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>Notifications</source>
        <translation>Notifiche</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="400"/>
        <source>Export Protobuf File</source>
        <translation>Esporta File Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="763"/>
        <source>Dashboard Font</source>
        <translation>Font Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="793"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1341"/>
        <source>Font Size</source>
        <translation>Dimensione Font</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">Esportazione Immagini</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">Salva Immagini per Impostazione Predefinita</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1358"/>
        <source>Show Timestamps</source>
        <translation>Mostra Timestamp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Data Transmission</source>
        <translation>Trasmissione Dati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1392"/>
        <source>Line Ending</source>
        <translation>Terminazione di Riga</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1405"/>
        <source>Input Mode</source>
        <translation>Modalità Ingresso</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1418"/>
        <source>Text Encoding</source>
        <translation>Codifica Testo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1431"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1444"/>
        <source>Echo Sent Data</source>
        <translation>Eco Dati Inviati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1463"/>
        <source>Escape Codes</source>
        <translation>Codici di Escape</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1478"/>
        <source>VT100 Emulation</source>
        <translation>Emulazione VT100</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1497"/>
        <source>ANSI Colors</source>
        <translation>Colori ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1555"/>
        <source>Delivery</source>
        <translation>Consegna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1570"/>
        <source>System Notifications</source>
        <translation>Notifiche di Sistema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1591"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Mostra eventi di Avviso/Critici come notifiche desktop del SO quando Serial Studio non è la finestra in primo piano.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1601"/>
        <source>Application Logs</source>
        <translation>Log Applicazione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1616"/>
        <source>Route Warnings to Notifications</source>
        <translation>Inoltra Avvisi alle Notifiche</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1637"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Disattivato per impostazione predefinita — QT e QML emettono avvisi frequentemente e abilitare questa opzione può oscurare allarmi reali. I messaggi critici vengono sempre instradati indipendentemente da questa impostazione.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1656"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1696"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1704"/>
        <source>Apply</source>
        <translation>Applica</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Configurazione Dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>Server API Attivo (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>Server API Pronto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>Server API Disattivato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Analisi Frame</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Solo Console (Nessuna Analisi)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Grafico Rapido (Valori Separati da Virgola)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Analizza tramite File Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Cambia File Progetto (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Seleziona File Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Esportazione Dati</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>Foglio di Calcolo CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Registrazione Sessione</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>Registrazione MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Log della Console</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">File CSV</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">Database Sessioni</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">File MDF4</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">Dump Console</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">Crea File CSV</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">Crea File MDF4</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">Crea Log di Sessione</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">Esporta Dati Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>Interfaccia I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Progetto Multi-dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Questo progetto riceve dati da %1 dispositivi indipendenti.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Ogni dispositivo ha le proprie impostazioni di connessione. Configurale nell'Editor Progetto sotto la scheda Sorgenti.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Apri Editor Progetto</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">Nuova Scorciatoia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>Scegli un'Icona</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">Salva Scorciatoia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>Nuovo Deployment</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="109"/>
        <source>Save Deployment</source>
        <translation>Salva Deployment</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>General</source>
        <translation>Generale</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Taskbar</source>
        <translation>Barra delle Applicazioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="164"/>
        <source>Logging</source>
        <translation>Registrazione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="221"/>
        <source>Identity</source>
        <translation>Identità</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="277"/>
        <source>Click to choose an icon</source>
        <translation>Clicca per scegliere un'icona</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="286"/>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="295"/>
        <source>Deployment Name</source>
        <translation>Nome Deployment</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="386"/>
        <source>Theme</source>
        <translation>Tema</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="396"/>
        <source>Same as Serial Studio</source>
        <translation>Uguale a Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="418"/>
        <source>Actions Panel</source>
        <translation>Pannello Azioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="429"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="638"/>
        <source>File Transmission</source>
        <translation>Trasmissione File</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="445"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Facendo doppio clic su questo deployment si accede direttamente alla dashboard in tempo reale per questo progetto. Non ci sono barra degli strumenti o pannello di configurazione, solo i dati, e Serial Studio si chiude non appena il dispositivo si disconnette.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="491"/>
        <source>Visibility</source>
        <translation>Visibilità</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="506"/>
        <source>Mode</source>
        <translation>Modalità</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="515"/>
        <source>Always shown</source>
        <translation>Sempre Visibile</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="516"/>
        <source>Auto-hide</source>
        <translation>Nascondi Automaticamente</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="517"/>
        <source>Hidden</source>
        <translation>Nascosta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Nascondere la barra delle applicazioni rimuove i pulsanti di riduzione/ingrandimento/chiusura della finestra e forza il layout automatico, così il dashboard riempie sempre l'area disponibile.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="536"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>La barra delle applicazioni scorre quando l'utente sposta il cursore vicino al bordo inferiore.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="538"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>La barra delle applicazioni è permanentemente visibile nella parte inferiore del dashboard.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="551"/>
        <source>Pinned Buttons</source>
        <translation>Pulsanti Bloccati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="568"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="582"/>
        <source>Notifications</source>
        <translation>Notifiche</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="596"/>
        <source>Clock</source>
        <translation>Orologio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="610"/>
        <source>Stopwatch</source>
        <translation>Cronometro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="624"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="749"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Le registrazioni vengono salvate nella cartella workspace di Serial Studio</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">Nome Collegamento</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="304"/>
        <source>Change Icon…</source>
        <translation>Cambia Icona…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="321"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="339"/>
        <source>Project</source>
        <translation>Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="349"/>
        <source>Choose a project file to begin</source>
        <translation>Scegli un file di progetto per iniziare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="371"/>
        <source>Behavior</source>
        <translation>Comportamento</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="408"/>
        <source>Fullscreen</source>
        <translation>Schermo Intero</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">Facendo doppio clic su questo collegamento si accede direttamente alla dashboard in tempo reale per questo progetto. Non ci sono barra degli strumenti o pannello di configurazione, solo i dati, e Serial Studio si chiude non appena il dispositivo si disconnette.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">Incorpora Progetto</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">Facendo doppio clic su questo collegamento si accede direttamente al dashboard in tempo reale per questo progetto. Non ci sono barra degli strumenti o pannello di configurazione, solo i dati, e Serial Studio si chiude non appena il dispositivo si disconnette.

Attiva Incorpora Progetto per integrare il progetto nel collegamento, in modo che continui a funzionare anche se il file originale viene spostato o eliminato.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="688"/>
        <source>Recorders</source>
        <translation>Registratori</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="703"/>
        <source>CSV File</source>
        <translation>File CSV</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="713"/>
        <source>MDF4 File</source>
        <translation>File MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="723"/>
        <source>Session Database</source>
        <translation>Database di Sessione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="733"/>
        <source>Console Log</source>
        <translation>Log della Console</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">Le registrazioni vengono salvate nella posizione predefinita di ciascun modulo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="778"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="787"/>
        <source>Save</source>
        <translation>Salva</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>Ripeti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>Taglia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>Incolla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>Seleziona Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>Formatta Documento</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>Formatta Selezione</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>Valida</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Verifica che lo script compili correttamente</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">Ripristina i parametri del modello ai valori predefiniti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>Ripristina allo script di analisi predefinito</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>Apri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>Importa un file di script per l'analisi dei dati</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Apri la documentazione di aiuto per l'analisi dei dati</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Linguaggio:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Nativo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>Seleziona Modello…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>Annulla l'ultima modifica al codice</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>Ripristina la modifica precedentemente annullata</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>Taglia il codice selezionato negli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>Copia il codice selezionato negli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>Incolla il codice dagli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>Aiuto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>Piattaforma:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>Integrato</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">Modello:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>Testa con Dati di Esempio</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Valuta</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Duplica</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Crea una copia di questa sorgente dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Rimuovi questa sorgente dati</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>La sorgente dati primaria non può essere rimossa</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="24"/>
        <source>Session Player</source>
        <translation>Riproduttore Sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="91"/>
        <source>Loading session…</source>
        <translation>Caricamento sessione…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="278"/>
        <source>Workspaces</source>
        <translation>Aree di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="415"/>
        <source>Actions</source>
        <translation>Azioni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="438"/>
        <source>No Actions Available</source>
        <translation>Nessuna Azione Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="468"/>
        <source>Plugins</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="516"/>
        <source>No Plugins Installed</source>
        <translation>Nessun Plugin Installato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="101"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="552"/>
        <source>Auto Layout</source>
        <translation>Layout Automatico</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="109"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="564"/>
        <source>Full Screen</source>
        <translation>Schermo Intero</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="115"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="576"/>
        <source>Add External Window</source>
        <translation>Aggiungi Finestra Esterna</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="157"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="861"/>
        <source>Help Center</source>
        <translation>Centro Assistenza</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="692"/>
        <source>Tools</source>
        <translation>Strumenti</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="821"/>
        <source>No Tools Available</source>
        <translation>Nessuno Strumento Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="889"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Quit</source>
        <translation>Esci</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="947"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="948"/>
        <source>Hide</source>
        <translation>Nascondi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="366"/>
        <source>New Workspace…</source>
        <translation>Nuova Area di Lavoro…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="135"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="765"/>
        <source>Clock</source>
        <translation>Orologio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="143"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Stopwatch</source>
        <translation>Cronometro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="163"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="789"/>
        <source>Sessions</source>
        <translation>Sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="170"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>File Transmission</source>
        <translation>Trasmissione File</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="177"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="806"/>
        <source>AI Assistant</source>
        <translation>Assistente AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="352"/>
        <source>Show "%1"</source>
        <translation>Mostra "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Mostra Tutte le Aree di Lavoro Nascoste</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="380"/>
        <source>No Workspaces Available</source>
        <translation>Nessuna Area di Lavoro Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="506"/>
        <source>Manage Plugins…</source>
        <translation>Gestisci Plugin…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="596"/>
        <source>Export</source>
        <translation>Esporta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="627"/>
        <source>CSV File</source>
        <translation>File CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="633"/>
        <source>MDF4 File</source>
        <translation>File MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="639"/>
        <source>Console Transcript</source>
        <translation>Trascrizione Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="648"/>
        <source>Session Database</source>
        <translation>Database Sessione</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="662"/>
        <source>No Export Formats Available</source>
        <translation>Nessun Formato di Esportazione Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="121"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="748"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="127"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Notifications</source>
        <translation>Notifiche</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="151"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="780"/>
        <source>Preferences</source>
        <translation>Preferenze</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="936"/>
        <source>Edit…</source>
        <translation>Modifica…</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="882"/>
        <source>Resume</source>
        <translation>Riprendi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="883"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Disconnect</source>
        <translation>Disconnetti</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Interrompi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>Avvia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>Giro</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Ripristina</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Totale</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Nessun giro registrato</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Premi Giro mentre il cronometro è in esecuzione</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="160"/>
        <source>No Data Available</source>
        <translation>Nessun Dato Disponibile</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Valori Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Cerca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Gruppo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Unità</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(virtuale)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Copia codice di accesso %1 negli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Codice di accesso dataset copiato</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>Nessun dataset definito in questo progetto.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Nessun dataset corrisponde alla ricerca.</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>Parametro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>Valore</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Icona Personalizzata)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>Automatico</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>No</source>
        <translation>No</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>Yes</source>
        <translation>Sì</translation>
    </message>
</context>
<context>
    <name>TableFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="126"/>
        <source>Add Sub-folder</source>
        <translation>Aggiungi Sotto-cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="128"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Aggiungi una sotto-cartella all'interno di questa cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="136"/>
        <source>Add Shared Table</source>
        <translation>Aggiungi Tabella Condivisa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="138"/>
        <source>Add a shared table inside this folder</source>
        <translation>Aggiungi una tabella condivisa all'interno di questa cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="148"/>
        <source>Rename</source>
        <translation>Rinomina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="150"/>
        <source>Rename folder</source>
        <translation>Rinomina cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="158"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="160"/>
        <source>Delete folder</source>
        <translation>Elimina cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="173"/>
        <source>Title</source>
        <translation>Titolo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="174"/>
        <source>Registers</source>
        <translation>Registri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="264"/>
        <source>This folder is empty. Use the toolbar to add a table or sub-folder.</source>
        <translation>Questa cartella è vuota. Usa la barra degli strumenti per aggiungere una tabella o una sotto-cartella.</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Menu di Avvio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Menu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Cerca…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Impostazioni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Notifiche</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Orologio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Cronometro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>Assistente AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Riprendi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="986"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Connesso a %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="987"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Non connesso</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1011"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1021"/>
        <source>Status:</source>
        <translation>Stato:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1029"/>
        <source>Connected</source>
        <translation>Connesso</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1030"/>
        <source>Disconnected</source>
        <translation>Disconnesso</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1037"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1050"/>
        <source>Mode:</source>
        <translation>Modalità:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1063"/>
        <source>Messages sent:</source>
        <translation>Messaggi inviati:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1077"/>
        <source>Open MQTT Settings</source>
        <translation>Apri Impostazioni MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Trasmissione File</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">Cerca widget…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Rimuovi dallo Spazio di Lavoro</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Seleziona Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Cancella</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Invia Dati al Dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Mostra Timestamp</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Echo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>Emula VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Colori ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Visualizzazione: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">Lo Sapevi?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">Mantieni il firmware semplice inviando dati grezzi e lasciando che Serial Studio li analizzi in JavaScript, Lua o con i modelli integrati senza codice.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">Assegna a ogni canale la propria funzione per calibrare, filtrare o convertire unità. Delega i calcoli a Serial Studio e mantieni il firmware snello.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">Serve un valore che il dispositivo non invia? Un dataset virtuale calcola il proprio canale, come la potenza da tensione e corrente, tracciato e registrato come dato.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">Cattura anomalie come un oscilloscopio da banco. I grafici con asse temporale hanno modalità sweep e trigger, e puoi trascinare il livello di trigger direttamente sul grafico.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">Smetti di scorrere per trovare il widget giusto. Raggruppali nei tuoi spazi di lavoro e passa da uno all'altro dalla ricerca nella barra delle applicazioni.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">Non perdere mai più un test. Registra le sessioni in un database locale, poi sfogliale, etichettale e riproducile quando ne hai bisogno.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">Consegna un report rifinito al tuo team in pochi secondi. Esporta qualsiasi sessione in HTML o PDF, completa di grafici e statistiche min/max/media.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">Chiudi il ciclo senza strumenti aggiuntivi. I Controlli di Output ti permettono di inviare comandi al tuo dispositivo direttamente dalla dashboard.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">Costruisci una visualizzazione che nessun altro ha. Il widget Painter esegue il tuo script per disegnare grafiche completamente personalizzate dai dati in arrivo.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">Uno strumento per ogni collegamento. Serial Studio legge da UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT e Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">Salta la danza del terminale. Invia e ricevi file sul tuo collegamento seriale con i protocolli integrati XMODEM, YMODEM e ZMODEM.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">Hai già una mappa registri Modbus o un file DBC? Genera automaticamente un progetto pronto all'uso invece di costruirne uno manualmente.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">Descrivi cosa vuoi e lascia che l'Assistente AI lo costruisca. Può creare e modificare progetti per te attraverso otto provider di modelli.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">Suggerimento %1 di %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">Scopri di Più</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">Mostra Suggerimenti all'Avvio</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">Precedente</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">Avanti</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Chiudi</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>In attesa di approvazione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Completato</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Errore</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Negato</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Bloccato</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>In Esecuzione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Approva</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Nega</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>Argomenti</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>Copia</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Copia Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Seleziona Tutto</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Risultato</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Editor di Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Apri l'Editor di Progetto per creare o modificare il tuo layout JSON</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>Riproduci un file CSV come se fossero dati del sensore in tempo reale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Preferenze</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Apri impostazioni e preferenze dell'applicazione</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>Apri CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>Apri MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Sessioni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Sfoglia, riproduci ed esporta sessioni registrate</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">Scorciatoie</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">Crea una scorciatoia operatore per il progetto corrente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Estensioni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Sfoglia e installa estensioni</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">Configura connessione MQTT (pubblica o sottoscrivi)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Deploy</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">Crea un deployment operatore per il progetto corrente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Seleziona comunicazione porta seriale (UART)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Seleziona dispositivo di ingresso audio (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Seleziona comunicazione USB raw (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Rete</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>Seleziona comunicazione di rete TCP/UDP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>Seleziona comunicazione MODBUS (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>Seleziona comunicazione dispositivo HID (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Seleziona comunicazione Bluetooth Low Energy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>Bus CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>Seleziona comunicazione Bus CAN (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Processo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Seleziona comunicazione pipe di processo (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Esempi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Sfoglia progetti di esempio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Centro Assistenza</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Sfoglia documentazione, FAQ e wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Visualizza documentazione dettagliata e fai domande su DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Connetti o disconnetti dal dispositivo configurato</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>Informazioni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Apri Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Apri un progetto JSON esistente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Riproduci un file MDF4 come se fossero dati dal sensore in tempo reale (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>Assistente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>Chatta con un'IA per creare e modificare il tuo progetto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Crea un'app operatore per il progetto corrente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Mostra informazioni applicazione e dettagli licenza</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>Wiki e Chat AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Gestisci la licenza e attiva Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Disconnetti</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Connetti</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">Connetti o disconnetti dal dispositivo o dal broker MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Attiva</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Impostazioni Trigger</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Mantiene la forma d'onda stazionaria allineando ogni sweep a un evento di trigger.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Blocca un segnale ripetitivo in posizione, come il pulsante Auto su un oscilloscopio. Ogni scansione inizia nello stesso punto della forma d'onda, così rimane ferma invece di scorrere.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>Trigger</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">Modalità:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>Modalità</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>Automatico</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>Normale</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Singolo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto scorre liberamente quando nulla attraversa il livello.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normale attende un attraversamento.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Singola cattura una scansione, poi si ferma.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Slope:</source>
        <translation>Pendenza:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="273"/>
        <source>Trigger on a downward crossing</source>
        <translation>Trigger su un attraversamento discendente</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="319"/>
        <source>Timebase:</source>
        <translation>Base Temporale:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="388"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Lascia la base temporale vuota per usare l'intervallo temporale del grafico; riducila per ingrandire un segnale veloce. L'holdoff ignora nuovi trigger per un momento dopo ciascuno.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Segnale:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Valore da attraversare</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Fronte:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="254"/>
        <source>Rising</source>
        <translation>Salita</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="258"/>
        <source>Trigger on an upward crossing</source>
        <translation>Trigger su un attraversamento ascendente</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="269"/>
        <source>Falling</source>
        <translation>Discesa</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Una nuova scansione inizia ogni volta che il segnale attraversa il livello nella direzione scelta. Auto esegue anche in modalità libera quando non viene trovato alcun attraversamento.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="292"/>
        <source>Timing</source>
        <translation>Temporizzazione</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Base Temporale (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="332"/>
        <source>Match time range</source>
        <translation>Abbina intervallo temporale</translation>
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
        <translation type="vanished">La base temporale imposta quanto tempo mostra una scansione; lasciala vuota per usare l'intervallo temporale del grafico. Riducila per ingrandire un segnale veloce. L'holdoff ignora nuovi trigger per un momento dopo ciascuno.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="403"/>
        <source>Capture Next</source>
        <translation>Cattura Successivo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="405"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Arma per un'altra Cattura Singola</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Livello:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Livello di trigger</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Holdoff (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Tempo di holdoff</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Sorgente:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Riarma</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="418"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>Porta COM</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="79"/>
        <source>Baud Rate</source>
        <translation>Baud Rate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="163"/>
        <source>Data Bits</source>
        <translation>Bit di Dati</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="184"/>
        <source>Parity</source>
        <translation>Parità</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="205"/>
        <source>Stop Bits</source>
        <translation>Bit di Stop</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="226"/>
        <source>Flow Control</source>
        <translation>Controllo di Flusso</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="258"/>
        <source>Auto Reconnect</source>
        <translation>Riconnessione Automatica</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="276"/>
        <source>Send DTR Signal</source>
        <translation>Invia Segnale DTR</translation>
    </message>
</context>
<context>
    <name>UI::AlarmMonitor</name>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="195"/>
        <source>Alarm</source>
        <translation>Allarme</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>critical</source>
        <translation>critico</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>avviso</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Il valore %1%2 è entrato nella banda %3 (%4–%5).</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="205"/>
        <source>Alarms</source>
        <translation>Allarmi</translation>
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
        <translation>Notifiche</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1733"/>
        <source>Clock</source>
        <translation>Orologio</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1740"/>
        <source>Stopwatch</source>
        <translation>Cronometro</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1786"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1801"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Fallback)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1823"/>
        <source>LED Panel (%1)</source>
        <translation>Pannello LED (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="161"/>
        <source>Invalid</source>
        <translation>Non Valido</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1069"/>
        <source>Select Background Image</source>
        <translation>Seleziona Immagine di Sfondo</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1071"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Immagini (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>Dispositivo USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Modalità di Trasferimento</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Flusso Bulk</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Avanzato (Bulk + Control)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Isocrono</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Connetti a dispositivi USB utilizzando trasferimenti bulk, control o isocroni. Adatto per data logger, dispositivi con firmware personalizzato e strumenti USB.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>Specifiche USB (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>Endpoint IN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>Endpoint OUT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>Dimensione Massima Pacchetto</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Trasferimenti di Controllo Abilitati</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>L'invio di richieste di controllo errate può bloccare o danneggiare l'hardware connesso. Usare con cautela.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Informazioni sui trasferimenti di controllo USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>La dimensione del pacchetto deve corrispondere alla dimensione massima di trasferimento riportata dall'endpoint. Valori tipici: 192 B (audio FS), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Scaricare l'aggiornamento ora?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Scaricare l'aggiornamento ora?&lt;br /&gt;Questo è un aggiornamento obbligatorio; uscire ora chiuderà l'applicazione.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Registro modifiche:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>La versione %1 di %2 è stata rilasciata!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Nessun aggiornamento disponibile al momento</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Congratulazioni! Stai utilizzando l'ultima versione di %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="172"/>
        <source>Add Register</source>
        <translation>Aggiungi Registro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="175"/>
        <source>Add register</source>
        <translation>Aggiungi registro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="182"/>
        <source>Insert Constant</source>
        <translation>Inserisci Costante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="185"/>
        <source>Insert constant</source>
        <translation>Inserisci costante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="192"/>
        <source>Import</source>
        <translation>Importa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="195"/>
        <source>Import registers from CSV</source>
        <translation>Importa registri da CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="202"/>
        <source>Export</source>
        <translation>Esporta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="205"/>
        <source>Export registers to CSV</source>
        <translation>Esporta registri in CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="217"/>
        <source>Rename</source>
        <translation>Rinomina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="220"/>
        <source>Rename table</source>
        <translation>Rinomina tabella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="227"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="230"/>
        <source>Delete table</source>
        <translation>Elimina tabella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="244"/>
        <source>Help</source>
        <translation>Aiuto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="249"/>
        <source>Open help documentation for shared memory</source>
        <translation>Apri la documentazione di aiuto per la memoria condivisa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="289"/>
        <source>Permissions</source>
        <translation>Permessi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="290"/>
        <source>Register Name</source>
        <translation>Nome Registro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="291"/>
        <source>Default Value</source>
        <translation>Valore Predefinito</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read-Only</source>
        <translation>Sola Lettura</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read/Write</source>
        <translation>Lettura/scrittura</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="468"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Copia il codice di accesso %1 negli appunti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="501"/>
        <source>Delete register</source>
        <translation>Elimina registro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="518"/>
        <source>No registers.</source>
        <translation>Nessun registro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="568"/>
        <source>Register access code copied</source>
        <translation>Codice di accesso registro copiato</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="237"/>
        <source>Show Colorbar</source>
        <translation>Mostra Barra dei Colori</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="250"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Mostra Assi e Griglia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="263"/>
        <source>Show Crosshair</source>
        <translation>Mostra Mirino</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Pause</source>
        <translation>Pausa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Resume</source>
        <translation>Riprendi</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">Cancella Cronologia</translation>
    </message>
</context>
<context>
    <name>WebView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="70"/>
        <source>Web View Unavailable</source>
        <translation>Visualizzatore Web Non Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="81"/>
        <source>This build of Serial Studio was compiled without Qt WebEngine, so web pages cannot be displayed.</source>
        <translation>Questa build di Serial Studio è stata compilata senza QT WebEngine, quindi le pagine web non possono essere visualizzate.</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Benvenuto in %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio è un potente strumento di visualizzazione in tempo reale, progettato per ingegneri, studenti e maker.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>È possibile avviare una prova completamente funzionale di 14 giorni, attivarla con la propria chiave di licenza oppure scaricare e compilare autonomamente il codice sorgente GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>L'acquisto di Pro sostiene direttamente l'autore e contribuisce a finanziare lo sviluppo futuro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Compilare autonomamente la versione GPLv3 aiuta a far crescere la comunità e incoraggia i contributi tecnici.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Attendere…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>%1 giorni rimanenti nella prova.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">Stai utilizzando la versione di prova completa di %1 Pro. È valida per 14 giorni di uso personale e non commerciale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>Stai utilizzando la versione di prova completa di %1 Pro. È valida per 14 giorni di uso personale e non commerciale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Effettua l'upgrade a un piano a pagamento per continuare a utilizzare Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Oppure compila il codice sorgente GPLv3 per utilizzarlo gratuitamente.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Per visualizzare i piani di abbonamento disponibili, fai clic su "Effettua l'Upgrade" qui sotto.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Non ricordarmi più la versione di prova.
Comprendo che alla scadenza dovrò acquistare una licenza o compilare la versione GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>La versione di prova di %1 è scaduta.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Il periodo di prova è terminato. Per continuare a utilizzare %1 con tutte le funzionalità Pro, effettua l'upgrade a un piano a pagamento.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>Se preferisci, puoi anche compilare la versione open-source con licenza GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Grazie per aver provato %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Effettua L'upgrade</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Attiva</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>Apri in Modalità Limitata</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>Continua</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>Avvia Prova</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>Novità in %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua e Parser Integrati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>Analizza frame in Lua 5.4 o con modelli integrati senza codice, insieme a JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>Assistente AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>Un assistente integrato con otto provider che può creare e modificare progetti per te.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Device Control Loops</source>
        <translation>Cicli di Controllo del Dispositivo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Automate your device with an Arduino-style setup() and loop() routine that can read, write, and drive the dashboard.</source>
        <translation>Automatizza il tuo dispositivo con una routine in stile Arduino setup() e loop() che può leggere, scrivere e gestire il dashboard.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>Scansione e Trigger Oscilloscopio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>Scansione in stile oscilloscopio con cursore trigger animato trascinabile sul grafico.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Output Controls</source>
        <translation>Controlli di Uscita</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>Trasmetti al tuo dispositivo con widget di controllo e un motore di supporto protocollo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Dashboard Workspaces</source>
        <translation>Aree di Lavoro Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>Raggruppa i widget nelle tue dashboard e trovale dalla ricerca della barra delle applicazioni.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Session Database &amp; Reports</source>
        <translation>Database Sessioni e Report</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>Registra sessioni in SQLITE, riproducile ed esporta report HTML o PDF.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>Operator Deployments</source>
        <translation>Distribuzioni Operatore</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>Distribuisci una build bloccata a scopo singolo agli operatori con una modalità solo runtime.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>New Dashboard Widgets</source>
        <translation>Nuovi Widget Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>Quadranti Gauge e Meter con letture in tempo reale, più Orologio, Cronometro e Waterfall.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Dataset Transforms</source>
        <translation>Trasformazioni Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>Calibra, filtra e converti ogni canale con una funzione per dataset, oppure aggiungi dataset virtuali che calcolano nuovi canali.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>Disegna grafica completamente personalizzata dai dati in arrivo con il tuo script di disegno.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Data Tables</source>
        <translation>Tabelle Dati</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>Tabelle in stile registro live con dataset virtuali e celle modificabili.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Image Widget</source>
        <translation>Widget Immagine</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>Decodifica e visualizza frame immagine trasmessi direttamente dal tuo dispositivo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Notifications &amp; Alarms</source>
        <translation>Notifiche e Allarmi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>Allarmi dataset multi-banda con livelli di gravità, instradati al Centro Notifiche.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>Project Lock</source>
        <translation>Blocco Progetto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>Blocca un progetto per separare l'uso operativo dalla modifica, con un codice di accesso.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="193"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf e Trasferimento File</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="194"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>Input e pubblicazione MQTT, importazione Protobuf e trasferimenti XMODEM / YMODEM / ZMODEM.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="240"/>
        <source>Welcome to %1!</source>
        <translation>Benvenuto in %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="250"/>
        <source>Here's what's new in version %1.</source>
        <translation>Ecco le novità della versione %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="423"/>
        <source>Show on Startup</source>
        <translation>Mostra all'Avvio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="430"/>
        <source>Close</source>
        <translation>Chiudi</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Rimuovi dall'Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="345"/>
        <source>Device Disconnected</source>
        <translation>Dispositivo Disconnesso</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <source>Alarm</source>
        <translation type="vanished">Allarme</translation>
    </message>
    <message>
        <source>critical</source>
        <translation type="vanished">critico</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">avviso</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">Il valore %1%2 è entrato nella banda %3 (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Il valore %1%2 ha raggiunto l'allarme alto %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Il valore %1%2 ha raggiunto l'allarme basso %3%2</translation>
    </message>
    <message>
        <source>Alarms</source>
        <translation type="vanished">Allarmi</translation>
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
        <translation>NE</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>SE</source>
        <translation>SE</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="181"/>
        <source>SW</source>
        <translation>SO</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="184"/>
        <source>W</source>
        <translation>O</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="187"/>
        <source>NW</source>
        <translation>NO</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>Titolo</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>Valore</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">In attesa di dati…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>Immagini Satellitari</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Immagini Satellitari con Etichette</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>Mappa Stradale</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>Mappa Topografica</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>Terreno</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>Canvas Grigio Chiaro</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>Canvas Grigio Scuro</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>I livelli mappa aggiuntivi sono disponibili solo per gli utenti Pro.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Non possiamo offrire accesso illimitato perché la chiave API ArcGIS comporta costi reali.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Time (s)</source>
        <translation>Tempo (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Samples</source>
        <translation>Campioni</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Timeout dello script di trasmissione dopo %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>Il payload supera la dimensione massima</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="90"/>
        <source>Time (s)</source>
        <translation>Tempo (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="116"/>
        <source>Samples</source>
        <translation>Campioni</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="950"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Intervallo Griglia: %1 unità</translation>
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
        <translation>Scala di Grigi</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>Sconosciuto</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Modifica Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Nuova Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Nome:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>La Mia Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Seleziona i widget da includere:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Filtra widget…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
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
        <translation>Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="127"/>
        <source>Add Sub-folder</source>
        <translation>Aggiungi Sotto-cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="130"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Aggiungi una sotto-cartella all'interno di questa cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="138"/>
        <source>Add Workspace</source>
        <translation>Aggiungi Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="141"/>
        <source>Add a workspace inside this folder</source>
        <translation>Aggiungi uno spazio di lavoro all'interno di questa cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="151"/>
        <source>Rename</source>
        <translation>Rinomina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="153"/>
        <source>Rename folder</source>
        <translation>Rinomina cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="162"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="164"/>
        <source>Delete folder</source>
        <translation>Elimina cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="178"/>
        <source>Title</source>
        <translation>Titolo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="179"/>
        <source>Contents</source>
        <translation>Contenuti</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="269"/>
        <source>This folder is empty. Use the toolbar to add a workspace or sub-folder.</source>
        <translation>Questa cartella è vuota. Usa la barra degli strumenti per aggiungere uno spazio di lavoro o una sotto-cartella.</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="43"/>
        <source>Workspace</source>
        <translation>Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Add Widget</source>
        <translation>Aggiungi Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="146"/>
        <source>Add widget to workspace</source>
        <translation>Aggiungi widget all'area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="157"/>
        <source>Move Up</source>
        <translation>Sposta Su</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="159"/>
        <source>Move workspace up</source>
        <translation>Sposta lo spazio di lavoro verso l'alto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="169"/>
        <source>Move Down</source>
        <translation>Sposta Giù</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="171"/>
        <source>Move workspace down</source>
        <translation>Sposta l'area di lavoro verso il basso</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="181"/>
        <source>Change Icon</source>
        <translation>Cambia Icona</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Change workspace icon</source>
        <translation>Cambia l'icona dell'area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="198"/>
        <source>Rename</source>
        <translation>Rinomina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="200"/>
        <source>Rename workspace</source>
        <translation>Rinomina area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="209"/>
        <source>Delete</source>
        <translation>Elimina</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="211"/>
        <source>Delete workspace</source>
        <translation>Elimina area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="233"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="239"/>
        <source>Group</source>
        <translation>Gruppo</translation>
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
        <translation>Tipo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="285"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="323"/>
        <source>(unknown)</source>
        <translation>(sconosciuto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="303"/>
        <source>(group widget)</source>
        <translation>(widget di gruppo)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="353"/>
        <source>Remove widget from workspace</source>
        <translation>Rimuovi widget dallo spazio di lavoro</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">Rimuovi dall'area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="373"/>
        <source>No widgets in this workspace.</source>
        <translation>Nessun widget in quest'area di lavoro.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Aree di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="124"/>
        <source>Add Folder</source>
        <translation>Aggiungi Cartella</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="126"/>
        <source>Add a top-level folder</source>
        <translation>Aggiungi una cartella di primo livello</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="151"/>
        <source>Customize</source>
        <translation>Personalizza</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="153"/>
        <source>Edit workspaces manually</source>
        <translation>Modifica le aree di lavoro manualmente</translation>
    </message>
    <message>
        <source>Move Up</source>
        <translation type="vanished">Sposta Su</translation>
    </message>
    <message>
        <source>Move the selected workspace up</source>
        <translation type="vanished">Sposta l'area di lavoro selezionata verso l'alto</translation>
    </message>
    <message>
        <source>Move Down</source>
        <translation type="vanished">Sposta Giù</translation>
    </message>
    <message>
        <source>Move the selected workspace down</source>
        <translation type="vanished">Sposta l'area di lavoro selezionata verso il basso</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="135"/>
        <source>Add Workspace</source>
        <translation>Aggiungi Area di Lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="137"/>
        <source>Add workspace</source>
        <translation>Aggiungi area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="167"/>
        <source>Cleanup</source>
        <translation>Pulizia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="170"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>Rimuovi %1 riferimento/i widget il cui gruppo o dataset di destinazione non esiste più</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>No stale widget references in any workspace</source>
        <translation>Nessun riferimento widget obsoleto in alcuna area di lavoro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>Title</source>
        <translation>Titolo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="189"/>
        <source>Contents</source>
        <translation>Contenuti</translation>
    </message>
    <message>
        <source>Widgets</source>
        <translation type="vanished">Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="282"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>Nessuna area di lavoro. Aggiungine una con la barra degli strumenti sopra, oppure ripristina il layout automatico.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Il progetto non ha gruppi idonei -- aggiungi un gruppo con widget per popolare le aree di lavoro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="290"/>
        <source>Reset to Auto Layout</source>
        <translation>Ripristina Layout Automatico</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">Nessuna area di lavoro.</translation>
    </message>
</context>
</TS>