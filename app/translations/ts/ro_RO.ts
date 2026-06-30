<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="ro_RO" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Eroare Anthropic</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>Eroare de parsare flux: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>Cheie API invalidă (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>Limită de rată atinsă: %1</translation>
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
        <translation>Permiteți Controlul Dispozitivului prin AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="219"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>Aceasta permite asistentului AI să configureze dispozitive, să deschidă și să închidă conexiuni și să trimită date către hardware-ul dvs.

Fiecare acțiune asupra dispozitivului necesită în continuare aprobarea dvs. explicită per apel în chat, chiar și când auto-aprobarea este activată. Activați această opțiune doar dacă aveți încredere în furnizorul AI configurat cu acces la hardware.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="417"/>
        <source>Switch AI provider?</source>
        <translation>Schimbați furnizorul AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="418"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Schimbarea furnizorului șterge conversația curentă. Doriți să continuați?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="421"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="458"/>
        <source>AI Assistant is not available in this build</source>
        <translation>Asistentul AI nu este disponibil în această versiune</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">Asistentul AI necesită o licență Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="463"/>
        <source>Set an API key first</source>
        <translation>Setați mai întâi o cheie API</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">Asistentul AI necesită o licență Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="169"/>
        <source>AI Assistant is not available in this build</source>
        <translation>Asistentul AI nu este disponibil în această versiune</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="175"/>
        <source>AI subsystem not initialized</source>
        <translation>Subsistemul AI nu este inițializat</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="181"/>
        <source>Already busy with a previous request</source>
        <translation>Deja ocupat cu o cerere anterioară</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="495"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Bugetul de apeluri de instrumente a fost atins pentru această rundă; nu vor mai fi executate instrumente.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1093"/>
        <source>Waiting for %1 to respond. Loading the model and processing the prompt can take a while on local hardware...</source>
        <translation>Se așteaptă răspunsul de la %1. Încărcarea modelului și procesarea promptului pot dura ceva timp pe hardware local...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1952"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>Ați atins bugetul de apeluri de instrumente pentru această rundă. Nu mai solicitați instrumente. Rezumați ceea ce ați găsit până acum și, dacă sarcina este incompletă, menționați ce pași rămân pentru ca utilizatorul să vă poată spune să continuați.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Bugetul de apeluri de instrumente depășit</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="940"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Modelul a returnat un răspuns gol. Încercați să reformulați, să treceți la un alt model sau să verificați dacă cererea este permisă de filtrele de siguranță ale furnizorului.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1097"/>
        <source>Sending request to %1...</source>
        <translation>Se trimite cererea către %1...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1110"/>
        <source>Provider returned no reply</source>
        <translation>Furnizorul nu a returnat niciun răspuns</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="146"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Prompt blocat de filtrul de siguranță Gemini: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="200"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini s-a oprit fără a produce un răspuns: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="262"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="265"/>
        <source>Invalid API key (%1)</source>
        <translation>Cheie API invalidă (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="267"/>
        <source>Rate limited: %1</source>
        <translation>Limită de rată: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="269"/>
        <source>Invalid API key</source>
        <translation>Cheie API invalidă</translation>
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
        <translation>Cheie API invalidă (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="431"/>
        <source>Rate limited: %1</source>
        <translation>Limită de rată: %1</translation>
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
        <translation>Exportă Fișier Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>Imposibil de pornit serverul GRPC</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>Eșec la conectarea la %1</translation>
    </message>
</context>
<context>
    <name>API::ProcessLauncher</name>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="82"/>
        <source>No program specified</source>
        <translation>Niciun program specificat</translation>
    </message>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="88"/>
        <source>Program "%1" not found in PATH</source>
        <translation>Programul "%1" nu a fost găsit în PATH</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>Imposibil de pornit serverul TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>Permiteți Conexiuni API Externe?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Expunerea serverului API către gazde externe permite altor dispozitive din rețea să se conecteze la Serial Studio pe portul 7777.

Activați doar în rețele de încredere. Clienții nesiguri pot citi date live sau trimite comenzi către dispozitiv.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>Imposibil de repornit serverul TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>Permiteți controlul dispozitivului prin API?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Un program care utilizează API-ul local al Serial Studio solicită să trimită date către dispozitivul conectat. Permiteți clienților API să scrie pe dispozitiv?</translation>
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
        <translation>Conexiune în așteptare invalidă</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>Despre</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>Versiunea %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Copyright © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>Toate Drepturile Rezervate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 este software liber: îl puteți redistribui și/sau modifica conform termenilor Licenței Publice Generale GNU publicată de Free Software Foundation; fie versiunea 3 a Licenței, fie (la alegerea dumneavoastră) orice versiune ulterioară.

%1 este distribuit în speranța că va fi util, dar FĂRĂ NICIO GARANȚIE; fără nici măcar garanția implicită de VANDABILITATE sau POTRIVIRE PENTRU UN ANUMIT SCOP. Consultați Licența Publică Generală GNU pentru mai multe detalii.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Această configurație este licențiată pentru uz comercial și proprietar. Poate fi utilizată în aplicații cu sursă închisă și comerciale, sub rezerva termenilor licenței comerciale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Această configurație este destinată exclusiv uzului personal și evaluării. Utilizarea comercială este interzisă decât dacă este activată o licență comercială validă.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>Acest software este furnizat „ca atare" fără nicio garanție de orice fel, expresă sau implicită, inclusiv, dar fără a se limita la, garanțiile de vandabilitate sau potrivire pentru un anumit scop. În niciun caz autorul nu va fi răspunzător pentru daune rezultate din utilizarea acestui software.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Gestionare Licență</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Donează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Verificare Actualizări</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>Ce Este Nou</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">Sfaturi și Trucuri</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>Acord de Licență</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>Raportează Eroare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>Mulțumiri</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>Benchmark</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>Site Web</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="186"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="187"/>
        <source>Settings</source>
        <translation>Setări</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="245"/>
        <source>G-FORCE</source>
        <translation>FORȚĂ-G</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="283"/>
        <source>PITCH ↕</source>
        <translation>TANGAJ ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="320"/>
        <source>ROLL ↔</source>
        <translation>RULIU ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>Configurare Accelerometru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Configurează intervalul de afișare și unitățile de intrare ale accelerometrului.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Interval de Afișare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>G Maxim:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Introduceți valoarea G maximă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Configurare Intrare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Intrarea este deja în G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>Mulțumiri</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>Despre QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Schimbă Pictograma</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Schimbă pictograma utilizată pentru această acțiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Duplică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Duplică această acțiune cu toate setările sale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Șterge această acțiune din proiect</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Adaugă Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Widget-uri Disponibile</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Faceți clic pe un rând pentru a-l adăuga în spațiul de lucru.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Căutare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Nume</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(grup întreg)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Deja în spațiul de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Adaugă în spațiul de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Niciun widget disponibil.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Niciun widget nu corespunde.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 widget-uri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 din %2 widget-uri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Benzi de Alarmă</translation>
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
        <translation>Avertisment</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>Critic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>Tahometru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>Inactiv</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>Funcționare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>Atenție</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>Zonă Roșie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>Turometru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>Croazieră</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>Rapid</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>Viteză Maximă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>Temperatură Motor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>Rece</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>Cald</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>Supraîncălzire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>Presiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>Vid</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>Ridicat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>Explozie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>Tensiune Baterie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>Scăzut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>Nominal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>Nivel Combustibil</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>Gol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>Rezervă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>Putere Semnal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>Fără Semnal</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>Slab</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>Bun</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>Încărcare CPU / Sistem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>Ocupat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>Supraîncărcare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Avertisment / Critic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>Indicator (Pornit / Oprit)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>Pornit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>Indicator de Defecțiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>Defecțiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>Alege Culoarea Benzii</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>Presetări</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>Presetare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>Alege presetare…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>Clipire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>Resetează la valoarea implicită a severității</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Clic pentru a alege o culoare. Clic dreapta pentru a reseta la valoarea implicită a severității.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>Clic pentru a alege o culoare personalizată.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>Clipește LED-ul în timp ce valoarea se află în această bandă.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Nicio bandă definită. Alege o presetare mai sus sau adaugă o bandă pentru a începe.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>Aplică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>Aplică modificările la setul de date.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Aplică Presetare</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Înlocuiește benzile curente cu presetarea selectată, scalată la intervalul acestui set de date.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>Interval</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>Benzi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>Adaugă Bandă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Adaugă o bandă nouă continuând de la ultima.</translation>
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
        <translation>Severitate</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>Culoare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>Etichetă</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Alege o culoare personalizată.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">automat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(opțional)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>Mută în sus.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>Mută în jos.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>Elimină această bandă.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Nicio bandă definită. Aplică o presetare sau adaugă o bandă pentru a începe.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>Previzualizare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>Renunță la modificări.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Salvează</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Salvează modificările la setul de date.</translation>
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
        <translation>Chat nou</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>Toggle chat list</source>
        <translation>Comută lista de chat-uri</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="238"/>
        <source>Settings</source>
        <translation>Setări</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="274"/>
        <source>Manage API keys…</source>
        <translation>Gestionează cheile API…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="341"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>Export CSV vs MDF4 - care este diferența?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="344"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Bară și Indicator - când să folosesc fiecare?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="416"/>
        <source>How can I help with your project?</source>
        <translation>Cu ce te pot ajuta la proiectul tău?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="417"/>
        <source>Set up your API key to get started</source>
        <translation>Configurează cheia API pentru a începe</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="429"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Descrie ce ai dori să construiești și voi configura sursele, grupurile, seturile de date, analizoarele de trame și transformările pentru tine.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>Pentru a începe conversația, inserează o cheie API pentru furnizorul selectat. Cheile sunt criptate pe această mașină și nu părăsesc niciodată computerul tău, cu excepția comunicării cu furnizorul ales.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="453"/>
        <source>Open API Key Setup</source>
        <translation>Deschide Configurarea Cheii API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="463"/>
        <source>Get a key from %1</source>
        <translation>Obține o cheie de la %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="337"/>
        <source>List the sources in this project</source>
        <translation>Listează sursele din acest proiect</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="334"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Ajută-mă să descopăr funcțiile Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="335"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Ce poate face această aplicație pentru telemetria mea?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="336"/>
        <source>Walk me through what this project already contains</source>
        <translation>Ghidează-mă prin ceea ce conține deja acest proiect</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="340"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Ce este o bază de date de sesiuni și de ce aș folosi una?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">Export CSV vs MDF4 — care este diferența?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="342"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Ce este un analizor de trame și când am nevoie de unul?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="343"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>Când ar trebui să folosesc Lua vs JavaScript pentru analizor?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Plot, Bară și Indicator — când să folosesc fiecare?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="345"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Care este diferența dintre o transformare și un analizor de trame?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="348"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Adaugă o sursă UART pentru un Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="349"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Configurează un proiect IMU de la zero</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="350"/>
        <source>Configure an MQTT subscriber</source>
        <translation>Configurează un abonat MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="351"/>
        <source>Add a CAN bus source</source>
        <translation>Adaugă o sursă magistrală CAN</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="352"/>
        <source>Set up a Modbus poller</source>
        <translation>Configurează un dispozitiv de interogare Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="353"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Adaugă o sursă de rețea (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="354"/>
        <source>Write a CSV frame parser for me</source>
        <translation>Scrie un analizor de trame CSV pentru mine</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="355"/>
        <source>Help me parse a JSON frame</source>
        <translation>Ajută-mă să parsez o trama JSON</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="356"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>Adaugă o transformare de netezire EMA la un set de date</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="357"/>
        <source>Decode hexadecimal frames</source>
        <translation>Decodifică trame hexazecimale</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="358"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Calibrează un senzor cu o transformare liniară</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="361"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Sugerează widget-uri de tablou de bord pentru datele mele</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="362"/>
        <source>Build an executive overview workspace</source>
        <translation>Construiește un spațiu de lucru pentru prezentare generală executivă</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="363"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Adaugă un widget painter pentru o vizualizare personalizată</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="364"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Afișează Grafic, FFT și Waterfall pentru un set de date</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="365"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Grupează seturile de date în spații de lucru utile</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="644"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>Trageți fișiere sau dosare pentru ca asistentul să le citească</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="689"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>Dosarul „%1" a fost adăugat - disponibil în această sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="690"/>
        <source>Added "%1" - readable this session</source>
        <translation>„%1" a fost adăugat - disponibil în această sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="774"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Întreabă Serial Studio orice…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Clear conversation</source>
        <translation>Șterge conversația</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="838"/>
        <source>Stop generating</source>
        <translation>Oprește generarea</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="839"/>
        <source>Send message (Enter)</source>
        <translation>Trimite mesaj (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="159"/>
        <source>Provider</source>
        <translation>Furnizor</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="192"/>
        <source>Model selection</source>
        <translation>Selectare model</translation>
    </message>
    <message>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation type="vanished">Execută acțiunile de editare fără a întreba de fiecare dată. Acțiunile blocate rămân blocate.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Auto-approve edits</source>
        <translation>Aprobă automat editările</translation>
    </message>
    <message>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation type="vanished">Permite AI-ului să configureze dispozitive, să conecteze/deconecteze și să trimită date. Fiecare acțiune solicită în continuare aprobarea dvs.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="259"/>
        <source>Allow device control</source>
        <translation>Permite controlul dispozitivului</translation>
    </message>
    <message>
        <source>Manage API keys</source>
        <translation type="vanished">Gestionează cheile API</translation>
    </message>
    <message>
        <source>Working</source>
        <translation type="vanished">Se Lucrează</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation type="vanished">Gata</translation>
    </message>
    <message>
        <source>  •  cache %1k tok</source>
        <translation type="vanished">•  cache %1k tok</translation>
    </message>
    <message>
        <source>  •  cache write %1k tok</source>
        <translation type="vanished">scriere cache %1k tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Niciun Microfon Detectat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Conectează un microfon sau verifică setările</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Dispozitiv de Intrare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Rată de Eșantionare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Format Eșantion</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Canale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Dispozitiv de Ieșire</translation>
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
        <translation type="vanished">Furnizați numele de utilizator și parola pentru locația de descărcare.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">&amp;Nume utilizator:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Parolă:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="65"/>
        <source>Axis Range Configuration</source>
        <translation>Configurare Interval Axă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="179"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Configurează intervalul vizibil pentru axele graficului. Valorile se actualizează în timp real pe măsură ce tastezi.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="187"/>
        <source>X Axis</source>
        <translation>Axa X</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="212"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="283"/>
        <source>Minimum:</source>
        <translation>Minim:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="224"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="295"/>
        <source>Enter min value</source>
        <translation>Introduceți valoarea minimă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="233"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="304"/>
        <source>Maximum:</source>
        <translation>Maxim:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="245"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="316"/>
        <source>Enter max value</source>
        <translation>Introduceți valoarea maximă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="260"/>
        <source>Y Axis</source>
        <translation>Axa Y</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="335"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="345"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Recuperează Copia de Siguranță</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Fără Titlu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Proiect Încărcat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Salvare Automată</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Înainte de Restaurare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Înainte de Ștergerea Setului de Date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Înainte de Ștergerea Grupului</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Înainte de Proiect Nou</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Înainte de Deschiderea Proiectului</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Înainte de Încărcarea JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Înainte de Aplicarea Șablonului</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Înainte de Ștergerea Acțiunii</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Înainte de Ștergerea Widget-ului de Ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Înainte de Mutarea Setului de Date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Înainte de Mutarea Grupului</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Înainte de Ștergerea Spațiului de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Înainte de Ștergerea Tuturor Spațiilor de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Înainte de Eliminarea Widget-ului</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Înainte de Reordonarea Spațiilor de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Înainte de Operația în Lot</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Înainte de Adăugarea Caroului</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (și încă %2)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>Codul analizorului de trame diferă de asemenea și va fi înlocuit.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Titlul se schimbă de la "%1" la "%2". Structura grupului rămâne neschimbată.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Aceleași grupuri și seturi de date, dar codul analizorului de trame diferă — restaurarea îl va înlocui.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Aceleași grupuri și seturi de date ca proiectul curent. Restaurarea poate totuși anula modificările la nivel de câmp.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Restaurarea elimină %1 și readuce %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Restaurarea elimină %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Restaurarea readuce %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Alege o copie de siguranță pentru restaurare. Proiectul curent este salvat automat mai întâi, astfel încât restaurarea este reversibilă.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Nicio copie de siguranță pentru acest proiect încă. Editează sau salvează proiectul pentru a porni copia de siguranță rotativă.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Deschide Folder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Restaurează</translation>
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
        <translation>%1 cadre/s</translation>
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
        <translation>Trecut</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>Eșuat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>Benchmark Hotpath</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Măsoară cât de rapid poate acest calculator să extragă, să analizeze și să vizualizeze cadrele prin pipeline-ul de date al Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>Interfața va îngheța în timpul rulării</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Fiecare fază rulează la capacitate maximă pe firul principal, astfel încât fereastra nu mai răspunde până la finalizare. Proiectul curent este reîncărcat automat la încheierea testului de performanță.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>Cadre per fază:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>Durată minimă:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>Etape</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>Parsere</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>Export date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>Tablou de Bord</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>Date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>Doar valori numerice</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>Mixt (numeric + text)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>Selectați cel puțin o etapă și un tip de date pentru a rula un benchmark.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>În Execuție %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>Se Pregătește...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>Debit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>Timp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>Rezultat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>Rulați un test pentru a vedea rezultatele</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>Promovat/Respins se aplică doar etapelor de procesare date și analiză (procesare date și Parser încorporat numeric 1024 K cadre/s; încorporat mixt 512 K; Lua numeric 256 K; JavaScript numeric și Lua mixt 128 K; JavaScript mixt 64 K). Etapele de export și tablou de bord sunt informative.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Promovat/Respins se aplică doar fazelor de conductă de date și analiză (conductă de date și Parser încorporat numeric 1024 K cadre/s; Parser încorporat mixt 512 K; Lua numeric 256 K; JavaScript numeric și Lua mixt 128 K; JavaScript mixt 64 K). Fazele de export și tablou de bord sunt informative.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Pass/Fail se aplică fazelor de pipeline de date și parser (pipeline de date și Native numeric 1024 K cadre/s; Native mixt 512 K; Lua numeric 256 K; JavaScript numeric și Lua mixt 128 K; JavaScript mixt 64 K). Fazele de export și dashboard sunt informative.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Promovat/Respins se aplică fazelor de conductă de date și analiză (conductă date 1024 K cadre/s; numeric: Lua 256 K, JavaScript 128 K; mixt: Lua 128 K, JavaScript 64 K). Fazele de export și tablou de bord sunt informative.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Promovat/Respins se aplică doar fazelor de analiză (țintă Lua 256 K cadre/s, JavaScript 128 K). Fazele de export și tablou de bord sunt informative.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>Se Execută...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>Execută Benchmark</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>Conductă de date</translation>
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
        <translation type="vanished">Lua + export date</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + tablou de bord</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Parser nativ (numeric)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Parser nativ (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Parser încorporat (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Parser încorporat (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Parser Lua (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>Parser JavaScript (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Parser Lua (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>Parser JavaScript (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Încorporat + export date (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + export date (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + export date (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Încorporat + export date (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + export date (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + export date (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Încorporat + tablou de bord (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + tablou de bord (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 K cadre</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 K cadre</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 K cadre</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 M cadre</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>1 second</source>
        <translation>1 secundă</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>2 secunde</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 secunde</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 secunde</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - Benchmark Hotpath</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), sarcină: minim %3 cadre, minim %4 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>Debit</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>Țintă</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>Timp</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>Rezultat</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 cadre/s</translation>
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
        <translation>Trecut</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>Eșuat</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + tablou de bord (numeric)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>Încorporat + tablou de bord (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + tablou de bord (mixt)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + tablou de bord (mixt)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Se afișează tabloul de bord...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Serviciu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Caracteristică</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Se Scanează…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Niciun Adaptor Bluetooth Detectat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Conectează un adaptor Bluetooth sau verifică setările sistemului</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Acest Sistem de Operare nu Este Suportat Încă.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio va fi actualizat pentru a funcționa cu acest sistem de operare de îndată ce QT îl va suporta oficial</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Niciun Driver CAN Găsit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>Instalează drivere hardware CAN pentru sistemul tău</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>Driver CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="140"/>
        <source>Interface</source>
        <translation>Interfață</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="169"/>
        <source>Bitrate</source>
        <translation>Rată de Biți</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="223"/>
        <source>Flexible Data-Rate</source>
        <translation>Rată Flexibilă de Date</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="243"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="263"/>
        <source>Listen-Only</source>
        <translation>Doar Ascultare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="283"/>
        <source>DBC Database</source>
        <translation>Bază de Date DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="287"/>
        <source>Import DBC File…</source>
        <translation>Importă Fișier DBC…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="320"/>
        <source>No CAN Interfaces Found</source>
        <translation>Nicio Interfață CAN Găsită</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>Selectează fișier CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>Fișiere CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>Conexiune Dispozitiv Activă</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Pentru a utiliza această funcție, trebuie să vă deconectați de la dispozitiv. Doriți să continuați?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>Verifică permisiunile și locația fișierului</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Date Insuficiente în Fișierul CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>Nu se poate citi fișierul CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>Fișierul CSV trebuie să conțină cel puțin un rând de date pentru a continua. Verificați fișierul și încercați din nou.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>CSV Invalid</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>Fișierul CSV nu conține date sau anteturi.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>Selectați o coloană de dată/oră</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>Setare manuală a intervalului</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>Selectare Dată/oră CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Alegeți cum să gestionați datele de dată/oră:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>Setare Interval</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Introduceți intervalul dintre rânduri în milisecunde:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>Selectează Coloana Dată/oră</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Selectați coloana care conține datele de dată/oră:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>Selecție Invalidă</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>Coloana selectată nu este validă.</translation>
    </message>
</context>
<context>
    <name>ChatSidebar</name>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="44"/>
        <source>Chats</source>
        <translation>Chat-uri</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="57"/>
        <location filename="../../qml/AI/ChatSidebar.qml" line="115"/>
        <source>New chat</source>
        <translation>Chat nou</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="125"/>
        <source>%1 messages</source>
        <translation>%1 mesaje</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="147"/>
        <source>Rename...</source>
        <translation>Redenumește…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="158"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="197"/>
        <source>Rename chat</source>
        <translation>Redenumește chat-ul</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="217"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="222"/>
        <source>Rename</source>
        <translation>Redenumește</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>Consolă</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>Exportul Consolei este o funcție Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Această funcție necesită o licență. Achiziționați una pentru a activa exportul consolei.</translation>
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
        <translation>Fără Terminare de Linie</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>Linie Nouă</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>Retur de Car</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>Text Simplu</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>Hexazecimal</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>Fără Checksum</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>Dispozitiv %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Inserează Constantă</translation>
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
        <translation>Viteza luminii în vid</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Constanta Planck</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Sarcina elementară</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Constanta Avogadro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Constanta Boltzmann</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Constanta Stefan-Boltzmann</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>Mecanică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>Gravitație standard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Constanta gravitațională</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Presiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Atmosferă standard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Presiune barometrică la nivelul mării</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>Temperatură</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>Zero absolut (Celsius)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Punct de îngheț al apei</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Punct de fierbere al apei (1 atm)</translation>
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
        <translation>Gaze și Fluide</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Constantă universală a gazelor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Constantă specifică a gazelor (aer uscat)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Constantă specifică a gazelor (vapori de apă)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Densitate aer (nivel mării, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Densitatea apei (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Viteza sunetului în aer (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Raportul capacităților termice (aer uscat)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>Electromagnetism</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>Permitivitatea vidului</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Permeabilitatea vidului</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Matematică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Numărul lui Euler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Raportul de aur</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Constante Fizice</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>Valori predefinite în unități SI. Faceți clic pe un rând pentru a-l insera în %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Căutare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Simbol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Nume</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Valoare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Categorie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Nicio constantă nu corespunde.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 constante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 din %2 constante</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script de Control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Loop</source>
        <translation>Buclă de Control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>Refă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>Decupează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>Lipește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>Selectează Tot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>Formatează Documentul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>Formatează Selecția</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control loop</source>
        <translation>Resetează la bucla de control implicită</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control loop file</source>
        <translation>Importă un fișier buclă de control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control loop documentation</source>
        <translation>Deschide documentația buclei de control</translation>
    </message>
    <message>
        <source>Reset to the default control script</source>
        <translation type="vanished">Resetează la scriptul de control implicit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>Deschide</translation>
    </message>
    <message>
        <source>Import a control script file</source>
        <translation type="vanished">Importă un fișier script de control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>Anulează ultima editare de cod</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>Refă editarea anulată anterior</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>Decupează codul selectat în clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>Copiază codul selectat în clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>Lipește cod din clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>Ajutor</translation>
    </message>
    <message>
        <source>Open the control script documentation</source>
        <translation type="vanished">Deschide documentația scriptului de control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>Validează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Verifică că scriptul se compilează corect</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Opțiuni de Recuperare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio s-a închis neașteptat de mai multe ori la rând.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>Blocări consecutive: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Ultima etapă raportată: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Detectat la: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Alege o acțiune de recuperare. Serial Studio se va închide după aplicare pentru ca următoarea pornire să fie curată.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Resetează Backend-ul de Randare la Implicit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Sari peste Restaurarea Ultimului Proiect Deschis</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Resetează toate preferințele</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Continuă Oricum</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>Player CSV</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>Previzualizare Fișier DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>Fișier DBC: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Revizuiește mesajele și semnalele CAN pentru import într-un proiect nou Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Mesaje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Nume Mesaj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>ID CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Semnale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>Nu s-au găsit mesaje în fișierul DBC.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Total: %1 mesaje, %2 semnale</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Creează Proiect</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="262"/>
        <source>Dashboard %1</source>
        <translation>Tablou de Bord %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>Trimite</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Nicio funcție de transmisie definită</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="326"/>
        <source>Set Wallpaper…</source>
        <translation>Setează Imagine de Fundal…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="332"/>
        <source>Clear Wallpaper</source>
        <translation>Șterge Imagine de Fundal</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="342"/>
        <source>Tile Windows</source>
        <translation>Aranjează Ferestrele</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="361"/>
        <source>Pro features detected in this project.</source>
        <translation>Funcționalități Pro detectate în acest proiect.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="363"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Widget-uri de rezervă sunt active. Achiziționați o licență pentru funcționalitate completă.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="504"/>
        <source>Empty Workspace</source>
        <translation>Spațiu de Lucru Gol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="518"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Utilizați bara de căutare pentru a găsi și adăuga widget-uri, sau faceți clic dreapta pe un widget dintr-un alt spațiu de lucru pentru a-l adăuga aici.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="533"/>
        <source>Search Widgets</source>
        <translation>Căutare Widget-uri</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>Tablou de Bord</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="215"/>
        <source>API Server Active (%1)</source>
        <translation>Server API Activ (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="216"/>
        <source>API Server Ready</source>
        <translation>Server API Pregătit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="217"/>
        <source>API Server Off</source>
        <translation>Server API Oprit</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="155"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="307"/>
        <source>Send</source>
        <translation>Trimite</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="295"/>
        <source>Enter command…</source>
        <translation>Introduceți comandă…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Nicio funcție de transmisie definită</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Introduceți comandă…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Trimite</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Nicio funcție de transmisie definită</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>PORNIT</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>OPRIT</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>Nicio funcție de transmisie definită</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Resume</source>
        <translation>Reia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="313"/>
        <source>Awaiting data…</source>
        <translation>Se așteaptă date…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="368"/>
        <source>Open %1 in a separate window</source>
        <translation>Deschide %1 într-o fereastră separată</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="289"/>
        <source>Select Javascript file to import</source>
        <translation>Selectează fișier JavaScript de importat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="368"/>
        <source>Code Validation Failed</source>
        <translation>Validare Cod Eșuată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="358"/>
        <source>Line %1: %2</source>
        <translation>Linia %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="369"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>Scriptul trebuie să definească o funcție setup() și/sau loop().</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="374"/>
        <source>Code Validation Successful</source>
        <translation>Validare Cod Reușită</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="375"/>
        <source>No syntax errors detected in the control loop.</source>
        <translation>Nu au fost detectate erori de sintaxă în bucla de control.</translation>
    </message>
    <message>
        <source>No syntax errors detected in the control script.</source>
        <translation type="vanished">Nu s-au detectat erori de sintaxă în scriptul de control.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>Importă Fișier DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>Fișiere DBC (*.DBC);;Toate Fișierele (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>Eșec la parsarea fișierului DBC: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>Verifică formatul fișierului și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>Eroare Import DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>Fișierul DBC nu conține mesaje</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Fișierul selectat nu conține definiții de mesaje CAN.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>Avertisment Import DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>Prezentare Generală</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>Activ</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Eșec la crearea fișierului de proiect temporar</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">Verifică dacă aplicația are permisiuni de scriere în directorul temporar.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>Fișier DBC importat cu succes cu %1 mesaje și %2 semnale.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editorul de proiect este acum deschis pentru personalizare.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Încărcarea proiectului importat a eșuat</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">JSON-ul proiectului generat nu a putut fi încărcat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>Au fost omise %1 semnal(e) care utilizează multiplexare extinsă (SG_MUL_VAL_); doar multiplexarea simplă este suportată.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>Import DBC Finalizat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="254"/>
        <source>CAN Bus</source>
        <translation>Magistrala CAN</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="63"/>
        <source>Dataset Value Transform</source>
        <translation>Transformare Valoare Set de Date</translation>
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
        <translation>Limbaj:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="132"/>
        <source>Template:</source>
        <translation>Șablon:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Introduceți valoarea brută (de ex., 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Test</source>
        <translation>Testează</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="112"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="143"/>
        <source>Input:</source>
        <translation>Intrare:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="146"/>
        <source>Output:</source>
        <translation>Ieșire:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="114"/>
        <source>Apply</source>
        <translation>Aplică</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="115"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="224"/>
        <source>Transform — %1</source>
        <translation>Transformare — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="304"/>
        <source>The value transform has a syntax error and was not applied.</source>
        <translation>Transformarea valorii conține o eroare de sintaxă și nu a fost aplicată.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>The value transform must define a transform(value) function.</source>
        <translation>Transformarea valorii trebuie să definească o funcție transform(value).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="314"/>
        <source>Define a transform(value) function that returns a number, or use Clear to remove the transform.</source>
        <translation>Definiți o funcție transform(value) care returnează un număr sau folosiți Șterge pentru a elimina transformarea.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="332"/>
        <source>Enter a value</source>
        <translation>Introduceți o valoare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="339"/>
        <source>Invalid number</source>
        <translation>Număr invalid</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="408"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatează Documentul	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="409"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatează Selecția	ctrl+i</translation>
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
-- Definiți o funcție transform(value) care primește citirea
-- live a setului de date și returnează un număr transformat.
-- Dacă nu este definită nicio funcție transform(), valoarea
-- brută este păstrată și nimic nu este salvat cu proiectul.
--
-- Exemplu:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Variabilele globale declarate la nivel superior persistă
-- între cadre, ceea ce este util pentru filtre, acumulatori
-- și orice altă transformare cu stare, la fel ca un script
-- de analiză a cadrelor:
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
-- Utilizați meniul Template pentru exemple gata făcute sau
-- faceți clic pe Testează pentru a încerca funcția.
--</translation>
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
 * Definește o funcție transform(value) care primește citirea
 * live a setului de date și returnează un număr transformat. Dacă
 * nu este definită nicio funcție transform(), valoarea brută este
 * păstrată și nimic nu este salvat cu proiectul.
 *
 * Exemplu:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Variabilele globale declarate la nivel superior persistă între
 * cadre, ceea ce este util pentru filtre, acumulatori și orice
 * altă transformare cu stare -- exact ca un script de analiză cadre:
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
 * Folosește meniul Șablon pentru exemple gata făcute, sau
 * apasă Test pentru a încerca funcția ta.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="608"/>
        <source>Failed to create the Lua engine.</source>
        <translation>Eșec la crearea motorului Lua.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="646"/>
        <source>Line %1: %2</source>
        <translation>Linia %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="800"/>
        <source>Select Template…</source>
        <translation>Selectează Șablon…</translation>
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
 * Definește o funcție transform(value) care primește citirea
 * live a setului de date și returnează un număr transformat.
 * Dacă nu este definită nicio funcție transform(), valoarea
 * brută este păstrată și nimic nu este salvat cu proiectul.
 *
 * Exemplu:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Variabilele globale declarate la nivel superior persistă
 * între cadre, ceea ce este util pentru filtre, acumulatori
 * și orice altă transformare cu stare — la fel ca un script
 * de analiză a cadrelor:
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
 * Folosește meniul Șablon pentru exemple gata făcute sau
 * fă clic pe Testează pentru a încerca funcția ta.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <source>Engine error</source>
        <translation>Eroare motor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="714"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="727"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="744"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="755"/>
        <source>Error: %1</source>
        <translation>Eroare: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="720"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="748"/>
        <source>Error: transform() not defined</source>
        <translation>Eroare: transform() nu este definită</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="732"/>
        <source>Error: transform() must return a number</source>
        <translation>Eroare: transform() trebuie să returneze un număr</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1705"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1819"/>
        <source>Channel %1</source>
        <translation>Canal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1830"/>
        <source>Audio Input</source>
        <translation>Intrare Audio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1714"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1835"/>
        <source>Quick Plot</source>
        <translation>Grafic Rapid</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1429"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>Transformarea JavaScript a depășit bugetul</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1430"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>O transformare de dataset a durat mai mult de %1 ms; seturile de date rămase din cadrul frame-ului au revenit la valorile brute până la următorul frame. Profilează sau simplifică codul de transformare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="230"/>
        <source>Frame pool exhausted</source>
        <translation>Pool-ul de frame-uri epuizat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="232"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>Un consumator din aval (dashboard, export CSV/MDF4, sesiune DB sau abonat API) nu golește frame-urile suficient de rapid. Serial Studio revine la alocări per-frame până se elimină backlog-ul. Dezactivează un consumator intensiv sau reduce rata de date.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1666"/>
        <source>Device A</source>
        <translation>Dispozitiv A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1721"/>
        <source>Quick Plot Data</source>
        <translation>Date Grafic Rapid</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1733"/>
        <source>Multiple Plots</source>
        <translation>Grafice Multiple</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>Text simplu (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>Hexazecimal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>Binar (octeți bruti)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>Doar delimitator final</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>Delimitatori inițial + final</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>Doar delimitator inițial</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Fără delimitatori (bloc întreg)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>Fără Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>Selectează Șablon Parser de Trame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>Alege un șablon de încărcat:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Intrare hexazecimală invalidă.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>Niciun șablon selectat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 trame extrase | %2 octeți consumați | %3 octeți în buffer | %4 eliminați</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>JSON Invalid: %1</translation>
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
        <translation type="vanished">Niciunul</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Intrare HEX Invalidă</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Introduceți octeți hexazecimali valizi.

Format valid: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(nicio trama)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">Extracția nu a produs o trama completă. Verificați delimitatorii de început / sfârșit și modul de detecție.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 trame extrase | %2 octeți consumați | %3 octeți în buffer | %4 eliminați</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Configurare Pipeline</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Rezultate Pipeline</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Detecție</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Decodor</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Checksum</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Delimitator Inițial</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">Delimitator Final</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Delimitatori Hex</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Doar delimitator final</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Delimitatori inițial + final</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Doar delimitator inițial</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Fără delimitatori (bloc întreg)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Text simplu (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexazecimal</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binar (octeți bruti)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Șterge</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Evaluează</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Introduceți octeții fluxului brut aici...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Pregătire</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Intrare Date Trame</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Rezultate Analizor Trame</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Introduceți date trame aici…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Index Set de Date</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Valoare</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Introduceți datele cadrului mai sus, activați modul HEX dacă este necesar, apoi faceți clic pe „Evaluează" pentru a rula parserul de cadre.

Exemplu (Text): a,b,c,d,e,f
Exemplu (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Testează Parserul de Cadre</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Introduceți octeți hexazecimali (ex., 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(gol)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Nu s-au returnat date</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Change Scripting Language?</source>
        <translation>Schimbați Limbajul de Scriptare?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="220"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Schimbarea limbajului de scriptare înlocuiește codul parserului curent cu șablonul echivalent în noul limbaj.

Orice modificări nesalvate se pierd. Continuați?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Javascript file to import</source>
        <translation>Selectează fișier JavaScript de importat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Lua file to import</source>
        <translation>Selectează fișier Lua de importat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>Code Validation Successful</source>
        <translation>Validare Cod Reușită</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="415"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>Nu au fost detectate erori de sintaxă în codul parserului.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Select Frame Parser Template</source>
        <translation>Selectează Șablon Parser de Trame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="526"/>
        <source>Choose a template to load:</source>
        <translation>Alege un șablon de încărcat:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>Importă Hartă Registre Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Hărți Registre Modbus (*.CSV *.XML *.JSON);;Fișiere CSV (*.CSV);;Fișiere XML (*.XML);;Fișiere JSON (*.JSON);;Toate Fișierele (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>Niciun registru găsit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Fișierul nu a putut fi parsat sau nu conține definiții de registre.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>Import Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>Prezentare Generală</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>Pornit</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Încărcarea proiectului importat a eșuat</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">JSON-ul proiectului generat nu a putut fi încărcat.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Eșec la crearea fișierului de proiect temporar</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Verifică permisiunile de scriere în directorul temporar.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>Au fost importate cu succes %1 registre în %2 grupuri.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editorul de proiect este acum deschis pentru personalizare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>Import Modbus Finalizat</translation>
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
        <translation type="vanished">Text simplu (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Hexazecimal</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binar (octeți bruti)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Doar delimitator final</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Delimitatori inițial + final</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Doar delimitator inițial</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Fără delimitatori (bloc întreg)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">Fără Checksum</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">Intrare hexazecimală invalidă.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">Niciun șablon selectat.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 trame extrase | %2 octeți consumați | %3 octeți în buffer | %4 eliminați</translation>
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
        <translation>Selectează fișier JavaScript pentru import</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>Selectează Șablon Widget de Ieșire</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>Alege un șablon de încărcat:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>Selectează fișier JavaScript de importat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>Selectează Șablon Widget Painter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>Alege un șablon de încărcat:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>Adaugă seturi de date pentru acest șablon?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" necesită %2 set(uri) de date; grupul curent are %3.

Adaugă %4 set(uri) de date folosind valorile implicite ale șablonului?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2491"/>
        <source>Project Information</source>
        <translation>Informații Proiect</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2501"/>
        <source>Project Title</source>
        <translation>Titlu Proiect</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2502"/>
        <source>Untitled Project</source>
        <translation>Proiect Fără Titlu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2503"/>
        <source>Name or description of the project</source>
        <translation>Nume sau descriere a proiectului</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2634"/>
        <source>Time</source>
        <translation>Timp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2634"/>
        <source>Samples</source>
        <translation>Eșantioane</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2646"/>
        <source>Plot every curve against time or against the sample number</source>
        <translation>Trasează fiecare curbă în funcție de timp sau de numărul eșantionului</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2662"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2664"/>
        <source>Web address to load in this widget</source>
        <translation>Adresa web de încărcat în acest widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2772"/>
        <source>Frame Detection</source>
        <translation>Detectare Cadru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2786"/>
        <source>Frame Detection Method</source>
        <translation>Metodă de Detectare Cadru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2787"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Selectați cum sunt identificate cadrele de date primite</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2797"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Delimitatori Hexazecimali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2798"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Introduceți secvențele de început/sfârșit de cadru ca valori hexazecimale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Frame Start Delimiter</source>
        <translation>Delimitator de Început Cadru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2815"/>
        <source>e.g. /*</source>
        <translation>ex. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Secvență care marchează începutul unui cadru de date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2828"/>
        <source>Frame End Delimiter</source>
        <translation>Delimitator Sfârșit Cadru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2829"/>
        <source>e.g. */</source>
        <translation>ex. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2830"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Secvență care marchează sfârșitul unui cadru de date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2836"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Procesare și Validare Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2847"/>
        <source>Data Conversion Method</source>
        <translation>Metodă Conversie Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2848"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Selectați cum sunt decodate datele binare primite înainte de parsare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2864"/>
        <source>Checksum Algorithm</source>
        <translation>Algoritm Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2865"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Selectați algoritmul checksum utilizat pentru validarea cadrelor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2521"/>
        <source>Group Information</source>
        <translation>Informații Grup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2531"/>
        <source>Group Title</source>
        <translation>Titlu Grup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2532"/>
        <source>Untitled Group</source>
        <translation>Grup Fără Titlu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2533"/>
        <source>Title or description of this dataset group</source>
        <translation>Titlul sau descrierea acestui grup de seturi de date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2704"/>
        <source>Composite Widget</source>
        <translation>Widget Compozit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2705"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Selectați modul în care acest grup de seturi de date ar trebui vizualizat (opțional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2583"/>
        <source>Image Configuration</source>
        <translation>Configurare Imagine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3444"/>
        <source>Virtual Dataset</source>
        <translation>Set de Date Virtual</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3445"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Seturile de date virtuale își calculează valoarea din transformări și tabele de date, nu necesită un index de cadru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3848"/>
        <source>Fixed decimal places for the value display; overrides the format (-1 = auto)</source>
        <translation>Zecimale fixe pentru afișajul valorii; suprascrie formatul (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3977"/>
        <source>Auto-detect</source>
        <translation>Detectare Automată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3977"/>
        <source>Manual Delimiters</source>
        <translation>Delimitatori Manuali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2596"/>
        <source>Detection Mode</source>
        <translation>Mod Detectare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1421"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1424"/>
        <source>Frame Parser</source>
        <translation>Analizor de Trame</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation type="vanished">Grupuri</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1696"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1709"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1710"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1727"/>
        <source>Shared Memory</source>
        <translation>Memorie Partajată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1696"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1715"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1716"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5438"/>
        <source>Dataset Values</source>
        <translation>Valori Set de Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1876"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1889"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1890"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1905"/>
        <source>Workspaces</source>
        <translation>Spații de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1922"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1925"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1926"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script de Control</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1999"/>
        <source>Publishing</source>
        <translation>Publicare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2009"/>
        <source>Enable Publishing</source>
        <translation>Activează Publicarea</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2010"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Difuzează cadrele, octeții bruti și notificările către broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2021"/>
        <source>Payload</source>
        <translation>Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2022"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Selectează ce se publică: date parsate din tablou de bord sau octeți RX bruti</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2032"/>
        <source>Publish Rate (Hz)</source>
        <translation>Rată Publicare (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2033"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>De câte ori pe secundă se publică (1-30 Hz). Rate mai mari cresc sarcina brokerului; datele din tabloul de bord sunt limitate în rată astfel încât un broker lent nu blochează niciodată parsarea cadrelor.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2045"/>
        <source>Topic Base</source>
        <translation>Bază Topic</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2046"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2047"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Subiect de bază utilizat pentru publicarea cadrelor și a octeților bruti</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2057"/>
        <source>Script Topic</source>
        <translation>Subiect Script</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2058"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2082"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>Implicit Subiect de Bază când este gol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2059"/>
        <source>Topic the user script publishes to</source>
        <translation>Subiect pe care scriptul utilizatorului publică</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2069"/>
        <source>Publish Notifications</source>
        <translation>Publică Notificări</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2070"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Oglindește notificările tabloului de bord către un subiect dedicat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2081"/>
        <source>Notification Topic</source>
        <translation>Subiect Notificări</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2083"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Subiect unde sunt oglindite notificările tabloului de bord</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2096"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2106"/>
        <source>Hostname</source>
        <translation>Hostname</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2107"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2108"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>Hostname sau adresa IP a brokerului MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2117"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2118"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>Port TCP expus de broker (1883 simplu, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2128"/>
        <source>Custom Client ID</source>
        <translation>ID Client Personalizat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2130"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Dezactivat: un ID aleatoriu nou este generat la fiecare încărcare de proiect. Activat: se folosește ID-ul de mai jos.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2141"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2142"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Identificator trimis către broker la CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2155"/>
        <source>Protocol Version</source>
        <translation>Versiune Protocol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2156"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>Versiunea protocolului MQTT utilizată la CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2165"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2166"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Secunde între pachetele PINGREQ în stare inactivă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2175"/>
        <source>Clean Session</source>
        <translation>Sesiune Curată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2176"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Elimină orice stare de sesiune persistentă la CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2191"/>
        <source>Username</source>
        <translation>Nume Utilizator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2192"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Nume de utilizator pentru autentificarea la broker (lăsați gol pentru anonim)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2202"/>
        <source>Password</source>
        <translation>Parolă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2203"/>
        <source>Password for broker authentication</source>
        <translation>Parolă pentru autentificarea la broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2214"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2224"/>
        <source>Use SSL/TLS</source>
        <translation>Utilizare SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2225"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Tunelare conexiune broker prin TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2238"/>
        <source>Protocol</source>
        <translation>Protocol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2239"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Familia de protocol TLS negociată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2249"/>
        <source>Peer Verify</source>
        <translation>Verificare Peer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2250"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Cât de strict este validat lanțul de certificate al brokerului</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2260"/>
        <source>Verify Depth</source>
        <translation>Adâncime Verificare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Lungimea maximă a lanțului de certificate acceptată (0 = nelimitată)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2548"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3182"/>
        <source>Device %1</source>
        <translation>Dispozitiv %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2566"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2731"/>
        <source>Input Device</source>
        <translation>Dispozitiv de Intrare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2567"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Selectați dispozitivul conectat care furnizează date pentru acest grup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2598"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Detectarea automată citește octeții magici JPEG/PNG; Manual folosește secvențe explicite de start/sfârșit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2608"/>
        <source>Start Sequence (Hex)</source>
        <translation>Secvență de Start (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2609"/>
        <source>e.g. FF D8 FF</source>
        <translation>de ex. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2610"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Octeți hex care marchează începutul unei trame de imagine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2619"/>
        <source>End Sequence (Hex)</source>
        <translation>Secvență de Încheiere (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2620"/>
        <source>e.g. FF D9</source>
        <translation>de ex. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2621"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Octeți hex care marchează sfârșitul unei trame de imagine</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">Identitate</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2741"/>
        <source>Device Name</source>
        <translation>Nume Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2742"/>
        <source>Device 1</source>
        <translation>Dispozitiv 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2743"/>
        <source>Human-readable name for this input device</source>
        <translation>Nume lizibil pentru acest dispozitiv de intrare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2752"/>
        <source>Bus Type</source>
        <translation>Tip Magistrală</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2753"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Selectați interfața hardware pentru acest dispozitiv de intrare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2755"/>
        <source>Serial Port</source>
        <translation>Port Serial</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2755"/>
        <source>Network Socket</source>
        <translation>Socket de Rețea</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2755"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2757"/>
        <source>Audio Input</source>
        <translation>Intrare Audio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2757"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2757"/>
        <source>CAN Bus</source>
        <translation>Magistrala CAN</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2757"/>
        <source>Raw USB</source>
        <translation>USB Brut</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2758"/>
        <source>HID Device</source>
        <translation>Dispozitiv HID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2758"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2758"/>
        <source>MQTT Subscriber</source>
        <translation>Abonat MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2913"/>
        <source>Connection Settings</source>
        <translation>Setări de Conexiune</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3150"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3420"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5105"/>
        <source>General Information</source>
        <translation>Informații Generale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3159"/>
        <source>Action Title</source>
        <translation>Titlu Acțiune</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3161"/>
        <source>Untitled Action</source>
        <translation>Acțiune Fără Titlu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3162"/>
        <source>Name or description of this action</source>
        <translation>Nume sau descriere a acestei acțiuni</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3171"/>
        <source>Action Icon</source>
        <translation>Pictogramă Acțiune</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3172"/>
        <source>Default Icon</source>
        <translation>Pictogramă Implicită</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3173"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Pictograma afișată pentru această acțiune în tabloul de bord</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3200"/>
        <source>Target Device</source>
        <translation>Dispozitiv Țintă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3201"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Selectează dispozitivul conectat căruia această acțiune îi trimite date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3213"/>
        <source>Data Payload</source>
        <translation>Sarcină de Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3224"/>
        <source>Send as Binary</source>
        <translation>Trimite ca Binar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3225"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Trimite date binare brute când această acțiune este declanșată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3236"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3248"/>
        <source>Command</source>
        <translation>Comandă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3237"/>
        <source>Transmit Data (Hex)</source>
        <translation>Transmite Date (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3238"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Sarcină hexazecimală de trimis când acțiunea este declanșată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3249"/>
        <source>Transmit Data</source>
        <translation>Transmite Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3250"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Sarcină text de trimis când acțiunea este declanșată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3261"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5164"/>
        <source>Text Encoding</source>
        <translation>Codificare Text</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3262"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Codificarea caracterelor folosită pentru serializarea sarcinii text</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3286"/>
        <source>End-of-Line Sequence</source>
        <translation>Secvență de Terminare A Liniei</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3287"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>Caractere EOL de adăugat la mesaj (ex. </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3299"/>
        <source>Execution Behavior</source>
        <translation>Comportament de Execuție</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3310"/>
        <source>Auto-Execute on Connect</source>
        <translation>Execuție Automată la Conectare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3311"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Declanșează automat această acțiune când dispozitivul se conectează</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3317"/>
        <source>Timer Behavior</source>
        <translation>Comportament Temporizator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3326"/>
        <source>Timer Mode</source>
        <translation>Mod Temporizator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3329"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Alege când și cum această acțiune ar trebui să se repete automat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3336"/>
        <source>Interval (ms)</source>
        <translation>Interval (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3340"/>
        <source>Timer Interval (ms)</source>
        <translation>Interval Temporizator (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3341"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Milisecunde între fiecare declanșare repetată a acestei acțiuni</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3348"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3352"/>
        <source>Repeat Count</source>
        <translation>Număr de Repetări</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3353"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Numărul de ori de trimis comanda la fiecare declanșare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3430"/>
        <source>Untitled Dataset</source>
        <translation>Set de Date Fără Titlu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3431"/>
        <source>Dataset Title</source>
        <translation>Titlu Set de Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3432"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Numele setului de date, utilizat pentru etichetare și identificare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3461"/>
        <source>Hide on Dashboard</source>
        <translation>Ascunde în Tablou de Bord</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3462"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Suprimă caseta de tablou de bord independentă a acestui set de date; widget-ul painter poate citi în continuare valorile sale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3508"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Limita inferioară a intervalului de valori al setului de date; widget-urile și FFT revin la aceasta când propriul interval este lăsat nesetat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Limita superioară a intervalului de valori al setului de date; widget-urile și FFT revin la aceasta când propriul interval este lăsat nesetat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3579"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Alegeți Timp sau un set de date pentru a controla Axa X în grafice</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3592"/>
        <source>Frequency Analysis</source>
        <translation>Analiză de Frecvență</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3639"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Alege Timp (implicit) sau orice set de date a cărui valoare controlează axa Y -- produce o diagramă Campbell când este legat de ex. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3689"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3778"/>
        <source>Minimum Value (optional)</source>
        <translation>Valoare Maximă (opțional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3690"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Limită superioară pentru normalizarea datelor; revine la intervalul de valori al setului de date când nu este setată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3702"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3791"/>
        <source>Maximum Value (optional)</source>
        <translation>Valoare Minimă (opțional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3703"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Limită inferioară pentru normalizarea datelor; revine la intervalul de valori al setului de date când nu este setată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3779"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Limită inferioară a intervalului pentru indicator sau bară; revine la intervalul de valori al setului de date când nu este setată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3792"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Limită superioară a intervalului pentru indicator sau bară; revine la intervalul de valori al setului de date când nu este setată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3844"/>
        <source>Decimal Points</source>
        <translation>Puncte Zecimale</translation>
    </message>
    <message>
        <source>Decimal places shown in the data grid value column (-1 = auto)</source>
        <translation type="vanished">Zecimale afișate în coloana de valori a grilei de date (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3891"/>
        <source>On</source>
        <translation>Pornit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3935"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>LED-ul se aprinde când valoarea atinge sau depășește acest prag; definește benzi de alarmă pentru culori multi-stare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3992"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3993"/>
        <source>Web View</source>
        <translation>Vizualizare Web</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5439"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Valori brute și transformate pentru fiecare set de date (doar citire)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5448"/>
        <source>Shared table defined in this project</source>
        <translation>Tabel partajat definit în acest proiect</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6071"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>Elimini 1 referință widget al cărei grup sau set de date țintă nu mai există?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6072"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>Elimini %1 referințe widget ale căror grupuri sau seturi de date țintă nu mai există?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6077"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Aceasta va afecta doar plasarea tile-urilor în spațiul de lucru; niciun grup, set de date sau date nu sunt șterse.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6080"/>
        <source>Clean Up Workspaces</source>
        <translation>Curăță Spațiile de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3475"/>
        <source>Frame Index</source>
        <translation>Index Cadru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1588"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1589"/>
        <source>Dashboard Widgets</source>
        <translation>Widget-uri Dashboard</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1946"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1949"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1950"/>
        <source>Control Loop</source>
        <translation>Buclă de Control</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3476"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Poziția cadrului utilizată pentru alinierea seturilor de date în timp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3485"/>
        <source>Measurement Unit</source>
        <translation>Unitate de Măsură</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3486"/>
        <source>Volts, Amps, etc.</source>
        <translation>Volți, Amperi, etc.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3487"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Unitate de măsură, cum ar fi volți sau amperi (opțional)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3534"/>
        <source>Plot Settings</source>
        <translation>Setări Grafic</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3557"/>
        <source>Enable Plot Widget</source>
        <translation>Activează Widget Grafic</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3559"/>
        <source>Plot data in real-time</source>
        <translation>Afișează date în timp real</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2645"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3578"/>
        <source>X-Axis Source</source>
        <translation>Sursă Axa X</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Alege setul de date de utilizat pentru Axa X în grafice</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Valoare Minimă Grafic (opțional)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Limită inferioară pentru intervalul de afișare al graficului</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Valoare Maximă Grafic (opțional)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Limita superioară pentru intervalul de afișare al graficului</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">Configurare FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3603"/>
        <source>Enable FFT Analysis</source>
        <translation>Activează Analiza FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Efectuează analiza în domeniul frecvenței pentru setul de date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3614"/>
        <source>Enable Waterfall Plot</source>
        <translation>Activează Grafic Waterfall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3615"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Afișează o spectrogramă derulantă a conținutului de frecvență în timp (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <source>Waterfall Y Axis</source>
        <translation>Axa Y Waterfall</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="vanished">Alege Timp (implicit) sau orice set de date a cărui valoare controlează axa Y — produce o diagramă Campbell când este legat de ex. RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3666"/>
        <source>FFT Window Size</source>
        <translation>Dimensiune Fereastră FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3667"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Numărul de eșantioane utilizate pentru fiecare fereastră de calcul FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3678"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>Rată de Eșantionare FFT (Hz, obligatoriu)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3679"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Frecvența de eșantionare utilizată pentru FFT (în Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Valoare Minimă (recomandat)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Limită inferioară pentru normalizarea datelor</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Valoare Maximă (recomandat)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Limită superioară pentru normalizarea datelor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3728"/>
        <source>Widget Settings</source>
        <translation>Setări Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3751"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3752"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Selectează widget-ul vizual utilizat pentru afișarea acestui set de date</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Valoare Minimă de Afișare (obligatoriu)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Limită inferioară a intervalului de afișare pentru indicator sau bară</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Valoare Maximă de Afișare (obligatoriu)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Limită superioară a intervalului de afișare pentru indicator sau bară</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3808"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3843"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4004"/>
        <source>Auto</source>
        <translation>Automat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3809"/>
        <source>Tick Count</source>
        <translation>Număr de Marcaje</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3813"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Numărul de marcaje majore pe scala cadranului (0 = ajustare automată la dimensiunea widget-ului)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3832"/>
        <source>Label Format</source>
        <translation>Format Etichetă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3833"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Zecimale sau notație utilizată pe etichetele de marcaj și afișajul valorii</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Afișează Indicator Valoare</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Comută afișajul numeric încadrat care apare sub sau lângă widget</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Setări Alarme</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Activează Alarmele</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Declanșează o alarmă vizuală când valoarea depășește pragurile de alarmă</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Prag Inferior</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Declanșează o alarmă vizuală când valoarea scade sub acest prag</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Prag Superior</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Declanșează o alarmă vizuală când valoarea depășește acest prag</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3908"/>
        <source>LED Display Settings</source>
        <translation>Setări Afișaj LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3919"/>
        <source>Show in LED Panel</source>
        <translation>Afișează în Panoul LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3920"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Activează monitorizarea vizuală a stării folosind un afișaj LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3934"/>
        <source>LED On Threshold (required)</source>
        <translation>Prag de Aprindere LED (obligatoriu)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">LED-ul se aprinde când valoarea atinge sau depășește acest prag</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Off</source>
        <translation>Oprit</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Auto Start</source>
        <translation>Pornire Automată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Start on Trigger</source>
        <translation>Pornire la Declanșare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Toggle on Trigger</source>
        <translation>Comutare la Declanșare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3956"/>
        <source>Repeat N Times</source>
        <translation>Repetă de N Ori</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3959"/>
        <source>Plain Text (UTF8)</source>
        <translation>Text Simplu (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3959"/>
        <source>Hexadecimal</source>
        <translation>Hexazecimal</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3959"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3960"/>
        <source>Binary (Direct)</source>
        <translation>Binar (Direct)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3965"/>
        <source>No Checksum</source>
        <translation>Fără Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3969"/>
        <source>End Delimiter Only</source>
        <translation>Doar Delimitator Final</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3969"/>
        <source>Start Delimiter Only</source>
        <translation>Doar Delimitator Inițial</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3970"/>
        <source>Start + End Delimiter</source>
        <translation>Delimitator Inițial + Final</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3970"/>
        <source>No Delimiters</source>
        <translation>Fără Delimitatori</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Button</source>
        <translation>Buton</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Slider</source>
        <translation>Cursor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Toggle</source>
        <translation>Comutator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Text Field</source>
        <translation>Câmp Text</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3981"/>
        <source>Knob</source>
        <translation>Buton Rotativ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3985"/>
        <source>Data Grid</source>
        <translation>Grilă de Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3986"/>
        <source>GPS Map</source>
        <translation>Hartă GPS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3987"/>
        <source>Gyroscope</source>
        <translation>Giroscop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3988"/>
        <source>Multiple Plot</source>
        <translation>Grafic Multiplu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3989"/>
        <source>Accelerometer</source>
        <translation>Accelerometru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3990"/>
        <source>3D Plot</source>
        <translation>Grafic 3D</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3991"/>
        <source>Image View</source>
        <translation>Vizualizare Imagine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3994"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3997"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4012"/>
        <source>None</source>
        <translation>Niciunul</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3998"/>
        <source>Bar</source>
        <translation>Bară</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3999"/>
        <source>Gauge</source>
        <translation>Indicator</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4000"/>
        <source>Compass</source>
        <translation>Busolă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4001"/>
        <source>Meter</source>
        <translation>Contor</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4005"/>
        <source>Integer (0 decimals)</source>
        <translation>Întreg (0 zecimale)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4006"/>
        <source>1 decimal</source>
        <translation>1 zecimală</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4007"/>
        <source>2 decimals</source>
        <translation>2 zecimale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4008"/>
        <source>3 decimals</source>
        <translation>3 zecimale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4009"/>
        <source>Scientific</source>
        <translation>Științific</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4013"/>
        <source>New Line (\n)</source>
        <translation>Linie Nouă (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Carriage Return (\r)</source>
        <translation>Retur de Car (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4015"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>No</source>
        <translation>Nu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4019"/>
        <source>Yes</source>
        <translation>Da</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5115"/>
        <source>Label</source>
        <translation>Etichetă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5116"/>
        <source>Display label</source>
        <translation>Afișează Eticheta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5126"/>
        <source>Button Icon</source>
        <translation>Pictogramă Buton</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5135"/>
        <source>Colorize Icon</source>
        <translation>Colorează Pictograma</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5136"/>
        <source>Tint the icon with the button color</source>
        <translation>Nuanțează pictograma cu culoarea butonului</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5153"/>
        <source>Initial Value</source>
        <translation>Valoare Inițială</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5165"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Codificarea caracterelor utilizată când transmit() returnează o valoare de tip șir</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5183"/>
        <source>Value Range</source>
        <translation>Interval de Valori</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3507"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5193"/>
        <source>Minimum Value</source>
        <translation>Valoare Minimă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3520"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5202"/>
        <source>Maximum Value</source>
        <translation>Valoare Maximă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5211"/>
        <source>Step Size</source>
        <translation>Pas de Incrementare</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="629"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="638"/>
        <source>Lock Project</source>
        <translation>Blochează Proiectul</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="630"/>
        <source>Choose a password to lock the project:</source>
        <translation>Alegeți o parolă pentru a bloca proiectul:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="638"/>
        <source>Confirm the password:</source>
        <translation>Confirmați parola:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="643"/>
        <source>Passwords do not match</source>
        <translation>Parolele nu se potrivesc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="644"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>Cele două parole introduse nu se potrivesc. Proiectul nu a fost blocat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="678"/>
        <source>Unlock Project</source>
        <translation>Deblochează Proiectul</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="679"/>
        <source>Enter the project password:</source>
        <translation>Introduceți parola proiectului:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="689"/>
        <source>Incorrect password</source>
        <translation>Parolă incorectă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="690"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>Parola introdusă nu se potrivește cu cea stocată în fișierul proiectului.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="721"/>
        <source>New Project</source>
        <translation>Proiect Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="921"/>
        <source>Samples</source>
        <translation>Eșantioane</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1486"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Surse de date multiple necesită o licență Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1487"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro permite conectarea simultană la mai multe dispozitive. Actualizați pentru a debloca această funcționalitate.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1499"/>
        <source>Device %1</source>
        <translation>Dispozitiv %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Copie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1769"/>
        <source>Do you want to save your changes?</source>
        <translation>Doriți să salvați modificările?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1770"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Aveți modificări nesalvate în acest proiect!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="501"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="510"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="523"/>
        <source>Project error</source>
        <translation>Eroare proiect</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="501"/>
        <source>Project title cannot be empty!</source>
        <translation>Titlul proiectului nu poate fi gol!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="510"/>
        <source>You need to add at least one group!</source>
        <translation>Trebuie să adăugați cel puțin un grup!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="523"/>
        <source>You need to add at least one dataset!</source>
        <translation>Trebuie să adăugați cel puțin un set de date!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="570"/>
        <source>Your project needs a title</source>
        <translation>Proiectul necesită un titlu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="572"/>
        <source>Add a group to get started</source>
        <translation>Adaugă un grup pentru a începe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="574"/>
        <source>Add a dataset to a group</source>
        <translation>Adaugă un set de date la un grup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="588"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Deschide vizualizarea Proiect în partea de sus a arborelui și introdu un nume. Poți redenumi proiectul oricând.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="591"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>Grupurile organizează seturile de date în widget-uri de tablou de bord. Folosește butonul Grup din bara de instrumente de mai sus pentru a crea unul, apoi adaugă seturi de date la acesta.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="595"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>Seturile de date sunt valorile care apar pe tabloul de bord. Selectați un grup în arbore și utilizați butonul Set de Date din bara de instrumente pentru a adăuga unul.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="920"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="969"/>
        <source>Time</source>
        <translation>Timp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1531"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Doriți să ștergeți sursa de date "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1532"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>Grupurile care utilizează această sursă vor fi mutate la sursa implicită. Această acțiune nu poate fi anulată.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1809"/>
        <source>Save Serial Studio Project</source>
        <translation>Salvează Proiect Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1811"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2468"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Fișiere Proiect Serial Studio (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1832"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2070"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2459"/>
        <source>Untitled Project</source>
        <translation>Proiect Fără Titlu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2083"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2614"/>
        <source>Device A</source>
        <translation>Dispozitiv A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2295"/>
        <source>Select Project File</source>
        <translation>Selectează Fișier Proiect</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2297"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>Fișiere Proiect (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2341"/>
        <source>JSON validation error</source>
        <translation>Eroare de validare JSON</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2433"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Proiect actualizat de la un format de fișier anterior</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2434"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Acest proiect a fost salvat cu versiunea de schemă %1; versiunea curentă este %2. S-au aplicat valorile implicite pentru orice câmpuri noi. Salvează proiectul pentru a finaliza actualizarea.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2466"/>
        <source>Save Imported Project</source>
        <translation>Salvează Proiectul Importat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2658"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>Proiectele cu surse multiple necesită licență Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2659"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Acest proiect conține surse de date multiple. Doar prima sursă a fost încărcată. O licență Serial Studio Pro este necesară pentru a utiliza proiecte cu surse multiple.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2896"/>
        <source>Workspace IDs remapped on load</source>
        <translation>ID-urile spațiului de lucru remapate la încărcare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2897"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 ID-uri personalizate de spațiu de lucru s-au suprapus cu noua zonă automată rezervată și au fost mutate în zona utilizatorului. Salvează proiectul pentru a face remaparea permanentă.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3080"/>
        <source>Legacy frame parser function updated</source>
        <translation>Funcția de analiză a cadrelor moștenită a fost actualizată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3081"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Proiectul dumneavoastră a folosit o funcție veche de analiză a cadrelor cu un argument 'separator'. A fost migrat automat la noul format.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3322"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Doriți să ștergeți grupul "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3323"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3368"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3400"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4138"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Această acțiune nu poate fi anulată. Doriți să continuați?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3367"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Doriți să ștergeți acțiunea "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3399"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Doriți să ștergeți setul de date "%1"?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Copie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4050"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4086"/>
        <source>Output Controls</source>
        <translation>Controale de Ieșire</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4098"/>
        <source>New Button</source>
        <translation>Buton Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4101"/>
        <source>New Slider</source>
        <translation>Cursor Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4104"/>
        <source>New Toggle</source>
        <translation>Comutator Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4107"/>
        <source>New Text Field</source>
        <translation>Câmp Text Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4110"/>
        <source>New Knob</source>
        <translation>Buton Rotativ Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4137"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Doriți să ștergeți widget-ul de ieșire "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4302"/>
        <source>Group</source>
        <translation>Grup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4320"/>
        <source>New Dataset</source>
        <translation>Set de Date Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4323"/>
        <source>New Plot</source>
        <translation>Grafic Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4327"/>
        <source>New FFT Plot</source>
        <translation>Grafic FFT Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4331"/>
        <source>New Level Indicator</source>
        <translation>Indicator Nivel Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4335"/>
        <source>New Gauge</source>
        <translation>Indicator Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4339"/>
        <source>New Compass</source>
        <translation>Busolă Nouă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4345"/>
        <source>New Meter</source>
        <translation>Contor Nou</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Termometru Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4349"/>
        <source>New LED Indicator</source>
        <translation>Indicator LED Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4353"/>
        <source>New Waterfall</source>
        <translation>Waterfall Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4421"/>
        <source>Channel %1</source>
        <translation>Canal %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4492"/>
        <source>New Action</source>
        <translation>Acțiune Nouă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4633"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Sigur doriți să schimbați widget-ul la nivel de grup?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4635"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>Seturile de date existente pentru acest grup sunt șterse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4703"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4704"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4705"/>
        <source>Accelerometer %1</source>
        <translation>Accelerometru %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4720"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4720"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4720"/>
        <source>Gyro %1</source>
        <translation>Giroscop %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4735"/>
        <source>Latitude</source>
        <translation>Latitudine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4735"/>
        <source>Longitude</source>
        <translation>Longitudine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4735"/>
        <source>Altitude</source>
        <translation>Altitudine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4750"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4764"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4750"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4764"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4750"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4764"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5022"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5948"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6220"/>
        <source>Workspace</source>
        <translation>Spațiu de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5211"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5417"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6679"/>
        <source>Shared Table</source>
        <translation>Tabel Partajat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5288"/>
        <source>register</source>
        <translation>registru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5417"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6679"/>
        <source>New Shared Table</source>
        <translation>Tabel Partajat Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5417"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5440"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5459"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5483"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5510"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5529"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5551"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5573"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5948"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5969"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6203"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6220"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6242"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6437"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6458"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6662"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6679"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6700"/>
        <source>Name:</source>
        <translation>Nume:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5440"/>
        <source>Rename Table</source>
        <translation>Redenumește Tabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5459"/>
        <source>Rename Group</source>
        <translation>Redenumire Grup</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5483"/>
        <source>Rename Dataset</source>
        <translation>Redenumire Set de Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5510"/>
        <source>Rename Data Source</source>
        <translation>Redenumire Sursă de Date</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5529"/>
        <source>Rename Action</source>
        <translation>Redenumire Acțiune</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5550"/>
        <source>New Register</source>
        <translation>Registru Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5573"/>
        <source>Rename Register</source>
        <translation>Redenumește Registru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5607"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5632"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7341"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8110"/>
        <source>This action cannot be undone.</source>
        <translation>Această acțiune nu poate fi anulată.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5608"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Aceasta elimină %1 registru(e) împreună cu tabelul. Această acțiune nu poate fi anulată.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5611"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5631"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7340"/>
        <source>Delete "%1"?</source>
        <translation>Șterge "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5614"/>
        <source>Delete Table</source>
        <translation>Șterge Tabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5634"/>
        <source>Delete Register</source>
        <translation>Șterge Registru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5656"/>
        <source>Export Table</source>
        <translation>Exportă Tabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5658"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5699"/>
        <source>CSV files (*.csv)</source>
        <translation>Fișiere CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5697"/>
        <source>Import Table</source>
        <translation>Importă Tabel</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5948"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6220"/>
        <source>New Workspace</source>
        <translation>Spațiu de Lucru Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5969"/>
        <source>Rename Workspace</source>
        <translation>Redenumește Spațiul de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6015"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6203"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6303"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6437"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6521"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6662"/>
        <source>Folder</source>
        <translation>Dosar</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6203"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6437"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6662"/>
        <source>New Folder</source>
        <translation>Dosar Nou</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6242"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6458"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6700"/>
        <source>Rename Folder</source>
        <translation>Redenumește Dosarul</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6260"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6476"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6718"/>
        <source>Delete folder "%1"?</source>
        <translation>Ștergi dosarul „%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6261"/>
        <source>The folder is removed; its workspaces and sub-folders move up to the parent.</source>
        <translation>Dosarul este eliminat; workspace-urile și subdosarele sale sunt mutate la nivelul părinte.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6263"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6479"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6722"/>
        <source>Delete Folder</source>
        <translation>Șterge Dosarul</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6477"/>
        <source>The folder is removed; its groups and sub-folders move up to the parent.</source>
        <translation>Dosarul este eliminat; grupurile și subdosarele sale sunt mutate la nivelul părinte.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6719"/>
        <source>The folder is removed; its tables and sub-folders move up to the parent. The accessor path of those tables changes accordingly.</source>
        <translation>Folderul este șters; tabelele și sub-folderele sale urcă la nivelul părintelui. Calea de acces a acelor tabele se modifică în consecință.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6807"/>
        <source>Overview</source>
        <translation>Prezentare Generală</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6816"/>
        <source>All Data</source>
        <translation>Toate Datele</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7109"/>
        <source>Discard workspace customisations?</source>
        <translation>Renunță la personalizările spațiului de lucru?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7110"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Dezactivarea personalizării renunță la modificările tale și reconstruiește lista de spații de lucru din grupurile proiectului.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7113"/>
        <source>Customize Workspaces</source>
        <translation>Personalizează Spațiile de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7343"/>
        <source>Delete Workspace</source>
        <translation>Șterge Spațiul de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7724"/>
        <source>Project file removed from disk</source>
        <translation>Fișier proiect eliminat de pe disc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7725"/>
        <source>%1 was deleted or renamed by another program. Save the project to recreate it.</source>
        <translation>%1 a fost șters sau redenumit de alt program. Salvează proiectul pentru a-l recrea.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7746"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7767"/>
        <source>Project file changed on disk</source>
        <translation>Fișier proiect modificat pe disc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7747"/>
        <source>%1 was modified by another program. The in-memory project was kept; reopen the file to load the external changes.</source>
        <translation>%1 a fost modificat de alt program. Proiectul din memorie a fost păstrat; redeschide fișierul pentru a încărca modificările externe.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7763"/>
        <source>The project file was modified by another program.

Reload it and discard your unsaved changes?</source>
        <translation>Fișierul proiect a fost modificat de alt program.

Îl reîncarci și renunți la modificările nesalvate?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7765"/>
        <source>The project file was modified by another program.

Reload it?</source>
        <translation>Fișierul proiect a fost modificat de alt program.

Îl reîncarci?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7796"/>
        <source>File save error</source>
        <translation>Eroare la salvarea fișierului</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8109"/>
        <source>Delete %1 selected items?</source>
        <translation>Ștergi %1 elemente selectate?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8112"/>
        <source>Delete Items</source>
        <translation>Șterge Elemente</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2502"/>
        <source>File open error</source>
        <translation>Eroare la deschiderea fișierului</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Importare Fișier Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Fișiere Proto (*.proto);;Toate Fișierele (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Eșec la deschiderea fișierului proto: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Verifică calea fișierului și permisiunile de citire, apoi încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Eroare Import Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Fișierul proto este prea mare (limita este de 10 MB).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>Verifică că ai selectat fișierul de definiție .proto corect.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Eșec la parsarea fișierului proto la linia %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>Doar sintaxa proto3 este suportată. Verifică formatul fișierului și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Fișierul proto nu conține definiții de mesaje</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Fișierul selectat nu are blocuri `message` de importat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Avertisment Import Protobuf</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Eșec la încărcarea proiectului importat</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">JSON-ul proiectului generat nu a putut fi încărcat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>Importat cu succes %1 mesaj(e) și %2 câmp(uri) din fișierul proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editorul de proiect este acum deschis pentru personalizare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Import Protobuf Finalizat</translation>
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
        <translation>Intrare HEX Invalidă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Introduceți octeți hexazecimali valizi.

Format valid: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Niciun cod de funcție de transmisie de evaluat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>funcția de transmisie nu poate fi apelată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>Evaluează</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>Valoare de Intrare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>Ieșire Funcție de Transmisie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>Introduceți valoarea de transmis…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>Ieșirea șirului brut apare aici</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>Ieșirea octeților hex apare aici</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>Testează Funcția de Transmisie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Introduceți octeți hex (ex., 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(gol) Nicio dată returnată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 octeți</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 octet(i)</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>Memorie Partajată</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="167"/>
        <source>Add Folder</source>
        <translation>Adaugă Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="169"/>
        <source>Add a top-level folder</source>
        <translation>Adaugă un folder de nivel superior</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="177"/>
        <source>Add Shared Table</source>
        <translation>Adaugă Tabel Partajat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="179"/>
        <source>Add shared table</source>
        <translation>Adaugă tabel partajat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="188"/>
        <source>Help</source>
        <translation>Ajutor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="193"/>
        <source>Open help documentation for shared memory</source>
        <translation>Deschide documentația de ajutor pentru memoria partajată</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="202"/>
        <source>Title</source>
        <translation>Titlu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="203"/>
        <source>Registers</source>
        <translation>Registre</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="vanished">Nume</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="vanished">Descriere</translation>
    </message>
    <message>
        <source>Entries</source>
        <translation type="vanished">Intrări</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="297"/>
        <source>No shared tables.</source>
        <translation>Niciun tabel partajat.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Sesiuni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Deschide</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Deschide un fișier de sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Închide fișierul de sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Redare</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Redă sesiunea selectată pe tabloul de bord</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Deblocați fișierul de sesiune pentru a șterge sesiuni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Șterge sesiunea selectată</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Deblochează</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Blochează</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Deblocați fișierul de sesiune pentru a permite ștergeri</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Setați o parolă pentru a preveni ștergerea sesiunilor</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>Exportă CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Exportă sesiunea selectată în CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>Exportă PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>Generează un raport PDF pentru sesiunea selectată</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Restaurează Proiectul</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Restaurează fișierul proiect din acest fișier de sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Se încarcă sesiunea…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>Se Lucrează…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="112"/>
        <source>Pro features detected in this project.</source>
        <translation>Funcționalități Pro detectate în acest proiect.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="114"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Se utilizează widget-uri de rezervă. Cumpără o licență pentru a debloca funcționalitatea completă.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="142"/>
        <source>Plots</source>
        <translation>Grafice</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="147"/>
        <source>Plot</source>
        <translation>Grafic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="151"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>Comută vizualizarea grafic 2D pentru acest set de date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>FFT Plot</source>
        <translation>Grafic FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="166"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>Comutați graficul FFT pentru a vizualiza conținutul de frecvență</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="178"/>
        <source>Waterfall</source>
        <translation>Waterfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="182"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Comută graficul în cascadă (spectrogramă) — folosește setările FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="199"/>
        <source>Widgets</source>
        <translation>Widget-uri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="205"/>
        <source>Bar/Level</source>
        <translation>Bară/nivel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="209"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Comutați indicatorul de bară/nivel pentru acest set de date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="220"/>
        <source>Gauge</source>
        <translation>Indicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="225"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Comutați widget-ul indicator pentru afișaj în stil analogic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="237"/>
        <source>Compass</source>
        <translation>Busolă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="241"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Comutați widget-ul busolă pentru date direcționale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="252"/>
        <source>Meter</source>
        <translation>Contor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="257"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Comutați widget-ul contor analogic (semi-arc)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="308"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>Definește intervale de valori colorate cu niveluri de severitate pentru indicatorul sau LED-ul acestui set de date.</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometru</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Comutați widget-ul termometru pentru date de temperatură</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="268"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="273"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>Comutați indicatorul LED pentru valori binare sau cu prag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="293"/>
        <source>Behavior</source>
        <translation>Comportament</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="298"/>
        <source>Alarm Bands</source>
        <translation>Benzi de Alarmă</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">Definește intervale de valori colorate cu niveluri de severitate pentru indicatorul acestui set de date.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Transform</source>
        <translation>Transformare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="318"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Editează o expresie de transformare a valorii pentru calibrare, filtrare sau conversie de unități</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="331"/>
        <source>Duplicate</source>
        <translation>Duplică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="336"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Duplică acest set de date cu aceeași configurație</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="344"/>
        <source>Delete this dataset from the group</source>
        <translation>Șterge acest set de date din grup</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>Susține Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>Susține dezvoltarea %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio este un software gratuit și open-source susținut de voluntari. Ia în considerare o donație sau obținerea unei licențe Pro pentru a susține eforturile de dezvoltare :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Poți susține acest proiect și prin distribuire, raportarea erorilor și propunerea de funcționalități noi!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>Donează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>Obține Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="142"/>
        <source>Stop</source>
        <translation>Oprește</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="143"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="377"/>
        <source>Downloading updates</source>
        <translation>Se descarcă actualizări</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="470"/>
        <source>Time remaining</source>
        <translation>Timp rămas</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <source>unknown</source>
        <translation>necunoscut</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Error</source>
        <translation>Eroare</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Cannot find downloaded update!</source>
        <translation>Nu se poate găsi actualizarea descărcată!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="260"/>
        <source>Download complete!</source>
        <translation>Descărcare finalizată!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="268"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Faceți clic pe „OK" pentru a începe instalarea actualizării</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="270"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>Pentru a instala actualizarea, poate fi necesar să închideți aplicația.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="261"/>
        <source>The installer opens separately</source>
        <translation>Programul de instalare se deschide separat</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="274"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>Pentru a instala actualizarea, poate fi necesar să închideți aplicația. Aceasta este o actualizare obligatorie, ieșirea acum va închide aplicația.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="290"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Faceți clic pe butonul „Deschide" pentru a aplica actualizarea</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="303"/>
        <source>Updater</source>
        <translation>Actualizator</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="307"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Sigur doriți să anulați descărcarea?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="309"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Sigur doriți să anulați descărcarea? Aceasta este o actualizare obligatorie, ieșirea acum va închide aplicația</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="364"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="371"/>
        <source>%1 bytes</source>
        <translation>%1 octeți</translation>
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
        <translation>din</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="421"/>
        <source>Downloading Updates</source>
        <translation>Descărcare Actualizări</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Time Remaining</source>
        <translation>Timp Rămas</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Unknown</source>
        <translation>Necunoscut</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="446"/>
        <source>about %1 hours</source>
        <translation>aproximativ %1 ore</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="448"/>
        <source>about one hour</source>
        <translation>aproximativ o oră</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="456"/>
        <source>%1 minutes</source>
        <translation>%1 minute</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="458"/>
        <source>1 minute</source>
        <translation>1 minut</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="465"/>
        <source>%1 seconds</source>
        <translation>%1 secunde</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="467"/>
        <source>1 second</source>
        <translation>1 secundă</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Timp rămas: 0 minute</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Deschide</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>Navigator Exemple</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>Înapoi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Descarcă și Deschide</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>Vizualizare pe GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>Căutare în Exemple…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="245"/>
        <source>Fetching examples…</source>
        <translation>Se preiau exemple…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>Loading...</source>
        <translation>Se Încarcă...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="569"/>
        <source>No README available.</source>
        <translation>Niciun README disponibil.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="609"/>
        <source>Copied to Clipboard</source>
        <translation>Copiat în Clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="672"/>
        <source>No screenshot available</source>
        <translation>Nicio captură de ecran disponibilă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="704"/>
        <source>Details</source>
        <translation>Detalii</translation>
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
        <translation>Dificultate:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="787"/>
        <source>Project:</source>
        <translation>Proiect:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="829"/>
        <source>No Results Found</source>
        <translation>Niciun Rezultat Găsit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="840"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Verifică ortografia sau încearcă un alt termen de căutare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="855"/>
        <source>%1 examples</source>
        <translation>%1 exemple</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="864"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>Manager de Extensii</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>Reîmprospătează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>Depozite</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>Setări Depozit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>Înapoi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>Instalează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>Dezinstalează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>Rulează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>Oprește</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>Căutare extensii…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>Se preiau extensii…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>În Execuție</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>Actualizare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>Instalat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>Indisponibil</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>Nicio descriere disponibilă.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>Detalii</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>Tip:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>Autor:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>Versiune:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>Licență:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>Nicio previzualizare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>IEȘIRE PLUGIN</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Nicio ieșire încă. Rulează plugin-ul pentru a vedea jurnalul aici.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>Nicio previzualizare disponibilă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>Depozite</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>Adaugă URL-uri către depozite la distanță sau căi către dosare locale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>LOCAL</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL sau cale locală…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>Răsfoiește…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>Adaugă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>Niciun Rezultat Găsit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Verificați ortografia sau încercați un alt termen de căutare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>Nicio Extensie Disponibilă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Adăugați un URL de depozit sau o cale locală în setările Repos, apoi reîmprospătați.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 extensii</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="167"/>
        <source>Interpolation: %1</source>
        <translation>Interpolare: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="195"/>
        <source>Show Area Under Plot</source>
        <translation>Afișează Aria Sub Grafic</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="213"/>
        <source>Show X Axis Label</source>
        <translation>Afișează Eticheta Axei X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="225"/>
        <source>Show Y Axis Label</source>
        <translation>Afișează Eticheta Axei Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="243"/>
        <source>Show Crosshair</source>
        <translation>Afișează Cursor Încrucișat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Resume</source>
        <translation>Reia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="269"/>
        <source>Reset View</source>
        <translation>Resetează Vizualizarea</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="275"/>
        <source>Axis Range Settings</source>
        <translation>Setări Interval Axă</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="304"/>
        <source>Magnitude (dB)</source>
        <translation>Magnitudine (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="305"/>
        <source>Frequency (Hz)</source>
        <translation>Frecvență (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Plasați aici fișiere Proiect și CSV</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>Transmisie Fișier</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>Protocol de Transfer:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>Selectare Fișier:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>Selectează Fișier…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>Interval de Transmisie:</translation>
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
        <translation>Dimensiune Bloc:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>octeți</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>Timeout:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>Reîncercări Maxime:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>Progres: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 octeți</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>Erori: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>Jurnal de Activitate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>Pauză Transmisie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>Reia Transmisia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>Oprește Transmisia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>Începe Transmisia</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>Analizor de Trame</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>Dispozitiv %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="399"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1446"/>
        <source>Output Panel</source>
        <translation>Panou de Ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="443"/>
        <source>Control</source>
        <translation>Control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="466"/>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="482"/>
        <source>%1 regs</source>
        <translation>%1 reg</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="483"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="505"/>
        <source>empty</source>
        <translation>gol</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script de Control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="528"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="931"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Deschide editorul de cod de transformare pentru acest set de date.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1216"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1219"/>
        <source>Dataset Container</source>
        <translation>Container Seturi de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1228"/>
        <source>Multi-Plot</source>
        <translation>Grafic Multiplu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1231"/>
        <source>Multiple Plot</source>
        <translation>Grafic Multiplu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1240"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <source>Accelerometer</source>
        <translation>Accelerometru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1252"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1255"/>
        <source>Gyroscope</source>
        <translation>Giroscop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1264"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1267"/>
        <source>GPS Map</source>
        <translation>Hartă GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1275"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <source>3D Plot</source>
        <translation>Grafic 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1286"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1289"/>
        <source>Image View</source>
        <translation>Vizualizare Imagine</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1298"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1301"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1310"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1313"/>
        <source>Web View</source>
        <translation>Vizualizare Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1322"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1325"/>
        <source>Data Grid</source>
        <translation>Grilă de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1338"/>
        <source>Generic</source>
        <translation>Generic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1351"/>
        <source>Plot</source>
        <translation>Grafic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1364"/>
        <source>FFT Plot</source>
        <translation>Grafic FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1377"/>
        <source>Gauge</source>
        <translation>Indicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1390"/>
        <source>Level Indicator</source>
        <translation>Indicator de Nivel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1403"/>
        <source>Compass</source>
        <translation>Busolă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1416"/>
        <source>Meter</source>
        <translation>Contor</translation>
    </message>
    <message>
        <source>Edit Control Script…</source>
        <translation type="vanished">Editează Script de Control…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="503"/>
        <source>Control Loop</source>
        <translation>Buclă de Control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1429"/>
        <source>LED Indicator</source>
        <translation>Indicator LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1458"/>
        <source>Slider</source>
        <translation>Cursor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1471"/>
        <source>Toggle</source>
        <translation>Comutator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1484"/>
        <source>Knob</source>
        <translation>Buton Rotativ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1497"/>
        <source>Text Field</source>
        <translation>Câmp Text</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1510"/>
        <source>Button</source>
        <translation>Buton</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1534"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1610"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1698"/>
        <source>Add Group</source>
        <translation>Adaugă Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1550"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1714"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1759"/>
        <source>Add Dataset</source>
        <translation>Adaugă Set de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1564"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1640"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1728"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1773"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1980"/>
        <source>Add Output</source>
        <translation>Adaugă Ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1653"/>
        <source>Add Action</source>
        <translation>Adaugă Acțiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1589"/>
        <source>Add Data Source</source>
        <translation>Adaugă Sursă de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1596"/>
        <source>Add Data Table</source>
        <translation>Adaugă Tabel de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1664"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1800"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1867"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1995"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2029"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2085"/>
        <source>Rename…</source>
        <translation>Redenumește…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1672"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1830"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1900"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1952"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2003"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2059"/>
        <source>Duplicate</source>
        <translation>Duplică</translation>
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
        <translation>Șterge…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1744"/>
        <source>Edit Frame Parser…</source>
        <translation>Editează Analizor de Trame…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1786"/>
        <source>Edit Painter Code…</source>
        <translation>Editează Cod Painter…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1808"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1876"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1928"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2037"/>
        <source>Move Up</source>
        <translation>Mută Sus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1819"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1888"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1940"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2048"/>
        <source>Move Down</source>
        <translation>Mută Jos</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1856"/>
        <source>Edit Transform Code…</source>
        <translation>Editează Cod Transform…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2111"/>
        <source>Edit Code…</source>
        <translation>Editează Cod…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2126"/>
        <source>Edit Control Loop…</source>
        <translation>Editează Bucla de Control…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="349"/>
        <source>Action</source>
        <translation>Acțiune</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">Niciun grup definit încă</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Testează Parserul de Cadre</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>Cadru %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>Decodor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>Rânduri</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 rând(uri)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Rând %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Configurare Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Detecție</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Sfârșit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Delimitatori Hex</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Introducere Date Trame</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Introduceți octeți hex (ex. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Introduceți octeții fluxului brut aici...</translation>
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
        <translation>Eșantionul nu conține delimitatorii de trame configurați, astfel încât nicio tramă nu va fi extrasă. Introduceți-i în eșantion (ex. 
 pentru linie nouă) sau ajustați modul de detecție.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Rezultate Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Pregătire</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Valoare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>Extracția nu a produs o tramă completă. Verificați delimitatorii de început / sfârșit și modul de detecție.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Introduceți date de probă mai sus și apăsați Evaluează pentru a previzualiza rezultatul analizat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Evaluează</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Anulează</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Refă</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Decupează</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Copiază</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Lipește</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Selectează Tot</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Formatează Documentul</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Formatează Selecția</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Resetează</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Resetează la scriptul de parsare implicit</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Deschide</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Importă un fișier script pentru parsarea datelor</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Deschide documentația de ajutor pentru parsarea datelor</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Limbaj:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Selectează Șablon…</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Anulează ultima editare de cod</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Refă editarea anulată anterior</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Decupează codul selectat în clipboard</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Copiază codul selectat în clipboard</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Lipește codul din clipboard</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Ajutor</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Testează cu Date Exemplu</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Evaluează</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Centrare Automată</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Trasează Traiectoria</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Mărește</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Micșorează</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Afișează Vremea</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>Suprapunere Meteo NASA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Hartă de Bază: %1</translation>
    </message>
</context>
<context>
    <name>GroupFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="132"/>
        <source>Add Sub-folder</source>
        <translation>Adaugă Sub-folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="134"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Adaugă un sub-folder în interiorul acestui folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="142"/>
        <source>Add Group</source>
        <translation>Adaugă Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="144"/>
        <source>Add a group inside this folder</source>
        <translation>Adaugă un grup în acest dosar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="154"/>
        <source>Rename</source>
        <translation>Redenumește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="156"/>
        <source>Rename folder</source>
        <translation>Redenumește dosarul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="164"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="166"/>
        <source>Delete folder</source>
        <translation>Șterge dosarul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="179"/>
        <source>Title</source>
        <translation>Titlu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="180"/>
        <source>Contents</source>
        <translation>Conținut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="270"/>
        <source>This folder is empty. Use the toolbar to add a group or sub-folder.</source>
        <translation>Acest dosar este gol. Folosește bara de instrumente pentru a adăuga un grup sau un subdosar.</translation>
    </message>
</context>
<context>
    <name>GroupTemplateMenu</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="43"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="45"/>
        <source>Dataset Container</source>
        <translation>Container Seturi de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="51"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="53"/>
        <source>Data Grid</source>
        <translation>Grilă de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="59"/>
        <source>Multi-Plot</source>
        <translation>Grafic Multiplu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="61"/>
        <source>Multiple Plot</source>
        <translation>Grafic Multiplu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="67"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="69"/>
        <source>3D Plot</source>
        <translation>Grafic 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="75"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="77"/>
        <source>Accelerometer</source>
        <translation>Accelerometru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="83"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="85"/>
        <source>Gyroscope</source>
        <translation>Giroscop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="91"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="93"/>
        <source>GPS Map</source>
        <translation>Hartă GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="99"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="101"/>
        <source>Image View</source>
        <translation>Vizualizare Imagine</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="107"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="109"/>
        <source>Web View</source>
        <translation>Vizualizare Web</translation>
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
        <translation>Funcționalități Pro detectate în acest proiect.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="102"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Se utilizează widget-uri de rezervă. Achiziționați o licență pentru a debloca funcționalitatea completă.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="132"/>
        <source>Add Dataset</source>
        <translation>Adaugă Set de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Dataset</source>
        <translation>Set de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="142"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Adaugă un set de date generic la grupul curent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="149"/>
        <source>Plot</source>
        <translation>Grafic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="153"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>Adaugă un grafic 2D pentru vizualizarea datelor numerice</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="161"/>
        <source>FFT Plot</source>
        <translation>Grafic FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="166"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>Adaugă un grafic FFT pentru vizualizarea în domeniul frecvenței</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="172"/>
        <source>Waterfall</source>
        <translation>Waterfall</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="177"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Adaugă un grafic în cascadă (spectrogramă) — folosește setările FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="183"/>
        <source>Bar/Level</source>
        <translation>Bară/nivel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="187"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Adaugă un indicator de bară sau nivel pentru valori scalate</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Indicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="198"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Adaugă un widget indicator pentru vizualizare în stil analogic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="205"/>
        <source>Compass</source>
        <translation>Busolă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="209"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Adaugă o busolă pentru afișarea datelor direcționale sau unghiulare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="215"/>
        <source>Meter</source>
        <translation>Contor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="219"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Adaugă un widget contor analogic (semi-arc)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometru</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Adaugă un widget termometru pentru date de temperatură</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="226"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="231"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>Adaugă un indicator LED pentru semnale de stare binare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="244"/>
        <source>Add Control</source>
        <translation>Adaugă Control</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="250"/>
        <source>Button</source>
        <translation>Buton</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="254"/>
        <source>Add a button that sends a command on click</source>
        <translation>Adaugă un buton care trimite o comandă la clic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="260"/>
        <source>Slider</source>
        <translation>Cursor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="264"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Adaugă un cursor pentru trimiterea valorilor numerice scalate</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="270"/>
        <source>Toggle</source>
        <translation>Comutator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="274"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Adaugă un comutator pentru comenzi pornit/oprit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Text Field</source>
        <translation>Câmp Text</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="284"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Adaugă un câmp text pentru tastarea și trimiterea comenzilor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="290"/>
        <source>Knob</source>
        <translation>Buton Rotativ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="294"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Adaugă un buton rotativ pentru controlul punctului de setare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="313"/>
        <source>Edit Code</source>
        <translation>Editează Cod</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="317"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Editează codul JavaScript care desenează acest widget painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="330"/>
        <source>Duplicate</source>
        <translation>Duplică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="334"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Duplică grupul curent și conținutul său</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="339"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="344"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Șterge grupul curent și toate seturile de date conținute</translation>
    </message>
</context>
<context>
    <name>GroupsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="33"/>
        <source>Dashboard Widgets</source>
        <translation>Widget-uri Dashboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="127"/>
        <source>Add Folder</source>
        <translation>Adaugă Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="129"/>
        <source>Add a top-level folder</source>
        <translation>Adaugă un folder de nivel superior</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="137"/>
        <source>Add Group</source>
        <translation>Adaugă Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="139"/>
        <source>Add a group from a template</source>
        <translation>Adaugă un grup dintr-un șablon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="154"/>
        <source>Title</source>
        <translation>Titlu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="155"/>
        <source>Contents</source>
        <translation>Conținut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="245"/>
        <source>No groups yet. Use the toolbar to add a group or a folder.</source>
        <translation>Niciun grup încă. Folosește bara de instrumente pentru a adăuga un grup sau un folder.</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="410"/>
        <source>ROLL ↔</source>
        <translation>RULIU ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="440"/>
        <source>YAW ↻</source>
        <translation>GIRAȚIE ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="470"/>
        <source>PITCH ↕</source>
        <translation>TANGAJ ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>Dispozitiv HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Pagină de Utilizare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Utilizare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Conectați gamepad-uri, joystick-uri, volan-uri, controlere de zbor și alte dispozitive USB de clasă HID.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>Tabele de Utilizare HID (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>Centru de Ajutor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>Se încarcă paginile de ajutor…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>Căutare…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>Se Încarcă…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>Selectați o pagină din bara laterală</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>Copiat în Clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>Vizualizare Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>%1 pagini</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>Închide</translation>
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
        <translation>Socket de Rețea</translation>
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
        <translation>Magistrala CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>USB Device</source>
        <translation>Dispozitiv USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>HID Device</source>
        <translation>Dispozitiv HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>MQTT Subscriber</source>
        <translation>Abonat MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="685"/>
        <source>Your trial period has ended.</source>
        <translation>Perioada de probă s-a încheiat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="686"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Pentru a continua utilizarea Serial Studio, activați licența.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="752"/>
        <source>channels</source>
        <translation>canale</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="805"/>
        <source> channels</source>
        <translation>canale</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="953"/>
        <source>Unsigned 8-bit</source>
        <translation>Fără Semn 8-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="954"/>
        <source>Signed 16-bit</source>
        <translation>Cu Semn 16-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="955"/>
        <source>Signed 24-bit</source>
        <translation>Cu Semn 24-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="956"/>
        <source>Signed 32-bit</source>
        <translation>Cu Semn 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="957"/>
        <source>Float 32-bit</source>
        <translation>Virgulă Mobilă 32-bit</translation>
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
        <translation>Dispozitiv de Intrare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1350"/>
        <source>Sample Rate</source>
        <translation>Rată de Eșantionare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1358"/>
        <source>Sample Format</source>
        <translation>Format Eșantion</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1366"/>
        <source>Channels</source>
        <translation>Canale</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>Eroare Modul I/O BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>Selectează Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>Selectează Serviciu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>Selectează Caracteristică</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>Eroare la configurarea serviciului BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>Eroare de operațiune</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>Eroare la scrierea caracteristicii</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>Eroare la scrierea descriptorului</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>Eroare necunoscută</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>Eroare la citirea caracteristicii</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>Eroare la citirea descriptorului</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>Dispozitiv BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>Serviciu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>Caracteristică de Notificare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>Caracteristică</translation>
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
        <translation>Magistrala CAN Indisponibilă</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="331"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>Nu s-au găsit plugin-uri pentru magistrala CAN pe acest sistem.

Suportul pentru magistrala CAN pe macOS este limitat și poate necesita drivere hardware de la terți.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="336"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Nu există plugin-uri disponibile pentru magistrala CAN pe această platformă.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="348"/>
        <source>Invalid CAN Configuration</source>
        <translation>Configurație CAN Invalidă</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="356"/>
        <source>Invalid Selection</source>
        <translation>Selecție Invalidă</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="365"/>
        <source>No Devices Available</source>
        <translation>Niciun Dispozitiv Disponibil</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="274"/>
        <source>CAN Device Creation Failed</source>
        <translation>Crearea Dispozitivului CAN A Eșuat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="301"/>
        <source>CAN Connection Failed</source>
        <translation>Conexiunea CAN A Eșuat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="319"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>Niciun plugin de magistrală CAN găsit pe acest sistem.

Pe Linux, asigură-te că modulele kernel SOCKETCAN sunt încărcate.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="325"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>Niciun plugin de magistrală CAN găsit pe acest sistem.

Pe Windows, instalează drivere hardware CAN (PEAK, VECTOR etc.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="349"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>Configurația magistralei CAN este incompletă. Selectează un plugin și o interfață valide.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="357"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>Plugin-ul sau interfața selectată nu mai este disponibilă. Reîmprospătează listele și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="366"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>Lista de plugin-uri sau interfețe este goală. Reîmprospătează listele și asigură-te că hardware-ul CAN este conectat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="275"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>Imposibil de creat dispozitivul de magistrală CAN. Verifică hardware-ul și driverele.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Imposibil de conectat la dispozitivul de magistrală CAN. Verifică conexiunea hardware și setările.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="695"/>
        <source>CAN Bus Error</source>
        <translation>Eroare Magistrală CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="696"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>A apărut o eroare, dar dispozitivul CAN nu mai este disponibil.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="703"/>
        <source>Error code: %1</source>
        <translation>Cod eroare: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="706"/>
        <source>CAN Bus Communication Error</source>
        <translation>Eroare de Comunicare Magistrală CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="785"/>
        <source>No CAN driver selected</source>
        <translation>Niciun driver CAN selectat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="725"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>Încărcați mai întâi modulele kernel SOCKETCAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="721"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>Conectați un adaptor %1, apoi reîmprospătați</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="728"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Configurați mai întâi o interfață CAN virtuală</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="730"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="750"/>
        <source>No interfaces found for %1</source>
        <translation>Nicio interfață găsită pentru %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="734"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>Instalați &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;driverele PEAK CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="738"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>Instalați &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;driverele VECTOR CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="742"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>Instalează &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;driverele CAN SysTec&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="745"/>
        <source>Install %1 drivers</source>
        <translation>Instalează driverele %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="748"/>
        <source>Install %1 drivers for macOS</source>
        <translation>Instalează driverele %1 pentru macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="762"/>
        <source>

If the interface is down, bring it up first:
sudo ip link set %1 up type can bitrate %2</source>
        <translation>Dacă interfața este oprită, activează-o mai întâi:
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
        <translation>Interfață</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="885"/>
        <source>Bitrate</source>
        <translation>Rată de Biți</translation>
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
        <translation>Doar Ascultare</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="382"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>Inițializarea libusb pentru adaptorul CANable a eșuat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="410"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>Imposibil de enumerat dispozitivele USB.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="430"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>Adaptorul CANable selectat nu mai este conectat sau o altă aplicație îl are deschis. Pe Windows, dispozitivul trebuie să utilizeze driverul WinUSB (candleLight îl instalează automat).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="443"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>Nu s-a putut revendica interfața USB CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="510"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>Adaptorul CANable nu este deschis pentru scriere.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="545"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>Nu s-a reușit transmiterea tramei CAN către adaptor.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="561"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>Eroare magistrală CAN raportată de adaptorul CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="616"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>O tramă CAN nu a fost confirmată pe magistrală.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="715"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>Adaptorul CANable a respins negocierea formatului gazdă.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="722"/>
        <source>Could not read CANable timing constants.</source>
        <translation>Nu s-au putut citi constantele de temporizare CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="728"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>Rata de transmisie %1 bps nu este suportată de acest adaptor CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="734"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>Adaptorul CANable a respins rata de transmisie solicitată.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="748"/>
        <source>Could not start the CANable channel.</source>
        <translation>Nu s-a putut porni canalul CANable.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>Eroare necunoscută</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Verifică dacă utilizatorul tău este în grupul 'plugdev' sau dacă o regulă udev acordă acces la acest dispozitiv.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>Eșec la deschiderea "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>Eroare Dispozitiv HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>Dispozitivul HID a fost deconectat sau a întâmpinat o eroare fatală de citire.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>Selectează Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>Dispozitiv HID</translation>
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
        <translation>TLS 1.3 sau Mai Nou</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 sau Mai Nou</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>Orice Protocol</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>Doar Protocoale Securizate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>Niciunul</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>Interogare Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>Verificare Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>Verificare Automată Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>Funcționalitatea MQTT Necesită o Licență Comercială</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">Abonarea la un broker MQTT este disponibilă doar cu o licență comercială Serial Studio validă (nivelul Hobbyist sau superior).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>Abonarea la un broker MQTT este disponibilă doar cu o licență Serial Studio validă sau o perioadă de probă activă.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>Utilizare Bază de Date Sistem</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>Încărcare din Folder…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Selectare Director Certificate PEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>Nume Gazdă</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>Filtru Subiect</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>Nume de Utilizator</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>Parolă</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>Versiune MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Sesiune Curată</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Keep Alive Automat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS Activat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>Protocol SSL</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>Mod Verificare Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>Adâncime Verificare Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>Eroare Abonare MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Eșec la abonarea la topic-ul „%1".</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Versiune Protocol MQTT Invalidă</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>Broker-ul a respins versiunea protocolului MQTT configurată.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>ID Client Respins</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>Broker-ul a respins ID-ul clientului. Încercați un identificator diferit.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>Server MQTT Indisponibil</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>Broker-ul este momentan indisponibil. Reîncercați mai târziu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>Eroare de Autentificare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>Acreditările furnizate au fost respinse de broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>Eroare de Autorizare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>Contul nu are permisiunea pentru această operațiune.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>Eroare de Rețea sau Transport</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Problemă la nivelul rețelei/transportului în timpul conectării la broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>Violare Protocol MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>Broker-ul a raportat o violare de protocol și a închis conexiunea.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>Eroare MQTT 5</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>A apărut o eroare la nivel de protocol MQTT 5.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>Eroare MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>A apărut o eroare MQTT neașteptată.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>Port Serial Invalid</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Inițializare Modbus Eșuată</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Conexiune Modbus Eșuată</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>Portul serial selectat "%1" nu mai este disponibil. Reîmprospătează lista de porturi și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Imposibil de creat dispozitivul Modbus. Verifică configurația sistemului și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Imposibil de conectat la "%1". Verifică setările de conexiune.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="609"/>
        <source>None</source>
        <translation>Niciunul</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="610"/>
        <source>Even</source>
        <translation>Par</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="611"/>
        <source>Odd</source>
        <translation>Impar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="612"/>
        <source>Space</source>
        <translation>Spațiu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="613"/>
        <source>Mark</source>
        <translation>Marcaj</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="665"/>
        <source>Holding Registers (0x03)</source>
        <translation>Registre de Reținere (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="666"/>
        <source>Input Registers (0x04)</source>
        <translation>Registre de Intrare (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="667"/>
        <source>Coils (0x01)</source>
        <translation>Bobine (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="668"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Intrări Discrete (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>No register groups configured</source>
        <translation>Niciun grup de registre configurat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="851"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Adaugă cel puțin un grup de registre înainte de a genera un proiect.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="865"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="890"/>
        <source>Modbus Project Generator</source>
        <translation>Generator de Proiect Modbus</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Eșec la crearea fișierului de proiect temporar</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Verifică permisiunile de scriere în directorul temporar.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="862"/>
        <source>Failed to load generated project</source>
        <translation>Eșec la încărcarea proiectului generat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="863"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>JSON-ul proiectului generat nu a putut fi încărcat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="885"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Proiect generat cu succes cu %1 grupuri și %2 seturi de date.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="888"/>
        <source>The project editor is now open for customization.</source>
        <translation>Editorul de proiect este acum deschis pentru personalizare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="903"/>
        <source>Modbus Project</source>
        <translation>Proiect Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="928"/>
        <source>Holding Registers</source>
        <translation>Registre de Reținere</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="929"/>
        <source>Input Registers</source>
        <translation>Registre de Intrare</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="930"/>
        <source>Coils</source>
        <translation>Bobine</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="931"/>
        <source>Discrete Inputs</source>
        <translation>Intrări Discrete</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="945"/>
        <source>Unknown</source>
        <translation>Necunoscut</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="958"/>
        <source>Register %1</source>
        <translation>Registru %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Coil %1</source>
        <translation>Bobină %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Discrete %1</source>
        <translation>Discret %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1372"/>
        <source>Error code: %1</source>
        <translation>Cod eroare: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1375"/>
        <source>Modbus Communication Error</source>
        <translation>Eroare de Comunicare Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1387"/>
        <source>Select Port</source>
        <translation>Selectează Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1540"/>
        <source>Protocol</source>
        <translation>Protocol</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1548"/>
        <source>Slave Address</source>
        <translation>Adresă Slave</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1557"/>
        <source>Poll Interval (ms)</source>
        <translation>Interval Interogare (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1597"/>
        <source>Host / IP</source>
        <translation>Gazdă / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1604"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1619"/>
        <source>Serial Port</source>
        <translation>Port Serial</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1627"/>
        <source>Baud Rate</source>
        <translation>Rată Baud</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1635"/>
        <source>Parity</source>
        <translation>Paritate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1643"/>
        <source>Data Bits</source>
        <translation>Biți de Date</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1651"/>
        <source>Stop Bits</source>
        <translation>Biți de Stop</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="547"/>
        <source>Network socket error</source>
        <translation>Eroare socket de rețea</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="563"/>
        <source>Socket Type</source>
        <translation>Tip Socket</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="571"/>
        <source>Remote Address</source>
        <translation>Adresă la Distanță</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="579"/>
        <source>TCP Port</source>
        <translation>Port TCP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="588"/>
        <source>UDP Local Port</source>
        <translation>Port UDP Local</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="597"/>
        <source>UDP Remote Port</source>
        <translation>Port UDP la Distanță</translation>
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
        <translation>Procesul nu a putut fi pornit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Executabilul "%1" nu a fost găsit în PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>Selectează Executabil</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>Selectează Directorul de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Selectează Named Pipe / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>Procesul s-a oprit forțat.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>Cod ieșire: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>Procesul "%1" s-a oprit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>Eroare necunoscută</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>Eroare Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>Eroare Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>Nu s-a putut deschide named pipe: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>Mod</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>Lansează Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>Pipe Numit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>Executabil</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>Director de Lucru</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>Cale Pipe</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="206"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>Rata de transfer %1 bps nu este suportată de USB-CAN Analyzer.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="216"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Nu s-a putut deschide portul serial %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="227"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>Eșec la inițializarea USB-CAN Analyzer.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="264"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>USB-CAN Analyzer nu este deschis pentru scriere.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="310"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>Eroare magistrală CAN raportată de USB-CAN Analyzer.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="167"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>Rata de transfer %1 bps nu este o rată slcan standard.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="177"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Nu s-a putut deschide portul serial %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="190"/>
        <source>Failed to open the slcan channel.</source>
        <translation>Eșec la deschiderea canalului slcan.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="229"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>Adaptorul slcan nu este deschis pentru scriere.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="267"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>Eroare magistrală CAN raportată de adaptorul slcan.</translation>
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
        <translation>Niciunul</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="349"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="733"/>
        <source>Select Port</source>
        <translation>Selectează Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="392"/>
        <source>Even</source>
        <translation>Par</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="393"/>
        <source>Odd</source>
        <translation>Impar</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="394"/>
        <source>Space</source>
        <translation>Space</translation>
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
        <translation>"%1" nu este o cale validă</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="563"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Introduceți o altă cale pentru a înregistra un dispozitiv serial personalizat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="823"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Dispozitivul specificat nu a putut fi găsit. Verificați conexiunea și încercați din nou.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="830"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>A apărut o eroare necunoscută. Verificați dispozitivul și încercați din nou.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="832"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Dispozitivul nu este deschis. Deschideți dispozitivul înainte de a încerca această operațiune.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="261"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Conectarea la portul serial "%1" a eșuat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="798"/>
        <source>Unknown</source>
        <translation>Necunoscut</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="799"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Eroare critică pe portul serial "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="800"/>
        <source>Unknown error</source>
        <translation>Eroare necunoscută</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="822"/>
        <source>No error occurred.</source>
        <translation>Nu a apărut nicio eroare.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="824"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Permisiune refuzată. Asigurați-vă că aplicația are drepturile de acces necesare la dispozitiv.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="825"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Deschiderea dispozitivului a eșuat. Este posibil să fie deja în uz sau indisponibil.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="826"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>A apărut o eroare la scrierea datelor pe dispozitiv.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>A apărut o eroare la citirea datelor de pe dispozitiv.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>A apărut o eroare critică de resurse. Este posibil ca dispozitivul să fi fost deconectat sau nu mai este accesibil.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>Operația solicitată nu este suportată pe acest dispozitiv.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="831"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>Operațiunea a expirat. Dispozitivul ar putea să nu răspundă.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Serial Port</source>
        <translation>Port Serial</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Baud Rate</source>
        <translation>Rată de Transmisie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Parity</source>
        <translation>Paritate</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Data Bits</source>
        <translation>Biți de Date</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>Stop Bits</source>
        <translation>Biți de Stop</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1034"/>
        <source>Flow Control</source>
        <translation>Control Flux</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1042"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1049"/>
        <source>Auto-Reconnect</source>
        <translation>Reconectare Automată</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="156"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="164"/>
        <source>USB Error</source>
        <translation>Eroare USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="157"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Inițializarea subsistemului USB a eșuat. Verificați dacă libusb este disponibil pe sistem.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="214"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="572"/>
        <source>USB Device Error</source>
        <translation>Eroare Dispozitiv USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>Nu s-a putut deschide dispozitivul USB: %1.

Pe Linux, asigurați-vă că aveți permisiuni de citire/scriere pe nodul dispozitivului (adăugați o regulă udev sau rulați ca root). Pe macOS, driverul kernel poate necesita detașare mai întâi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="165"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Niciun dispozitiv USB selectat. Selectați un dispozitiv și reîncercați.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="171"/>
        <source>Unknown Device</source>
        <translation>Dispozitiv Necunoscut</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <source>Failed to open "%1"</source>
        <translation>Deschiderea „%1" a eșuat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="215"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Nu s-a putut revendica interfața %1 pe dispozitivul USB.

Un alt driver sau aplicație ar putea să îl dețină deja deschis. Pe Linux, încercați descărcarea driverului kernel (ex. cdc_acm) sau adăugarea unei reguli udev.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="360"/>
        <source>Select Device</source>
        <translation>Selectează Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="379"/>
        <source>Select IN Endpoint</source>
        <translation>Selectează Endpoint IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="390"/>
        <source>None (Read-only)</source>
        <translation>Niciunul (Doar Citire)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="464"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Activați Transferurile de Control USB Avansate?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="465"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Aceasta activează transferurile de control în plus față de transferurile bulk. Trimiterea de cereri de control incorecte poate bloca sau deteriora hardware-ul conectat. Activați doar dacă știți ce faceți.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="469"/>
        <source>Advanced USB Mode</source>
        <translation>Mod USB Avansat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="573"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>Dispozitivul USB a fost deconectat sau a întâmpinat o eroare fatală de citire.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="710"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Nu a fost găsit niciun endpoint izocron IN pe acest dispozitiv, dar sunt disponibile endpoint-uri bulk.

Comutați Modul de Transfer la „Flux Bulk" și încercați din nou.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="715"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Nu a fost găsit niciun endpoint bulk IN pe acest dispozitiv, dar sunt disponibile endpoint-uri izocrone.

Comutați Modul de Transfer la „Izocron" și încercați din nou.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="719"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Nu a fost găsit niciun endpoint IN utilizabil pe acest dispozitiv.

Dispozitivul poate să nu expună endpoint-uri de date în configurația sa activă sau poate necesita un driver specific.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1211"/>
        <source>USB Device</source>
        <translation>Dispozitiv USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1219"/>
        <source>Transfer Mode</source>
        <translation>Mod de Transfer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Bulk Stream</source>
        <translation>Flux Bulk</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Advanced Control</source>
        <translation>Control Avansat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Isochronous</source>
        <translation>Izocron</translation>
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
        <translation>Dimensiune Pachet ISO</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>Niciun fișier selectat…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>Text Simplu</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Binar Brut</translation>
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
        <translation>Selectează fișier de transmis</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>Fișier selectat: %1 (%2 octeți)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>Eroare la deschiderea fișierului: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>Se inițiază transferul %1…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>Transmisie completă</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>Transmisie text simplu completă</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Transmisie binară brută completă (%1 octeți)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>Transfer complet</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Transfer finalizat cu succes (%1 octeți)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>Transfer eșuat: %1</translation>
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
        <translation>Cadre pierdute</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>Datele primite sosesc mai rapid decât poate procesa Serial Studio; %1 cadre au fost pierdute. Reduceți rata de date sau dezactivați un consumator intensiv.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>Nu se poate deschide fișierul: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>Transfer anulat</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>Transfer anulat de utilizator</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>Se așteaptă receptorul…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Receptor pregătit (mod CRC), se trimit datele…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Prea multe reîncercări, transfer întrerupt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>Numărul maxim de reîncercări a fost depășit</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>NAK primit, se reîncearcă blocul %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>Eșec la poziționarea în fișier</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Transfer anulat de receptor</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Receptorul a anulat transferul</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>Transfer finalizat</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>Citirea fișierului a returnat mai multe date decât s-au solicitat</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Se trimite blocul %1 (%2 octeți)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>Se Trimite EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Timeout, se reîncearcă (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>Transfer expirat</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>Expirare: niciun răspuns de la receptor</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>Nu se poate deschide fișierul: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Transfer anulat de receptor</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Receptorul a anulat transferul</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>Se așteaptă receptorul…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>Se trimite primul EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Prea multe reîncercări, transfer întrerupt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>Număr maxim de reîncercări depășit</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>NAK primit, se reîncearcă blocul %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>Eșec la poziționarea în fișier</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>Se trimite al doilea EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>Se trimite marcatorul de sfârșit de lot…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>Transfer complet</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Se trimite antetul fișierului: %1 (%2 octeți)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Se trimite blocul %1 (%2/%3 octeți)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>Nu se poate deschide fișierul: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Fișierul este prea mare pentru ZMODEM (%1 octeți, limită 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>Transfer anulat</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>Transfer anulat de utilizator</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>Nepotrivire CRC antet hex, se abandonează trama</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Se trimit informații fișier: %1 (%2 octeți)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>Eșec la poziționarea la offset-ul %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>Citirea fișierului a returnat mai multe date decât s-au solicitat</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Receptorul solicită date de la offset-ul %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>Receptorul a omis fișierul</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>Prea multe erori, transfer întrerupt</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>Număr maxim de reîncercări depășit</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>Se trimite ZRQINIT, se așteaptă receptorul…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Date fișier trimise, se așteaptă confirmarea…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>Se Trimite ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Receptor pregătit, se trimit informațiile fișierului…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>NAK primit, se reîncearcă (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>Transfer finalizat</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Transfer anulat de receptor</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Receptorul a anulat transferul</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>Receptorul a raportat o eroare de fișier</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>Timeout la transfer</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>Timeout: niciun răspuns de la receptor</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Timeout, se reîncearcă (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>Selectează Pictogramă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>Căutare Online…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
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
        <translation>Tonuri de Gri</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Contrast Ridicat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Intens</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Viziune Nocturnă</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Infraroșu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Albastru Închis</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Chihlimbar</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Exportă Imagini</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Deschide Dosarul de Export</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Mărește</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Micșorează</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Afișează Cursor Încrucișat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>Se Așteaptă Imaginea…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>Chei API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. Implicit este Claude Haiku 4.5 ($1 intrare / $5 ieșire per milion de token-uri). Sonnet 4.6 și OPUS 4.7 sunt de asemenea disponibile. Suportă streaming, utilizare instrumente, gândire extinsă și cache-uire prompturi.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. Implicit este GPT-4o mini ($0.15 intrare / $0.60 ieșire per milion de token-uri). GPT-4o, GPT-4 Turbo și o1-mini sunt de asemenea disponibile.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. Implicit este Gemini 2.0 Flash, care are un nivel gratuit generos (supus limitelor de rată). Gemini 1.5 Pro și Gemini 1.5 Flash sunt de asemenea disponibile.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Adaugă propriile chei API. Acestea sunt criptate în repaus cu o cheie specifică mașinii și nu părăsesc niciodată computerul tău, cu excepția comunicării cu furnizorul selectat.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>Cheie setată</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>Fără cheie</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">O cheie este înregistrată — lipește una nouă pentru a o înlocui</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. Implicit este GPT-5 mini pentru lucru agentic rapid și economic. GPT-5.2 este opțiunea generală mai puternică, iar GPT-5.2 Chat urmărește modelul utilizat curent în ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. API compatibil OpenAI. Implicit este deepseek-chat (V3). deepseek-reasoner (R1) este de asemenea disponibil. Adesea cea mai ieftină opțiune cloud pentru utilizarea instrumentelor.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. O singură cheie, ~200 de modele de la Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen și altele. Modelele gratuite (cu sufix :free) au limite de rată, dar nu necesită facturare suplimentară. Plată pe măsură pentru restul.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Inferență accelerată hardware (LPU) pentru modele Llama, Mixtral, Gemma, DeepSeek-R1 distill și Qwen foarte rapide. Nivel gratuit generos cu limite zilnice de token-uri.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. Implicit este Mistral Large. Codestral vizează completarea codului, Pixtral gestionează viziunea, iar modelele Ministral sunt optimizate pentru utilizare edge / latență redusă.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Server local de modele. Funcționează cu orice endpoint compatibil OpenAI -- Ollama, llama-server al llama.cpp, LM Studio sau vLLM. Nimic nu părăsește mașina ta. Lista de modele este interogată live de la server.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>O cheie este înregistrată -- lipește una nouă pentru a o înlocui</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>Lipește cheia ta API aici</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>Ascunde cheia</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>Afișează cheia în timpul tastării</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>Obține cheia</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Deschide consola furnizorului pentru a crea o cheie nouă</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>Salvează</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>Elimină cheia stocată pentru %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Instalează Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Deschide pagina de descărcare Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>Aplică</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>Reinterogare listă modele</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Nicio cheie API configurată încă. Adaugă o cheie pentru a începe.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">Nicio cheie API configurată încă. Adaugă cel puțin una mai sus pentru a începe.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>Un furnizor este pregătit.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 furnizori sunt pregătiți.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Licențiere</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Vă rugăm așteptați…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Activează Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Lipește cheia ta de licență mai jos pentru a debloca funcțiile Pro precum MQTT, grafice 3D și altele.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>Licența ta include 5 activări de dispozitiv.
Planurile includ opțiuni Lunare, Anuale și pe Viață.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Lipește cheia ta de licență aici…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Lipește</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>Selectează Tot</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>Produs</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>Licențiat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>E-mail Licențiat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>Utilizare Dispozitive</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 dispozitive în uz (plan Nelimitat)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 din %2 dispozitive utilizate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>ID Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>Cheie de Licență</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>Portal Client</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>Cumpără Licență</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>Activează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>Dezactivează</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="517"/>
        <source>There was an issue validating your license.</source>
        <translation>A apărut o problemă la validarea licenței.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="535"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="716"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="835"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>Cheia de licență furnizată nu aparține Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="536"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Verifică că ai achiziționat licența din magazinul oficial Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="547"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="725"/>
        <source>This license key was activated on a different device.</source>
        <translation>Această cheie de licență a fost activată pe un alt dispozitiv.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="548"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="726"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Dezactivează-o mai întâi acolo sau contactează suportul pentru asistență.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="559"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="736"/>
        <source>This license is not currently active.</source>
        <translation>Această licență nu este activă în prezent.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="560"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Este posibil să fi expirat sau să fi fost dezactivată (stare: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="570"/>
        <source>Something went wrong on the server.</source>
        <translation>Ceva nu a funcționat pe server.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="571"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="747"/>
        <source>No activation ID was returned.</source>
        <translation>Nu s-a returnat niciun ID de activare.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="581"/>
        <source>Could not validate your license at this time.</source>
        <translation>Nu s-a putut valida licența în acest moment.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="582"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="756"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="845"/>
        <source>Try again later.</source>
        <translation>Încearcă din nou mai târziu.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="717"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="836"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Verifică din nou că ai achiziționat licența din magazinul oficial Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <source>Something went wrong on the server…</source>
        <translation>Ceva nu a funcționat pe server…</translation>
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
        <translation>Licența ta a fost activată cu succes.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="654"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Mulțumim că susții Serial Studio!
Acum ai acces la toate funcționalitățile premium.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="708"/>
        <source>There was an issue activating your license.</source>
        <translation>A apărut o problemă la activarea licenței tale.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Could not activate your license at this time.</source>
        <translation>Nu s-a putut activa licența ta în acest moment.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="827"/>
        <source>There was an issue deactivating your license.</source>
        <translation>A apărut o problemă la dezactivarea licenței tale.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="844"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>Nu s-a putut dezactiva licența în acest moment.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="854"/>
        <source>Your license has been deactivated.</source>
        <translation>Licența a fost dezactivată.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="855"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Accesul la funcțiile Pro a fost eliminat.
Îți mulțumim din nou pentru susținerea Serial Studio!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>Exportul MDF4 este o funcție Pro.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Această funcție necesită o licență. Achiziționați una pentru a activa exportul MDF4.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>Selectează fișier MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>Fișiere MDF4 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>Deconectare de la dispozitiv?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>Trebuie să te deconectezi de la dispozitivul curent înainte de a deschide un fișier MDF4.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>Nu se poate deschide fișierul MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Fișierul poate fi corupt sau într-un format neacceptat.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>Fișier MDF4 Invalid</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Citirea structurii fișierului a eșuat. Fișierul poate fi corupt.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>Fără date în fișier</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>Fișierul MDF4 nu conține date de măsurare.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>Nume Gazdă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>ex. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Filtru Topic</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>ex. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Regenerează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Nume de Utilizator</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Parolă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>Versiune</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Sesiune Curată</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>Utilizare SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>Protocol SSL</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Verificare Peer</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Adâncime Verificare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>Certificate CA</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Încarcă din Dosar…</translation>
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
        <translation type="vanished">TLS 1.3 sau Mai Nou</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 sau Mai Nou</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">Orice Protocol</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">Doar Protocoale Securizate</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Niciunul</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">Interogare Peer</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">Verificare Peer</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">Verificare Automată Peer</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">Utilizare Bază de Date Sistem</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">Abonat MQTT</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">Publisher MQTT</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">Funcția MQTT Necesită o Licență Comercială</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">Încarcă din Dosar…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">Conectarea la brokeri MQTT este disponibilă doar cu o licență comercială Serial Studio validă (nivelul Hobbyist sau superior).

Pentru a debloca această funcție, vă rugăm să activați licența sau să vizitați magazinul.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">Topic MQTT Lipsește</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">Trebuie să specificați un topic înainte de a vă conecta ca publisher.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">Eroare Configurare</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">Topic MQTT Nesetat</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">Nu veți primi niciun mesaj până când un topic este configurat.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">Avertisment Configurare</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">Topic MQTT Invalid</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Topic-ul „%1" nu este valid.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">Selectează Directorul Certificatelor PEM</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">Eroare Abonare</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Eșec la abonarea la topic-ul „%1".</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">Versiune Protocol MQTT Invalidă</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">Broker-ul MQTT a respins conexiunea din cauza unei versiuni de protocol nesuportate. Asigură-te că clientul și broker-ul suportă aceeași versiune de protocol.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">ID Client Respins</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">Broker-ul a respins ID-ul clientului. Poate fi malformat, prea lung sau deja în uz. Încearcă să folosești un ID client diferit.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">Server MQTT Indisponibil</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">Conexiunea de rețea a fost stabilită, dar broker-ul este momentan indisponibil. Verifică starea broker-ului și încearcă din nou mai târziu.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">Eroare de Autentificare</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">Numele de utilizator sau parola furnizată este incorectă sau malformată. Verifică din nou acreditările și încearcă din nou.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">Eroare de Autorizare</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">Broker-ul MQTT a refuzat conexiunea din cauza permisiunilor insuficiente. Asigură-te că contul tău are drepturile de acces necesare.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">Eroare de Rețea sau Transport</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">A apărut o problemă la nivelul rețelei sau al stratului de transport, cauzând o defecțiune neașteptată a conexiunii. Verifică conexiunea de rețea și setările broker-ului.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">Violare Protocol MQTT</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">Clientul a detectat o violare a protocolului MQTT și a închis conexiunea. Verifică implementarea MQTT pentru conformitate.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">Eroare Necunoscută</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">A apărut o eroare neașteptată. Verificați jurnalele pentru mai multe detalii sau reporniți aplicația.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">Eroare MQTT 5</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">A apărut o eroare la nivelul protocolului MQTT 5. Verificați jurnalele broker-ului sau codurile de motiv pentru mai multe detalii.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">Autentificare MQTT Eșuată</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">Autentificare eșuată: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">Este necesară autentificare extinsă, dar MQTT 5.0 nu este activat.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">Necunoscut</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">Autentificare MQTT Necesară</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">Broker-ul MQTT necesită autentificare folosind metoda: "%1".

Furniză acreditările necesare.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">Introduceți Nume Utilizator MQTT</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">Nume de Utilizator:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">Introduceți Parola MQTT</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">Parolă:</translation>
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
        <translation>TLS 1.3 sau Mai Nou</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 sau Mai Nou</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>Orice Protocol</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>Doar Protocoale Securizate</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>Niciunul</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>Interogare Peer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>Verificare Peer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>Verificare Automată Peer</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>Date RX Brute</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>Script Personalizat</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Date Dashboard (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Date Dashboard (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>Publisher MQTT indisponibil</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>Este necesară o licență comercială validă pentru a utiliza publicarea MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>Testare Conexiune MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Selectează Directorul Certificatelor PEM</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>Broker MQTT accesibil</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>Broker MQTT inaccesibil</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>Conexiunea la broker-ul MQTT a eșuat</translation>
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
        <translation>Octeți cadru exemplu (text sau hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Test</source>
        <translation>Testează</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="100"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Apply</source>
        <translation>Aplică</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="103"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="112"/>
        <source>Language:</source>
        <translation>Limbaj:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="115"/>
        <source>Template:</source>
        <translation>Șablon:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="126"/>
        <source>Frame:</source>
        <translation>Cadru:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="130"/>
        <source>Output:</source>
        <translation>Ieșire:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="270"/>
        <source>Enter a frame</source>
        <translation>Introduceți un cadru</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="277"/>
        <source>Invalid hex</source>
        <translation>Hex invalid</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatează Documentul	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="361"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatează Selecția	ctrl+i</translation>
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
-- Definește o funcție mqtt(frame) care primește octeții bruti
-- ai unui cadru parsat și returnează payload-ul de publicat către
-- broker, sau nil pentru a sări peste acest cadru.
--
-- Argumentul frame corespunde cu ceea ce vede scriptul Frame Parser:
-- un string Lua care conține octeții dintre delimitatorii
-- FrameReader.
--
-- Exemplu:
--   function mqtt(frame)
--     return frame    -- transmite ca atare
--   end
--
-- Folosește meniul Template pentru exemple gata făcute.
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
 * Definește o funcție mqtt(frame) care primește octeții bruti
 * ai unui cadru parsat și returnează payload-ul de publicat către
 * broker, sau null pentru a sări peste acest cadru.
 *
 * Argumentul frame corespunde cu ceea ce vede scriptul Frame Parser:
 * un string care conține octeții dintre delimitatorii FrameReader.
 *
 * Exemplu:
 *   function mqtt(frame) {
 *     return frame;   // transmite ca atare
 *   }
 *
 * Folosește meniul Template pentru exemple gata făcute.
 */
</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="615"/>
        <source>Script is empty</source>
        <translation>Scriptul este gol</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="622"/>
        <source>Lua engine error</source>
        <translation>Eroare motor Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="696"/>
        <source>Error: %1</source>
        <translation>Eroare: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="652"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() nu este definită</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- cadru sărit)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="671"/>
        <source>(non-string return)</source>
        <translation>(returnare non-string)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="701"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- trama omisă)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="779"/>
        <source>Select Template…</source>
        <translation>Selectează Șablon…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Configurează hostname-ul și portul brokerului înainte de a testa conexiunea.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Conectat cu succes la %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Timeout după 5 secunde fără a ajunge la broker.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">Configurare MQTT</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT este o funcție Pro</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">Activați licența sau vizitați magazinul pentru a debloca suportul MQTT.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">General</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">Autentificare</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">Opțiuni MQTT</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">Proprietăți SSL</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">Gazdă</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">Port</translation>
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
        <translation type="vanished">Sesiune Curată</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">Nume Utilizator</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">Nume Utilizator MQTT</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">Parolă</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">Parolă MQTT</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">Versiune</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">Mod</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">Subiect</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">ex. sensors/temperature sau home/+/status</translation>
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
        <translation type="vanished">Subiect Will</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">ex. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Mesaj Will</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">ex. Dispozitiv deconectat neașteptat</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">Activează SSL</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">Protocol SSL</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">Adâncime Verificare</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">Mod Verificare</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Închide</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">Deconectare</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">Conectare</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="205"/>
        <source>Console Only Mode</source>
        <translation>Mod Doar Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="208"/>
        <source>Quick Plot Mode</source>
        <translation>Mod Grafic Rapid</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="215"/>
        <source>Empty Project</source>
        <translation>Proiect Gol</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="696"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="704"/>
        <source>Waiting for data…</source>
        <translation>Se așteaptă date…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="705"/>
        <source>Connecting to device…</source>
        <translation>Se conectează la dispozitiv…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Exemplu de cod</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Completor</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Evidențiator</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Stil</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>spații</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>Copiat în Clipboard</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>Player MDF4</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Eroare</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Tu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Descoperire</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Execuție</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>Aprobă %1 acțiuni în %2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Aceste apeluri vor rula împreună. Extinde fiecare card mai jos pentru a inspecta argumentele.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Aprobă toate</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Respinge toate</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Copiază Tot</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Selectează Tot</translation>
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
        <translation>Asistent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>Eroare</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Descoperire</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Execuție</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>În Execuție</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>Așteaptă aprobare</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Finalizat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Refuzat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Blocat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Aprobă</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Respinge</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Aprobă toate</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Respinge toate</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Rezultat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>Aprobă {n} acțiuni în {family}?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Aceste apeluri vor rula împreună. Extinde fiecare card pentru a inspecta argumentele.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>Eșec la încărcarea README: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>Temă</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>Analizor de Trame</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>Șablon Proiect</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>Toate Tipurile</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>Resetează Extensiile</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Aceasta dezinstalează toate extensiile, elimină toate depozitele personalizate și restaurează setările implicite. Continuați?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>Selectați Folderul Depozitului de Extensii</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1017"/>
        <source>Installed (repository no longer available)</source>
        <translation>Instalat (depozitul nu mai este disponibil)</translation>
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
        <translation>Eroare Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Plugin-ul "%1" nu este instalat.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1335"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>Extensia "%1" nu este un plugin (tip: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1356"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Nu se poate citi fișierul de metadate al plugin-ului:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Plugin-ul "%1" necesită GRPC, dar această versiune nu include suport GRPC.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Acest plugin folosește GRPC pentru streaming de date de înaltă performanță. Serverul API trebuie activat.

Doriți să îl activați acum?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1393"/>
        <source>API Server Required</source>
        <translation>Server API Necesar</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1422"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Plugin-ul "%1" nu are câmpul 'entry' în info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Entry point not found:
%1</source>
        <translation>Punct de intrare negăsit:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Plugin-ul "%1" are o cale invalidă pentru punctul de intrare.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1484"/>
        <source>Missing Dependency</source>
        <translation>Dependență Lipsă</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Acest plugin necesită "%1" dar nu a fost găsit pe sistemul dumneavoastră.

Doriți să deschideți pagina de descărcare?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>Plugin-urile necesită serverul API pentru a comunica cu Serial Studio. Doriți să îl activați acum?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Repornire Necesară</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>Noul backend de randare va intra în vigoare după repornirea Serial Studio. Repornești acum pentru a aplica modificarea?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>Eșec la încărcarea paginii: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="152"/>
        <source>Invalid icon identifier</source>
        <translation>Identificator de pictogramă invalid</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="222"/>
        <source>Empty SVG data received</source>
        <translation>Date SVG goale primite</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Comandă Rapidă Windows (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>Aplicație macOS (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Intrare Desktop (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Folosește o pictogramă .icns pentru cel mai bun rezultat în Finder și Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Lasă pictograma goală pentru a moșteni pictograma executabilului Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Plasează fișierul în ~/.local/share/applications/ pentru a-l expune în lansatorul de aplicații.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Imagine Pictogramă Apple (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Pictogramă Windows (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>Imagine Vectorială sau Raster (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>Este necesară o licență Pro pentru a genera comenzi rapide.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>Nu a fost furnizată nicio cale de ieșire.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>Eșec la scrierea fișierului de comandă rapidă.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Nu s-a putut înlocui comanda rapidă existentă la %1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Nu s-a putut crea structura de directoare a pachetului .app.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Nu s-a putut scrie lansatorul pachetului: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Nu s-a putut marca lansatorul pachetului ca executabil.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Nu s-a putut scrie Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Scrierea de comenzi rapide Windows nu este disponibilă pe această platformă.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Scrierea de comenzi rapide Linux nu este disponibilă pe această platformă.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>Nu s-a putut inițializa COM (necesar pentru scrierea comenzilor rapide .lnk).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) a eșuat (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) a eșuat (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Salvarea fișierului .lnk a eșuat (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>Scrierea de comenzi rapide macOS nu este disponibilă pe această platformă.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Nu s-a putut deschide calea comenzii rapide pentru scriere: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Comandă rapidă Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Nu s-a putut marca comanda rapidă ca executabilă.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>Sistem</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">Verificare automată a actualizărilor?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">%1 să verifice automat actualizările? Poți verifica manual actualizările oricând din dialogul „Despre"</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>Salvează</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>Salvează tot</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>Deschide</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>Da</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>Da la toate</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>Nu</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>Nu la toate</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>Abandonează</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>Reîncearcă</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>Ignoră</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>Renunță</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>Ajutor</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>Aplică</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>Restabilește Valorile Implicite</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>Selectează Locația Spațiului de Lucru</translation>
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
        <translation>Port Serial</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="93"/>
        <source>Baud Rate</source>
        <translation>Rată de Transmisie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="184"/>
        <source>Parity</source>
        <translation>Paritate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="205"/>
        <source>Data Bits</source>
        <translation>Biți de Date</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="226"/>
        <source>Stop Bits</source>
        <translation>Biți de Stop</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="247"/>
        <source>Host</source>
        <translation>Gazdă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="257"/>
        <source>IP Address</source>
        <translation>Adresă IP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="271"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="280"/>
        <source>TCP Port</source>
        <translation>Port TCP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="308"/>
        <source>Slave Address</source>
        <translation>Adresă Slave</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="313"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="328"/>
        <source>Poll Interval (ms)</source>
        <translation>Interval Interogare (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="333"/>
        <source>Polling interval</source>
        <translation>Interval de interogare</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="361"/>
        <source>Configure Register Groups…</source>
        <translation>Configurare Grupuri Registre…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="371"/>
        <source>Import Register Map…</source>
        <translation>Importă Hartă Registre…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="386"/>
        <source>%1 group(s) configured</source>
        <translation>%1 grup(uri) configurat(e)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="387"/>
        <source>No groups configured</source>
        <translation>Niciun grup configurat</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Grupuri Registre Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Configurează mai multe grupuri de registre pentru a interoga diferite tipuri de registre în secvență.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Adaugă Grup Nou</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Tip Registru:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Adresă Inițială:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Număr Registre:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Adaugă Grup</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Grupuri Configurate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>Tip</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>Număr</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Acțiune</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Elimină</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Niciun grup configurat.
Adaugă grupuri mai sus pentru a interoga mai multe tipuri de registre.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Total grupuri: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Generează Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Șterge Tot</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Previzualizare Hartă Registre Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>Fișier: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Revizuiește registrele pentru import într-un proiect nou Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>Registre</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>Nume</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Adresă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Tip</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Tip Date</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Unități</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>Niciun registru găsit în fișier.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Total: %1 registre în %2 grupuri</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Creează Proiect</translation>
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
        <translation>Conectat la broker</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Neconectat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Testează Conexiunea</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Testează broker-ul cu setările curente</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Activează publicarea mai întâi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Editează Script</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Editează scriptul de publicare (Lua sau JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Încarcă Certificate CA</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>Adaugă certificate PEM dintr-un folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>Activează SSL/TLS mai întâi</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolează</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="305"/>
        <source>Interpolation: %1</source>
        <translation>Interpolare: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Afișează Legendele</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="327"/>
        <source>Show X Axis Label</source>
        <translation>Afișează Eticheta Axei X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="338"/>
        <source>Show Y Axis Label</source>
        <translation>Afișează Eticheta Axei Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="350"/>
        <source>Show Crosshair</source>
        <translation>Afișează Cursor Încrucișat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Resume</source>
        <translation>Reia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="374"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Mod Baleiaj / Declanșare</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="386"/>
        <source>Trigger Settings</source>
        <translation>Setări Declanșare</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="410"/>
        <source>Reset View</source>
        <translation>Resetează Vizualizarea</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="416"/>
        <source>Axis Range Settings</source>
        <translation>Setări Interval Axă</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Eșantioane</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>Octeți per valoare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Numărul de octeți combinați în fiecare valoare de canal.</translation>
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
        <translation>Ordinea octeților utilizată la combinarea valorilor multi-octet.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>Valori semnate</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Interpretează fiecare valoare ca semnat în complement față de doi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>Tabel de rutare tag-uri</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Intrări tag:index separate prin virgulă, ex. 1:0,2:1,3:2. Tag-urile pot fi zecimale sau hexazecimale cu prefix 0x.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>Validează checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Respinge mesajele cu checksum Fletcher invalid.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Respinge mesajele cu checksum aditiv invalid.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>Versiune protocol</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Selectează marcatorul de start așteptat (0xFE pentru v1, 0xFD pentru v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>Validează CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Respinge cadrele cu o sumă de control CRC-24Q invalidă.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>Număr de canale</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Numărul de canale de ieșire (registre sau bobine).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>Decalaj registru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Decalaj de adresă scăzut din ecourile de scriere unică (40001 pentru hărți Modicon vechi).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>Registre semnate</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Interpretează registrele pe 16 biți ca valori semnate în complement față de doi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>Aspect payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>Array-ul emite fiecare element în ordine; map-ul direcționează cheile prin lista de chei.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>Chei (mod hartă)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Chei de hartă separate prin virgulă în ordinea canalelor. Utilizate doar pentru aspectul de hartă.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Câmpuri scalare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Câmpuri JSON separate prin virgulă, repetate în fiecare cadru generat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Câmp array de eșantioane</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>Câmp JSON care conține array-ul de eșantioane grupate.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Câmp array de înregistrări</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>Câmp JSON care conține array-ul de obiecte înregistrare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Câmpuri înregistrare (în ordinea canalelor)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Câmpuri înregistrare separate prin virgulă. Poziția fiecărui câmp stabilește indexul canalului său.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>Lățimi coloane</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Număr de caractere separate prin virgulă per câmp. Lăsați gol pentru separare pe spații albe.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>Elimină spații albe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Elimină spațiile din jurul fiecărui câmp secționat.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>Chei (în ordinea canalelor)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Nume de chei separate prin virgulă. Poziția fiecărei chei stabilește indexul canalului său.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>Separator perechi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>Caracter între perechile cheie=valoare.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>Separator cheie-valoare</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>Caracter între o cheie și valoarea sa.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>Doar valori numerice</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Ignoră perechile a căror valoare nu este un număr.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>Tabel de rutare comenzi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Intrări separate prin punct și virgulă în format listă NUME:index, de ex. CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>Prefix vorbitor</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>Identificator vorbitor de două litere, de ex. GP pentru GPS sau GN pentru receptoare multi-constelație.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Respinge propozițiile al căror checksum *hh nu se potrivește.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Nume parametri separați prin virgulă. Poziția fiecărei chei stabilește indexul canalului său.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>Câmpuri (în ordinea canalelor)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Nume câmpuri separate prin virgulă. Poziția fiecărui câmp stabilește indexul canalului său.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>Etichete (în ordinea canalelor)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Nume de etichete separate prin virgulă. Poziția fiecărei etichete stabilește indexul canalului său.</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">Blocuri de Registre</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">Blocuri de registre interogate cu intrări tipizate și scalate. Scrise de importatorul de hartă de registre Modbus.</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">Hartă de Semnale</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">Mesaje CAN cu layout-urile de biți ale semnalelor și scalare. Scrise de importatorul DBC.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Tip Socket</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Port Local</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>Introdu 0 pentru port automat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Adresă la Distanță</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Port la Distanță</translation>
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
        <translation>Filtrare după canal…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Șterge toate notificările</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(fără titlu)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Nicio notificare încă</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">Transformările de seturi de date și scripturile widget-urilor de ieșire pot posta evenimente aici prin notifyInfo / notifyWarning / notifyCritical.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>Căutare Pictograme Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>Descărcare eșuată: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Căutare pictograme (ex. temperatură, săgeată, redare)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>Căutare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>Caută pictograme mai sus pentru a începe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Widget-urile de ieșire necesită o licență Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Buton</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Trimite o comandă la clic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Cursor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Trimite valori numerice scalate</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Comutator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Trimite comenzi pornit/oprit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Câmp Text</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Tastează și trimite comenzi arbitrare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Buton Rotativ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Intrare rotativă pentru valori de referință</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Poți configura widget-uri de ieșire, dar acestea apar pe tabloul de bord doar cu o licență Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Duplică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Duplică acest widget de ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Șterge acest widget de ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Funcție de Transmisie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Importă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Importă funcția de transmisie dintr-un fișier .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Șablon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Selectează un șablon de funcție de transmisie predefinit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Testează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Testează funcția de transmisie cu date de intrare exemplu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Refă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Decupează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Lipește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Selectează Tot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Formatează Documentul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Formatează Selecția</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Eroare Widget Painter</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Editor Cod Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Importă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Importă cod painter dintr-un fișier .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Șablon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Selectează un șablon de desenare predefinit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Formatează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Reformatează codul de desenare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Testează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Deschide o previzualizare live cu valori simulate de dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Previzualizare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Decupează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Lipește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Selectează Tot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Refă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Formatează Documentul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Formatează Selecția</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Previzualizare Live Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Previzualizare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Seturi de date simulate</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>Se așteaptă primul cadru...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Șterge consola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpolează</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="294"/>
        <source>Interpolation: %1</source>
        <translation>Interpolare: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="307"/>
        <source>Show Area Under Plot</source>
        <translation>Afișează Aria Sub Grafic</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="326"/>
        <source>Show X Axis Label</source>
        <translation>Afișează Eticheta Axei X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="337"/>
        <source>Show Y Axis Label</source>
        <translation>Afișează Eticheta Axei Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="349"/>
        <source>Show Crosshair</source>
        <translation>Afișează Cursor Încrucișat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Resume</source>
        <translation>Reia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="373"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Mod Baleiaj / Declanșare</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="385"/>
        <source>Trigger Settings</source>
        <translation>Setări Declanșare</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="409"/>
        <source>Reset View</source>
        <translation>Resetează Vizualizarea</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="415"/>
        <source>Axis Range Settings</source>
        <translation>Setări Interval Axă</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="202"/>
        <source>Interpolate</source>
        <translation>Interpolează</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="220"/>
        <source>Orbit Navigation</source>
        <translation>Navigare Orbitală</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="230"/>
        <source>Pan Navigation</source>
        <translation>Navigare Panoramare</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="241"/>
        <source>Orthogonal View</source>
        <translation>Vizualizare Ortogonală</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="247"/>
        <source>Top View</source>
        <translation>Vizualizare de Sus</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="253"/>
        <source>Left View</source>
        <translation>Vizualizare din Stânga</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="259"/>
        <source>Front View</source>
        <translation>Vizualizare Frontală</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="276"/>
        <source>Auto Center</source>
        <translation>Centrare Automată</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="292"/>
        <source>Anaglyph 3D</source>
        <translation>3D Anaglifă</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="306"/>
        <source>Invert Eye Positions</source>
        <translation>Inversează Pozițiile Ochilor</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="71"/>
        <source>None</source>
        <translation>Niciunul</translation>
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
        <translation>Liniară</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1392"/>
        <source>Time</source>
        <translation>Timp</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1415"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Trage pentru a muta, clic dreapta pentru a șterge</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1418"/>
        <source>Click to place cursor</source>
        <translation>Clic pentru a plasa cursorul</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1420"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Clic pentru a plasa al doilea cursor — Trage pentru a muta</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Vizitează Site-ul Web</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Cumpără Licență</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Activează</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Asistent — Funcție Pro</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>Asistentul este o funcție Serial Studio Pro. Activează licența pentru a-l debloca.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Activează</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>Mod</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>Lansează Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Pipe Numit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Executabil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/cale/către/executabil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Răsfoiește</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Argumente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 valoare1 --arg2 valoare2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Director de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(opțional) /director/de/lucru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Cale Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Alege Proces în Execuție…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Lansează un proces copil și capturează stdout-ul acestuia, sau conectează-te la un pipe cu nume scris de un proces existent.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Află despre pipe-urile cu nume</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Selectează Proces în Execuție</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Selectează un proces în execuție pentru a deriva o sugestie de cale pentru pipe-ul cu nume.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Filtrează Procesele</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Tastează pentru a filtra după nume…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Reîmprospătează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Procese în Execuție</translation>
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
        <translation>Niciun proces nu corespunde filtrului.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Niciun proces activ găsit.
Faceți clic pe Reîmprospătare pentru a actualiza lista.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 proces(e)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Selectează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>modificat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="366"/>
        <source>This project is password protected</source>
        <translation>Acest proiect este protejat prin parolă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="367"/>
        <source>Editing is available in Project mode</source>
        <translation>Editarea este disponibilă în modul Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="378"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Introduceți parola pentru a efectua modificări sau deschideți un alt proiect.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="379"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Comutați la modul Proiect pentru a încărca și edita un proiect.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="401"/>
        <source>Unlock</source>
        <translation>Deblochează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="402"/>
        <source>Switch to Project Mode</source>
        <translation>Comută la Modul Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="421"/>
        <source>Open Other Project</source>
        <translation>Deschide Alt Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="422"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="438"/>
        <source>Create New Project</source>
        <translation>Creează Proiect Nou</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>Structura Proiectului</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>Căutare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="349"/>
        <source>Move Up</source>
        <translation>Mută Sus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="359"/>
        <source>Move Down</source>
        <translation>Mută Jos</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="374"/>
        <source>Rename</source>
        <translation>Redenumește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="387"/>
        <source>Disable</source>
        <translation>Dezactivează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="387"/>
        <source>Enable</source>
        <translation>Activează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="410"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Duplică Selecția (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="411"/>
        <source>Duplicate</source>
        <translation>Duplică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="429"/>
        <source>Delete Selected (%1)</source>
        <translation>Șterge Selecția (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="430"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="481"/>
        <source>New Folder</source>
        <translation>Folder Nou</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="491"/>
        <source>New Sub-folder</source>
        <translation>Subfolder Nou</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="507"/>
        <source>Move to Folder</source>
        <translation>Mută în Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="514"/>
        <source>Top Level</source>
        <translation>Nivel Superior</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="547"/>
        <source>Move Here</source>
        <translation>Mută Aici</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Nou</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Creează un proiect JSON nou</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Deschide</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Deschide un proiect JSON existent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Salvează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Salvează proiectul curent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Salvează Ca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Salvează proiectul curent sub un nume nou</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">Deblochează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Blochează</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">Deblocați Editorul de Proiect cu parola proiectului</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Importă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Generează un proiect dintr-o schemă Protocol Buffers (.proto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Setează o parolă și blochează Editorul de Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Adaugă Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Adaugă o nouă sursă de date (dispozitiv) la proiect</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Acțiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Adaugă o nouă acțiune la proiect</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Restaurează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Restaurează un instantaneu automat recent al proiectului curent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Adaugă un panou nou de control ieșire cu un buton</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Cursor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Adaugă un control cursor de ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Comutator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Adaugă un control comutator de ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Buton Rotativ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Adaugă un control buton rotativ de ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Câmp Text</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Adaugă un control câmp text de ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Buton</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Adaugă un control buton de ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>Set de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>Adaugă un set de date generic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Grafic</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>Adaugă un set de date grafic 2D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>Grafic FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Adaugă un grafic de transformată Fourier rapidă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Indicator</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Adaugă un widget indicator pentru date numerice</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Indicator Nivel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Adaugă un indicator de nivel cu bară verticală</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>Busolă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Adaugă un widget busolă pentru date direcționale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>Indicator LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Adaugă un indicator de stare tip LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Adaugă un grup container de seturi de date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Container Seturi de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Imagine</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Adaugă un vizualizator de flux imagine/video</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Vizualizare Imagine</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="450"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Web View</source>
        <translation>Vizualizare Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="452"/>
        <source>Add an web viewer</source>
        <translation>Adaugă un vizualizator web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="462"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="466"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Adaugă un widget painter personalizat randat în JavaScript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="467"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Widget-urile painter necesită o licență Pro — adăugarea unuia va reveni la o grilă de date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="468"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="480"/>
        <source>Table</source>
        <translation>Tabel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Add a data table view</source>
        <translation>Adaugă o vizualizare tabel de date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Data Grid</source>
        <translation>Grilă de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="491"/>
        <source>Multi-Plot</source>
        <translation>Grafic Multiplu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="493"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>Adaugă un grafic 2D cu multiple semnale</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Multiple Plot</source>
        <translation>Grafic Multiplu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="500"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>3D Plot</source>
        <translation>Grafic 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <source>Add a 3D plot visualization</source>
        <translation>Adaugă o vizualizare grafic 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="511"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="515"/>
        <source>Accelerometer</source>
        <translation>Accelerometru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>Adaugă un grup pentru date de accelerometru pe 3 axe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="521"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="524"/>
        <source>Gyroscope</source>
        <translation>Giroscop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>Adaugă un grup pentru date de giroscop pe 3 axe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="530"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="535"/>
        <source>GPS Map</source>
        <translation>Hartă GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="533"/>
        <source>Add a map widget for GPS data</source>
        <translation>Adaugă un widget hartă pentru date GPS</translation>
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
        <translation>Deschide Asistentul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="560"/>
        <source>Help Center</source>
        <translation>Centru de Ajutor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="564"/>
        <source>Open the Project Editor documentation</source>
        <translation>Deschide documentația Editorului de Proiect</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Rezumat Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>Funcționalități Pro detectate în acest proiect.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Se utilizează widget-uri de rezervă. Cumpără o licență pentru a debloca funcționalitatea completă.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Titlu Proiect:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Proiect Fără Titlu</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Puncte:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="145"/>
        <source>Settings</source>
        <translation>Setări</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="170"/>
        <source>Time Range:</source>
        <translation>Interval Timp:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="212"/>
        <source>Point Count:</source>
        <translation>Număr de Puncte:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="230"/>
        <source>Change-Driven Transforms:</source>
        <translation>Transformări Conduse de Schimbări:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="239"/>
        <source>Run a dataset's transform only when one of its inputs changes. Speeds up large table-driven projects; off by default.</source>
        <translation>Execută transformarea unui set de date doar când una dintre intrările sale se modifică. Accelerează proiectele mari bazate pe tabele; dezactivat implicit.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="286"/>
        <source>Source</source>
        <translation>Sursă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="287"/>
        <source>Sources</source>
        <translation>Surse</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="292"/>
        <source>Group</source>
        <translation>Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="293"/>
        <source>Groups</source>
        <translation>Grupuri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="298"/>
        <source>Dataset</source>
        <translation>Set de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="299"/>
        <source>Datasets</source>
        <translation>Seturi de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="304"/>
        <source>Action</source>
        <translation>Acțiune</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="305"/>
        <source>Actions</source>
        <translation>Acțiuni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="393"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Faceți dublu clic pe un bloc pentru a-l edita. Clic dreapta oriunde pentru a adăuga un grup, set de date, acțiune, tabel de date sau dispozitiv.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Previzualizare Fișier Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Fișier Proto: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Parcurgeți mesajele de mai jos, apoi creați proiectul. Fiecare mesaj devine un grup în tablou de bord; blocurile de canale cu tipuri identice primesc un MultiPlot, iar mesajele cu tipuri mixte primesc un DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Afișare câmpuri pentru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Câmpuri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Etichetă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Nume Câmp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Tip</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Niciun câmp în mesajul selectat.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Total: %1 mesaje, %2 câmpuri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Creează Proiect</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Nicio eroare</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Broker-ul a respins conexiunea din cauza unei versiuni de protocol nesuportate. Potrivește versiunea MQTT a broker-ului și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Broker-ul a respins ID-ul clientului. Poate fi malformat, prea lung sau deja în uz. Regenerează-l și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>Rețeaua a ajuns la broker, dar broker-ul este momentan indisponibil. Verifică starea acestuia și încearcă din nou mai târziu.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>Numele de utilizator sau parola este incorectă sau malformată. Verifică din nou acreditările și încearcă din nou.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Broker-ul a refuzat conexiunea din cauza permisiunilor insuficiente. Verifică că contul are ACL-urile necesare.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>O problemă la nivelul rețelei sau al stratului de transport a împiedicat conexiunea. Verifică conectivitatea, porturile și configurația TLS.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>Clientul a detectat o violare a protocolului MQTT și a închis conexiunea. Verifică compatibilitatea broker-ului și clientului.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>A apărut o eroare neașteptată. Verifică jurnalele broker-ului și consola aplicației pentru detalii.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>A apărut o eroare la nivelul protocolului MQTT 5. Inspectează codul de motiv al broker-ului pentru detalii.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Eroare MQTT nespecificată (cod %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>Nu s-a putut încărca textul de bun venit :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="251"/>
        <source>Network error</source>
        <translation>Eroare de rețea</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="254"/>
        <location filename="../../src/Licensing/Trial.cpp" line="270"/>
        <location filename="../../src/Licensing/Trial.cpp" line="302"/>
        <source>Trial Activation Error</source>
        <translation>Eroare de Activare Trial</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>Invalid server response</source>
        <translation>Răspuns invalid de la server</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="268"/>
        <source>The server returned malformed data: %1</source>
        <translation>Serverul a returnat date malformate: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>Unexpected server response</source>
        <translation>Răspuns neașteptat de la server</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="300"/>
        <source>The server response is missing required fields.</source>
        <translation>Răspunsul serverului nu conține câmpurile necesare.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Eroare Fișier Ieșire Consolă</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>Nu se poate deschide fișierul pentru scriere!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Adaptor Bluetooth invalid!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>Platformă sau sistem de operare nesuportat</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>Metodă de descoperire nesuportată</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>Eroare generală de I/O</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>Analizor de Trame Dezactivat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Analizorul de trame Lua pentru sursa %1 a expirat pentru %2 trame consecutive și a fost dezactivat pentru a menține Serial Studio responsiv.

Cauza cea mai probabilă: o buclă infinită sau o operație extrem de lentă în corpul scriptului. Corectați scriptul și reîncărcați proiectul pentru a reactiva analizarea.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>Eroare de Sintaxă Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>Codul parserului conține o eroare:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>Eroare de Execuție Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>Codul parserului a declanșat o eroare:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>Funcție Parse Lipsă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>Funcția 'parse' nu este definită în script.

Asigurați-vă că codul include:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>Eroare de Execuție A Funcției Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>Funcția parse conține o eroare:

%1

Vă rugăm să corectați eroarea în corpul funcției.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Analizorul de trame JavaScript pentru sursa %1 a expirat pentru %2 trame consecutive și a fost dezactivat pentru a menține Serial Studio responsiv.

Cauza cea mai probabilă: o buclă infinită sau o operație extrem de lentă în corpul scriptului. Corectați scriptul și reîncărcați proiectul pentru a reactiva analizarea.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>Javascript A Expirat</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>Codul parser nu a terminat evaluarea în %1 ms și a fost întrerupt.

Cauza cea mai probabilă: o buclă infinită la nivelul superior al scriptului.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>Eroare de Sintaxă Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>Codul parserului conține o eroare de sintaxă la linia %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>A Apărut O Excepție Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>Codul parserului a declanșat următoarele excepții:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>Funcția 'parse' nu este definită în script.

Asigurați-vă că codul dvs. include:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>Funcția parse conține o eroare la linia %1:

%2

Vă rugăm să corectați eroarea în corpul funcției.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>Declarație de Funcție Invalidă</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Nu a fost găsit niciun export 'parse' apelabil.

Definește una dintre variante:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>Funcția 'parse' trebuie să accepte cel puțin un parametru (payload-ul frame-ului).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Nu a fost găsită o declarație validă a funcției 'parse'.

Format așteptat:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>Parametru Funcție Invalid</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Funcția 'parse' trebuie să aibă cel puțin un parametru.

Format așteptat:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>Semnătură Funcție Depreciată</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>Funcția 'parse' folosește formatul vechi cu doi parametri: parse(%1, %2)

Acest format nu mai este suportat. Vă rugăm să actualizați la noul format cu un singur parametru:
function parse(%1) { ... }

Parametrul separator nu mai este necesar.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Critical</source>
        <translation>Critic</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Warning</source>
        <translation>Avertisment</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="577"/>
        <source>Project file not found</source>
        <translation>Fișier proiect negăsit</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="578"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Fișierul proiect referențiat de această comandă rapidă nu a putut fi găsit:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="581"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Șterge această comandă rapidă?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="585"/>
        <source>Delete Shortcut</source>
        <translation>Șterge Comanda Rapidă</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="587"/>
        <source>Quit</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>Timp (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>Frec: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>Timp: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Nicio cheie API OpenAI configurată. Deschide Gestionare Chei pentru a adăuga una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="236"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Nicio cheie API Anthropic configurată. Deschide Gestionare Chei pentru a adăuga una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Nicio cheie API Gemini configurată. Deschide Gestionare Chei pentru a adăuga una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="351"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Nicio adresă URL de server de model local configurată. Deschide Gestionare Chei pentru a seta una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Nicio cheie API DeepSeek setată. Deschide Gestionare Chei pentru a adăuga una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Nicio cheie API Mistral configurată. Deschide Gestionare Chei pentru a adăuga una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Nicio cheie API OpenRouter configurată. Deschide Gestionare Chei pentru a adăuga una.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Nicio cheie API Groq configurată. Deschide Gestionare Chei pentru a adăuga una.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1149"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Analizorul de trame utilizează mai mult de %1% din timpul procesorului.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1151"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio elimină trame pentru a menține aplicația responsivă. Simplificați sau optimizați scriptul analizorului de trame pentru a reduce sarcina de lucru.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>Se aștepta %1, s-a primit '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Se așteaptă nume enum după 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>Se așteaptă nume oneof</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Eticheta câmpului '%1' în afara intervalului (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Se așteaptă tip cheie în map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Se așteaptă tip valoare în map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>Se așteaptă nume câmp map</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>Se așteaptă etichetă câmp map</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>Se așteaptă tip câmp, s-a primit '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>Se așteaptă nume câmp după tip</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>Se așteaptă număr etichetă câmp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Imbricare mesaje prea adâncă (limită %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>Se așteaptă nume mesaj</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Token neașteptat '%1' în domeniul fișierului</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Cuvânt cheie de nivel superior nesuportat '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Automat (Implicit Platformă)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Software (Fallback)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Rând %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">Cadru %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Decodor</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">Rânduri</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 rând(uri)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>Configurația parserului nativ nu este un obiect JSON valid.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Șablon parser nativ necunoscut: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Eroare Parser Încorporat</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Eroare Parser Nativ</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append nu este suportat pentru GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Deschiderea gzip atât pentru citire cât și pentru scriere nu este suportată</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>Puteți deschide un gzip fie pentru citire, fie pentru scriere. Care este cazul?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Nu s-a putut executa gzopen() pe fișier</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append nu este suportat pentru QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite nu este suportat pentru QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>Eroare API ZIP/UNZIP %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>Generează Raport PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Generează Raport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="69"/>
        <source>Solid</source>
        <translation>Solid</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="70"/>
        <source>Dashed</source>
        <translation>Punctat</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="71"/>
        <source>Dotted</source>
        <translation>Punctat</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="93"/>
        <source>A4 (210 × 297 mm)</source>
        <translation>A4 (210 × 297 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="94"/>
        <source>A3 (297 × 420 mm)</source>
        <translation>A3 (297 × 420 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="95"/>
        <source>A2 (420 × 594 mm)</source>
        <translation>A2 (420 × 594 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="96"/>
        <source>A1 (594 × 841 mm)</source>
        <translation>A1 (594 × 841 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="97"/>
        <source>A0 (841 × 1189 mm)</source>
        <translation>A0 (841 × 1189 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="98"/>
        <source>A5 (148 × 210 mm)</source>
        <translation>A5 (148 × 210 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="99"/>
        <source>A6 (105 × 148 mm)</source>
        <translation>A6 (105 × 148 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="100"/>
        <source>B4 (250 × 353 mm)</source>
        <translation>B4 (250 × 353 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="101"/>
        <source>B5 (176 × 250 mm)</source>
        <translation>B5 (176 × 250 mm)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="102"/>
        <source>Letter (8.5 × 11 in)</source>
        <translation>Letter (8,5 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8,5 × 14 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="104"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7,25 × 10,5 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="106"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="120"/>
        <source>%1 — Session Report</source>
        <translation>%1 — Raport Sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="122"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="560"/>
        <source>Session Report</source>
        <translation>Raport Sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="462"/>
        <source>Branding</source>
        <translation>Branding</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="468"/>
        <source>Page</source>
        <translation>Pagină</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="474"/>
        <source>Sections</source>
        <translation>Secțiuni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="480"/>
        <source>Datasets</source>
        <translation>Seturi de Date</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="528"/>
        <source>Identity</source>
        <translation>Identitate</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="542"/>
        <source>Company</source>
        <translation>Companie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="549"/>
        <source>e.g. Acme Test Systems</source>
        <translation>ex. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="553"/>
        <source>Document title</source>
        <translation>Titlu document</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="564"/>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="571"/>
        <source>Prepared by (optional)</source>
        <translation>Pregătit de (opțional)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="580"/>
        <source>Logo</source>
        <translation>Logo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="593"/>
        <source>File</source>
        <translation>Fișier</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="604"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG sau SVG (opțional)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="606"/>
        <source>Browse…</source>
        <translation>Răsfoiește…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="609"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="650"/>
        <source>Paper</source>
        <translation>Hârtie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="662"/>
        <source>Page size</source>
        <translation>Dimensiune pagină</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="790"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Adnotează valorile minime, maxime și medii pe grafice</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="822"/>
        <source>Include datasets</source>
        <translation>Include seturi de date</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="908"/>
        <source>Loading datasets...</source>
        <translation>Se încarcă seturi de date...</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="928"/>
        <source>Select at least one dataset to include.</source>
        <translation>Selectează cel puțin un set de date de inclus.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="945"/>
        <source>Export HTML</source>
        <translation>Exportă HTML</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="778"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Pagină de copertă (logo, titlu document, subtitlu test)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="781"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Informații test (proiect, marcaje temporale, clasificare și note)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="784"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Rezumat măsurători (min, max, medie, deviație std. per parametru)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="787"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Tendințe parametri (grafic serie temporală per parametru numeric)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="677"/>
        <source>Plot appearance</source>
        <translation>Aspect grafic</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="691"/>
        <source>Line width</source>
        <translation>Grosime linie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="723"/>
        <source>Line style</source>
        <translation>Stil linie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="763"/>
        <source>Include</source>
        <translation>Include</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="937"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="945"/>
        <source>Export PDF</source>
        <translation>Exportă PDF</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Generare Raport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>Se Lucrează…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Acest proces poate dura câteva secunde pentru sesiuni cu mulți parametri. Fereastra se închide automat când raportul este gata.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>Conexiune Pierdută</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>Dispozitiv Indisponibil</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>Conexiunea la dispozitiv a fost pierdută.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio nu a putut accesa dispozitivul.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Verificați cablul, alimentarea și că nicio altă aplicație nu a preluat controlul dispozitivului. Puteți încerca reconectarea, comutarea la un alt dispozitiv sau ieșirea.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Asigură-te că este conectat, pornit și nu este deja utilizat de altă aplicație. Poți reîncerca, alege un alt dispozitiv sau închide.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>Alegeți Alt Dispozitiv</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">Reconfigurează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>Încearcă din Nou</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>Reconectare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Alege dispozitivul corect, apoi apasă Conectare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>Interfață I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>Conectare</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="338"/>
        <source>Data Grids</source>
        <translation>Grile de Date</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="341"/>
        <source>Multiple Data Plots</source>
        <translation>Grafice Multiple de Date</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="344"/>
        <source>Accelerometers</source>
        <translation>Accelerometre</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="347"/>
        <source>Gyroscopes</source>
        <translation>Giroscoape</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="350"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="353"/>
        <source>FFT Plots</source>
        <translation>Grafice FFT</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="356"/>
        <source>LED Panels</source>
        <translation>Panouri LED</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="359"/>
        <source>Data Plots</source>
        <translation>Grafice de Date</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="362"/>
        <source>Bars</source>
        <translation>Bare</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="365"/>
        <source>Gauges</source>
        <translation>Indicatoare</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="368"/>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="371"/>
        <source>Clock</source>
        <translation>Ceas</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="374"/>
        <source>Stopwatch</source>
        <translation>Cronometru</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="377"/>
        <source>Compasses</source>
        <translation>Busole</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="380"/>
        <source>Meters</source>
        <translation>Contoare</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Termometre</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>3D Plots</source>
        <translation>Grafice 3D</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Web Views</source>
        <translation>Vizualizări Web</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="390"/>
        <source>Image Views</source>
        <translation>Vizualizări Imagine</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="393"/>
        <source>Output Panels</source>
        <translation>Panouri de Ieșire</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="396"/>
        <source>Notifications</source>
        <translation>Notificări</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="399"/>
        <source>Waterfalls</source>
        <translation>Cascade</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="402"/>
        <source>Painter Widgets</source>
        <translation>Widget-uri Painter</translation>
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
        <translation>Sistem</translation>
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
        <translation>Detalii Sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Selectează o sesiune pentru a vizualiza detalii.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Proiect:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Început:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Încheiat:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(în desfășurare)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Frame-uri:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Notițe</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Adaugă notițe sesiune…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Notițele sunt doar-citire pentru sesiunile finalizate.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Etichetă nouă…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Deblocați fișierul de sesiune pentru a șterge sesiuni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Etichete</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Adaugă</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Redare</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>Exportă CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Generează Raport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Sesiuni</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Căutare</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>Dată</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>Cadre</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>Etichete</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>Nu s-au găsit sesiuni.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Niciun fișier de sesiune deschis.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1026"/>
        <source>Select logo image</source>
        <translation>Selectează imaginea logo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1028"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Imagini (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Open Session File</source>
        <translation>Deschide Fișier de Sesiune</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="430"/>
        <source>Session files (*.db)</source>
        <translation>Fișiere de sesiune (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1225"/>
        <source>Cannot open session file</source>
        <translation>Nu se poate deschide fișierul de sesiune</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="655"/>
        <source>Delete session from %1?</source>
        <translation>Șterge sesiunea din %1?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="658"/>
        <source>Delete Session</source>
        <translation>Șterge Sesiunea</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1086"/>
        <source>No project data</source>
        <translation>Nicio dată de proiect</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Toate citirile și datele brute pentru această sesiune sunt eliminate permanent.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>Lock Session File</source>
        <translation>Blochează Fișierul de Sesiune</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="487"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Alegeți o parolă pentru a bloca fișierul de sesiune:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="496"/>
        <source>Confirm the password:</source>
        <translation>Confirmați parola:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="504"/>
        <source>Passwords do not match</source>
        <translation>Parolele nu se potrivesc</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="505"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>Cele două parole introduse nu se potrivesc. Fișierul de sesiune nu a fost blocat.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="541"/>
        <source>Unlock Session File</source>
        <translation>Deblochează Fișierul de Sesiune</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="542"/>
        <source>Enter the session file password:</source>
        <translation>Introduceți parola fișierului de sesiune:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="552"/>
        <source>Incorrect password</source>
        <translation>Parolă incorectă</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="553"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>Parola introdusă nu se potrivește cu cea stocată în fișierul de sesiune.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="645"/>
        <source>Session file locked</source>
        <translation>Fișier de sesiune blocat</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="646"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Deblocați fișierul de sesiune înainte de a șterge sesiunile înregistrate.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">Această sesiune nu conține un fișier de proiect încorporat — tabloul de bord revine la o dispunere de grafic rapid.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>Export Session to CSV</source>
        <translation>Exportă Sesiunea în CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>CSV files (*.csv)</source>
        <translation>Fișiere CSV (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">Export Finalizat</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">Sesiune exportată în:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">Se pregătește exportul…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Done</source>
        <translation>Finalizat</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="962"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Failed</source>
        <translation>Eșuat</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="968"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1008"/>
        <source>Report Failed</source>
        <translation>Raport Eșuat</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1009"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Nu s-a putut genera raportul. Verificați calea de ieșire și încercați din nou.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save PDF Report</source>
        <translation>Salvează Raport PDF</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="860"/>
        <source>Loading session data…</source>
        <translation>Se încarcă datele sesiunii…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save HTML Report</source>
        <translation>Salvează Raport HTML</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>PDF files (*.pdf)</source>
        <translation>Fișiere PDF (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>HTML files (*.html)</source>
        <translation>Fișiere HTML (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1087"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Acest fișier de sesiune nu conține un proiect încorporat.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1096"/>
        <source>Invalid project data</source>
        <translation>Date de proiect invalide</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1097"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>JSON-ul proiectului încorporat este malformat și nu poate fi restaurat.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Restore Project</source>
        <translation>Restaurează Proiectul</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Proiecte Serial Studio (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Cannot write file</source>
        <translation>Nu se poate scrie fișierul</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Check file permissions and try again.</source>
        <translation>Verifică permisiunile fișierului și încearcă din nou.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="77"/>
        <source>Empty file path</source>
        <translation>Cale de fișier goală</translation>
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
        <translation>Baza de date nu este deschisă</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="263"/>
        <source>Database not open or empty label</source>
        <translation>Baza de date nu este deschisă sau etichetă goală</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="331"/>
        <source>Invalid label</source>
        <translation>Etichetă invalidă</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="598"/>
        <source>Cancelled</source>
        <translation>Anulat</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="717"/>
        <source>Could not load session data</source>
        <translation>Nu s-au putut încărca datele sesiunii</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>Se asamblează raportul…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>Se scrie ieșirea…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>Raport de Sesiune</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>Proiect fără titlu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>Pregătit de</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>Generat pe %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>ID Test</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>Durată</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>Eșantioane</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>Parametri</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>Început</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>Încheiat</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>Proiect</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>Identificator Test</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>Ora de Început</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>Ora de Încheiere</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>Durată Totală</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>Eșantioane Achiziționate</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>Parametri Înregistrați</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>Clasificare</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>Note</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>Informații Test</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>Parametru</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>Unități</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>Minim</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>Maxim</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>Medie</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>Deviație Standard</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>Sumar Măsurători</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>faceți clic pe o coloană pentru a sorta</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 eșantioane pe parcursul a %2 secunde</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>Vizualizare Combinată A Parametrilor</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>faceți clic pe elementele legendei pentru a comuta semnalele</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>Tendințe Parametri</translation>
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
        <translation>Pagina %1 din %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>Se încarcă motorul de randare…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>Se randează graficele…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>Se Generează PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>Deschide Fișier de Sesiune</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>Fișiere de sesiune (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>Conexiune Dispozitiv Activă</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Pentru a utiliza această funcție, trebuie să vă deconectați de la dispozitiv. Doriți să continuați?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>Nu se poate deschide fișierul de sesiune</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>Eroare necunoscută</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>Nicio dată de proiect</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Această sesiune nu conține un fișier de proiect încorporat — tabloul de bord revine la o dispunere de grafic rapid.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>Verifică permisiunile fișierului și încearcă din nou.</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">Nu s-au găsit sesiuni</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">Acest fișier nu conține nicio sesiune înregistrată.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">Sesiunea nu are coloane</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">Sesiunea selectată nu are definițiile coloanelor.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">Sesiunea nu are înregistrări</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">Sesiunea selectată nu conține nicio trama.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Cale de fișier goală</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Acest fișier nu conține nicio sesiune înregistrată.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Anulat</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>Sesiunea selectată nu conține definițiile coloanelor.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>Sesiunea selectată nu conține niciun cadru.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Preferințe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="166"/>
        <source>Language</source>
        <translation>Limbaj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="182"/>
        <source>Theme</source>
        <translation>Temă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="289"/>
        <source>Workspace Folder</source>
        <translation>Dosar Spațiu de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="336"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>Activare Server API (Port 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="585"/>
        <source>Automatically Check for Updates</source>
        <translation>Verificare Automată a Actualizărilor</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Dashboard</source>
        <translation>Tablou de Bord</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="402"/>
        <source>Export…</source>
        <translation>Export…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="649"/>
        <source>Data Plotting</source>
        <translation>Reprezentare Grafică Date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Point Count</source>
        <translation>Număr de Puncte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="741"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Rată de Reîmprospătare UI (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="872"/>
        <source>Show Actions Panel</source>
        <translation>Afișează Panoul de Acțiuni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1051"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Afișează Întotdeauna Butoanele din Bara de Activități</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="85"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1163"/>
        <source>Console</source>
        <translation>Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="151"/>
        <source>Appearance</source>
        <translation>Aspect</translation>
    </message>
    <message>
        <source>Files &amp; Updates</source>
        <translation type="vanished">Fișiere și Actualizări</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="vanished">Avansat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>Allow External API Connections</source>
        <translation>Permite Conexiuni API Externe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="888"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Ascunde Automat Bara de Unelte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <source>Taskbar</source>
        <translation>Bară de Activități</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="470"/>
        <source>Rendering Backend</source>
        <translation>Backend de Randare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="370"/>
        <source>API Access Token</source>
        <translation>Token de Acces API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Startup</source>
        <translation>Pornire</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="202"/>
        <source>Window</source>
        <translation>Fereastră</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="221"/>
        <source>Custom Window Decorations</source>
        <translation>Decorațiuni Fereastră Personalizate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Window Shadow</source>
        <translation>Umbră Fereastră</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="264"/>
        <source>Window decoration changes apply after restarting %1.</source>
        <translation>Modificările decorațiunilor ferestrei se aplică după repornirea %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="273"/>
        <source>Files</source>
        <translation>Fișiere</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>API &amp; Plugins</source>
        <translation>API și Plugin-uri</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="452"/>
        <source>Graphics</source>
        <translation>Grafică</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="508"/>
        <source>System</source>
        <translation>Sistem</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="524"/>
        <source>Apply Performance Hints</source>
        <translation>Aplică Sugestii de Performanță</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="538"/>
        <source>Keep Display Awake</source>
        <translation>Menține Ecranul Activ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="558"/>
        <source>Performance hints raise process priority and opt out of OS power throttling. Changes take effect the next time Serial Studio starts.</source>
        <translation>Sugestiile de performanță cresc prioritatea procesului și dezactivează limitarea de putere a sistemului de operare. Modificările au efect la următoarea pornire a Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="569"/>
        <source>Updates &amp; News</source>
        <translation>Actualizări și Noutăți</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="599"/>
        <source>Show What's New on Startup</source>
        <translation>Afișează Ce Este Nou la Pornire</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="664"/>
        <source>Time Range</source>
        <translation>Interval de Timp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Small</source>
        <translation>Mic</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Large</source>
        <translation>Mare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Extra Large</source>
        <translation>Extra Mare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Custom</source>
        <translation>Personalizat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="857"/>
        <source>Layout</source>
        <translation>Aspect</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="904"/>
        <source>Auto-Layout Margin</source>
        <translation>Margine Aspect Automat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="929"/>
        <source>Auto-Layout Spacing</source>
        <translation>Spațiere Aspect Automat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="961"/>
        <source>Video Export</source>
        <translation>Export Video</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="979"/>
        <source>Save Videos by Default</source>
        <translation>Salvează Videoclipuri în Mod Implicit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1030"/>
        <source>Behavior</source>
        <translation>Comportament</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1066"/>
        <source>Show Search Field</source>
        <translation>Afișează Câmp de Căutare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1081"/>
        <source>Auto-hide Taskbar</source>
        <translation>Ascunde Automat Bara de Activități</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1099"/>
        <source>Hide Delay (ms)</source>
        <translation>Întârziere Ascundere (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1123"/>
        <source>Pinned Buttons</source>
        <translation>Butoane Fixate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1141"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Trageți un buton fixat pe bara de activități pentru a-l reordona.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1162"/>
        <source>Settings</source>
        <translation>Setări</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1165"/>
        <source>Clock</source>
        <translation>Ceas</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1166"/>
        <source>Stopwatch</source>
        <translation>Cronometru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1167"/>
        <source>Pause / Resume</source>
        <translation>Pauză / Reia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1168"/>
        <source>File Transmission</source>
        <translation>Transmisie Fișier</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1169"/>
        <source>AI Assistant</source>
        <translation>Asistent AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1298"/>
        <source>Display</source>
        <translation>Afișare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1313"/>
        <source>Display Mode</source>
        <translation>Mod Afișare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="778"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1326"/>
        <source>Font Family</source>
        <translation>Familie Font</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="92"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>Notifications</source>
        <translation>Notificări</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="400"/>
        <source>Export Protobuf File</source>
        <translation>Exportă Fișier Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="763"/>
        <source>Dashboard Font</source>
        <translation>Font Tablou de Bord</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="793"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1341"/>
        <source>Font Size</source>
        <translation>Dimensiune Font</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">Export Imagine</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">Salvează Imagini în Mod Implicit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1358"/>
        <source>Show Timestamps</source>
        <translation>Afișează Marcaje Temporale</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Data Transmission</source>
        <translation>Transmisie Date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1392"/>
        <source>Line Ending</source>
        <translation>Terminare de Linie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1405"/>
        <source>Input Mode</source>
        <translation>Mod Intrare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1418"/>
        <source>Text Encoding</source>
        <translation>Codificare Text</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1431"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1444"/>
        <source>Echo Sent Data</source>
        <translation>Afișează Datele Trimise</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1463"/>
        <source>Escape Codes</source>
        <translation>Coduri de Escape</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1478"/>
        <source>VT100 Emulation</source>
        <translation>Emulare VT100</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1497"/>
        <source>ANSI Colors</source>
        <translation>Culori ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1555"/>
        <source>Delivery</source>
        <translation>Livrare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1570"/>
        <source>System Notifications</source>
        <translation>Notificări de Sistem</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1591"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Afișează evenimentele de avertizare/critice ca notificări desktop ale sistemului de operare când Serial Studio nu este fereastra activă.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1601"/>
        <source>Application Logs</source>
        <translation>Jurnale Aplicație</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1616"/>
        <source>Route Warnings to Notifications</source>
        <translation>Redirecționează Avertizările către Notificări</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1637"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Dezactivat implicit — QT și QML emit avertismente frecvent și activarea acestei opțiuni poate acoperi alarmele reale. Mesajele critice sunt întotdeauna direcționate indiferent de această setare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1656"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1696"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1704"/>
        <source>Apply</source>
        <translation>Aplică</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Configurare Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>Server API Activ (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>Server API Pregătit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>Server API Dezactivat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Parsare Trame</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Doar Consolă (Fără Parsare)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Grafic Rapid (Valori Separate Prin Virgulă)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Analizează prin Fișier Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Schimbă Fișier Proiect (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Selectează Fișier Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Export Date</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>Foaie de Calcul CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Înregistrare Sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>Înregistrare MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Jurnal Consolă</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">Fișier CSV</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">Bază de Date Sesiuni</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">Fișier MDF4</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">Descărcare Consolă</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">Creează Fișier CSV</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">Creează Fișier MDF4</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">Creează Jurnal Sesiune</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">Exportă Date Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>Interfață I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Proiect Multi-dispozitiv</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Acest proiect transmite date de la %1 dispozitive independente.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Fiecare dispozitiv are propriile setări de conexiune. Configurează-le în Editorul de Proiect sub fila Surse.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Deschide Editorul de Proiect</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">Comandă Rapidă Nouă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>Alege o Pictogramă</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">Salvează Comanda Rapidă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>Implementare Nouă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="109"/>
        <source>Save Deployment</source>
        <translation>Salvează Implementarea</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>General</source>
        <translation>General</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Taskbar</source>
        <translation>Bară de Activități</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="164"/>
        <source>Logging</source>
        <translation>Înregistrare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="221"/>
        <source>Identity</source>
        <translation>Identitate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="277"/>
        <source>Click to choose an icon</source>
        <translation>Clic pentru a alege o pictogramă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="286"/>
        <source>Name:</source>
        <translation>Nume:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="295"/>
        <source>Deployment Name</source>
        <translation>Nume Implementare</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="386"/>
        <source>Theme</source>
        <translation>Temă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="396"/>
        <source>Same as Serial Studio</source>
        <translation>Aceeași ca Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="418"/>
        <source>Actions Panel</source>
        <translation>Panou de Acțiuni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="429"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="638"/>
        <source>File Transmission</source>
        <translation>Transmisie Fișier</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="445"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Dublu-clic pe această implementare deschide direct tabloul de bord live pentru acest proiect. Fără bară de instrumente sau panou de configurare, doar datele, iar Serial Studio se închide imediat ce dispozitivul se deconectează.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="491"/>
        <source>Visibility</source>
        <translation>Vizibilitate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="506"/>
        <source>Mode</source>
        <translation>Mod</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="515"/>
        <source>Always shown</source>
        <translation>Afișat Permanent</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="516"/>
        <source>Auto-hide</source>
        <translation>Ascundere Automată</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="517"/>
        <source>Hidden</source>
        <translation>Ascuns</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Ascunderea barei de sarcini elimină butoanele de minimizare/maximizare/închidere a ferestrei și forțează aranjarea automată, astfel încât tabloul de bord ocupă întotdeauna zona disponibilă.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="536"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>Bara de sarcini apare când utilizatorul mută cursorul aproape de marginea de jos.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="538"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>Bara de sarcini este vizibilă permanent în partea de jos a tabloului de bord.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="551"/>
        <source>Pinned Buttons</source>
        <translation>Butoane Fixate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="568"/>
        <source>Console</source>
        <translation>Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="582"/>
        <source>Notifications</source>
        <translation>Notificări</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="596"/>
        <source>Clock</source>
        <translation>Ceas</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="610"/>
        <source>Stopwatch</source>
        <translation>Cronometru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="624"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="749"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Înregistrările sunt salvate în folderul workspace Serial Studio</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">Nume Scurtătură</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="304"/>
        <source>Change Icon…</source>
        <translation>Schimbă Pictograma…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="321"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="339"/>
        <source>Project</source>
        <translation>Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="349"/>
        <source>Choose a project file to begin</source>
        <translation>Alege un fișier de proiect pentru a începe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="371"/>
        <source>Behavior</source>
        <translation>Comportament</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="408"/>
        <source>Fullscreen</source>
        <translation>Ecran Complet</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">Dublu-clic pe această comandă rapidă deschide direct tabloul de bord live pentru acest proiect. Fără bară de instrumente sau panou de configurare, doar datele, iar Serial Studio se închide imediat ce dispozitivul se deconectează.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">Încorporează Proiectul</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">Dublu-clic pe această scurtătură duce pe cineva direct la tabloul de bord live pentru acest proiect. Nu există bară de unelte sau panou de configurare, doar datele, iar Serial Studio se închide imediat ce dispozitivul se deconectează.

Activează Încorporează Proiectul pentru a integra proiectul în scurtătură, astfel încât să continue să funcționeze chiar dacă fișierul original este mutat sau șters.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="688"/>
        <source>Recorders</source>
        <translation>Înregistrări</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="703"/>
        <source>CSV File</source>
        <translation>Fișier CSV</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="713"/>
        <source>MDF4 File</source>
        <translation>Fișier MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="723"/>
        <source>Session Database</source>
        <translation>Bază de Date Sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="733"/>
        <source>Console Log</source>
        <translation>Jurnal Consolă</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">Înregistrările sunt salvate în locația implicită a fiecărui modul.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="778"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="787"/>
        <source>Save</source>
        <translation>Salvează</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>Anulează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>Refă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>Decupează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>Lipește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>Selectează Tot</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>Formatează Documentul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>Formatează Selecția</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>Validează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Verifică că scriptul se compilează corect</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">Resetează parametrii șablonului la valorile implicite</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>Resetează la scriptul de parsare implicit</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>Deschide</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>Importă un fișier script pentru parsarea datelor</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Deschide documentația de ajutor pentru parsarea datelor</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Limbaj:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Nativ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>Selectează Șablon…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>Anulează ultima editare de cod</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>Refă editarea anulată anterior</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>Decupează codul selectat în clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>Copiază codul selectat în clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>Lipește codul din clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>Ajutor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>Platformă:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>Încorporat</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">Șablon:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>Testează cu Date Exemplu</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Evaluează</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Duplică</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Creează o copie a acestei surse de date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Elimină această sursă de date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>Sursa de date primară nu poate fi eliminată</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>Player Sesiuni</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>Se încarcă sesiunea…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="278"/>
        <source>Workspaces</source>
        <translation>Spații de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="380"/>
        <source>No Workspaces Available</source>
        <translation>Niciun Spațiu de Lucru Disponibil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="415"/>
        <source>Actions</source>
        <translation>Acțiuni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="438"/>
        <source>No Actions Available</source>
        <translation>Nicio Acțiune Disponibilă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="468"/>
        <source>Plugins</source>
        <translation>Plugin-uri</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="366"/>
        <source>New Workspace…</source>
        <translation>Spațiu de Lucru Nou…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="506"/>
        <source>Manage Plugins…</source>
        <translation>Gestionare Plugin-uri…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="516"/>
        <source>No Plugins Installed</source>
        <translation>Niciun Plugin Instalat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="101"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="552"/>
        <source>Auto Layout</source>
        <translation>Aspect Automat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="109"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="564"/>
        <source>Full Screen</source>
        <translation>Ecran Complet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="115"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="576"/>
        <source>Add External Window</source>
        <translation>Adaugă Fereastră Externă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="135"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="765"/>
        <source>Clock</source>
        <translation>Ceas</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="143"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Stopwatch</source>
        <translation>Cronometru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="163"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="789"/>
        <source>Sessions</source>
        <translation>Sesiuni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="170"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>File Transmission</source>
        <translation>Transmisie Fișier</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="177"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="806"/>
        <source>AI Assistant</source>
        <translation>Asistent AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="352"/>
        <source>Show "%1"</source>
        <translation>Afișează "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Afișează Toate Spațiile de Lucru Ascunse</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="596"/>
        <source>Export</source>
        <translation>Export</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="627"/>
        <source>CSV File</source>
        <translation>Fișier CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="633"/>
        <source>MDF4 File</source>
        <translation>Fișier MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="639"/>
        <source>Console Transcript</source>
        <translation>Transcriere Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="648"/>
        <source>Session Database</source>
        <translation>Bază de Date Sesiune</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="662"/>
        <source>No Export Formats Available</source>
        <translation>Niciun Format de Export Disponibil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="692"/>
        <source>Tools</source>
        <translation>Instrumente</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="821"/>
        <source>No Tools Available</source>
        <translation>Niciun Instrument Disponibil</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="889"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Quit</source>
        <translation>Închide</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="936"/>
        <source>Edit…</source>
        <translation>Editează…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="121"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="748"/>
        <source>Console</source>
        <translation>Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="127"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Notifications</source>
        <translation>Notificări</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="151"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="780"/>
        <source>Preferences</source>
        <translation>Preferințe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="157"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="861"/>
        <source>Help Center</source>
        <translation>Centru de Ajutor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="947"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="948"/>
        <source>Hide</source>
        <translation>Ascunde</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="882"/>
        <source>Resume</source>
        <translation>Reia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="883"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Disconnect</source>
        <translation>Deconectează</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Oprește</translation>
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
        <translation>Tură</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Resetează</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Total</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Nicio tură înregistrată</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Apasă Tură în timp ce cronometrul rulează</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="160"/>
        <source>No Data Available</source>
        <translation>Nicio Dată Disponibilă</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Valori Set de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Căutare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Grup</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Set de Date</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Unități</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(virtual)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Copiază codul de acces %1 în clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Cod de acces set de date copiat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>Niciun set de date definit în acest proiect.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Niciun set de date nu corespunde căutării.</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>Parametru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>Valoare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Pictogramă Personalizată)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>Automat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>No</source>
        <translation>Nu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>Yes</source>
        <translation>Da</translation>
    </message>
</context>
<context>
    <name>TableFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="126"/>
        <source>Add Sub-folder</source>
        <translation>Adaugă Subfolder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="128"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Adaugă un sub-dosar în acest dosar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="136"/>
        <source>Add Shared Table</source>
        <translation>Adaugă Tabel Partajat</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="138"/>
        <source>Add a shared table inside this folder</source>
        <translation>Adaugă un tabel partajat în acest dosar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="148"/>
        <source>Rename</source>
        <translation>Redenumește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="150"/>
        <source>Rename folder</source>
        <translation>Redenumește dosarul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="158"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="160"/>
        <source>Delete folder</source>
        <translation>Șterge dosarul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="173"/>
        <source>Title</source>
        <translation>Titlu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="174"/>
        <source>Registers</source>
        <translation>Registre</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="264"/>
        <source>This folder is empty. Use the toolbar to add a table or sub-folder.</source>
        <translation>Acest dosar este gol. Folosește bara de instrumente pentru a adăuga un tabel sau sub-dosar.</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Meniu Start</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Meniu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Căutare…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Setări</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Consolă</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Notificări</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Ceas</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Cronometru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>Asistent AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Reia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="970"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Conectat la %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="971"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Neconectat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="995"/>
        <source>MQTT Publisher</source>
        <translation>Publisher MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1005"/>
        <source>Status:</source>
        <translation>Stare:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Connected</source>
        <translation>Conectat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1014"/>
        <source>Disconnected</source>
        <translation>Deconectat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1021"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1034"/>
        <source>Mode:</source>
        <translation>Mod:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1047"/>
        <source>Messages sent:</source>
        <translation>Mesaje trimise:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1061"/>
        <source>Open MQTT Settings</source>
        <translation>Deschide Setări MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Transmisie Fișier</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">Căutare widget-uri…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Elimină din Spațiul de Lucru</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Selectează Tot</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Trimite Date către Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Afișează Marcaj Temporal</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Ecou</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>Emulează VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Culori ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Afișare: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">Știați Că?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">Păstrați firmware-ul simplu trimițând date brute și lăsând Serial Studio să le parseze în JavaScript, Lua sau șabloane Built-In fără cod.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">Atribuiți fiecărui canal propria funcție pentru calibrare, filtrare sau conversie de unități. Transferați calculele către Serial Studio și mențineți firmware-ul compact.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">Aveți nevoie de o valoare pe care dispozitivul nu o trimite niciodată? Un dataset virtual calculează propriul canal, cum ar fi puterea din tensiune și curent, afișat și înregistrat ca date.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">Detectați anomalii ca un osciloscop de bancă. Graficele cu axă temporală au mod de baleiaj și declanșare, iar nivelul de declanșare poate fi ajustat direct pe grafic.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">Nu mai derulați pentru a găsi widget-ul potrivit. Grupați-le în propriile workspace-uri și navigați între ele din căutarea din bara de activități.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">Nu mai pierdeți niciodată o rulare de test. Înregistrați sesiuni într-o bază de date locală, apoi răsfoiți, etichetați și redați-le oricând aveți nevoie.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">Furnizați un raport finalizat echipei în câteva secunde. Exportați orice sesiune în HTML sau PDF, completă cu grafice și statistici min/max/medie.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">Închide bucla fără unelte suplimentare. Controalele de Ieșire îți permit să trimiți comenzi înapoi către dispozitiv direct din tablou de bord.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">Construiește o vizualizare pe care nimeni altcineva nu o are. Widget-ul Painter rulează propriul tău script pentru a desena grafică complet personalizată din datele primite.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">O unealtă pentru fiecare legătură. Serial Studio citește din UART, TCP/UDP, Bluetooth LE, Modbus, Magistrala CAN, audio, USB, HID, MQTT și Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">Omite dansul terminalului. Trimite și primește fișiere prin legătura ta serială cu protocoalele integrate XMODEM, YMODEM și ZMODEM.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">Ai deja o hartă de registre Modbus sau un fișier DBC? Generează automat un proiect gata de utilizat în loc să construiești unul manual.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">Descrie ce vrei și lasă Asistentul AI să construiască. Poate crea și edita proiecte pentru tine pe opt furnizori de modele.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">Sfat %1 din %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">Află Mai Multe</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">Afișează Sfaturi la Pornire</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">Anterior</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">Următorul</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Închide</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>În așteptarea aprobării</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Finalizat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Eroare</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Refuzat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Blocat</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>În Execuție</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Aprobă</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Respinge</translation>
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
        <translation>Copiază</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Copiază Tot</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Selectează Tot</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Rezultat</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Editor de Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Deschide Editorul de Proiect pentru a crea sau modifica aspectul JSON</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Deschide Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Deschide un proiect JSON existent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>Deschide CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>Redă un fișier CSV ca și cum ar fi date live de la senzori</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>Deschide MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Redă un fișier MDF4 ca și cum ar fi date live de la senzori (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Sesiuni</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Navighează, redă și exportă sesiuni înregistrate</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">Scurtături</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">Creează o scurtătură operator pentru proiectul curent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Extensii</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Navighează și instalează extensii</translation>
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
        <translation>Conversează cu un AI pentru a construi și edita proiectul</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">Configurează conexiunea MQTT (publicare sau abonare)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Construiește o aplicație operator pentru proiectul curent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Preferințe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Deschide setările și preferințele aplicației</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Selectează comunicarea prin port serial (UART)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Selectează dispozitivul de intrare audio (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Selectează comunicarea USB brută (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Rețea</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>Selectează comunicarea prin rețea TCP/UDP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Răsfoiește proiectele exemplu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Centru de Ajutor</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Răsfoiește documentația, întrebările frecvente și wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>Selectează comunicarea MODBUS (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Implementează</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">Creează o implementare operator pentru proiectul curent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>Selectează comunicarea prin dispozitiv HID (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Selectează comunicația Bluetooth Low Energy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>Magistrala CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>Selectează comunicația pe magistrala CAN (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Selectează comunicația prin conductă de proces (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>Despre</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Afișează informații despre aplicație și detalii licență</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Exemple</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>Wiki &amp; Chat AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Vizualizează documentația detaliată și pune întrebări pe DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Activează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Gestionează licența și activează Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Deconectează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Conectează</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Conectează sau deconectează de la dispozitivul configurat</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">Conectează sau deconectează de la dispozitiv sau broker MQTT</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Setări Declanșare</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Menține forma de undă staționară prin alinierea fiecărui baleiaj la un eveniment de declanșare.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Blochează un semnal repetitiv pe loc, ca butonul Auto de pe un osciloscop. Fiecare baleiaj începe în același punct al formei de undă, astfel încât rămâne fix în loc în loc să deruleze.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>Declanșare</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">Mod:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>Mod</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>Automat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>Normal</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Unic</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto rulează liber când nimic nu traversează nivelul.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal așteaptă o traversare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single capturează un baleiaj, apoi se oprește.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Slope:</source>
        <translation>Pantă:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="273"/>
        <source>Trigger on a downward crossing</source>
        <translation>Declanșare la o traversare descendentă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="319"/>
        <source>Timebase:</source>
        <translation>Bază de Timp:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="388"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Lasă baza de timp goală pentru a folosi intervalul de timp al graficului; micșorează-o pentru a mări un semnal rapid. Holdoff ignoră declanșările noi pentru un moment după fiecare.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Semnal:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Valoare de Traversat</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Flanc:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="254"/>
        <source>Rising</source>
        <translation>Crescător</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="258"/>
        <source>Trigger on an upward crossing</source>
        <translation>Declanșare la o traversare ascendentă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="269"/>
        <source>Falling</source>
        <translation>Descrescător</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Un nou baleiaj începe de fiecare dată când semnalul traversează nivelul în direcția aleasă. Auto rulează liber și când nu se găsește nicio traversare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="292"/>
        <source>Timing</source>
        <translation>Temporizare</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Bază de Timp (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="332"/>
        <source>Match time range</source>
        <translation>Potrivire Interval de Timp</translation>
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
        <translation type="vanished">Baza de timp stabilește cât timp afișează un baleiaj; lasă-l gol pentru a folosi intervalul de timp al graficului. Micșorează-l pentru a mări un semnal rapid. Holdoff ignoră declanșările noi pentru un moment după fiecare declanșare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="403"/>
        <source>Capture Next</source>
        <translation>Capturează Următorul</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="405"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Armează pentru încă o captură unică</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Nivel:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Nivel de declanșare</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Holdoff (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Timp holdoff</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Sursă:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Rearmează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="418"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>Port COM</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="79"/>
        <source>Baud Rate</source>
        <translation>Rată Baud</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="163"/>
        <source>Data Bits</source>
        <translation>Biți de Date</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="184"/>
        <source>Parity</source>
        <translation>Paritate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="205"/>
        <source>Stop Bits</source>
        <translation>Biți de Stop</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="226"/>
        <source>Flow Control</source>
        <translation>Control Flux</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="258"/>
        <source>Auto Reconnect</source>
        <translation>Reconectare Automată</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="276"/>
        <source>Send DTR Signal</source>
        <translation>Trimite Semnal DTR</translation>
    </message>
</context>
<context>
    <name>UI::AlarmMonitor</name>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="195"/>
        <source>Alarm</source>
        <translation>Alarmă</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>critical</source>
        <translation>critic</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>avertisment</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Valoarea %1%2 a intrat în banda %3 (%4–%5).</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="205"/>
        <source>Alarms</source>
        <translation>Alarme</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1717"/>
        <source>Console</source>
        <translation>Consolă</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1725"/>
        <source>Notifications</source>
        <translation>Notificări</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1733"/>
        <source>Clock</source>
        <translation>Ceas</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1740"/>
        <source>Stopwatch</source>
        <translation>Cronometru</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1786"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1801"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Rezervă)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1823"/>
        <source>LED Panel (%1)</source>
        <translation>Panou LED (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="161"/>
        <source>Invalid</source>
        <translation>Invalid</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1069"/>
        <source>Select Background Image</source>
        <translation>Selectează Imaginea de Fundal</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1071"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Imagini (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>Dispozitiv USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Mod de Transfer</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Flux Bulk</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Avansat (Bulk + Control)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Izocron</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Conectare la dispozitive USB folosind transferuri bulk, control sau izocrone. Potrivit pentru înregistratoare de date, dispozitive cu firmware personalizat și instrumente USB.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>Specificații USB (USB.org)</translation>
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
        <translation>Dimensiune Maximă Pachet</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Transferuri de Control Activate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>Trimiterea de cereri de control incorecte poate bloca sau deteriora hardware-ul conectat. Utilizați cu precauție.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Aflați despre transferurile de control USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>Dimensiunea pachetului trebuie să corespundă dimensiunii maxime de transfer raportată de endpoint. Valori tipice: 192 B (audio FS), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Doriți să descărcați actualizarea acum?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Doriți să descărcați actualizarea acum?&lt;br /&gt;Aceasta este o actualizare obligatorie, ieșirea acum va închide aplicația.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Jurnal de modificări:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>Versiunea %1 a %2 a fost lansată!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Nu sunt disponibile actualizări momentan</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Felicitări! Rulați ultima versiune a %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="172"/>
        <source>Add Register</source>
        <translation>Adaugă Registru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="175"/>
        <source>Add register</source>
        <translation>Adaugă registru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="182"/>
        <source>Insert Constant</source>
        <translation>Inserează Constantă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="185"/>
        <source>Insert constant</source>
        <translation>Inserează constantă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="192"/>
        <source>Import</source>
        <translation>Importă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="195"/>
        <source>Import registers from CSV</source>
        <translation>Importă registre din CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="202"/>
        <source>Export</source>
        <translation>Exportă</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="205"/>
        <source>Export registers to CSV</source>
        <translation>Exportă registre în CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="217"/>
        <source>Rename</source>
        <translation>Redenumește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="220"/>
        <source>Rename table</source>
        <translation>Redenumește tabel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="227"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="230"/>
        <source>Delete table</source>
        <translation>Șterge tabel</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="244"/>
        <source>Help</source>
        <translation>Ajutor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="249"/>
        <source>Open help documentation for shared memory</source>
        <translation>Deschide documentația de ajutor pentru memoria partajată</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="289"/>
        <source>Permissions</source>
        <translation>Permisiuni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="290"/>
        <source>Register Name</source>
        <translation>Nume Registru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="291"/>
        <source>Default Value</source>
        <translation>Valoare Implicită</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read-Only</source>
        <translation>Doar Citire</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read/Write</source>
        <translation>Citire/scriere</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="468"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Copiază codul de acces %1 în clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="501"/>
        <source>Delete register</source>
        <translation>Șterge registru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="518"/>
        <source>No registers.</source>
        <translation>Niciun registru.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="568"/>
        <source>Register access code copied</source>
        <translation>Cod de acces registru copiat</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="237"/>
        <source>Show Colorbar</source>
        <translation>Afișează Bara de Culori</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="250"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Afișează Axe și Grilă</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="263"/>
        <source>Show Crosshair</source>
        <translation>Afișează Cursor Încrucișat</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Pause</source>
        <translation>Pauză</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Resume</source>
        <translation>Reia</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">Șterge Istoric</translation>
    </message>
</context>
<context>
    <name>WebView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="70"/>
        <source>Web View Unavailable</source>
        <translation>Vizualizare Web Indisponibilă</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="81"/>
        <source>This build of Serial Studio was compiled without Qt WebEngine, so web pages cannot be displayed.</source>
        <translation>Această versiune de Serial Studio a fost compilată fără QT WebEngine, astfel încât paginile web nu pot fi afișate.</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Bun venit la %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio este un instrument puternic de vizualizare în timp real, creat pentru ingineri, studenți și creatori.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>Puteți începe o perioadă de probă complet funcțională de 14 zile, să o activați cu cheia de licență sau să descărcați și să compilați singur codul sursă GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Achiziționarea Pro susține direct autorul și ajută la finanțarea dezvoltării viitoare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Compilarea versiunii GPLv3 ajută la creșterea comunității și încurajează contribuțiile tehnice.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Vă rugăm așteptați…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>%1 zile rămase în perioada de probă.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">Folosești în prezent versiunea de probă completă a %1 Pro. Este valabilă 14 zile pentru uz personal, necomercial.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>Folosești în prezent versiunea de probă completă a %1 Pro. Este valabilă 14 zile pentru uz personal, necomercial.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Actualizează la un plan plătit pentru a continua să folosești Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Sau compilează codul sursă GPLv3 pentru a-l folosi gratuit.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Pentru a vedea planurile de abonament disponibile, apasă „Actualizează Acum" mai jos.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Nu mă mai înștiința despre perioada de probă.
Înțeleg că la expirare va trebui să cumpăr o licență sau să compilez versiunea GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>Perioada de probă %1 a expirat.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Perioada de probă s-a încheiat. Pentru a continua să folosești %1 cu toate funcționalitățile Pro, actualizează la un plan plătit.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>Dacă preferi, poți de asemenea compila versiunea open-source sub licența GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Mulțumim că ai încercat %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Actualizează Acum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Activează</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>Deschide în Mod Limitat</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>Continuă</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>Începe Perioada de Probă</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>Noutăți în %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua și Parsere Integrate</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>Parsează cadre în Lua 5.4 sau cu șabloane integrate fără cod, alături de JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>Asistent AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>Un asistent integrat în aplicație cu opt furnizori care poate construi și edita proiecte pentru tine.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Device Control Loops</source>
        <translation>Bucle de Control Dispozitiv</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Automate your device with an Arduino-style setup() and loop() routine that can read, write, and drive the dashboard.</source>
        <translation>Automatizează dispozitivul cu o rutină setup() și loop() în stil Arduino care poate citi, scrie și controla tabloul de bord.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>Baleiaj și Declanșare Osciloscop</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>Baleiaj în stil osciloscop cu un cursor de declanșare animat pe care îl poți trage pe grafic.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Output Controls</source>
        <translation>Controale de Ieșire</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>Transmite înapoi către dispozitivul tău cu widget-uri de control și un motor de asistență pentru protocoale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Dashboard Workspaces</source>
        <translation>Spații de Lucru Tablou de Bord</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>Grupează widget-urile în propriile tale tablouri de bord și găsește-le din căutarea din bara de activități.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Session Database &amp; Reports</source>
        <translation>Bază de Date Sesiune &amp; Rapoarte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>Înregistrează sesiuni în SQLITE, redă-le și exportă rapoarte HTML sau PDF.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>Operator Deployments</source>
        <translation>Implementări pentru Operator</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>Livrează o versiune blocată, cu scop unic, operatorilor, cu un mod doar de rulare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>New Dashboard Widgets</source>
        <translation>Widget-uri Noi pentru Tabloul de Bord</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>Fețe de Indicator și Contor cu afișaj live, plus Ceas, Cronometru și Waterfall.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Dataset Transforms</source>
        <translation>Transformări de Conjunturi de Date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>Calibrează, filtrează și convertește fiecare canal cu o funcție per set de date sau adaugă seturi de date virtuale care calculează noi canale.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>Desenează grafice complet personalizate din datele primite folosind propriul tău script de desenare.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Data Tables</source>
        <translation>Tabele de Date</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>Tabele de tip registru, în timp real, cu seturi de date virtuale și celule editabile.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Image Widget</source>
        <translation>Widget Imagine</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>Decodifică și afișează cadre de imagine transmise direct de pe dispozitivul tău.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Notifications &amp; Alarms</source>
        <translation>Notificări și Alarme</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>Alarme pe seturi de date multi-bandă cu niveluri de severitate, direcționate către Centrul de Notificări.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>Project Lock</source>
        <translation>Blocare Proiect</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>Blochează un proiect pentru a separa utilizarea operatorului de editare, cu un cod de acces.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="193"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf și Transfer de Fișiere</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="194"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>Intrare și publicare MQTT, import Protobuf și transferuri XMODEM / YMODEM / ZMODEM.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="240"/>
        <source>Welcome to %1!</source>
        <translation>Bine ai venit în %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="250"/>
        <source>Here's what's new in version %1.</source>
        <translation>Iată ce este nou în versiunea %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="423"/>
        <source>Show on Startup</source>
        <translation>Afișează la pornire</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="430"/>
        <source>Close</source>
        <translation>Închide</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Elimină din Spațiul de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="345"/>
        <source>Device Disconnected</source>
        <translation>Dispozitiv Deconectat</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <source>Alarm</source>
        <translation type="vanished">Alarmă</translation>
    </message>
    <message>
        <source>critical</source>
        <translation type="vanished">critic</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">avertisment</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">Valoarea %1%2 a intrat în banda %3 (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Valoarea %1%2 a atins alarma superioară %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Valoarea %1%2 a atins alarma inferioară %3%2</translation>
    </message>
    <message>
        <source>Alarms</source>
        <translation type="vanished">Alarme</translation>
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
        <translation>SV</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="184"/>
        <source>W</source>
        <translation>V</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="187"/>
        <source>NW</source>
        <translation>NV</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>Titlu</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>Valoare</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">Se așteaptă date…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>Imagini Satelit</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Imagini Satelit cu Etichete</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>Hartă Stradală</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>Hartă Topografică</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>Teren</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>Pânză Gri Deschis</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>Pânză Gri Închis</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>Straturile de hartă suplimentare sunt disponibile doar pentru utilizatorii Pro.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Nu putem oferi acces nelimitat deoarece cheia API ArcGIS generează costuri reale.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Time (s)</source>
        <translation>Timp (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Samples</source>
        <translation>Eșantioane</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Scriptul de transmisie a expirat după %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>Sarcina utilă depășește dimensiunea maximă</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="90"/>
        <source>Time (s)</source>
        <translation>Timp (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="116"/>
        <source>Samples</source>
        <translation>Eșantioane</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="950"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Interval Grilă: %1 unitate/unități</translation>
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
        <translation>Tonuri de Gri</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>Necunoscut</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Editare Spațiu de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Spațiu de Lucru Nou</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Nume:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>Spațiul Meu de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Selectează widget-urile de inclus:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Filtrează widget-uri…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Anulează</translation>
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
        <translation>Dosar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="127"/>
        <source>Add Sub-folder</source>
        <translation>Adaugă Subdosar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="130"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Adaugă un subdosar în interiorul acestui dosar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="138"/>
        <source>Add Workspace</source>
        <translation>Adaugă Spațiu de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="141"/>
        <source>Add a workspace inside this folder</source>
        <translation>Adaugă un spațiu de lucru în interiorul acestui dosar</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="151"/>
        <source>Rename</source>
        <translation>Redenumește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="153"/>
        <source>Rename folder</source>
        <translation>Redenumește dosarul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="162"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="164"/>
        <source>Delete folder</source>
        <translation>Șterge dosarul</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="178"/>
        <source>Title</source>
        <translation>Titlu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="179"/>
        <source>Contents</source>
        <translation>Conținut</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="269"/>
        <source>This folder is empty. Use the toolbar to add a workspace or sub-folder.</source>
        <translation>Acest folder este gol. Folosește bara de instrumente pentru a adăuga un spațiu de lucru sau un sub-folder.</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="43"/>
        <source>Workspace</source>
        <translation>Spațiu de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Add Widget</source>
        <translation>Adaugă Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="146"/>
        <source>Add widget to workspace</source>
        <translation>Adaugă widget în spațiul de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="157"/>
        <source>Move Up</source>
        <translation>Mută Sus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="159"/>
        <source>Move workspace up</source>
        <translation>Mută spațiul de lucru în sus</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="169"/>
        <source>Move Down</source>
        <translation>Mută Jos</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="171"/>
        <source>Move workspace down</source>
        <translation>Mută spațiul de lucru în jos</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="181"/>
        <source>Change Icon</source>
        <translation>Schimbă Pictograma</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Change workspace icon</source>
        <translation>Schimbă pictograma spațiului de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="198"/>
        <source>Rename</source>
        <translation>Redenumește</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="200"/>
        <source>Rename workspace</source>
        <translation>Redenumește spațiul de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="209"/>
        <source>Delete</source>
        <translation>Șterge</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="211"/>
        <source>Delete workspace</source>
        <translation>Șterge spațiul de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="233"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="239"/>
        <source>Group</source>
        <translation>Grup</translation>
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
        <translation>Tip</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="285"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="323"/>
        <source>(unknown)</source>
        <translation>(necunoscut)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="303"/>
        <source>(group widget)</source>
        <translation>(widget de grup)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="353"/>
        <source>Remove widget from workspace</source>
        <translation>Elimină widget-ul din spațiul de lucru</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">Elimină din spațiul de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="373"/>
        <source>No widgets in this workspace.</source>
        <translation>Niciun widget în acest spațiu de lucru.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Spații de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="124"/>
        <source>Add Folder</source>
        <translation>Adaugă Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="126"/>
        <source>Add a top-level folder</source>
        <translation>Adaugă un folder de nivel superior</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="151"/>
        <source>Customize</source>
        <translation>Personalizează</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="153"/>
        <source>Edit workspaces manually</source>
        <translation>Editează spațiile de lucru manual</translation>
    </message>
    <message>
        <source>Move Up</source>
        <translation type="vanished">Mută în Sus</translation>
    </message>
    <message>
        <source>Move the selected workspace up</source>
        <translation type="vanished">Mută spațiul de lucru selectat în sus</translation>
    </message>
    <message>
        <source>Move Down</source>
        <translation type="vanished">Mută în Jos</translation>
    </message>
    <message>
        <source>Move the selected workspace down</source>
        <translation type="vanished">Mută spațiul de lucru selectat în jos</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="135"/>
        <source>Add Workspace</source>
        <translation>Adaugă Spațiu de Lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="137"/>
        <source>Add workspace</source>
        <translation>Adaugă spațiu de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="167"/>
        <source>Cleanup</source>
        <translation>Curățare</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="170"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>Elimină %1 referință(e) widget al cărei grup sau set de date țintă nu mai există</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>No stale widget references in any workspace</source>
        <translation>Nicio referință widget învechită în niciun spațiu de lucru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>Title</source>
        <translation>Titlu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="189"/>
        <source>Contents</source>
        <translation>Conținut</translation>
    </message>
    <message>
        <source>Widgets</source>
        <translation type="vanished">Widget-uri</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="282"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>Niciun spațiu de lucru. Adaugă unul cu bara de instrumente de mai sus sau resetează la aspectul automat.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Proiectul nu are grupuri eligibile -- adaugă un grup cu widget-uri pentru a popula spațiile de lucru.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="290"/>
        <source>Reset to Auto Layout</source>
        <translation>Resetează la Aspect Automat</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">Niciun spațiu de lucru.</translation>
    </message>
</context>
</TS>