<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="pl_PL" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Błąd Anthropic</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>Błąd parsowania strumienia: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>Nieprawidłowy klucz API (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>Ograniczenie częstotliwości: %1</translation>
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
        <translation>Zezwolić na Sterowanie Urządzeniem przez AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="219"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>Pozwala to asystentowi AI na konfigurowanie urządzeń, otwieranie i zamykanie połączeń oraz wysyłanie danych do sprzętu.

Każda akcja urządzenia nadal wymaga wyraźnej zgody dla każdego wywołania w czacie, nawet gdy automatyczne zatwierdzanie jest włączone. Włącz to tylko wtedy, gdy ufasz skonfigurowanemu dostawcy AI w zakresie dostępu do sprzętu.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="417"/>
        <source>Switch AI provider?</source>
        <translation>Przełączyć dostawcę AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="418"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Przełączenie na innego dostawcę wyczyści bieżącą konwersację. Kontynuować?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="421"/>
        <source>Assistant</source>
        <translation>Asystent</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="458"/>
        <source>AI Assistant is not available in this build</source>
        <translation>Asystent AI nie jest dostępny w tej kompilacji</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">Asystent AI wymaga licencji Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="463"/>
        <source>Set an API key first</source>
        <translation>Najpierw ustaw klucz API</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">Asystent AI wymaga licencji Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="169"/>
        <source>AI Assistant is not available in this build</source>
        <translation>Asystent AI nie jest dostępny w tej kompilacji</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="175"/>
        <source>AI subsystem not initialized</source>
        <translation>Podsystem AI nie został zainicjalizowany</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="181"/>
        <source>Already busy with a previous request</source>
        <translation>Zajęty poprzednim żądaniem</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="495"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Osiągnięto limit wywołań narzędzi dla tej tury; żadne kolejne narzędzia nie zostaną uruchomione.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1093"/>
        <source>Waiting for %1 to respond. Loading the model and processing the prompt can take a while on local hardware...</source>
        <translation>Oczekiwanie na odpowiedź %1. Ładowanie modelu i przetwarzanie zapytania może zająć chwilę na lokalnym sprzęcie...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1952"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>Osiągnięto limit wywołań narzędzi dla tej tury. Nie żądaj więcej narzędzi. Podsumuj dotychczasowe wyniki, a jeśli zadanie jest niekompletne, wskaż pozostałe kroki, aby użytkownik mógł polecić kontynuację.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Przekroczono limit wywołań narzędzi</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="940"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Model zwrócił pustą odpowiedź. Spróbuj przeformułować zapytanie, przełączyć na inny model lub sprawdzić, czy żądanie jest dozwolone przez filtry bezpieczeństwa dostawcy.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1097"/>
        <source>Sending request to %1...</source>
        <translation>Wysyłanie żądania do %1…</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1110"/>
        <source>Provider returned no reply</source>
        <translation>Dostawca nie zwrócił odpowiedzi</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="146"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Zapytanie zablokowane przez filtr bezpieczeństwa Gemini: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="200"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini zatrzymał się bez wygenerowania odpowiedzi: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="262"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="265"/>
        <source>Invalid API key (%1)</source>
        <translation>Nieprawidłowy klucz API (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="267"/>
        <source>Rate limited: %1</source>
        <translation>Ograniczenie częstotliwości: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="269"/>
        <source>Invalid API key</source>
        <translation>Nieprawidłowy klucz API</translation>
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
        <translation>Nieprawidłowy klucz API (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="431"/>
        <source>Rate limited: %1</source>
        <translation>Ograniczenie częstotliwości: %1</translation>
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
        <translation>Eksportuj Plik Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>Nie można uruchomić serwera GRPC</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>Nie udało się powiązać z %1</translation>
    </message>
</context>
<context>
    <name>API::ProcessLauncher</name>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="82"/>
        <source>No program specified</source>
        <translation>Nie określono programu</translation>
    </message>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="88"/>
        <source>Program "%1" not found in PATH</source>
        <translation>Program „%1" nie został znaleziony w PATH</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>Nie można uruchomić serwera TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>Zezwolić na Zewnętrzne Połączenia API?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Udostępnienie serwera API hostom zewnętrznym umożliwia innym urządzeniom w sieci połączenie z Serial Studio na porcie 7777.

Włączaj tę opcję tylko w zaufanych sieciach. Niezaufani klienci mogą odczytywać dane na żywo lub wysyłać polecenia do urządzenia.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>Nie można ponownie uruchomić serwera TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>Zezwolić na sterowanie urządzeniem przez API?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Program korzystający z lokalnego API Serial Studio żąda wysłania danych do podłączonego urządzenia. Zezwolić klientom API na zapis do urządzenia?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>API server</source>
        <translation>Serwer API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>Invalid pending connection</source>
        <translation>Nieprawidłowe oczekujące połączenie</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>O Programie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>Wersja %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Copyright © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>Wszelkie Prawa Zastrzeżone</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 jest wolnym oprogramowaniem: można je rozpowszechniać i/lub modyfikować na warunkach Powszechnej Licencji Publicznej GNU opublikowanej przez Free Software Foundation; w wersji 3 tej Licencji lub (według uznania) dowolnej późniejszej wersji.

%1 jest rozpowszechniane w nadziei, że będzie użyteczne, ale BEZ JAKIEJKOLWIEK GWARANCJI; nawet domyślnej gwarancji PRZYDATNOŚCI HANDLOWEJ lub PRZYDATNOŚCI DO OKREŚLONYCH CELÓW. Więcej szczegółów w Powszechnej Licencji Publicznej GNU.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Ta konfiguracja jest licencjonowana do użytku komercyjnego i własnościowego. Może być używana w aplikacjach zamkniętych i komercyjnych, zgodnie z warunkami licencji komercyjnej.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Ta konfiguracja jest przeznaczona wyłącznie do celów osobistych i ewaluacyjnych. Użytek komercyjny jest zabroniony, chyba że aktywowano ważną licencję komercyjną.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>To oprogramowanie jest dostarczane „takim, jakie jest", bez jakiejkolwiek gwarancji, wyraźnej lub dorozumianej, w tym między innymi gwarancji przydatności handlowej lub przydatności do określonego celu. Autor nie ponosi odpowiedzialności za jakiekolwiek szkody wynikające z użytkowania tego oprogramowania.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Zarządzaj Licencją</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Wspomóż</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Sprawdź Aktualizacje</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>Co Nowego</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">Wskazówki i Triki</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>Umowa Licencyjna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>Zgłoś Błąd</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>Podziękowania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>Test Wydajności</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>Witryna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="186"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="187"/>
        <source>Settings</source>
        <translation>Ustawienia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="245"/>
        <source>G-FORCE</source>
        <translation>PRZYSPIESZENIE</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="283"/>
        <source>PITCH ↕</source>
        <translation>POCHYLENIE ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="320"/>
        <source>ROLL ↔</source>
        <translation>PRZECHYLENIE ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>Konfiguracja Akcelerometru</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Skonfiguruj zakres wyświetlania akcelerometru i jednostki wejściowe.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Zakres Wyświetlania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>Maks. G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Wprowadź maksymalną wartość G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Konfiguracja Wejścia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Wejście już w G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>Podziękowania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>O QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Zmień Ikonę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Zmień ikonę używaną dla tej akcji</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Duplikuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Duplikuj tę akcję ze wszystkimi jej ustawieniami</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Usuń tę akcję z projektu</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Dodaj Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Dostępne Widgety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Kliknij wiersz, aby dodać go do obszaru roboczego.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Grupa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(cała grupa)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Już w obszarze roboczym</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Dodaj do obszaru roboczego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Brak dostępnych widżetów.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Brak pasujących widżetów.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>Widżety: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 z %2 widżetów</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Pasma Alarmów</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <source>Info</source>
        <translation>Informacje</translation>
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
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>Krytyczny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>Obrotomierz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>Bezczynność</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>Praca</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>Uwaga</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>Czerwona Strefa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>Prędkościomierz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>Rejs</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>Szybko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>Maksymalna Prędkość</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>Temperatura Silnika</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>Zimno</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>Normalny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>Ciepło</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>Przegrzanie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>Ciśnienie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>Próżnia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>Wysoki</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>Wybuch</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>Napięcie Baterii</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>Niski</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>Nominalny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>Poziom Paliwa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>Pusty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>Rezerwa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>Siła Sygnału</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>Brak Sygnału</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>Słaby</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>Dobry</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>Obciążenie CPU / Systemu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>Zajęty</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>Przeciążenie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Ostrzeżenie / Krytyczny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>Wskaźnik (Wł. / Wył.)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>Wł.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>Wskaźnik Usterki</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>Usterka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>Wybierz Kolor Pasma</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>Presety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>Preset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>Wybierz szablon…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>Migaj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>Przywróć domyślny kolor poziomu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Kliknij, aby wybrać kolor. Kliknij prawym przyciskiem, aby przywrócić domyślny kolor poziomu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>Kliknij, aby wybrać niestandardowy kolor.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>Migaj diodą LED, gdy wartość znajduje się w tym paśmie.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Brak zdefiniowanych zakresów. Wybierz szablon powyżej lub dodaj zakres, aby rozpocząć.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>Zastosuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>Zastosuj zmiany w zbiorze danych.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Zastosuj Preset</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Zastąp bieżące pasma wybranym presetem, przeskalowanym do zakresu tego zbioru danych.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>Zakres</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>Pasma</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>Dodaj Pasmo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Dodaj nowe pasmo kontynuujące od ostatniego.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="437"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="443"/>
        <source>Max</source>
        <translation>Maks.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="449"/>
        <source>Severity</source>
        <translation>Ważność</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>Kolor</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>Etykieta</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Wybierz niestandardowy kolor.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">auto</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(opcjonalnie)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>Przenieś w górę.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>Przenieś w dół.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>Usuń ten zakres.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Brak zdefiniowanych zakresów. Zastosuj szablon lub dodaj zakres, aby rozpocząć.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>Podgląd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>Odrzuć zmiany.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Zapisz</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Zapisz zmiany w zbiorze danych.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>Asystent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="79"/>
        <source>New chat</source>
        <translation>Nowa rozmowa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>Toggle chat list</source>
        <translation>Przełącz listę rozmów</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="238"/>
        <source>Settings</source>
        <translation>Ustawienia</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="274"/>
        <source>Manage API keys…</source>
        <translation>Zarządzaj kluczami API…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="341"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>Eksport CSV vs MDF4 — jaka jest różnica?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="344"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Wykres, Słupek i Wskaźnik — kiedy używać każdego?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="416"/>
        <source>How can I help with your project?</source>
        <translation>Jak mogę pomóc z Twoim projektem?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="417"/>
        <source>Set up your API key to get started</source>
        <translation>Skonfiguruj klucz API, aby rozpocząć</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="429"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Opisz, co chcesz zbudować, a skonfiguruję dla Ciebie źródła, grupy, zbiory danych, parsery ramek i transformacje.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>Aby rozpocząć rozmowę, wklej klucz API dla wybranego dostawcy. Klucze są szyfrowane na tym komputerze i nigdy nie opuszczają Twojego komputera, z wyjątkiem komunikacji z wybranym dostawcą.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="453"/>
        <source>Open API Key Setup</source>
        <translation>Otwórz Konfigurację Klucza API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="463"/>
        <source>Get a key from %1</source>
        <translation>Uzyskaj klucz z %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="337"/>
        <source>List the sources in this project</source>
        <translation>Wyświetl źródła w tym projekcie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="334"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Pomóż mi odkryć funkcje Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="335"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Co ta aplikacja może zrobić dla mojej telemetrii?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="336"/>
        <source>Walk me through what this project already contains</source>
        <translation>Przeprowadź mnie przez to, co ten projekt już zawiera</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="340"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Czym jest baza danych sesji i dlaczego miałbym jej używać?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">Eksport CSV vs MDF4 — jaka jest różnica?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="342"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Czym jest parser ramek i kiedy go potrzebuję?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="343"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>Kiedy powinienem użyć Lua vs JavaScript dla parsera?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Wykres, Słupek i Wskaźnik — kiedy używać każdego?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="345"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Jaka jest różnica między transformacją a parserem ramek?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="348"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Dodaj źródło UART dla Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="349"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Skonfiguruj projekt IMU od podstaw</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="350"/>
        <source>Configure an MQTT subscriber</source>
        <translation>Skonfiguruj subskrybenta MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="351"/>
        <source>Add a CAN bus source</source>
        <translation>Dodaj źródło magistrali CAN</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="352"/>
        <source>Set up a Modbus poller</source>
        <translation>Skonfiguruj odpytywanie Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="353"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Dodaj źródło sieciowe (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="354"/>
        <source>Write a CSV frame parser for me</source>
        <translation>Napisz dla mnie parser ramek CSV</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="355"/>
        <source>Help me parse a JSON frame</source>
        <translation>Pomóż mi sparsować ramkę JSON</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="356"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>Dodaj transformację wygładzania EMA do zestawu danych</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="357"/>
        <source>Decode hexadecimal frames</source>
        <translation>Dekoduj ramki szesnastkowe</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="358"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Skalibruj czujnik za pomocą transformacji liniowej</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="361"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Zaproponuj widżety pulpitu dla moich danych</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="362"/>
        <source>Build an executive overview workspace</source>
        <translation>Zbuduj przestrzeń roboczą przeglądu wykonawczego</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="363"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Dodaj widget painter dla niestandardowej wizualizacji</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="364"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Pokaż Wykres, FFT i Waterfall dla Jednego Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="365"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Pogrupuj Moje Zestawy Danych w Użyteczne Obszary Robocze</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="644"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>Upuść pliki lub foldery, aby asystent mógł je odczytać</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="689"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>Dodano folder "%1" - dostępny w tej sesji</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="690"/>
        <source>Added "%1" - readable this session</source>
        <translation>Dodano "%1" - dostępny w tej sesji</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="774"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Zapytaj Serial Studio o cokolwiek…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Clear conversation</source>
        <translation>Wyczyść konwersację</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="838"/>
        <source>Stop generating</source>
        <translation>Zatrzymaj generowanie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="839"/>
        <source>Send message (Enter)</source>
        <translation>Wyślij wiadomość (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="159"/>
        <source>Provider</source>
        <translation>Dostawca</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="192"/>
        <source>Model selection</source>
        <translation>Wybór modelu</translation>
    </message>
    <message>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation type="vanished">Wykonuj akcje edycji bez pytania za każdym razem. Zablokowane akcje pozostają zablokowane.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Auto-approve edits</source>
        <translation>Automatycznie zatwierdzaj edycje</translation>
    </message>
    <message>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation type="vanished">Pozwól AI konfigurować urządzenia, łączyć/rozłączać i wysyłać dane. Każda akcja nadal wymaga Twojej zgody.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="259"/>
        <source>Allow device control</source>
        <translation>Zezwól na sterowanie urządzeniem</translation>
    </message>
    <message>
        <source>Manage API keys</source>
        <translation type="vanished">Zarządzaj kluczami API</translation>
    </message>
    <message>
        <source>Working</source>
        <translation type="vanished">Przetwarzanie</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation type="vanished">Gotowy</translation>
    </message>
    <message>
        <source>  •  cache %1k tok</source>
        <translation type="vanished">•  pamięć podręczna %1k tok</translation>
    </message>
    <message>
        <source>  •  cache write %1k tok</source>
        <translation type="vanished">zapis cache %1k tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Nie Wykryto Mikrofonu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Podłącz mikrofon lub sprawdź ustawienia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Urządzenie Wejściowe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Częstotliwość Próbkowania</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Format Próbki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Kanały</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Urządzenie Wyjściowe</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">Okno Dialogowe</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">Podaj nazwę użytkownika i hasło dla lokalizacji pobierania.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">&amp;Nazwa użytkownika:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Hasło:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="65"/>
        <source>Axis Range Configuration</source>
        <translation>Konfiguracja Zakresu Osi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="179"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Skonfiguruj widoczny zakres dla osi wykresu. Wartości aktualizują się w czasie rzeczywistym podczas wpisywania.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="187"/>
        <source>X Axis</source>
        <translation>Oś X</translation>
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
        <translation>Wprowadź wartość min</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="233"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="304"/>
        <source>Maximum:</source>
        <translation>Maksimum:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="245"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="316"/>
        <source>Enter max value</source>
        <translation>Wprowadź wartość maks</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="260"/>
        <source>Y Axis</source>
        <translation>Oś Y</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="335"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="345"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Odzyskaj Kopię Zapasową</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Bez Tytułu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Projekt Wczytany</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Automatyczny Zapis</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Przed Przywróceniem</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Przed Usunięciem Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Przed Usunięciem Grupy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Przed Nowym Projektem</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Przed Otwarciem Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Przed Wczytaniem JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Przed Zastosowaniem Szablonu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Przed Usunięciem Akcji</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Przed Usunięciem Widżetu Wyjściowego</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Przed Przeniesieniem Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Przed Przeniesieniem Grupy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Przed Usunięciem Obszaru Roboczego</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Przed Wyczyszczeniem Wszystkich Obszarów Roboczych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Przed Usunięciem Widgetu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Przed Zmianą Kolejności Obszarów Roboczych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Przed Operacją Wsadową</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Przed Dodaniem Kafelka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (i %2 więcej)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>Kod parsera ramek również się różni i zostanie zastąpiony.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Tytuł zmienia się z "%1" na "%2". Struktura grupy bez zmian.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Te same grupy i zbiory danych, ale kod parsera ramek się różni — przywrócenie go zastąpi.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Te same grupy i zestawy danych co w bieżącym projekcie. Przywrócenie może nadal cofnąć edycje na poziomie pól.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Przywrócenie usuwa %1 i przywraca %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Przywrócenie usuwa %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Przywrócenie przywraca %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Wybierz kopię zapasową do przywrócenia. Bieżący projekt jest najpierw zapisywany automatycznie, więc przywrócenie jest odwracalne.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Brak jeszcze kopii zapasowych dla tego projektu. Edytuj lub zapisz projekt, aby rozpocząć rotacyjne tworzenie kopii zapasowych.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Otwórz Folder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Przywróć</translation>
    </message>
</context>
<context>
    <name>Benchmark</name>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="32"/>
        <source>Benchmark</source>
        <translation>Test Wydajności</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="79"/>
        <source>%1 frames/s</source>
        <translation>%1 ramek/s</translation>
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
        <translation>Zaliczony</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>Niezaliczony</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>Test Wydajności Ścieżki Krytycznej</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Mierzy, jak szybko ten komputer może wyodrębniać, analizować i wizualizować ramki przez potok danych Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>Interfejs zawiesi się podczas wykonywania testu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Każda faza działa z pełną mocą w głównym wątku, więc okno przestaje odpowiadać do momentu zakończenia. Bieżący projekt zostanie automatycznie przeładowany po zakończeniu testu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>Ramki na fazę:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>Minimalny czas trwania:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>Etapy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>Parsery</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>Eksport danych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>Pulpit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>Dane</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>Tylko wartości numeryczne</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>Mieszany (numeryczny + tekst)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>Wybierz co najmniej jeden etap i jeden typ danych, aby uruchomić test wydajności.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>Uruchomione %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>Przygotowanie...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>Potok</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>Przepustowość</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>Czas</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>Wynik</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>Uruchom test, aby zobaczyć wyniki</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>Zaliczenie/niezaliczenie dotyczy wyłącznie etapów potoku danych i parsera (potok danych i parser wbudowany numeryczny: 1024 K ramek/s; wbudowany mieszany: 512 K; Lua numeryczny: 256 K; JavaScript numeryczny i Lua mieszany: 128 K; JavaScript mieszany: 64 K). Etapy eksportu i pulpitu mają charakter informacyjny.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Zaliczenie/niezaliczenie dotyczy faz potoku danych i parsera (potok danych i parser wbudowany numeryczny: 1024 K ramek/s; wbudowany mieszany: 512 K; Lua numeryczny: 256 K; JavaScript numeryczny i Lua mieszany: 128 K; JavaScript mieszany: 64 K). Fazy eksportu i pulpitu mają charakter informacyjny.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Zaliczony/Niezaliczony dotyczy faz potoku danych i parsera (potok danych i natywny numeryczny 1024 K ramek/s; natywny mieszany 512 K; Lua numeryczny 256 K; JavaScript numeryczny i Lua mieszany 128 K; JavaScript mieszany 64 K). Fazy eksportu i pulpitu mają charakter informacyjny.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Zaliczenie/niezaliczenie dotyczy faz potoku danych i parsera (potok danych: 1024 K ramek/s; numeryczny: Lua 256 K, JavaScript 128 K; mieszany: Lua 128 K, JavaScript 64 K). Fazy eksportu i pulpitu mają charakter informacyjny.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Zaliczenie/niezaliczenie dotyczy wyłącznie faz parsera (cel Lua: 256 K ramek/s, JavaScript: 128 K). Fazy eksportu i pulpitu mają charakter informacyjny.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>Uruchamianie…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>Uruchom Test Wydajności</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>Potok danych</translation>
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
        <translation type="vanished">Lua + eksport danych</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + pulpit</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Parser natywny (numeryczny)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Parser natywny (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Parser wbudowany (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Parser wbudowany (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Parser Lua (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>Parser JavaScript (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Parser Lua (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>Parser JavaScript (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Parser wbudowany + eksport danych (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + eksport danych (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + eksport danych (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Parser wbudowany + eksport danych (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + eksport danych (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + eksport danych (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Parser wbudowany + pulpit (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + pulpit (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 tys. ramek</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>250 K frames</source>
        <translation>250 tys. ramek</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>500 K frames</source>
        <translation>500 tys. ramek</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>1 M frames</source>
        <translation>1 mln ramek</translation>
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
        <translation>Serial Studio %1 - Test Wydajności Ścieżki Krytycznej</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), obciążenie: minimum %3 ramek, minimum %4 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>Potok</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>Przepustowość</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>Cel</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>Czas</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>Wynik</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="225"/>
        <source>%1 frames/s</source>
        <translation>%1 ramek/s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="219"/>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>n/a</source>
        <translation>nie dotyczy</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>Zaliczony</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>Niezaliczony</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + panel (numeryczny)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>Wbudowany + panel (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + panel (mieszany)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + panel (mieszany)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Wyświetlanie pulpitu...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Urządzenie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Usługa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Charakterystyka</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Skanowanie…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Nie Wykryto Adaptera Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Podłącz adapter Bluetooth lub sprawdź ustawienia systemowe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Ten System Operacyjny Nie Jest Jeszcze Obsługiwany.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio zostanie zaktualizowane, aby działać z tym systemem operacyjnym, gdy QT oficjalnie go obsłuży</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Nie Znaleziono Sterowników CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>Zainstaluj sterowniki sprzętowe CAN dla swojego systemu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>Sterownik CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="140"/>
        <source>Interface</source>
        <translation>Interfejs</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="169"/>
        <source>Bitrate</source>
        <translation>Przepływność</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="223"/>
        <source>Flexible Data-Rate</source>
        <translation>Elastyczna Szybkość Transmisji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="243"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="263"/>
        <source>Listen-Only</source>
        <translation>Tylko Nasłuch</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="283"/>
        <source>DBC Database</source>
        <translation>Baza Danych DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="287"/>
        <source>Import DBC File…</source>
        <translation>Importuj Plik DBC…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="320"/>
        <source>No CAN Interfaces Found</source>
        <translation>Nie Znaleziono Interfejsów CAN</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>Wybierz plik CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>Pliki CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>Aktywne Połączenie z Urządzeniem</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Aby użyć tej funkcji, należy rozłączyć się z urządzeniem. Kontynuować?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>Sprawdź uprawnienia i lokalizację pliku</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Niewystarczające Dane w Pliku CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>Nie można odczytać pliku CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>Plik CSV musi zawierać co najmniej jeden wiersz danych, aby kontynuować. Sprawdź plik i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>Nieprawidłowy Plik CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>Plik CSV nie zawiera żadnych danych ani nagłówków.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>Wybierz kolumnę daty/czasu</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>Ustaw interwał ręcznie</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>Wybór Daty/czasu CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Wybierz sposób obsługi danych daty/czasu:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>Ustaw Interwał</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Wprowadź interwał między wierszami w milisekundach:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>Wybierz Kolumnę Daty/czasu</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Wybierz kolumnę zawierającą dane daty/czasu:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>Nieprawidłowy Wybór</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>Wybrana kolumna jest nieprawidłowa.</translation>
    </message>
</context>
<context>
    <name>ChatSidebar</name>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="44"/>
        <source>Chats</source>
        <translation>Rozmowy</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="57"/>
        <location filename="../../qml/AI/ChatSidebar.qml" line="115"/>
        <source>New chat</source>
        <translation>Nowa rozmowa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="125"/>
        <source>%1 messages</source>
        <translation>Komunikaty: %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="147"/>
        <source>Rename...</source>
        <translation>Zmień Nazwę…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="158"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="197"/>
        <source>Rename chat</source>
        <translation>Zmień nazwę rozmowy</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="217"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="222"/>
        <source>Rename</source>
        <translation>Zmień Nazwę</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>Konsola</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="331"/>
        <source>Console Export is a Pro feature.</source>
        <translation>Eksport Konsoli jest funkcją Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Ta funkcja wymaga licencji. Zakup licencję, aby włączyć eksport konsoli.</translation>
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
        <translation>Bez Zakończenia Linii</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>Nowa Linia</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>Powrót Karetki</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>Zwykły Tekst</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>Szesnastkowy</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>Bez Sumy Kontrolnej</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>Urządzenie %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Wstaw Stałą</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>Podstawowe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>Prędkość światła w próżni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Stała Plancka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Ładunek elementarny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Stała Avogadra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Stała Boltzmanna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Stała Stefana-Boltzmanna</translation>
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
        <translation>Przyspieszenie grawitacyjne</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Stała grawitacyjna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Ciśnienie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Atmosfera normalna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Ciśnienie barometryczne na poziomie morza</translation>
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
        <translation>Zero bezwzględne (Celsjusz)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Punkt zamarzania wody</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Punkt wrzenia wody (1 atm)</translation>
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
        <translation>Gazy i Ciecze</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Uniwersalna stała gazowa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Indywidualna stała gazowa (suche powietrze)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Indywidualna stała gazowa (para wodna)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Gęstość powietrza (poziom morza, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Gęstość wody (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Prędkość dźwięku w powietrzu (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Współczynnik ciepła właściwego (suche powietrze)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>Elektromagnetyzm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>Przenikalność elektryczna próżni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Przenikalność magnetyczna próżni</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Matematyka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Liczba Eulera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Złoty podział</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Stałe Fizyczne</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>Wartości predefiniowane w jednostkach SI. Kliknij wiersz, aby wstawić do %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Symbol</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Wartość</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Kategoria</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Brak pasujących stałych.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>Stałe: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 z %2 stałych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Skrypt Sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Loop</source>
        <translation>Pętla Sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>Cofnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>Ponów</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>Wytnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>Wklej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>Zaznacz Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>Formatuj Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>Formatuj Zaznaczenie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control loop</source>
        <translation>Zresetuj do domyślnej pętli sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control loop file</source>
        <translation>Importuj plik pętli sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control loop documentation</source>
        <translation>Otwórz dokumentację pętli sterowania</translation>
    </message>
    <message>
        <source>Reset to the default control script</source>
        <translation type="vanished">Zresetuj do domyślnego skryptu sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>Otwórz</translation>
    </message>
    <message>
        <source>Import a control script file</source>
        <translation type="vanished">Importuj plik skryptu sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>Cofnij ostatnią edycję kodu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>Ponów poprzednio cofniętą edycję</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>Wytnij zaznaczony kod do schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>Skopiuj zaznaczony kod do schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>Wklej kod ze schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <source>Open the control script documentation</source>
        <translation type="vanished">Otwórz dokumentację skryptu sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>Waliduj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Sprawdź, czy skrypt kompiluje się poprawnie</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Opcje Odzyskiwania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio został niespodziewanie zamknięty kilka razy z rzędu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>Kolejne awarie: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Ostatnio zgłoszony etap: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Wykryto o: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Wybierz akcję odzyskiwania. Serial Studio zostanie zamknięty po jej zastosowaniu, aby następne uruchomienie rozpoczęło się od nowa.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Przywróć Domyślny Backend Renderowania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Pomiń Przywracanie Ostatnio Otwartego Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Resetuj wszystkie preferencje</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Kontynuuj Mimo To</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>Odtwarzacz CSV</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>Podgląd Pliku DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>Plik DBC: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Przejrzyj komunikaty CAN i sygnały do zaimportowania do nowego projektu Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Komunikaty</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Nazwa Komunikatu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>IDENTYFIKATOR CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Sygnały</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>Nie znaleziono komunikatów w pliku DBC.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Razem: %1 komunikatów, %2 sygnałów</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Utwórz Projekt</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="262"/>
        <source>Dashboard %1</source>
        <translation>Pulpit %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>Wyślij</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Nie zdefiniowano funkcji transmisji</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="326"/>
        <source>Set Wallpaper…</source>
        <translation>Ustaw Tapetę…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="332"/>
        <source>Clear Wallpaper</source>
        <translation>Wyczyść Tapetę</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="342"/>
        <source>Tile Windows</source>
        <translation>Rozmieść Okna Kafelkowo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="361"/>
        <source>Pro features detected in this project.</source>
        <translation>Wykryto funkcje Pro w tym projekcie.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="363"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Aktywne są widżety zastępcze. Zakup licencję, aby uzyskać pełną funkcjonalność.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="504"/>
        <source>Empty Workspace</source>
        <translation>Pusty Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="518"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Użyj paska wyszukiwania, aby znaleźć i dodać widżety, lub kliknij prawym przyciskiem widżet w innym obszarze roboczym, aby dodać go tutaj.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="533"/>
        <source>Search Widgets</source>
        <translation>Wyszukaj Widżety</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>Pulpit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="215"/>
        <source>API Server Active (%1)</source>
        <translation>Serwer API Aktywny (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="216"/>
        <source>API Server Ready</source>
        <translation>Serwer API Gotowy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="217"/>
        <source>API Server Off</source>
        <translation>Serwer API Wyłączony</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="155"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="307"/>
        <source>Send</source>
        <translation>Wyślij</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="295"/>
        <source>Enter command…</source>
        <translation>Wprowadź polecenie…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Nie zdefiniowano funkcji transmisji</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Wprowadź polecenie…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Wyślij</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Nie zdefiniowano funkcji transmisji</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>WŁ.</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>WYŁ.</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>Nie zdefiniowano funkcji transmisji</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Resume</source>
        <translation>Wznów</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="313"/>
        <source>Awaiting data…</source>
        <translation>Oczekiwanie na dane…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="368"/>
        <source>Open %1 in a separate window</source>
        <translation>Otwórz %1 w osobnym oknie</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="289"/>
        <source>Select Javascript file to import</source>
        <translation>Wybierz plik Javascript do zaimportowania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="368"/>
        <source>Code Validation Failed</source>
        <translation>Walidacja Kodu Nie Powiodła Się</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="358"/>
        <source>Line %1: %2</source>
        <translation>Linia %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="369"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>Skrypt musi definiować funkcję setup() i/lub loop().</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="374"/>
        <source>Code Validation Successful</source>
        <translation>Walidacja Kodu Zakończona Sukcesem</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="375"/>
        <source>No syntax errors detected in the control loop.</source>
        <translation>Nie wykryto błędów składni w pętli sterowania.</translation>
    </message>
    <message>
        <source>No syntax errors detected in the control script.</source>
        <translation type="vanished">Nie wykryto błędów składni w skrypcie sterującym.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>Importuj Plik DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>Pliki DBC (*.DBC);;Wszystkie Pliki (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>Nie udało się przetworzyć pliku DBC: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>Sprawdź format pliku i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>Błąd Importu DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>Plik DBC nie zawiera komunikatów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Wybrany plik nie zawiera żadnych definicji komunikatów CAN.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>Ostrzeżenie Importu DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>Przegląd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>Aktywny</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Nie udało się utworzyć tymczasowego pliku projektu</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">Sprawdź, czy aplikacja ma uprawnienia do zapisu w katalogu tymczasowym.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>Pomyślnie zaimportowano plik DBC z %1 komunikatami i %2 sygnałami.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>Edytor projektu jest teraz otwarty do dostosowania.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Nie udało się załadować zaimportowanego projektu</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Wygenerowany JSON projektu nie mógł zostać załadowany.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>Pominięto %1 sygnał(ów) używających rozszerzonego multipleksowania (SG_MUL_VAL_); obsługiwane jest tylko proste multipleksowanie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>Import DBC Zakończony</translation>
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
        <translation>Transformacja Wartości Zbioru Danych</translation>
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
        <translation>Język:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="132"/>
        <source>Template:</source>
        <translation>Szablon:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Wprowadź wartość surową (np. 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Test</source>
        <translation>Testuj</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="112"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="143"/>
        <source>Input:</source>
        <translation>Wejście:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="146"/>
        <source>Output:</source>
        <translation>Wyjście:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="114"/>
        <source>Apply</source>
        <translation>Zastosuj</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="115"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="224"/>
        <source>Transform — %1</source>
        <translation>Transformacja — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="304"/>
        <source>The value transform has a syntax error and was not applied.</source>
        <translation>Transformacja wartości zawiera błąd składni i nie została zastosowana.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>The value transform must define a transform(value) function.</source>
        <translation>Transformacja wartości musi definiować funkcję transform(value).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="314"/>
        <source>Define a transform(value) function that returns a number, or use Clear to remove the transform.</source>
        <translation>Zdefiniuj funkcję transform(value) zwracającą liczbę lub użyj Wyczyść, aby usunąć transformację.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="332"/>
        <source>Enter a value</source>
        <translation>Wprowadź wartość</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="339"/>
        <source>Invalid number</source>
        <translation>Nieprawidłowa liczba</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="408"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatuj Dokument	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="409"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatuj Zaznaczenie	ctrl+i</translation>
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
-- Zdefiniuj funkcję transform(value), która otrzymuje bieżący
-- odczyt zestawu danych i zwraca przekształconą liczbę. Jeśli
-- funkcja transform() nie jest zdefiniowana, surowa wartość
-- jest zachowywana i nic nie jest zapisywane z projektem.
--
-- Przykład:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Zmienne globalne zadeklarowane na najwyższym poziomie
-- utrzymują się między ramkami, co jest przydatne dla filtrów,
-- akumulatorów i innych transformacji stanowych, podobnie jak
-- w skrypcie parsera ramek:
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
-- Użyj menu Szablon, aby uzyskać gotowe przykłady, lub
-- kliknij Testuj, aby wypróbować swoją funkcję.
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
 * Zdefiniuj funkcję transform(value), która otrzymuje bieżący
 * odczyt zestawu danych i zwraca przekształconą liczbę. Jeśli
 * funkcja transform() nie jest zdefiniowana, surowa wartość
 * jest zachowywana i nic nie jest zapisywane z projektem.
 *
 * Przykład:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Zmienne globalne zadeklarowane na najwyższym poziomie
 * utrzymują się między ramkami, co jest przydatne dla filtrów,
 * akumulatorów i innych przekształceń stanowych -- tak jak
 * w skrypcie parsera ramek:
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
 * Użyj menu rozwijanego Szablon dla gotowych przykładów lub
 * kliknij Test, aby wypróbować swoją funkcję.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="608"/>
        <source>Failed to create the Lua engine.</source>
        <translation>Nie udało się utworzyć silnika Lua.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="646"/>
        <source>Line %1: %2</source>
        <translation>Linia %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="800"/>
        <source>Select Template…</source>
        <translation>Wybierz Szablon…</translation>
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
 * Zdefiniuj funkcję transform(value), która otrzymuje bieżący
 * odczyt zestawu danych i zwraca przekształconą liczbę. Jeśli
 * funkcja transform() nie jest zdefiniowana, surowa wartość
 * jest zachowywana i nic nie jest zapisywane z projektem.
 *
 * Przykład:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Zmienne globalne zadeklarowane na najwyższym poziomie
 * utrzymują się między ramkami, co jest przydatne dla filtrów,
 * akumulatorów i innych przekształceń stanowych — tak jak
 * w skrypcie parsera ramek:
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
 * Użyj menu rozwijanego Szablon dla gotowych przykładów lub
 * kliknij Test, aby wypróbować swoją funkcję.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <source>Engine error</source>
        <translation>Błąd silnika</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="714"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="727"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="744"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="755"/>
        <source>Error: %1</source>
        <translation>Błąd: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="720"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="748"/>
        <source>Error: transform() not defined</source>
        <translation>Błąd: transform() nie jest zdefiniowana</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="732"/>
        <source>Error: transform() must return a number</source>
        <translation>Błąd: transform() musi zwracać liczbę</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1705"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1819"/>
        <source>Channel %1</source>
        <translation>Kanał %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1830"/>
        <source>Audio Input</source>
        <translation>Wejście Audio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1714"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1835"/>
        <source>Quick Plot</source>
        <translation>Szybki Wykres</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1429"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>Limit transformacji JavaScript przekroczony</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1430"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>Transformacja zbioru danych trwała dłużej niż %1 ms; pozostałe zbiory danych w ramce zostały ustawione na wartości surowe do następnej ramki. Przeanalizuj lub uprość kod transformacji.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="230"/>
        <source>Frame pool exhausted</source>
        <translation>Wyczerpano pulę ramek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="232"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>Jeden z odbiorców (dashboard, eksport CSV/MDF4, baza sesji lub subskrybent API) nie pobiera ramek wystarczająco szybko. Serial Studio przechodzi na alokacje per-ramka do czasu opróżnienia zaległości. Wyłącz obciążonego odbiorcę lub zmniejsz szybkość transmisji danych.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1666"/>
        <source>Device A</source>
        <translation>Urządzenie A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1721"/>
        <source>Quick Plot Data</source>
        <translation>Dane Szybkiego Wykresu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1733"/>
        <source>Multiple Plots</source>
        <translation>Wiele Wykresów</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>Zwykły tekst (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>Szesnastkowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>Binarny (surowe bajty)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>Tylko ogranicznik końcowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>Ogranicznik początkowy + końcowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>Tylko ogranicznik początkowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Bez ograniczników (cały fragment)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>Bez Sumy Kontrolnej</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>Wybierz Szablon Parsera Ramek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>Wybierz szablon do załadowania:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Nieprawidłowe dane szesnastkowe.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>Nie wybrano szablonu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>Wyodrębniono ramek: %1 | zużyto bajtów: %2 | buforowanych bajtów: %3 | odrzucono: %4</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>Nieprawidłowy JSON: %1</translation>
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
        <translation type="vanished">Brak</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Nieprawidłowe Wejście HEX</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Wprowadź prawidłowe bajty szesnastkowe.

Prawidłowy format: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(brak ramek)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">Ekstrakcja nie wygenerowała kompletnej ramki. Sprawdź ograniczniki początkowy/końcowy oraz tryb detekcji.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">Wyodrębniono ramek: %1 | zużyto bajtów: %2 | buforowanych bajtów: %3 | odrzucono: %4</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Konfiguracja Potoku</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Wyniki Potoku</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Wykrywanie</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Dekoder</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Suma Kontrolna</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Ogranicznik Początkowy</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">Ogranicznik Końcowy</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Ograniczniki Szesnastkowe</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Tylko ogranicznik końcowy</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Ogranicznik początkowy + końcowy</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Tylko ogranicznik początkowy</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Bez ograniczników (cały fragment)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Zwykły tekst (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Szesnastkowy</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binarny (surowe bajty)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Wyczyść</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Oceń</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Wprowadź tutaj surowe bajty strumienia...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Etap</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Wejście Danych Ramki</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Wyniki Parsera Ramki</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Wprowadź dane ramki tutaj…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Indeks Zestawu Danych</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Wartość</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Wprowadź dane ramki powyżej, włącz tryb HEX jeśli potrzeba, następnie kliknij „Oceń", aby uruchomić parser ramki.

Przykład (Tekst): a,b,c,d,e,f
Przykład (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Testuj Parser Ramki</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Wprowadź bajty hex (np. 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(puste)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Brak zwróconych danych</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Change Scripting Language?</source>
        <translation>Zmienić Język Skryptowy?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="220"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Przełączenie języka skryptowego zastępuje bieżący kod parsera równoważnym szablonem w nowym języku.

Wszystkie niezapisane zmiany zostaną utracone. Kontynuować?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Javascript file to import</source>
        <translation>Wybierz plik Javascript do zaimportowania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Lua file to import</source>
        <translation>Wybierz plik Lua do zaimportowania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>Code Validation Successful</source>
        <translation>Walidacja Kodu Zakończona Sukcesem</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="415"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>Nie wykryto błędów składni w kodzie parsera.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Select Frame Parser Template</source>
        <translation>Wybierz Szablon Parsera Ramek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="526"/>
        <source>Choose a template to load:</source>
        <translation>Wybierz szablon do załadowania:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>Importuj Mapę Rejestrów Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Mapy Rejestrów Modbus (*.CSV *.XML *.JSON);;Pliki CSV (*.CSV);;Pliki XML (*.XML);;Pliki JSON (*.JSON);;Wszystkie Pliki (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>Nie znaleziono rejestrów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Nie można przetworzyć pliku lub nie zawiera on definicji rejestrów.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>Import Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>Przegląd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>Włączony</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Nie udało się załadować zaimportowanego projektu</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Wygenerowany JSON projektu nie mógł zostać załadowany.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Nie udało się utworzyć tymczasowego pliku projektu</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Sprawdź uprawnienia do zapisu w katalogu tymczasowym.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>Pomyślnie zaimportowano %1 rejestrów w %2 grupach.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>Edytor projektu jest teraz otwarty do dostosowania.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>Import Modbus Zakończony</translation>
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
        <translation type="vanished">Zwykły tekst (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Szesnastkowy</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binarny (surowe bajty)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Tylko ogranicznik końcowy</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Ogranicznik początkowy + końcowy</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Tylko ogranicznik początkowy</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Bez ograniczników (cały fragment)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">Bez Sumy Kontrolnej</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">Nieprawidłowe dane szesnastkowe.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">Nie wybrano szablonu.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">Wyodrębniono ramek: %1 | zużyto bajtów: %2 | buforowanych bajtów: %3 | odrzucono: %4</translation>
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
        <translation>Wybierz plik Javascript do zaimportowania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>Wybierz Szablon Widgetu Wyjściowego</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>Wybierz szablon do wczytania:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>Wybierz plik Javascript do zaimportowania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>Wybierz Szablon Widgetu Malarskiego</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>Wybierz szablon do wczytania:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>Dodać zbiory danych dla tego szablonu?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" oczekuje %2 zbiorów danych; bieżąca grupa ma %3.

Dodać %4 zbiorów danych używając domyślnych wartości szablonu?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2491"/>
        <source>Project Information</source>
        <translation>Informacje o Projekcie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2501"/>
        <source>Project Title</source>
        <translation>Tytuł Projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2502"/>
        <source>Untitled Project</source>
        <translation>Projekt Bez Tytułu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2503"/>
        <source>Name or description of the project</source>
        <translation>Nazwa lub opis projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2634"/>
        <source>Time</source>
        <translation>Czas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2634"/>
        <source>Samples</source>
        <translation>Próbki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2646"/>
        <source>Plot every curve against time or against the sample number</source>
        <translation>Wykreśl każdą krzywą względem czasu lub względem numeru próbki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2662"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2664"/>
        <source>Web address to load in this widget</source>
        <translation>Adres internetowy do załadowania w tym widżecie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2772"/>
        <source>Frame Detection</source>
        <translation>Wykrywanie Ramek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2786"/>
        <source>Frame Detection Method</source>
        <translation>Metoda Wykrywania Ramek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2787"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Wybierz sposób identyfikacji przychodzących ramek danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2797"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Ograniczniki Szesnastkowe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2798"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Wprowadź sekwencje początku/końca ramki jako wartości szesnastkowe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Frame Start Delimiter</source>
        <translation>Ogranicznik Początku Ramki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2815"/>
        <source>e.g. /*</source>
        <translation>np. /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Sekwencja oznaczająca początek ramki danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2828"/>
        <source>Frame End Delimiter</source>
        <translation>Ogranicznik Końca Ramki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2829"/>
        <source>e.g. */</source>
        <translation>np. */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2830"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Sekwencja oznaczająca koniec ramki danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2836"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Przetwarzanie i Walidacja Ładunku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2847"/>
        <source>Data Conversion Method</source>
        <translation>Metoda Konwersji Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2848"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Wybierz sposób dekodowania przychodzących danych binarnych przed parsowaniem</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2864"/>
        <source>Checksum Algorithm</source>
        <translation>Algorytm Sumy Kontrolnej</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2865"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Wybierz algorytm sumy kontrolnej używany do walidacji ramek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2521"/>
        <source>Group Information</source>
        <translation>Informacje o Grupie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2531"/>
        <source>Group Title</source>
        <translation>Tytuł Grupy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2532"/>
        <source>Untitled Group</source>
        <translation>Grupa Bez Tytułu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2533"/>
        <source>Title or description of this dataset group</source>
        <translation>Tytuł lub opis tej grupy zestawów danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2704"/>
        <source>Composite Widget</source>
        <translation>Widget Kompozytowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2705"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Wybierz sposób wizualizacji tej grupy zestawów danych (opcjonalnie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2583"/>
        <source>Image Configuration</source>
        <translation>Konfiguracja Obrazu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3444"/>
        <source>Virtual Dataset</source>
        <translation>Wirtualny Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3445"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Wirtualne zestawy danych obliczają swoją wartość z transformacji i tabel danych, nie wymagają indeksu ramki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3848"/>
        <source>Fixed decimal places for the value display; overrides the format (-1 = auto)</source>
        <translation>Stała liczba miejsc dziesiętnych dla wyświetlacza wartości; nadpisuje format (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3977"/>
        <source>Auto-detect</source>
        <translation>Wykryj Automatycznie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3977"/>
        <source>Manual Delimiters</source>
        <translation>Ręczne Ograniczniki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2596"/>
        <source>Detection Mode</source>
        <translation>Tryb Wykrywania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1421"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1424"/>
        <source>Frame Parser</source>
        <translation>Parser Ramek</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation type="vanished">Grupy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1696"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1709"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1710"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1727"/>
        <source>Shared Memory</source>
        <translation>Pamięć Współdzielona</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1696"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1715"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1716"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5438"/>
        <source>Dataset Values</source>
        <translation>Wartości Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1876"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1889"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1890"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1905"/>
        <source>Workspaces</source>
        <translation>Obszary Robocze</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1922"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1925"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1926"/>
        <source>MQTT Publisher</source>
        <translation>Wydawca MQTT</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Skrypt Sterujący</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1999"/>
        <source>Publishing</source>
        <translation>Publikowanie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2009"/>
        <source>Enable Publishing</source>
        <translation>Włącz Publikowanie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2010"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Transmituj ramki, surowe bajty i powiadomienia do brokera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2021"/>
        <source>Payload</source>
        <translation>Ładunek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2022"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Wybiera, co jest publikowane: przetworzone dane pulpitu lub surowe bajty RX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2032"/>
        <source>Publish Rate (Hz)</source>
        <translation>Częstotliwość Publikowania (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2033"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>Ile razy na sekundę publikować (1-30 Hz). Wyższe częstotliwości zwiększają obciążenie brokera; dane pulpitu są ograniczane częstotliwościowo, więc wolny broker nigdy nie blokuje parsowania ramek.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2045"/>
        <source>Topic Base</source>
        <translation>Podstawa Tematu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2046"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/urządzenie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2047"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Podstawowy temat używany do publikowania ramek i surowych bajtów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2057"/>
        <source>Script Topic</source>
        <translation>Temat Skryptu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2058"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2082"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>Domyślnie używa Tematu Podstawowego, gdy pusty</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2059"/>
        <source>Topic the user script publishes to</source>
        <translation>Temat, do którego publikuje skrypt użytkownika</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2069"/>
        <source>Publish Notifications</source>
        <translation>Publikuj Powiadomienia</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2070"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Duplikuj powiadomienia z panelu do dedykowanego tematu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2081"/>
        <source>Notification Topic</source>
        <translation>Temat Powiadomień</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2083"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Temat, do którego duplikowane są powiadomienia z panelu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2096"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2106"/>
        <source>Hostname</source>
        <translation>Nazwa Hosta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2107"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2108"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>Nazwa hosta lub adres IP brokera MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2117"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2118"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>Port TCP udostępniany przez brokera (1883 zwykły, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2128"/>
        <source>Custom Client ID</source>
        <translation>Niestandardowe ID Klienta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2130"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Wył.: generowane jest nowe losowe ID przy każdym załadowaniu projektu. Wł.: użyj ID poniżej.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2141"/>
        <source>Client ID</source>
        <translation>ID Klienta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2142"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Identyfikator wysyłany do brokera podczas CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2155"/>
        <source>Protocol Version</source>
        <translation>Wersja Protokołu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2156"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>Wersja protokołu MQTT używana podczas CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2165"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2166"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Sekundy między pakietami PINGREQ w stanie bezczynności</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2175"/>
        <source>Clean Session</source>
        <translation>Czysta Sesja</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2176"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Odrzuć wszelkie trwałe stany sesji podczas CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2191"/>
        <source>Username</source>
        <translation>Nazwa Użytkownika</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2192"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Nazwa użytkownika do uwierzytelniania brokera (pozostaw puste dla anonimowego)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2202"/>
        <source>Password</source>
        <translation>Hasło</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2203"/>
        <source>Password for broker authentication</source>
        <translation>Hasło do uwierzytelniania brokera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2214"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2224"/>
        <source>Use SSL/TLS</source>
        <translation>Użyj SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2225"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Tuneluj połączenie z brokerem przez TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2238"/>
        <source>Protocol</source>
        <translation>Protokół</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2239"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Negocjowana rodzina protokołu TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2249"/>
        <source>Peer Verify</source>
        <translation>Weryfikacja Partnera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2250"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Jak ściśle weryfikowany jest łańcuch certyfikatów brokera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2260"/>
        <source>Verify Depth</source>
        <translation>Głębokość Weryfikacji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Maksymalna akceptowana długość łańcucha certyfikatów (0 = nieograniczona)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2548"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3182"/>
        <source>Device %1</source>
        <translation>Urządzenie %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2566"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2731"/>
        <source>Input Device</source>
        <translation>Urządzenie Wejściowe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2567"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Wybierz, które podłączone urządzenie dostarcza dane dla tej grupy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2598"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Automatyczne wykrywanie odczytuje bajty magiczne JPEG/PNG; Ręczne używa jawnych sekwencji początkowych/końcowych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2608"/>
        <source>Start Sequence (Hex)</source>
        <translation>Sekwencja Początkowa (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2609"/>
        <source>e.g. FF D8 FF</source>
        <translation>np. FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2610"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Bajty hex oznaczające początek ramki obrazu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2619"/>
        <source>End Sequence (Hex)</source>
        <translation>Sekwencja Końcowa (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2620"/>
        <source>e.g. FF D9</source>
        <translation>np. FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2621"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Bajty hex oznaczające koniec ramki obrazu</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">Tożsamość</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2741"/>
        <source>Device Name</source>
        <translation>Nazwa Urządzenia</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2742"/>
        <source>Device 1</source>
        <translation>Urządzenie 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2743"/>
        <source>Human-readable name for this input device</source>
        <translation>Czytelna dla człowieka nazwa tego urządzenia wejściowego</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2752"/>
        <source>Bus Type</source>
        <translation>Typ Magistrali</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2753"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Wybierz interfejs sprzętowy dla tego urządzenia wejściowego</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2755"/>
        <source>Serial Port</source>
        <translation>Port Szeregowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2755"/>
        <source>Network Socket</source>
        <translation>Gniazdo Sieciowe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2755"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2757"/>
        <source>Audio Input</source>
        <translation>Wejście Audio</translation>
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
        <translation>Surowe USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2758"/>
        <source>HID Device</source>
        <translation>Urządzenie HID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2758"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2758"/>
        <source>MQTT Subscriber</source>
        <translation>Subskrybent MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2913"/>
        <source>Connection Settings</source>
        <translation>Ustawienia Połączenia</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3150"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3420"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5105"/>
        <source>General Information</source>
        <translation>Informacje Ogólne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3159"/>
        <source>Action Title</source>
        <translation>Tytuł Akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3161"/>
        <source>Untitled Action</source>
        <translation>Akcja Bez Tytułu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3162"/>
        <source>Name or description of this action</source>
        <translation>Nazwa lub opis tej akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3171"/>
        <source>Action Icon</source>
        <translation>Ikona Akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3172"/>
        <source>Default Icon</source>
        <translation>Ikona Domyślna</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3173"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Ikona wyświetlana dla tej akcji w panelu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3200"/>
        <source>Target Device</source>
        <translation>Urządzenie Docelowe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3201"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Wybierz, do którego podłączonego urządzenia ta akcja wysyła dane</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3213"/>
        <source>Data Payload</source>
        <translation>Ładunek Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3224"/>
        <source>Send as Binary</source>
        <translation>Wyślij jako Binarny</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3225"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Wysyła surowe dane binarne po wyzwoleniu tej akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3236"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3248"/>
        <source>Command</source>
        <translation>Polecenie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3237"/>
        <source>Transmit Data (Hex)</source>
        <translation>Transmituj Dane (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3238"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Ładunek szesnastkowy do wysłania po wyzwoleniu akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3249"/>
        <source>Transmit Data</source>
        <translation>Transmituj Dane</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3250"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Ładunek tekstowy do wysłania po wyzwoleniu akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3261"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5164"/>
        <source>Text Encoding</source>
        <translation>Kodowanie Tekstu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3262"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Kodowanie znaków używane do serializacji ładunku tekstowego</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3286"/>
        <source>End-of-Line Sequence</source>
        <translation>Sekwencja Zakończenia Linii</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3287"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>Znaki EOL dołączane do komunikatu (np. </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3299"/>
        <source>Execution Behavior</source>
        <translation>Zachowanie Wykonania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3310"/>
        <source>Auto-Execute on Connect</source>
        <translation>Automatyczne Wykonanie przy Połączeniu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3311"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Automatycznie wyzwól tę akcję po połączeniu urządzenia</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3317"/>
        <source>Timer Behavior</source>
        <translation>Zachowanie Timera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3326"/>
        <source>Timer Mode</source>
        <translation>Tryb Timera</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3329"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Wybierz, kiedy i jak ta akcja ma być automatycznie powtarzana</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3336"/>
        <source>Interval (ms)</source>
        <translation>Interwał (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3340"/>
        <source>Timer Interval (ms)</source>
        <translation>Interwał Timera (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3341"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Milisekundy między każdym powtórzonym wyzwoleniem tej akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3348"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3352"/>
        <source>Repeat Count</source>
        <translation>Liczba Powtórzeń</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3353"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Liczba wysłań komendy przy każdym wyzwoleniu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3430"/>
        <source>Untitled Dataset</source>
        <translation>Zestaw Danych Bez Tytułu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3431"/>
        <source>Dataset Title</source>
        <translation>Tytuł Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3432"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Nazwa zestawu danych, używana do etykietowania i identyfikacji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3461"/>
        <source>Hide on Dashboard</source>
        <translation>Ukryj na Pulpicie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3462"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Ukryj samodzielny kafelek tego zbioru danych na pulpicie; widget malarski nadal może odczytywać jego wartości</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3508"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Dolna granica zakresu wartości zbioru danych; widżety i FFT używają jej, gdy ich własny zakres nie jest ustawiony</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Górna granica zakresu wartości zbioru danych; widżety i FFT używają jej, gdy ich własny zakres nie jest ustawiony</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3579"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Wybierz Czas lub zbiór danych do sterowania osią X na wykresach</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3592"/>
        <source>Frequency Analysis</source>
        <translation>Analiza Częstotliwości</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3639"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Wybierz Czas (domyślnie) lub dowolny zestaw danych, którego wartość steruje osią Y -- tworzy diagram Campbella po powiązaniu np. z RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3689"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3778"/>
        <source>Minimum Value (optional)</source>
        <translation>Wartość Minimalna (opcjonalnie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3690"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Dolna granica normalizacji danych; domyślnie używany jest zakres wartości zbioru danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3702"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3791"/>
        <source>Maximum Value (optional)</source>
        <translation>Wartość Maksymalna (opcjonalnie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3703"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Górna granica normalizacji danych; domyślnie używany jest zakres wartości zbioru danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3779"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Dolna granica zakresu wskaźnika lub paska; domyślnie używany jest zakres wartości zbioru danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3792"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Górna granica zakresu wskaźnika lub paska; domyślnie używany jest zakres wartości zbioru danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3844"/>
        <source>Decimal Points</source>
        <translation>Miejsca Dziesiętne</translation>
    </message>
    <message>
        <source>Decimal places shown in the data grid value column (-1 = auto)</source>
        <translation type="vanished">Liczba miejsc dziesiętnych wyświetlanych w kolumnie wartości siatki danych (-1 = auto)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3891"/>
        <source>On</source>
        <translation>Wł.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3935"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>LED zapala się, gdy wartość osiągnie lub przekroczy ten próg; zdefiniuj pasma alarmowe dla kolorów wielostanowych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3992"/>
        <source>Painter Widget</source>
        <translation>Widget Malarski</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3993"/>
        <source>Web View</source>
        <translation>Widok Internetowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5439"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Wartości surowe i przekształcone dla każdego zestawu danych (tylko do odczytu)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5448"/>
        <source>Shared table defined in this project</source>
        <translation>Tabela współdzielona zdefiniowana w tym projekcie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6071"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>Usunąć 1 odniesienie widgetu, którego docelowa grupa lub zbiór danych już nie istnieje?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6072"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>Usunąć %1 odniesień widgetów, których docelowe grupy lub zbiory danych już nie istnieją?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6077"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Wpłynie to tylko na rozmieszczenie kafelków w obszarze roboczym; żadne grupy, zbiory danych ani dane nie zostaną usunięte.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6080"/>
        <source>Clean Up Workspaces</source>
        <translation>Wyczyść Obszary Robocze</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3475"/>
        <source>Frame Index</source>
        <translation>Indeks Ramki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1588"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1589"/>
        <source>Dashboard Widgets</source>
        <translation>Widżety Pulpitu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1946"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1949"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1950"/>
        <source>Control Loop</source>
        <translation>Pętla Sterowania</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3476"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Pozycja ramki używana do wyrównywania zestawów danych w czasie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3485"/>
        <source>Measurement Unit</source>
        <translation>Jednostka Miary</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3486"/>
        <source>Volts, Amps, etc.</source>
        <translation>Wolty, Ampery, itp.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3487"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Jednostka miary, np. wolty lub ampery (opcjonalnie)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3534"/>
        <source>Plot Settings</source>
        <translation>Ustawienia Wykresu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3557"/>
        <source>Enable Plot Widget</source>
        <translation>Włącz Widget Wykresu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3559"/>
        <source>Plot data in real-time</source>
        <translation>Wykreślaj dane w czasie rzeczywistym</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2645"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3578"/>
        <source>X-Axis Source</source>
        <translation>Źródło Osi X</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Wybierz zestaw danych do użycia dla osi X na wykresach</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Minimalna Wartość Wykresu (opcjonalnie)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Dolna granica zakresu wyświetlania wykresu</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Maksymalna Wartość Wykresu (opcjonalnie)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Górna granica zakresu wyświetlania wykresu</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">Konfiguracja FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3603"/>
        <source>Enable FFT Analysis</source>
        <translation>Włącz Analizę FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Wykonaj analizę w dziedzinie częstotliwości dla zbioru danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3614"/>
        <source>Enable Waterfall Plot</source>
        <translation>Włącz Wykres Wodospadowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3615"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Pokaż przewijany spektrogram zawartości częstotliwości w czasie (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <source>Waterfall Y Axis</source>
        <translation>Oś Y Wykresu Wodospadowego</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="vanished">Wybierz Czas (domyślnie) lub dowolny zestaw danych, którego wartość steruje osią Y — tworzy diagram Campbella po powiązaniu np. z RPM</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3666"/>
        <source>FFT Window Size</source>
        <translation>Rozmiar Okna FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3667"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Liczba próbek używanych dla każdego okna obliczeniowego FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3678"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>Częstotliwość Próbkowania FFT (Hz, wymagane)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3679"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Częstotliwość próbkowania używana dla FFT (w Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Wartość Minimalna (zalecane)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Dolna granica normalizacji danych</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Wartość Maksymalna (zalecana)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Górna granica normalizacji danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3728"/>
        <source>Widget Settings</source>
        <translation>Ustawienia Widgetu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3751"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3752"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Wybierz widget wizualny używany do wyświetlania tego zbioru danych</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Minimalna Wartość Wyświetlania (wymagana)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Dolna granica zakresu wyświetlania wskaźnika lub paska</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Maksymalna Wartość Wyświetlania (wymagana)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Górna granica zakresu wyświetlania wskaźnika lub paska</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3808"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3843"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4004"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3809"/>
        <source>Tick Count</source>
        <translation>Liczba Podziałek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3813"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Liczba głównych podziałek na skali tarczy (0 = automatyczne dopasowanie do rozmiaru widżetu)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3832"/>
        <source>Label Format</source>
        <translation>Format Etykiety</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3833"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Liczba miejsc dziesiętnych lub notacja używana na etykietach podziałek i wyświetlaczu wartości</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Pokaż Wskaźnik Wartości</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Przełącz oprawiony odczyt numeryczny znajdujący się poniżej lub obok widżetu</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Ustawienia Alarmów</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Włącz Alarmy</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Wyzwala alarm wizualny, gdy wartość przekracza progi alarmowe</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Próg Dolny</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Wyzwala alarm wizualny, gdy wartość spadnie poniżej tego progu</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Próg Górny</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Wyzwala alarm wizualny, gdy wartość przekroczy ten próg</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3908"/>
        <source>LED Display Settings</source>
        <translation>Ustawienia Wyświetlacza LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3919"/>
        <source>Show in LED Panel</source>
        <translation>Pokaż w Panelu LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3920"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Włącza wizualne monitorowanie stanu za pomocą wyświetlacza LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3934"/>
        <source>LED On Threshold (required)</source>
        <translation>Próg Włączenia LED (wymagany)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">LED zapala się, gdy wartość osiągnie lub przekroczy ten próg</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Off</source>
        <translation>Wył.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Auto Start</source>
        <translation>Autostart</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Start on Trigger</source>
        <translation>Uruchom na Wyzwalaczu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3955"/>
        <source>Toggle on Trigger</source>
        <translation>Przełącz na Wyzwalaczu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3956"/>
        <source>Repeat N Times</source>
        <translation>Powtórz N Razy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3959"/>
        <source>Plain Text (UTF8)</source>
        <translation>Zwykły Tekst (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3959"/>
        <source>Hexadecimal</source>
        <translation>Szesnastkowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3959"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3960"/>
        <source>Binary (Direct)</source>
        <translation>Binarny (Bezpośredni)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3965"/>
        <source>No Checksum</source>
        <translation>Bez Sumy Kontrolnej</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3969"/>
        <source>End Delimiter Only</source>
        <translation>Tylko Ogranicznik Końcowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3969"/>
        <source>Start Delimiter Only</source>
        <translation>Tylko Ogranicznik Początkowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3970"/>
        <source>Start + End Delimiter</source>
        <translation>Ogranicznik Początkowy + Końcowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3970"/>
        <source>No Delimiters</source>
        <translation>Bez Ograniczników</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Button</source>
        <translation>Przycisk</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Slider</source>
        <translation>Suwak</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Toggle</source>
        <translation>Przełącznik</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3980"/>
        <source>Text Field</source>
        <translation>Pole Tekstowe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3981"/>
        <source>Knob</source>
        <translation>Pokrętło</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3985"/>
        <source>Data Grid</source>
        <translation>Siatka Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3986"/>
        <source>GPS Map</source>
        <translation>Mapa GPS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3987"/>
        <source>Gyroscope</source>
        <translation>Żyroskop</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3988"/>
        <source>Multiple Plot</source>
        <translation>Wykres Wielokrotny</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3989"/>
        <source>Accelerometer</source>
        <translation>Akcelerometr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3990"/>
        <source>3D Plot</source>
        <translation>Wykres 3D</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3991"/>
        <source>Image View</source>
        <translation>Widok Obrazu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3994"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3997"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4012"/>
        <source>None</source>
        <translation>Brak</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3998"/>
        <source>Bar</source>
        <translation>Słupek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3999"/>
        <source>Gauge</source>
        <translation>Wskaźnik</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4000"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4001"/>
        <source>Meter</source>
        <translation>Miernik</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4005"/>
        <source>Integer (0 decimals)</source>
        <translation>Liczba Całkowita (0 miejsc dziesiętnych)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4006"/>
        <source>1 decimal</source>
        <translation>1 miejsce dziesiętne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4007"/>
        <source>2 decimals</source>
        <translation>2 miejsca dziesiętne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4008"/>
        <source>3 decimals</source>
        <translation>3 miejsca dziesiętne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4009"/>
        <source>Scientific</source>
        <translation>Naukowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4013"/>
        <source>New Line (\n)</source>
        <translation>Nowa Linia (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Carriage Return (\r)</source>
        <translation>Powrót Karetki (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4015"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>No</source>
        <translation>Nie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4019"/>
        <source>Yes</source>
        <translation>Tak</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5115"/>
        <source>Label</source>
        <translation>Etykieta</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5116"/>
        <source>Display label</source>
        <translation>Wyświetl etykietę</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5126"/>
        <source>Button Icon</source>
        <translation>Ikona Przycisku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5135"/>
        <source>Colorize Icon</source>
        <translation>Koloryzuj Ikonę</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5136"/>
        <source>Tint the icon with the button color</source>
        <translation>Zabarwia ikonę kolorem przycisku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5153"/>
        <source>Initial Value</source>
        <translation>Wartość Początkowa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5165"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Kodowanie znaków używane gdy transmit() zwraca wartość tekstową</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5183"/>
        <source>Value Range</source>
        <translation>Zakres Wartości</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3507"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5193"/>
        <source>Minimum Value</source>
        <translation>Wartość Minimalna</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3520"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5202"/>
        <source>Maximum Value</source>
        <translation>Wartość Maksymalna</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5211"/>
        <source>Step Size</source>
        <translation>Rozmiar Kroku</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="629"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="638"/>
        <source>Lock Project</source>
        <translation>Zablokuj Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="630"/>
        <source>Choose a password to lock the project:</source>
        <translation>Wybierz hasło, aby zablokować projekt:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="638"/>
        <source>Confirm the password:</source>
        <translation>Potwierdź hasło:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="643"/>
        <source>Passwords do not match</source>
        <translation>Hasła nie pasują</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="644"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>Wprowadzone hasła nie pasują do siebie. Projekt nie został zablokowany.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="678"/>
        <source>Unlock Project</source>
        <translation>Odblokuj Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="679"/>
        <source>Enter the project password:</source>
        <translation>Wprowadź hasło projektu:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="689"/>
        <source>Incorrect password</source>
        <translation>Nieprawidłowe hasło</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="690"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>Wprowadzone hasło nie pasuje do hasła zapisanego w pliku projektu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="721"/>
        <source>New Project</source>
        <translation>Nowy Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="921"/>
        <source>Samples</source>
        <translation>Próbki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1486"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Wiele źródeł danych wymaga licencji Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1487"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro umożliwia jednoczesne łączenie z wieloma urządzeniami. Zaktualizuj, aby odblokować tę funkcję.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1499"/>
        <source>Device %1</source>
        <translation>Urządzenie %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Kopia)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1769"/>
        <source>Do you want to save your changes?</source>
        <translation>Zapisać zmiany?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1770"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Projekt zawiera niezapisane modyfikacje!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="501"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="510"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="523"/>
        <source>Project error</source>
        <translation>Błąd projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="501"/>
        <source>Project title cannot be empty!</source>
        <translation>Tytuł projektu nie może być pusty!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="510"/>
        <source>You need to add at least one group!</source>
        <translation>Należy dodać co najmniej jedną grupę!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="523"/>
        <source>You need to add at least one dataset!</source>
        <translation>Należy dodać co najmniej jeden zestaw danych!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="570"/>
        <source>Your project needs a title</source>
        <translation>Projekt wymaga tytułu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="572"/>
        <source>Add a group to get started</source>
        <translation>Dodaj grupę, aby rozpocząć</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="574"/>
        <source>Add a dataset to a group</source>
        <translation>Dodaj zestaw danych do grupy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="588"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Otwórz widok Projektu u góry drzewa i wprowadź nazwę. Projekt można zmienić w dowolnym momencie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="591"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>Grupy organizują zestawy danych w widżety pulpitu. Użyj przycisku Grupa na pasku narzędzi powyżej, aby utworzyć grupę, a następnie dodaj do niej zestawy danych.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="595"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>Zbiory danych to wartości wyświetlane na pulpicie. Wybierz grupę w drzewie i użyj przycisku Zbiór danych na pasku narzędzi, aby dodać nowy.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="920"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="969"/>
        <source>Time</source>
        <translation>Czas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1531"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Czy chcesz usunąć źródło danych "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1532"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>Grupy korzystające z tego źródła zostaną przeniesione do źródła domyślnego. Tej operacji nie można cofnąć.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1809"/>
        <source>Save Serial Studio Project</source>
        <translation>Zapisz Projekt Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1811"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2468"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Pliki Projektu Serial Studio (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1832"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2070"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2459"/>
        <source>Untitled Project</source>
        <translation>Projekt Bez Tytułu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2083"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2614"/>
        <source>Device A</source>
        <translation>Urządzenie A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2295"/>
        <source>Select Project File</source>
        <translation>Wybierz Plik Projektu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2297"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>Pliki Projektu (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2341"/>
        <source>JSON validation error</source>
        <translation>Błąd walidacji JSON</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2433"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Projekt zaktualizowany ze starszego formatu pliku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2434"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Ten projekt zapisano z wersją schematu %1; bieżąca wersja to %2. Domyślne wartości zostały zastosowane do nowych pól. Zapisz projekt, aby zatwierdzić aktualizację.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2466"/>
        <source>Save Imported Project</source>
        <translation>Zapisz Zaimportowany Projekt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2658"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>Projekty wieloźródłowe wymagają licencji Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2659"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Ten projekt zawiera wiele źródeł danych. Załadowano tylko pierwsze źródło. Licencja Serial Studio Pro jest wymagana do korzystania z projektów wieloźródłowych.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2896"/>
        <source>Workspace IDs remapped on load</source>
        <translation>Identyfikatory przestrzeni roboczych zmapowane ponownie przy wczytywaniu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2897"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 własnych identyfikatorów obszarów roboczych pokrywało się z nowym zarezerwowanym zakresem automatycznym i zostało przeniesionych do zakresu użytkownika. Zapisz projekt, aby utrwalić mapowanie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3080"/>
        <source>Legacy frame parser function updated</source>
        <translation>Zaktualizowano starszą funkcję parsera ramek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3081"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Twój projekt używał starszej funkcji parsera ramek z argumentem 'separator'. Została automatycznie zmigrowana do nowego formatu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3322"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Czy chcesz usunąć grupę "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3323"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3368"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3400"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4138"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Tej operacji nie można cofnąć. Czy chcesz kontynuować?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3367"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Czy chcesz usunąć akcję "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3399"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Czy chcesz usunąć zbiór danych "%1"?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Kopia)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4050"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4086"/>
        <source>Output Controls</source>
        <translation>Sterowanie Wyjściem</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4098"/>
        <source>New Button</source>
        <translation>Nowy Przycisk</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4101"/>
        <source>New Slider</source>
        <translation>Nowy Suwak</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4104"/>
        <source>New Toggle</source>
        <translation>Nowy Przełącznik</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4107"/>
        <source>New Text Field</source>
        <translation>Nowe Pole Tekstowe</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4110"/>
        <source>New Knob</source>
        <translation>Nowe Pokrętło</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4137"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Czy chcesz usunąć widget wyjściowy "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4302"/>
        <source>Group</source>
        <translation>Grupa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4320"/>
        <source>New Dataset</source>
        <translation>Nowy Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4323"/>
        <source>New Plot</source>
        <translation>Nowy Wykres</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4327"/>
        <source>New FFT Plot</source>
        <translation>Nowy Wykres FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4331"/>
        <source>New Level Indicator</source>
        <translation>Nowy Wskaźnik Poziomu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4335"/>
        <source>New Gauge</source>
        <translation>Nowy Wskaźnik</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4339"/>
        <source>New Compass</source>
        <translation>Nowy Kompas</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4345"/>
        <source>New Meter</source>
        <translation>Nowy Miernik</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Nowy Termometr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4349"/>
        <source>New LED Indicator</source>
        <translation>Nowy Wskaźnik LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4353"/>
        <source>New Waterfall</source>
        <translation>Nowy Wykres Wodospadowy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4421"/>
        <source>Channel %1</source>
        <translation>Kanał %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4492"/>
        <source>New Action</source>
        <translation>Nowa Akcja</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4633"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Czy na pewno chcesz zmienić widget na poziomie grupy?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4635"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>Istniejące zbiory danych dla tej grupy zostaną usunięte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4703"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4704"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4705"/>
        <source>Accelerometer %1</source>
        <translation>Akcelerometr %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4720"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4720"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4720"/>
        <source>Gyro %1</source>
        <translation>Żyroskop %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4735"/>
        <source>Latitude</source>
        <translation>Szerokość Geograficzna</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4735"/>
        <source>Longitude</source>
        <translation>Długość Geograficzna</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4735"/>
        <source>Altitude</source>
        <translation>Wysokość</translation>
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
        <translation>Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5211"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5417"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6679"/>
        <source>Shared Table</source>
        <translation>Tabela Współdzielona</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5288"/>
        <source>register</source>
        <translation>rejestr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5417"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6679"/>
        <source>New Shared Table</source>
        <translation>Nowa Tabela Współdzielona</translation>
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
        <translation>Nazwa:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5440"/>
        <source>Rename Table</source>
        <translation>Zmień Nazwę Tabeli</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5459"/>
        <source>Rename Group</source>
        <translation>Zmień Nazwę Grupy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5483"/>
        <source>Rename Dataset</source>
        <translation>Zmień Nazwę Zbioru Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5510"/>
        <source>Rename Data Source</source>
        <translation>Zmień Nazwę Źródła Danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5529"/>
        <source>Rename Action</source>
        <translation>Zmień Nazwę Akcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5550"/>
        <source>New Register</source>
        <translation>Nowy Rejestr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5573"/>
        <source>Rename Register</source>
        <translation>Zmień Nazwę Rejestru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5607"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5632"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7341"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8110"/>
        <source>This action cannot be undone.</source>
        <translation>Ta operacja jest nieodwracalna.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5608"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Spowoduje to usunięcie %1 rejestr(ów) wraz z tabelą. Ta operacja jest nieodwracalna.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5611"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5631"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7340"/>
        <source>Delete "%1"?</source>
        <translation>Usunąć "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5614"/>
        <source>Delete Table</source>
        <translation>Usuń Tabelę</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5634"/>
        <source>Delete Register</source>
        <translation>Usuń Rejestr</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5656"/>
        <source>Export Table</source>
        <translation>Eksportuj Tabelę</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5658"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5699"/>
        <source>CSV files (*.csv)</source>
        <translation>Pliki CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5697"/>
        <source>Import Table</source>
        <translation>Importuj Tabelę</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5948"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6220"/>
        <source>New Workspace</source>
        <translation>Nowy Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5969"/>
        <source>Rename Workspace</source>
        <translation>Zmień Nazwę Obszaru Roboczego</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6015"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6203"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6303"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6437"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6521"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6662"/>
        <source>Folder</source>
        <translation>Folder</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6203"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6437"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6662"/>
        <source>New Folder</source>
        <translation>Nowy Folder</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6242"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6458"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6700"/>
        <source>Rename Folder</source>
        <translation>Zmień Nazwę Folderu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6260"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6476"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6718"/>
        <source>Delete folder "%1"?</source>
        <translation>Usunąć folder „%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6261"/>
        <source>The folder is removed; its workspaces and sub-folders move up to the parent.</source>
        <translation>Folder zostanie usunięty; jego obszary robocze i podfoldery zostaną przeniesione do folderu nadrzędnego.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6263"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6479"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6722"/>
        <source>Delete Folder</source>
        <translation>Usuń Folder</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6477"/>
        <source>The folder is removed; its groups and sub-folders move up to the parent.</source>
        <translation>Folder zostanie usunięty; jego grupy i podfoldery zostaną przeniesione do folderu nadrzędnego.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6719"/>
        <source>The folder is removed; its tables and sub-folders move up to the parent. The accessor path of those tables changes accordingly.</source>
        <translation>Folder zostaje usunięty; jego tabele i podfoldery przenoszą się do folderu nadrzędnego. Ścieżka dostępu tych tabel zmienia się odpowiednio.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6807"/>
        <source>Overview</source>
        <translation>Przegląd</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6816"/>
        <source>All Data</source>
        <translation>Wszystkie Dane</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7109"/>
        <source>Discard workspace customisations?</source>
        <translation>Odrzucić dostosowania obszarów roboczych?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7110"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Wyłączenie dostosowywania odrzuci zmiany i odbuduje listę obszarów roboczych na podstawie grup projektu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7113"/>
        <source>Customize Workspaces</source>
        <translation>Dostosuj Obszary Robocze</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7343"/>
        <source>Delete Workspace</source>
        <translation>Usuń Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7724"/>
        <source>Project file removed from disk</source>
        <translation>Plik projektu usunięty z dysku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7725"/>
        <source>%1 was deleted or renamed by another program. Save the project to recreate it.</source>
        <translation>%1 został usunięty lub przemianowany przez inny program. Zapisz projekt, aby go odtworzyć.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7746"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7767"/>
        <source>Project file changed on disk</source>
        <translation>Plik projektu zmieniony na dysku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7747"/>
        <source>%1 was modified by another program. The in-memory project was kept; reopen the file to load the external changes.</source>
        <translation>%1 został zmodyfikowany przez inny program. Projekt w pamięci został zachowany; otwórz ponownie plik, aby załadować zewnętrzne zmiany.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7763"/>
        <source>The project file was modified by another program.

Reload it and discard your unsaved changes?</source>
        <translation>Plik projektu został zmodyfikowany przez inny program.

Przeładować go i odrzucić niezapisane zmiany?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7765"/>
        <source>The project file was modified by another program.

Reload it?</source>
        <translation>Plik projektu został zmodyfikowany przez inny program.

Przeładować go?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7796"/>
        <source>File save error</source>
        <translation>Błąd zapisu pliku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8109"/>
        <source>Delete %1 selected items?</source>
        <translation>Usunąć %1 zaznaczonych elementów?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8112"/>
        <source>Delete Items</source>
        <translation>Usuń Elementy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2502"/>
        <source>File open error</source>
        <translation>Błąd otwierania pliku</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Importuj Plik Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Pliki Proto (*.proto);;Wszystkie Pliki (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Nie udało się otworzyć pliku proto: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Sprawdź ścieżkę i uprawnienia do odczytu pliku, a następnie spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Błąd Importu Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Plik proto jest za duży (limit wynosi 10 MB).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>Sprawdź, czy wybrano prawidłowy plik definicji .proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Nie udało się przetworzyć pliku proto w linii %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>Obsługiwana jest tylko składnia proto3. Sprawdź format pliku i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Plik proto nie zawiera definicji komunikatów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Wybrany plik nie zawiera bloków `message` do zaimportowania.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Ostrzeżenie Importu Protobuf</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Nie udało się załadować zaimportowanego projektu</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Wygenerowany JSON projektu nie mógł zostać załadowany.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>Pomyślnie zaimportowano %1 komunikat(ów) i %2 pol(e/a) z pliku proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>Edytor projektu jest teraz otwarty do dostosowania.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Import Protobuf Zakończony</translation>
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
        <translation>Nieprawidłowe Wejście HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Wprowadź prawidłowe bajty szesnastkowe.

Prawidłowy format: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Brak kodu funkcji transmisji do oceny.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>funkcja transmisji nie jest wywoływalna</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>Oceń</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>Wartość Wejściowa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>Wyjście Funkcji Transmisji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>Wprowadź wartość do transmisji…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>Surowe wyjście tekstowe pojawia się tutaj</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>Wyjście bajtów hex pojawia się tutaj</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>Testuj Funkcję Transmisji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Wprowadź bajty hex (np. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(puste) Brak zwróconych danych</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="369"/>
        <source>0 bytes</source>
        <translation>0 bajtów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="408"/>
        <source>%1 byte(s)</source>
        <translation>%1 bajt(ów)</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>Pamięć Współdzielona</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="167"/>
        <source>Add Folder</source>
        <translation>Dodaj Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="169"/>
        <source>Add a top-level folder</source>
        <translation>Dodaj folder najwyższego poziomu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="177"/>
        <source>Add Shared Table</source>
        <translation>Dodaj Tabelę Współdzieloną</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="179"/>
        <source>Add shared table</source>
        <translation>Dodaj tabelę współdzieloną</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="188"/>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="193"/>
        <source>Open help documentation for shared memory</source>
        <translation>Otwórz dokumentację pomocy dla pamięci współdzielonej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="202"/>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="203"/>
        <source>Registers</source>
        <translation>Rejestry</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="vanished">Nazwa</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="vanished">Opis</translation>
    </message>
    <message>
        <source>Entries</source>
        <translation type="vanished">Wpisy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="297"/>
        <source>No shared tables.</source>
        <translation>Brak tabel współdzielonych.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Sesje</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Otwórz</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Otwórz plik sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Zamknij plik sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Odtwórz</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Odtwórz wybraną sesję na pulpicie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Odblokuj plik sesji, aby usunąć sesje</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Usuń wybraną sesję</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Odblokuj</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Zablokuj</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Odblokuj plik sesji, aby umożliwić usuwanie</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Ustaw hasło, aby zapobiec usuwaniu sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>Eksportuj CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Eksportuj wybraną sesję do CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>Eksportuj PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>Wygeneruj raport PDF dla wybranej sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Przywróć Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Przywróć plik projektu z tego pliku sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Ładowanie sesji…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>Przetwarzanie…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="112"/>
        <source>Pro features detected in this project.</source>
        <translation>Wykryto funkcje Pro w tym projekcie.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="114"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Używane są widżety zastępcze. Kup licencję, aby odblokować pełną funkcjonalność.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="142"/>
        <source>Plots</source>
        <translation>Wykresy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="147"/>
        <source>Plot</source>
        <translation>Wykres</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="151"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>Przełącz wizualizację wykresu 2D dla tego zbioru danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>FFT Plot</source>
        <translation>Wykres FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="166"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>Przełącz wykres FFT, aby wizualizować zawartość częstotliwościową</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="178"/>
        <source>Waterfall</source>
        <translation>Wykres Wodospadowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="182"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Przełącz wykres wodospadowy (spektrogram) — używa ustawień FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="199"/>
        <source>Widgets</source>
        <translation>Widżety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="205"/>
        <source>Bar/Level</source>
        <translation>Słupek/poziom</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="209"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Przełącz wskaźnik słupka/poziomu dla tego zbioru danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="220"/>
        <source>Gauge</source>
        <translation>Wskaźnik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="225"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Przełącz widget wskaźnika dla wyświetlania w stylu analogowym</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="237"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="241"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Przełącz widget kompasu dla danych kierunkowych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="252"/>
        <source>Meter</source>
        <translation>Miernik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="257"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Przełącz widget miernika analogowego (półokrągłego)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="308"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>Zdefiniuj kolorowe zakresy wartości z poziomami ważności dla wskaźnika lub LED tego zbioru danych.</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometr</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Przełącz widget termometru dla danych temperatury</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="268"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="273"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>Przełącz wskaźnik LED dla wartości binarnych lub progowych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="293"/>
        <source>Behavior</source>
        <translation>Zachowanie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="298"/>
        <source>Alarm Bands</source>
        <translation>Pasma Alarmowe</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">Zdefiniuj kolorowe zakresy wartości z poziomami ważności dla wskaźnika tego zbioru danych.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Transform</source>
        <translation>Transformacja</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="318"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Edytuj wyrażenie transformacji wartości do kalibracji, filtrowania lub konwersji jednostek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="331"/>
        <source>Duplicate</source>
        <translation>Duplikuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="336"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Duplikuj ten zestaw danych z tą samą konfiguracją</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="344"/>
        <source>Delete this dataset from the group</source>
        <translation>Usuń ten zestaw danych z grupy</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>Wspieraj Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>Wspieraj rozwój %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio to darmowe oprogramowanie open-source wspierane przez wolontariuszy. Rozważ darowiznę lub zakup licencji Pro, aby wesprzeć prace rozwojowe :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Możesz również wspierać ten projekt, udostępniając go, zgłaszając błędy i proponując nowe funkcje!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>Wspomóż</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>Zdobądź Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="142"/>
        <source>Stop</source>
        <translation>Zatrzymaj</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="143"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="377"/>
        <source>Downloading updates</source>
        <translation>Pobieranie aktualizacji</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="470"/>
        <source>Time remaining</source>
        <translation>Pozostały czas</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <source>unknown</source>
        <translation>nieznany</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Cannot find downloaded update!</source>
        <translation>Nie można znaleźć pobranej aktualizacji!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="260"/>
        <source>Download complete!</source>
        <translation>Pobieranie zakończone!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="268"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Kliknij „OK", aby rozpocząć instalację aktualizacji</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="270"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>Aby zainstalować aktualizację, może być konieczne zamknięcie aplikacji.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="261"/>
        <source>The installer opens separately</source>
        <translation>Instalator otwiera się osobno</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="274"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>Aby zainstalować aktualizację, może być konieczne zamknięcie aplikacji. To jest obowiązkowa aktualizacja — zamknięcie spowoduje zakończenie aplikacji.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="290"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Kliknij przycisk „Otwórz", aby zastosować aktualizację</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="303"/>
        <source>Updater</source>
        <translation>Aktualizator</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="307"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Anulować pobieranie?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="309"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Anulować pobieranie? To jest obowiązkowa aktualizacja — zamknięcie spowoduje zakończenie aplikacji.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="364"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="371"/>
        <source>%1 bytes</source>
        <translation>%1 bajtów</translation>
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
        <translation>z</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="421"/>
        <source>Downloading Updates</source>
        <translation>Pobieranie Aktualizacji</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Time Remaining</source>
        <translation>Pozostały Czas</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Unknown</source>
        <translation>Nieznany</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="446"/>
        <source>about %1 hours</source>
        <translation>około %1 godzin</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="448"/>
        <source>about one hour</source>
        <translation>około godziny</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="456"/>
        <source>%1 minutes</source>
        <translation>%1 minut</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="458"/>
        <source>1 minute</source>
        <translation>1 minuta</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="465"/>
        <source>%1 seconds</source>
        <translation>%1 sekund</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="467"/>
        <source>1 second</source>
        <translation>1 sekunda</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Pozostały czas: 0 minut</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Otwórz</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>Przeglądarka Przykładów</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>Wstecz</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Pobierz i Otwórz</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>Zobacz na GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>Szukaj w Przykładach…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="245"/>
        <source>Fetching examples…</source>
        <translation>Pobieranie przykładów…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>Loading...</source>
        <translation>Ładowanie…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="569"/>
        <source>No README available.</source>
        <translation>Brak dostępnego README.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="609"/>
        <source>Copied to Clipboard</source>
        <translation>Skopiowano do Schowka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="672"/>
        <source>No screenshot available</source>
        <translation>Brak dostępnego zrzutu ekranu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="704"/>
        <source>Details</source>
        <translation>Szczegóły</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="733"/>
        <source>Info</source>
        <translation>Informacje</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="756"/>
        <source>Category:</source>
        <translation>Kategoria:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="769"/>
        <source>Difficulty:</source>
        <translation>Trudność:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="787"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="829"/>
        <source>No Results Found</source>
        <translation>Nie Znaleziono Wyników</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="840"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Sprawdź pisownię lub spróbuj innego wyszukiwanego hasła.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="855"/>
        <source>%1 examples</source>
        <translation>Przykłady: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="864"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>Menedżer Rozszerzeń</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>Odśwież</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>Repozytoria</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>Ustawienia Repozytorium</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>Wstecz</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>Zainstaluj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>Odinstaluj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>Uruchom</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>Zatrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>Szukaj rozszerzeń…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>Pobieranie rozszerzeń…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>Uruchomione</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>Aktualizuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>Zainstalowane</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>Niedostępne</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>Brak dostępnego opisu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>Szczegóły</translation>
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
        <translation>Wersja:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>Licencja:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>Brak podglądu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>WYJŚCIE WTYCZKI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Brak wyjścia. Uruchom wtyczkę, aby zobaczyć jej dziennik tutaj.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>Podgląd niedostępny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>Repozytoria</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>Dodaj adresy URL do zdalnych repozytoriów lub ścieżki do folderów lokalnych.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>LOKALNY</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL lub ścieżka lokalna…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>Przeglądaj…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>Dodaj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>Nie Znaleziono Wyników</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Sprawdź pisownię lub spróbuj innego wyszukiwanego hasła.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>Brak Dostępnych Rozszerzeń</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Dodaj URL repozytorium lub ścieżkę lokalną w ustawieniach Repozytoriów, a następnie odśwież.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 rozszerzeń</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="167"/>
        <source>Interpolation: %1</source>
        <translation>Interpolacja: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="195"/>
        <source>Show Area Under Plot</source>
        <translation>Pokaż Obszar Pod Wykresem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="213"/>
        <source>Show X Axis Label</source>
        <translation>Pokaż Etykietę Osi X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="225"/>
        <source>Show Y Axis Label</source>
        <translation>Pokaż Etykietę Osi Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="243"/>
        <source>Show Crosshair</source>
        <translation>Pokaż Celownik</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Resume</source>
        <translation>Wznów</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="269"/>
        <source>Reset View</source>
        <translation>Resetuj Widok</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="275"/>
        <source>Axis Range Settings</source>
        <translation>Ustawienia Zakresu Osi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="304"/>
        <source>Magnitude (dB)</source>
        <translation>Amplituda (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="305"/>
        <source>Frequency (Hz)</source>
        <translation>Częstotliwość (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Upuść tutaj pliki projektów i CSV</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>Transmisja Pliku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>Protokół Transferu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>Wybór Pliku:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>Wybierz Plik…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>Interwał Transmisji:</translation>
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
        <translation>Rozmiar Bloku:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>bajtów</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>Limit Czasu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>Maks. Prób:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>Postęp: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 bajtów</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>Błędy: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>Dziennik Aktywności</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>Wstrzymaj Transmisję</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>Wznów Transmisję</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>Zatrzymaj Transmisję</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>Rozpocznij Transmisję</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>Parser Ramek</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>Urządzenie %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="399"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1446"/>
        <source>Output Panel</source>
        <translation>Panel Wyjściowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="443"/>
        <source>Control</source>
        <translation>Sterowanie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="466"/>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="482"/>
        <source>%1 regs</source>
        <translation>%1 rej.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="483"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="505"/>
        <source>empty</source>
        <translation>puste</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Skrypt Sterujący</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="528"/>
        <source>MQTT Publisher</source>
        <translation>Wydawca MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="931"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Otwórz edytor kodu transformacji dla tego zestawu danych.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1216"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1219"/>
        <source>Dataset Container</source>
        <translation>Kontener Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1228"/>
        <source>Multi-Plot</source>
        <translation>Wykres Wielokrotny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1231"/>
        <source>Multiple Plot</source>
        <translation>Wykres Wielokrotny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1240"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <source>Accelerometer</source>
        <translation>Akcelerometr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1252"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1255"/>
        <source>Gyroscope</source>
        <translation>Żyroskop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1264"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1267"/>
        <source>GPS Map</source>
        <translation>Mapa GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1275"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <source>3D Plot</source>
        <translation>Wykres 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1286"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1289"/>
        <source>Image View</source>
        <translation>Widok Obrazu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1298"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1301"/>
        <source>Painter Widget</source>
        <translation>Widget Malarski</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1310"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1313"/>
        <source>Web View</source>
        <translation>Widok Internetowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1322"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1325"/>
        <source>Data Grid</source>
        <translation>Siatka Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1338"/>
        <source>Generic</source>
        <translation>Ogólny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1351"/>
        <source>Plot</source>
        <translation>Wykres</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1364"/>
        <source>FFT Plot</source>
        <translation>Wykres FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1377"/>
        <source>Gauge</source>
        <translation>Wskaźnik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1390"/>
        <source>Level Indicator</source>
        <translation>Wskaźnik Poziomu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1403"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1416"/>
        <source>Meter</source>
        <translation>Miernik</translation>
    </message>
    <message>
        <source>Edit Control Script…</source>
        <translation type="vanished">Edytuj Skrypt Sterujący…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="503"/>
        <source>Control Loop</source>
        <translation>Pętla Sterowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1429"/>
        <source>LED Indicator</source>
        <translation>Wskaźnik LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1458"/>
        <source>Slider</source>
        <translation>Suwak</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1471"/>
        <source>Toggle</source>
        <translation>Przełącznik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1484"/>
        <source>Knob</source>
        <translation>Pokrętło</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1497"/>
        <source>Text Field</source>
        <translation>Pole Tekstowe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1510"/>
        <source>Button</source>
        <translation>Przycisk</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1534"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1610"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1698"/>
        <source>Add Group</source>
        <translation>Dodaj Grupę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1550"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1714"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1759"/>
        <source>Add Dataset</source>
        <translation>Dodaj Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1564"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1640"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1728"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1773"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1980"/>
        <source>Add Output</source>
        <translation>Dodaj Wyjście</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1653"/>
        <source>Add Action</source>
        <translation>Dodaj Akcję</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1589"/>
        <source>Add Data Source</source>
        <translation>Dodaj Źródło Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1596"/>
        <source>Add Data Table</source>
        <translation>Dodaj Tabelę Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1664"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1800"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1867"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1995"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2029"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2085"/>
        <source>Rename…</source>
        <translation>Zmień Nazwę…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1672"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1830"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1900"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1952"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2003"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2059"/>
        <source>Duplicate</source>
        <translation>Duplikuj</translation>
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
        <translation>Usuń…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1744"/>
        <source>Edit Frame Parser…</source>
        <translation>Edytuj Analizator Ramek…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1786"/>
        <source>Edit Painter Code…</source>
        <translation>Edytuj Kod Painter…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1808"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1876"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1928"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2037"/>
        <source>Move Up</source>
        <translation>Przenieś w Górę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1819"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1888"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1940"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2048"/>
        <source>Move Down</source>
        <translation>Przenieś w Dół</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1856"/>
        <source>Edit Transform Code…</source>
        <translation>Edytuj Kod Transformacji…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2111"/>
        <source>Edit Code…</source>
        <translation>Edytuj Kod…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2126"/>
        <source>Edit Control Loop…</source>
        <translation>Edytuj Pętle Sterowania…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>Grupa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="349"/>
        <source>Action</source>
        <translation>Akcja</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">Nie zdefiniowano jeszcze żadnych grup</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Testuj Parser Ramki</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>Ramka %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>Dekoder</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>Wiersze</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 wiersz(y)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Wiersz %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Konfiguracja Potoku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Wykrywanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Start</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Koniec</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Suma Kontrolna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Ograniczniki Szesnastkowe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Wprowadzanie Danych Ramki</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Wprowadź bajty szesnastkowe (np. 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Wprowadź tutaj surowe bajty strumienia...</translation>
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
        <translation>Próbka nie zawiera skonfigurowanych ograniczników ramki, więc żadna ramka nie zostanie wyodrębniona. Wpisz je do próbki (np. 
 dla nowej linii) lub dostosuj tryb wykrywania.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Wyniki Potoku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Etap</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Wartość</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>Ekstrakcja nie wygenerowała kompletnej ramki. Sprawdź ograniczniki początkowy/końcowy oraz tryb wykrywania.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Wprowadź przykładowe dane powyżej i naciśnij Oceń, aby wyświetlić podgląd przetworzonego wyjścia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Oceń</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Cofnij</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Ponów</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Wytnij</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Kopiuj</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Wklej</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Zaznacz Wszystko</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Formatuj Dokument</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Formatuj Zaznaczenie</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Resetuj</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Resetuj do domyślnego skryptu parsowania</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Otwórz</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Importuj plik skryptu do parsowania danych</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Otwórz dokumentację pomocy dla parsowania danych</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Język:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Wybierz Szablon…</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Testuj z Przykładowymi Danymi</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Oceń</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Cofnij ostatnią edycję kodu</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Ponów poprzednio cofniętą edycję</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Wytnij zaznaczony kod do schowka</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Skopiuj Zaznaczony Kod do Schowka</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Wklej Kod ze Schowka</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Pomoc</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Autocentrowanie</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Rysuj Trajektorię</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Pomniejsz</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Pokaż Pogodę</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>Nakładka Pogodowa NASA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Mapa Bazowa: %1</translation>
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
        <translation>Dodaj Podfolder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="134"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Dodaj podfolder wewnątrz tego folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="142"/>
        <source>Add Group</source>
        <translation>Dodaj Grupę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="144"/>
        <source>Add a group inside this folder</source>
        <translation>Dodaj grupę wewnątrz tego folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="154"/>
        <source>Rename</source>
        <translation>Zmień Nazwę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="156"/>
        <source>Rename folder</source>
        <translation>Zmień nazwę folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="164"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="166"/>
        <source>Delete folder</source>
        <translation>Usuń folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="179"/>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="180"/>
        <source>Contents</source>
        <translation>Zawartość</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="270"/>
        <source>This folder is empty. Use the toolbar to add a group or sub-folder.</source>
        <translation>Ten folder jest pusty. Użyj paska narzędzi, aby dodać grupę lub podfolder.</translation>
    </message>
</context>
<context>
    <name>GroupTemplateMenu</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="43"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="45"/>
        <source>Dataset Container</source>
        <translation>Kontener Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="51"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="53"/>
        <source>Data Grid</source>
        <translation>Siatka Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="59"/>
        <source>Multi-Plot</source>
        <translation>Wykres Wielokrotny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="61"/>
        <source>Multiple Plot</source>
        <translation>Wykres Wielokrotny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="67"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="69"/>
        <source>3D Plot</source>
        <translation>Wykres 3D</translation>
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
        <translation>Żyroskop</translation>
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
        <translation>Widok Obrazu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="107"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="109"/>
        <source>Web View</source>
        <translation>Widok Internetowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="115"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="117"/>
        <source>Painter Widget</source>
        <translation>Widget Malarski</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="100"/>
        <source>Pro features detected in this project.</source>
        <translation>Wykryto funkcje Pro w tym projekcie.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="102"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Używane są widżety zastępcze. Kup licencję, aby odblokować pełną funkcjonalność.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="132"/>
        <source>Add Dataset</source>
        <translation>Dodaj Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Dataset</source>
        <translation>Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="142"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Dodaj ogólny zestaw danych do bieżącej grupy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="149"/>
        <source>Plot</source>
        <translation>Wykres</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="153"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>Dodaj wykres 2D do wizualizacji danych numerycznych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="161"/>
        <source>FFT Plot</source>
        <translation>Wykres FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="166"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>Dodaj wykres FFT do wizualizacji w dziedzinie częstotliwości</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="172"/>
        <source>Waterfall</source>
        <translation>Wykres Wodospadowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="177"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Dodaj wykres wodospadowy (spektrogram) — używa ustawień FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="183"/>
        <source>Bar/Level</source>
        <translation>Słupek/poziom</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="187"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Dodaj wskaźnik słupkowy lub poziomowy dla wartości skalowanych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Wskaźnik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="198"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Dodaj widget wskaźnika dla wizualizacji w stylu analogowym</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="205"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="209"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Dodaj kompas do wyświetlania danych kierunkowych lub kątowych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="215"/>
        <source>Meter</source>
        <translation>Miernik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="219"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Dodaj widget miernika analogowego (półokrągłego)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Termometr</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Dodaj widget termometru dla danych temperatury</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="226"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="231"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>Dodaj wskaźnik LED dla binarnych sygnałów stanu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="244"/>
        <source>Add Control</source>
        <translation>Dodaj Kontrolkę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="250"/>
        <source>Button</source>
        <translation>Przycisk</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="254"/>
        <source>Add a button that sends a command on click</source>
        <translation>Dodaj przycisk wysyłający polecenie po kliknięciu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="260"/>
        <source>Slider</source>
        <translation>Suwak</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="264"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Dodaj suwak do wysyłania skalowanych wartości numerycznych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="270"/>
        <source>Toggle</source>
        <translation>Przełącznik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="274"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Dodaj przełącznik do poleceń włączania/wyłączania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Text Field</source>
        <translation>Pole Tekstowe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="284"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Dodaj pole tekstowe do wpisywania i wysyłania poleceń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="290"/>
        <source>Knob</source>
        <translation>Pokrętło</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="294"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Dodaj pokrętło obrotowe do sterowania wartością zadaną</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="313"/>
        <source>Edit Code</source>
        <translation>Edytuj Kod</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="317"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Edytuj kod JavaScript rysujący ten widget malarski</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="330"/>
        <source>Duplicate</source>
        <translation>Duplikuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="334"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Duplikuj bieżącą grupę i jej zawartość</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="339"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="344"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Usuń bieżącą grupę i wszystkie zawarte zbiory danych</translation>
    </message>
</context>
<context>
    <name>GroupsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="33"/>
        <source>Dashboard Widgets</source>
        <translation>Widżety Pulpitu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="127"/>
        <source>Add Folder</source>
        <translation>Dodaj Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="129"/>
        <source>Add a top-level folder</source>
        <translation>Dodaj folder najwyższego poziomu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="137"/>
        <source>Add Group</source>
        <translation>Dodaj Grupę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="139"/>
        <source>Add a group from a template</source>
        <translation>Dodaj grupę z szablonu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="154"/>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="155"/>
        <source>Contents</source>
        <translation>Zawartość</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="245"/>
        <source>No groups yet. Use the toolbar to add a group or a folder.</source>
        <translation>Brak grup. Użyj paska narzędzi, aby dodać grupę lub folder.</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="410"/>
        <source>ROLL ↔</source>
        <translation>PRZECHYLENIE ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="440"/>
        <source>YAW ↻</source>
        <translation>ODCHYLENIE ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="470"/>
        <source>PITCH ↕</source>
        <translation>POCHYLENIE ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>Urządzenie HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Strona Użycia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Użycie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Podłącz gamepady, joysticki, kierownice, kontrolery lotnicze i inne urządzenia USB klasy HID.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>Tabele Użycia HID (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>Centrum Pomocy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>Pobieranie stron pomocy…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>Szukaj…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>Ładowanie…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="291"/>
        <source>Select a page from the sidebar</source>
        <translation>Wybierz stronę z panelu bocznego</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="321"/>
        <source>Copied to Clipboard</source>
        <translation>Skopiowano do Schowka</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="353"/>
        <source>View Online</source>
        <translation>Wyświetl Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="372"/>
        <source>%1 pages</source>
        <translation>Strony: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="381"/>
        <source>Close</source>
        <translation>Zamknij</translation>
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
        <translation>Gniazdo Sieciowe</translation>
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
        <translation>Urządzenie USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>HID Device</source>
        <translation>Urządzenie HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>MQTT Subscriber</source>
        <translation>Subskrybent MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="685"/>
        <source>Your trial period has ended.</source>
        <translation>Okres próbny zakończony.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="686"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Aby kontynuować korzystanie z Serial Studio, aktywuj licencję.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="752"/>
        <source>channels</source>
        <translation>kanały</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="805"/>
        <source> channels</source>
        <translation>kanały</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="953"/>
        <source>Unsigned 8-bit</source>
        <translation>Bez Znaku 8-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="954"/>
        <source>Signed 16-bit</source>
        <translation>Ze Znakiem 16-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="955"/>
        <source>Signed 24-bit</source>
        <translation>Ze Znakiem 24-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="956"/>
        <source>Signed 32-bit</source>
        <translation>Ze Znakiem 32-bit</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="957"/>
        <source>Float 32-bit</source>
        <translation>Zmiennoprzecinkowy 32-bit</translation>
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
        <translation>Urządzenie Wejściowe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1350"/>
        <source>Sample Rate</source>
        <translation>Częstotliwość Próbkowania</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1358"/>
        <source>Sample Format</source>
        <translation>Format Próbki</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1366"/>
        <source>Channels</source>
        <translation>Kanały</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>Błąd Modułu We/wy BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>Wybierz Urządzenie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>Wybierz Usługę</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>Wybierz Charakterystykę</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>Błąd podczas konfiguracji usługi BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>Błąd operacji</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>Błąd zapisu charakterystyki</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>Błąd zapisu deskryptora</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>Nieznany błąd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>Błąd odczytu charakterystyki</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>Błąd odczytu deskryptora</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>Urządzenie BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>Usługa</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>Charakterystyka Powiadomień</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>Charakterystyka</translation>
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
        <translation>Magistrala CAN Niedostępna</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="331"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>Nie znaleziono wtyczek magistrali CAN w tym systemie.

Obsługa magistrali CAN w macOS jest ograniczona i może wymagać sterowników sprzętowych firm trzecich.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="336"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Brak dostępnych wtyczek magistrali CAN na tej platformie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="348"/>
        <source>Invalid CAN Configuration</source>
        <translation>Nieprawidłowa Konfiguracja CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="356"/>
        <source>Invalid Selection</source>
        <translation>Nieprawidłowy Wybór</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="365"/>
        <source>No Devices Available</source>
        <translation>Brak Dostępnych Urządzeń</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="274"/>
        <source>CAN Device Creation Failed</source>
        <translation>Tworzenie Urządzenia CAN Nie Powiodło Się</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="301"/>
        <source>CAN Connection Failed</source>
        <translation>Połączenie CAN Nie Powiodło Się</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="319"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>Nie znaleziono wtyczek magistrali CAN w tym systemie.

W systemie Linux upewnij się, że moduły jądra SOCKETCAN są załadowane.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="325"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>Nie znaleziono wtyczek magistrali CAN w tym systemie.

W systemie Windows zainstaluj sterowniki sprzętowe CAN (PEAK, VECTOR itp.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="349"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>Konfiguracja magistrali CAN jest niekompletna. Wybierz prawidłową wtyczkę i interfejs.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="357"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>Wybrana wtyczka lub interfejs nie jest już dostępny. Odśwież listy i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="366"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>Lista wtyczek lub interfejsów jest pusta. Odśwież listy i upewnij się, że sprzęt CAN jest podłączony.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="275"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>Nie można utworzyć urządzenia magistrali CAN. Sprawdź sprzęt i sterowniki.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Nie można połączyć się z urządzeniem magistrali CAN. Sprawdź połączenie sprzętowe i ustawienia.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="695"/>
        <source>CAN Bus Error</source>
        <translation>Błąd Magistrali CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="696"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>Wystąpił błąd, ale urządzenie CAN nie jest już dostępne.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="703"/>
        <source>Error code: %1</source>
        <translation>Kod błędu: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="706"/>
        <source>CAN Bus Communication Error</source>
        <translation>Błąd Komunikacji Magistrali CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="785"/>
        <source>No CAN driver selected</source>
        <translation>Nie wybrano sterownika CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="725"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>Najpierw załaduj moduły jądra SOCKETCAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="721"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>Podłącz adapter %1, a następnie odśwież</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="728"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Najpierw skonfiguruj wirtualny interfejs CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="730"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="750"/>
        <source>No interfaces found for %1</source>
        <translation>Nie znaleziono interfejsów dla %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="734"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>Zainstaluj &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;sterowniki PEAK CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="738"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>Zainstaluj &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;sterowniki VECTOR CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="742"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>Zainstaluj &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;sterowniki SysTec CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="745"/>
        <source>Install %1 drivers</source>
        <translation>Zainstaluj sterowniki %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="748"/>
        <source>Install %1 drivers for macOS</source>
        <translation>Zainstaluj sterowniki %1 dla macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="762"/>
        <source>

If the interface is down, bring it up first:
sudo ip link set %1 up type can bitrate %2</source>
        <translation>Jeśli interfejs jest wyłączony, najpierw go włącz:
sudo ip link set %1 up type can bitrate %2

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="869"/>
        <source>Plugin</source>
        <translation>Wtyczka</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="877"/>
        <source>Interface</source>
        <translation>Interfejs</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="885"/>
        <source>Bitrate</source>
        <translation>Przepływność</translation>
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
        <translation>Tylko Nasłuch</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="382"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>Nie udało się zainicjować libusb dla adaptera CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="410"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>Nie można wyliczyć urządzeń USB.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="430"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>Wybrany adapter CANable nie jest już podłączony lub inna aplikacja go używa. W systemie Windows urządzenie musi używać sterownika WinUSB (candleLight instaluje go automatycznie).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="443"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>Nie można przejąć interfejsu USB CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="510"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>Adapter CANable nie jest otwarty do zapisu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="545"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>Nie udało się przesłać ramki CAN do adaptera.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="561"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>Błąd magistrali CAN zgłoszony przez adapter CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="616"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>Ramka CAN nie została potwierdzona na magistrali.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="715"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>Adapter CANable odrzucił uzgadnianie formatu hosta.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="722"/>
        <source>Could not read CANable timing constants.</source>
        <translation>Nie można odczytać stałych czasowych CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="728"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>Przepływność %1 bps nie jest obsługiwana przez ten adapter CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="734"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>Adapter CANable odrzucił żądaną przepływność.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="748"/>
        <source>Could not start the CANable channel.</source>
        <translation>Nie można uruchomić kanału CANable.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>Nieznany błąd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Sprawdź, czy użytkownik należy do grupy 'plugdev' lub czy reguła udev przyznaje dostęp do tego urządzenia.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>Nie udało się otworzyć "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>Błąd Urządzenia HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>Urządzenie HID zostało odłączone lub wystąpił krytyczny błąd odczytu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>Wybierz Urządzenie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>Urządzenie HID</translation>
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
        <translation>TLS 1.3 lub Nowszy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 lub Nowszy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>Dowolny Protokół</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>Tylko Bezpieczne Protokoły</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>Brak</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>Zapytaj Partnera</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>Weryfikuj Partnera</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>Automatyczna Weryfikacja Partnera</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>Funkcja MQTT Wymaga Licencji Komercyjnej</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">Subskrypcja brokera MQTT jest dostępna wyłącznie z ważną licencją komercyjną Serial Studio (poziom Hobbyist lub wyższy).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>Subskrypcja brokera MQTT jest dostępna wyłącznie z ważną licencją Serial Studio lub aktywnym okresem próbnym.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>Użyj Systemowej Bazy Danych</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>Wczytaj z Folderu…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Wybierz Katalog Certyfikatów PEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>Nazwa Hosta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>Filtr Tematu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>ID Klienta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>Nazwa Użytkownika</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>Hasło</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>Wersja MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Czysta Sesja</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Automatyczny Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS Włączony</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>Protokół SSL</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>Tryb Weryfikacji Partnera</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>Głębokość Weryfikacji Partnera</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>Błąd Subskrypcji MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Nie udało się zasubskrybować tematu „%1".</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Nieprawidłowa Wersja Protokołu MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>Broker odrzucił skonfigurowaną wersję protokołu MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>Odrzucono Identyfikator Klienta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>Broker odrzucił identyfikator klienta. Spróbuj użyć innego identyfikatora.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>Serwer MQTT Niedostępny</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>Broker jest obecnie niedostępny. Spróbuj ponownie później.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>Błąd Uwierzytelniania</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>Podane dane uwierzytelniające zostały odrzucone przez broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>Błąd Autoryzacji</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>Konto nie posiada uprawnień do tej operacji.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>Błąd Sieci lub Transportu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Problem warstwy sieciowej/transportowej podczas łączenia z brokerem.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>Naruszenie Protokołu MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>Broker zgłosił naruszenie protokołu i zamknął połączenie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>Błąd MQTT 5</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>Wystąpił błąd protokołu MQTT 5.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>Błąd MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>Wystąpił nieoczekiwany błąd MQTT.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>Nieprawidłowy Port Szeregowy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Inicjalizacja Modbus Nie Powiodła Się</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Połączenie Modbus Nie Powiodło Się</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>Wybrany port szeregowy "%1" nie jest już dostępny. Odśwież listę portów i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Nie można utworzyć urządzenia Modbus. Sprawdź konfigurację systemu i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Nie można połączyć z "%1". Sprawdź ustawienia połączenia.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="609"/>
        <source>None</source>
        <translation>Brak</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="610"/>
        <source>Even</source>
        <translation>Parzyste</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="611"/>
        <source>Odd</source>
        <translation>Nieparzyste</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="612"/>
        <source>Space</source>
        <translation>Spacja</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="613"/>
        <source>Mark</source>
        <translation>Znacznik</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="665"/>
        <source>Holding Registers (0x03)</source>
        <translation>Rejestry Przechowywania (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="666"/>
        <source>Input Registers (0x04)</source>
        <translation>Rejestry Wejściowe (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="667"/>
        <source>Coils (0x01)</source>
        <translation>Cewki (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="668"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Wejścia Dyskretne (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>No register groups configured</source>
        <translation>Nie skonfigurowano grup rejestrów</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="851"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Dodaj co najmniej jedną grupę rejestrów przed wygenerowaniem projektu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="865"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="890"/>
        <source>Modbus Project Generator</source>
        <translation>Generator Projektu Modbus</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">Nie udało się utworzyć tymczasowego pliku projektu</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">Sprawdź uprawnienia do zapisu w katalogu tymczasowym.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="862"/>
        <source>Failed to load generated project</source>
        <translation>Nie udało się załadować wygenerowanego projektu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="863"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>Nie można było załadować wygenerowanego pliku JSON projektu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="885"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Pomyślnie wygenerowano projekt z %1 grupami i %2 zestawami danych.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="888"/>
        <source>The project editor is now open for customization.</source>
        <translation>Edytor projektu jest teraz otwarty do dostosowania.</translation>
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
        <translation>Rejestry Przechowywania</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="929"/>
        <source>Input Registers</source>
        <translation>Rejestry Wejściowe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="930"/>
        <source>Coils</source>
        <translation>Cewki</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="931"/>
        <source>Discrete Inputs</source>
        <translation>Wejścia Dyskretne</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="945"/>
        <source>Unknown</source>
        <translation>Nieznany</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="958"/>
        <source>Register %1</source>
        <translation>Rejestr %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Coil %1</source>
        <translation>Cewka %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Discrete %1</source>
        <translation>Dyskretne %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1372"/>
        <source>Error code: %1</source>
        <translation>Kod błędu: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1375"/>
        <source>Modbus Communication Error</source>
        <translation>Błąd Komunikacji Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1387"/>
        <source>Select Port</source>
        <translation>Wybierz Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1540"/>
        <source>Protocol</source>
        <translation>Protokół</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1548"/>
        <source>Slave Address</source>
        <translation>Adres Slave</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1557"/>
        <source>Poll Interval (ms)</source>
        <translation>Interwał Odpytywania (ms)</translation>
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
        <translation>Port Szeregowy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1627"/>
        <source>Baud Rate</source>
        <translation>Prędkość Transmisji</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1635"/>
        <source>Parity</source>
        <translation>Parzystość</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1643"/>
        <source>Data Bits</source>
        <translation>Bity Danych</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1651"/>
        <source>Stop Bits</source>
        <translation>Bity Stopu</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="547"/>
        <source>Network socket error</source>
        <translation>Błąd gniazda sieciowego</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="563"/>
        <source>Socket Type</source>
        <translation>Typ Gniazda</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="571"/>
        <source>Remote Address</source>
        <translation>Adres Zdalny</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="579"/>
        <source>TCP Port</source>
        <translation>Port TCP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="588"/>
        <source>UDP Local Port</source>
        <translation>Lokalny Port UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="597"/>
        <source>UDP Remote Port</source>
        <translation>Zdalny Port UDP</translation>
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
        <translation>Nie udało się uruchomić procesu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Plik wykonywalny „%1" nie został znaleziony w PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>Wybierz Plik Wykonywalny</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>Wybierz Katalog Roboczy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Wybierz Nazwany Potok / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>Proces uległ awarii.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>Kod wyjścia: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>Proces "%1" zatrzymany</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>Nieznany błąd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>Błąd Procesu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>Błąd Potoku</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>Nie można otworzyć nazwanego potoku: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>Tryb</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>Uruchom Proces</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>Nazwany Potok</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>Plik Wykonywalny</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>Argumenty</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>Katalog Roboczy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>Ścieżka Potoku</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="206"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>Przepływność %1 bps nie jest obsługiwana przez analizator USB-CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="216"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Nie można otworzyć portu szeregowego %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="227"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>Nie udało się zainicjować analizatora USB-CAN.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="264"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>Analizator USB-CAN nie jest otwarty do zapisu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="310"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>Błąd magistrali CAN zgłoszony przez analizator USB-CAN.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="167"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>Przepływność %1 bps nie jest standardową wartością slcan.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="177"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Nie można otworzyć portu szeregowego %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="190"/>
        <source>Failed to open the slcan channel.</source>
        <translation>Nie udało się otworzyć kanału slcan.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="229"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>Adapter slcan nie jest otwarty do zapisu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="267"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>Błąd magistrali CAN zgłoszony przez adapter slcan.</translation>
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
        <translation>Brak</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="349"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="733"/>
        <source>Select Port</source>
        <translation>Wybierz Port</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="392"/>
        <source>Even</source>
        <translation>Parzysta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="393"/>
        <source>Odd</source>
        <translation>Nieparzysta</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="394"/>
        <source>Space</source>
        <translation>Spacja</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="395"/>
        <source>Mark</source>
        <translation>Znacznik</translation>
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
        <translation>„%1" nie jest prawidłową ścieżką</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="563"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Wpisz inną ścieżkę, aby zarejestrować niestandardowe urządzenie szeregowe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="823"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Nie można znaleźć określonego urządzenia. Sprawdź połączenie i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="830"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>Wystąpił nieznany błąd. Sprawdź urządzenie i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="832"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Urządzenie nie jest otwarte. Otwórz urządzenie przed wykonaniem tej operacji.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="261"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Nie udało się połączyć z portem szeregowym „%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="798"/>
        <source>Unknown</source>
        <translation>Nieznany</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="799"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Błąd krytyczny na porcie szeregowym "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="800"/>
        <source>Unknown error</source>
        <translation>Nieznany błąd</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="822"/>
        <source>No error occurred.</source>
        <translation>Nie wystąpił błąd.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="824"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Odmowa dostępu. Upewnij się, że aplikacja ma niezbędne uprawnienia do urządzenia.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="825"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Nie udało się otworzyć urządzenia. Może być już w użyciu lub niedostępne.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="826"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>Wystąpił błąd podczas zapisu danych do urządzenia.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>Wystąpił błąd podczas odczytu danych z urządzenia.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>Wystąpił krytyczny błąd zasobu. Urządzenie mogło zostać odłączone lub nie jest już dostępne.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>Żądana operacja nie jest obsługiwana na tym urządzeniu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="831"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>Operacja przekroczyła limit czasu. Urządzenie może nie odpowiadać.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Serial Port</source>
        <translation>Port Szeregowy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Baud Rate</source>
        <translation>Prędkość Transmisji</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Parity</source>
        <translation>Parzystość</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Data Bits</source>
        <translation>Bity Danych</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>Stop Bits</source>
        <translation>Bity Stopu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1034"/>
        <source>Flow Control</source>
        <translation>Kontrola Przepływu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1042"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1049"/>
        <source>Auto-Reconnect</source>
        <translation>Automatyczne Ponowne Łączenie</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="156"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="164"/>
        <source>USB Error</source>
        <translation>Błąd USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="157"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Nie udało się zainicjować podsystemu USB. Sprawdź, czy libusb jest dostępny w systemie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="214"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="572"/>
        <source>USB Device Error</source>
        <translation>Błąd Urządzenia USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>Nie można otworzyć urządzenia USB: %1.

W systemie Linux upewnij się, że masz uprawnienia do odczytu/zapisu węzła urządzenia (dodaj regułę udev lub uruchom jako root). W systemie macOS może być konieczne wcześniejsze odłączenie sterownika jądra.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="165"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Nie wybrano urządzenia USB. Wybierz urządzenie i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="171"/>
        <source>Unknown Device</source>
        <translation>Nieznane Urządzenie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <source>Failed to open "%1"</source>
        <translation>Nie udało się otworzyć „%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="215"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Nie można przejąć interfejsu %1 urządzenia USB.

Inny sterownik lub aplikacja może już go używać. W systemie Linux spróbuj wyładować sterownik jądra (np. cdc_acm) lub dodać regułę udev.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="360"/>
        <source>Select Device</source>
        <translation>Wybierz Urządzenie</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="379"/>
        <source>Select IN Endpoint</source>
        <translation>Wybierz Punkt Końcowy IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="390"/>
        <source>None (Read-only)</source>
        <translation>Brak (Tylko Odczyt)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="464"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Włączyć Zaawansowane Transfery Sterujące USB?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="465"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Włącza transfery sterujące oprócz transferów masowych. Wysyłanie nieprawidłowych żądań sterujących może spowodować awarię lub uszkodzenie podłączonego sprzętu. Włączaj tylko jeśli wiesz, co robisz.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="469"/>
        <source>Advanced USB Mode</source>
        <translation>Tryb Zaawansowany USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="573"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>Urządzenie USB zostało odłączone lub wystąpił krytyczny błąd odczytu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="710"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Nie znaleziono izochronicznego punktu końcowego IN w tym urządzeniu, ale dostępne są punkty końcowe masowe.

Przełącz Tryb Transferu na „Strumień Masowy" i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="715"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Nie znaleziono masowego punktu końcowego IN w tym urządzeniu, ale dostępne są punkty końcowe izochroniczne.

Przełącz Tryb Transferu na „Izochroniczny" i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="719"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Nie znaleziono użytecznego punktu końcowego IN w tym urządzeniu.

Urządzenie może nie udostępniać punktów końcowych danych w aktywnej konfiguracji lub może wymagać specyficznego sterownika.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1211"/>
        <source>USB Device</source>
        <translation>Urządzenie USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1219"/>
        <source>Transfer Mode</source>
        <translation>Tryb Transferu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Bulk Stream</source>
        <translation>Strumień Masowy</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Advanced Control</source>
        <translation>Sterowanie Zaawansowane</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Isochronous</source>
        <translation>Izochroniczny</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1227"/>
        <source>IN Endpoint</source>
        <translation>Punkt Końcowy IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1235"/>
        <source>OUT Endpoint</source>
        <translation>Punkt Końcowy OUT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1243"/>
        <source>ISO Packet Size</source>
        <translation>Rozmiar Pakietu ISO</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>Nie wybrano pliku…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>Zwykły Tekst</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Surowy Binarny</translation>
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
        <translation>Wybierz plik do transmisji</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>Wybrany plik: %1 (%2 bajtów)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>Błąd otwierania pliku: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>Rozpoczynanie transmisji %1…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>Transmisja zakończona</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>Transmisja zwykłego tekstu zakończona</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Transmisja surowych danych binarnych zakończona (%1 bajtów)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>Transfer zakończony</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Transfer zakończony pomyślnie (%1 bajtów)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>Transfer nie powiódł się: %1</translation>
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
        <translation>Utracone ramki</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>Dane przychodzące szybciej niż Serial Studio może je przetworzyć; utracono %1 ramek. Zmniejsz szybkość transmisji lub wyłącz wymagający komponent.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>Nie można otworzyć pliku: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>Transfer anulowany</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>Transfer anulowany przez użytkownika</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>Oczekiwanie na odbiornik…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Odbiornik gotowy (tryb CRC), wysyłanie danych…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Zbyt wiele ponownych prób, transfer przerwany</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>Przekroczono maksymalną liczbę ponownych prób</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>Otrzymano NAK, ponowna próba bloku %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>Nie udało się przesunąć pozycji w pliku</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Transfer anulowany przez odbiorcę</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Odbiorca anulował transfer</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>Transfer zakończony</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>Odczyt pliku zwrócił więcej danych niż żądano</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Wysyłanie bloku %1 (%2 bajtów)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>Wysyłanie EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Przekroczono limit czasu, ponowna próba (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>Przekroczono limit czasu transmisji</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>Limit czasu: brak odpowiedzi od odbiornika</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>Nie można otworzyć pliku: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Transmisja anulowana przez odbiornik</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Odbiornik anulował transmisję</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>Oczekiwanie na odbiornik…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>Wysyłanie pierwszego EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Zbyt wiele ponownych prób, transmisja przerwana</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>Przekroczono maksymalną liczbę ponownych prób</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>Otrzymano NAK, ponowna próba bloku %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>Nie udało się przesunąć w pliku</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>Wysyłanie drugiego EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>Wysyłanie znacznika końca partii…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>Transfer zakończony</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Wysyłanie nagłówka pliku: %1 (%2 bajtów)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Wysyłanie bloku %1 (%2/%3 bajtów)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>Nie można otworzyć pliku: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Plik jest za duży dla ZMODEM (%1 bajtów, limit 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>Transfer anulowany</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>Transfer anulowany przez użytkownika</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>Niezgodność CRC nagłówka szesnastkowego, odrzucanie ramki</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Wysyłanie informacji o pliku: %1 (%2 bajtów)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>Nie udało się przesunąć do offsetu %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>Odczyt pliku zwrócił więcej danych niż żądano</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Odbiornik żąda danych od offsetu %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>Odbiornik pominął plik</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>Zbyt wiele błędów, transfer przerwany</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>Przekroczono maksymalną liczbę ponownych prób</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>Wysyłanie ZRQINIT, oczekiwanie na odbiornik…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Dane pliku wysłane, oczekiwanie na potwierdzenie…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>Wysyłanie ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Odbiornik gotowy, wysyłanie informacji o pliku…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>Otrzymano NAK, ponowna próba (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>Transfer zakończony</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Transfer anulowany przez odbiornik</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Odbiornik anulował transfer</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>Odbiornik zgłosił błąd pliku</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>Przekroczono limit czasu transferu</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>Limit czasu: brak odpowiedzi od odbiornika</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Limit czasu, ponowna próba (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>Wybierz Ikonę</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>Szukaj Online…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>Normalny</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>Skala Szarości</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Wysoki Kontrast</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Żywy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Noktowizja</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Podczerwień</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Ciemnoniebieski</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Bursztynowy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Eksportuj Obrazy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Otwórz Folder Eksportu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Powiększ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Pomniejsz</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Pokaż Celownik</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>Oczekiwanie na Obraz…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>Klucze API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. Domyślnie Claude Haiku 4.5 ($1 wejście / $5 wyjście na milion tokenów). Dostępne są również Sonnet 4.6 i OPUS 4.7. Obsługuje strumieniowanie, użycie narzędzi, rozszerzone myślenie i buforowanie promptów.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. Domyślnie GPT-4o mini ($0.15 wejście / $0.60 wyjście na milion tokenów). Dostępne są również GPT-4o, GPT-4 Turbo i o1-mini.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. Domyślnie Gemini 2.0 Flash, który ma hojny darmowy poziom (z ograniczeniami szybkości). Dostępne są również Gemini 1.5 Pro i Gemini 1.5 Flash.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Użyj własnych kluczy API. Są szyfrowane w spoczynku kluczem specyficznym dla maszyny i nigdy nie opuszczają komputera, z wyjątkiem komunikacji z wybranym dostawcą.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>Klucz ustawiony</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>Brak klucza</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">Klucz jest zapisany — wklej nowy, aby go zastąpić</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. Domyślnie GPT-5 mini dla szybkiej, ekonomicznej pracy agentowej. GPT-5.2 to mocniejsza opcja ogólnego przeznaczenia, a GPT-5.2 Chat śledzi model aktualnie używany w ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. API kompatybilne z OpenAI. Domyślnie deepseek-chat (V3). Dostępny jest również deepseek-reasoner (R1). Często najtańsza opcja chmurowa dla użycia narzędzi.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. Jeden klucz, ~200 modeli od Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen i innych. Modele z darmowym poziomem (z sufiksem :free) mają ograniczoną przepustowość, ale nie wymagają dodatkowych opłat. Płatność za użycie dla pozostałych.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Wnioskowanie przyspieszone sprzętowo (LPU) dla bardzo szybkich modeli Llama, Mixtral, Gemma, DeepSeek-R1 distill i Qwen. Hojny darmowy poziom z dziennymi limitami tokenów.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. Domyślnie Mistral Large. Codestral jest przeznaczony do uzupełniania kodu, Pixtral obsługuje wizję, a modele Ministral są dostrojone do użycia brzegowego / o niskich opóźnieniach.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Lokalny serwer modeli. Działa z dowolnym punktem końcowym kompatybilnym z OpenAI -- Ollama, llama-server z llama.cpp, LM Studio lub vLLM. Nic nie opuszcza twojej maszyny. Lista modeli jest pobierana na żywo z serwera.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>Klucz jest zapisany -- wklej nowy, aby go zastąpić</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>Wklej tutaj swój klucz API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>Ukryj klucz</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>Pokaż klucz podczas wpisywania</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>Pobierz klucz</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Otwórz konsolę dostawcy, aby utworzyć nowy klucz</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>Zapisz</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>Usuń zapisany klucz dla %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Zainstaluj Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Otwórz stronę pobierania Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>Zastosuj</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>Ponownie pobierz listę modeli</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Nie skonfigurowano jeszcze żadnych kluczy API. Dodaj klucz, aby rozpocząć.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">Nie skonfigurowano jeszcze żadnych kluczy API. Dodaj co najmniej jeden powyżej, aby rozpocząć.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>Jeden dostawca jest gotowy.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>Liczba gotowych dostawców: %1.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Licencjonowanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Proszę czekać…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Aktywuj Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Wklej poniżej swój klucz licencyjny, aby odblokować funkcje Pro, takie jak MQTT, wykresy 3D i więcej.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>Twoja licencja obejmuje 5 aktywacji urządzeń.
Plany obejmują opcje Miesięczne, Roczne i Dożywotnie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Wklej tutaj swój klucz licencyjny…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Wklej</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>Zaznacz Wszystko</translation>
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
        <translation>Licencjobiorca</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>E-mail Licencjobiorcy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>Użycie Urządzeń</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 urządzeń w użyciu (plan nielimitowany)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%1 z %2 urządzeń użytych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>ID Urządzenia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>Klucz Licencyjny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>Portal Klienta</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>Kup Licencję</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>Aktywuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>Dezaktywuj</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="517"/>
        <source>There was an issue validating your license.</source>
        <translation>Wystąpił problem z walidacją licencji.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="535"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="716"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="835"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>Podany klucz licencyjny nie należy do Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="536"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Sprawdź, czy licencja została zakupiona w oficjalnym sklepie Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="547"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="725"/>
        <source>This license key was activated on a different device.</source>
        <translation>Ten klucz licencyjny został aktywowany na innym urządzeniu.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="548"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="726"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Dezaktywuj go tam najpierw lub skontaktuj się z pomocą techniczną.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="559"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="736"/>
        <source>This license is not currently active.</source>
        <translation>Ta licencja nie jest obecnie aktywna.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="560"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Mogła wygasnąć lub zostać dezaktywowana (status: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="570"/>
        <source>Something went wrong on the server.</source>
        <translation>Coś poszło nie tak na serwerze.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="571"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="747"/>
        <source>No activation ID was returned.</source>
        <translation>Nie zwrócono ID aktywacji.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="581"/>
        <source>Could not validate your license at this time.</source>
        <translation>Nie można zwalidować licencji w tym momencie.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="582"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="756"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="845"/>
        <source>Try again later.</source>
        <translation>Spróbuj ponownie później.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="717"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="836"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Upewnij się, że licencja została zakupiona w oficjalnym sklepie Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <source>Something went wrong on the server…</source>
        <translation>Coś poszło nie tak na serwerze…</translation>
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
        <translation>Twoja licencja została pomyślnie aktywowana.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="654"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Dziękujemy za wspieranie Serial Studio!
Masz teraz dostęp do wszystkich funkcji premium.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="708"/>
        <source>There was an issue activating your license.</source>
        <translation>Wystąpił problem z aktywacją licencji.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Could not activate your license at this time.</source>
        <translation>Nie można teraz aktywować licencji.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="827"/>
        <source>There was an issue deactivating your license.</source>
        <translation>Wystąpił problem z dezaktywacją licencji.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="844"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>Nie udało się dezaktywować licencji w tym momencie.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="854"/>
        <source>Your license has been deactivated.</source>
        <translation>Twoja licencja została dezaktywowana.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="855"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Dostęp do funkcji Pro został usunięty.
Dziękujemy ponownie za wsparcie Serial Studio!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>Eksport MDF4 jest funkcją Pro.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Ta funkcja wymaga licencji. Zakup licencję, aby włączyć eksport MDF4.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>Wybierz plik MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>Pliki MDF4 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>Rozłączyć z urządzeniem?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>Musisz rozłączyć się z bieżącym urządzeniem przed otwarciem pliku MDF4.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>Nie można otworzyć pliku MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Plik może być uszkodzony lub w nieobsługiwanym formacie.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>Nieprawidłowy plik MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Nie udało się odczytać struktury pliku. Plik może być uszkodzony.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>Brak danych w pliku</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>Plik MDF4 nie zawiera danych pomiarowych.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>Nazwa Hosta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>np. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Port</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Filtr Tematu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>np. sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>ID Klienta</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Wygeneruj Ponownie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Nazwa Użytkownika</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Hasło</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>Wersja</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Czysta Sesja</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>Użyj SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>Protokół SSL</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Weryfikacja Partnera</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Głębokość Weryfikacji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>Certyfikaty CA</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Wczytaj z Folderu…</translation>
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
        <translation type="vanished">TLS 1.3 lub Nowszy</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 lub Nowszy</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">Dowolny Protokół</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">Tylko Bezpieczne Protokoły</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">Brak</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">Zapytaj Partnera</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">Weryfikuj Partnera</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">Automatyczna Weryfikacja Partnera</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">Użyj Systemowej Bazy Danych</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">Subskrybent MQTT</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">Wydawca MQTT</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">Funkcja MQTT Wymaga Licencji Komercyjnej</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">Wczytaj z Folderu…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">Łączenie z brokerami MQTT jest dostępne wyłącznie z ważną licencją komercyjną Serial Studio (wariant Hobbyist lub wyższy).

Aby odblokować tę funkcję, aktywuj licencję lub odwiedź sklep.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">Brak Tematu MQTT</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">Przed połączeniem jako wydawca należy określić temat.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">Błąd Konfiguracji</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">Temat MQTT Nie Został Ustawiony</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">Nie otrzymasz żadnych wiadomości, dopóki temat nie zostanie skonfigurowany.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">Ostrzeżenie Konfiguracji</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">Nieprawidłowy Temat MQTT</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">Temat „%1" jest nieprawidłowy.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">Wybierz Katalog Certyfikatów PEM</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">Błąd Subskrypcji</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">Nie udało się zasubskrybować tematu „%1".</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">Nieprawidłowa Wersja Protokołu MQTT</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">Broker MQTT odrzucił połączenie z powodu nieobsługiwanej wersji protokołu. Upewnij się, że klient i broker obsługują tę samą wersję protokołu.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">Odrzucono Identyfikator Klienta</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">Broker odrzucił identyfikator klienta. Może być nieprawidłowo sformatowany, zbyt długi lub już używany. Spróbuj użyć innego identyfikatora klienta.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">Serwer MQTT Niedostępny</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">Połączenie sieciowe zostało nawiązane, ale broker jest obecnie niedostępny. Sprawdź status brokera i spróbuj ponownie później.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">Błąd Uwierzytelniania</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">Podana nazwa użytkownika lub hasło jest nieprawidłowe lub niepoprawnie sformatowane. Sprawdź swoje dane uwierzytelniające i spróbuj ponownie.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">Błąd Autoryzacji</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">Broker MQTT odmówił połączenia z powodu niewystarczających uprawnień. Upewnij się, że Twoje konto posiada niezbędne prawa dostępu.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">Błąd Sieci lub Transportu</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">Wystąpił problem z siecią lub warstwą transportową, powodujący nieoczekiwane przerwanie połączenia. Sprawdź połączenie sieciowe i ustawienia brokera.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">Naruszenie Protokołu MQTT</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">Klient wykrył naruszenie protokołu MQTT i zamknął połączenie. Sprawdź implementację MQTT pod kątem zgodności.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">Nieznany Błąd</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">Wystąpił nieoczekiwany błąd. Sprawdź logi, aby uzyskać więcej szczegółów, lub uruchom aplikację ponownie.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">Błąd MQTT 5</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">Wystąpił błąd protokołu MQTT poziomu 5. Sprawdź logi brokera lub kody przyczyn, aby uzyskać więcej szczegółów.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">Uwierzytelnianie MQTT Nieudane</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">Uwierzytelnianie nieudane: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">Wymagane jest rozszerzone uwierzytelnianie, ale MQTT 5.0 nie jest włączony.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">Nieznany</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">Wymagane Uwierzytelnianie MQTT</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">Broker MQTT wymaga uwierzytelniania przy użyciu metody: „%1".

Podaj wymagane dane uwierzytelniające.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">Wprowadź Nazwę Użytkownika MQTT</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">Nazwa Użytkownika:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">Wprowadź Hasło MQTT</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">Hasło:</translation>
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
        <translation>TLS 1.3 lub Nowszy</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 lub Nowszy</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>Dowolny Protokół</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>Tylko Bezpieczne Protokoły</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>Brak</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>Zapytaj Partnera</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>Weryfikuj Partnera</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>Automatyczna Weryfikacja Partnera</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>Surowe Dane RX</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>Skrypt Niestandardowy</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Dane Pulpitu (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Dane Pulpitu (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>Publikator MQTT niedostępny</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>Wymagana jest ważna licencja komercyjna, aby korzystać z publikowania MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>Testuj Połączenie MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Wybierz Katalog Certyfikatów PEM</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>Broker MQTT osiągalny</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>Broker MQTT nieosiągalny</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>Połączenie z brokerem MQTT nie powiodło się</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>Wydawca MQTT</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="50"/>
        <source>MQTT Publisher Script</source>
        <translation>Skrypt Wydawcy MQTT</translation>
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
        <translation>Przykładowe bajty ramki (tekst lub hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Test</source>
        <translation>Testuj</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="100"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Apply</source>
        <translation>Zastosuj</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="103"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="112"/>
        <source>Language:</source>
        <translation>Język:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="115"/>
        <source>Template:</source>
        <translation>Szablon:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="126"/>
        <source>Frame:</source>
        <translation>Ramka:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="130"/>
        <source>Output:</source>
        <translation>Wyjście:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="270"/>
        <source>Enter a frame</source>
        <translation>Wprowadź ramkę</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="277"/>
        <source>Invalid hex</source>
        <translation>Nieprawidłowy hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Formatuj Dokument	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="361"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Formatuj Zaznaczenie	ctrl+i</translation>
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
-- Zdefiniuj funkcję mqtt(frame), która otrzymuje surowe bajty
-- jednej sparsowanej ramki i zwraca payload do opublikowania
-- do brokera, lub nil aby pominąć tę ramkę.
--
-- Argument frame odpowiada temu, co widzi skrypt Frame Parser:
-- łańcuch Lua zawierający bajty pomiędzy ogranicznikami
-- FrameReader.
--
-- Przykład:
--   function mqtt(frame)
--     return frame    -- przekaż bez zmian
--   end
--
-- Użyj menu Template dla gotowych przykładów.
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
 * Zdefiniuj funkcję mqtt(frame), która otrzymuje surowe bajty
 * jednej sparsowanej ramki i zwraca payload do opublikowania
 * do brokera, lub null aby pominąć tę ramkę.
 *
 * Argument frame odpowiada temu, co widzi skrypt Frame Parser:
 * łańcuch zawierający bajty pomiędzy ogranicznikami
 * FrameReader.
 *
 * Przykład:
 *   function mqtt(frame) {
 *     return frame;   // przekaż bez zmian
 *   }
 *
 * Użyj menu Template dla gotowych przykładów.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="615"/>
        <source>Script is empty</source>
        <translation>Skrypt jest pusty</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="622"/>
        <source>Lua engine error</source>
        <translation>Błąd silnika Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="696"/>
        <source>Error: %1</source>
        <translation>Błąd: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="652"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() nie jest zdefiniowana</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- ramka pominięta)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="671"/>
        <source>(non-string return)</source>
        <translation>(zwrot niebędący ciągiem znaków)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="701"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- ramka pominięta)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="779"/>
        <source>Select Template…</source>
        <translation>Wybierz Szablon…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Skonfiguruj nazwę hosta i port brokera przed testowaniem połączenia.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Pomyślnie połączono z %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Przekroczono limit czasu po 5 sekundach bez osiągnięcia brokera.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">Konfiguracja MQTT</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT jest Funkcją Pro</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">Aktywuj swoją licencję lub odwiedź sklep, aby odblokować obsługę MQTT.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">Ogólne</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">Uwierzytelnianie</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">Opcje MQTT</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">Właściwości SSL</translation>
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
        <translation type="vanished">ID Klienta</translation>
    </message>
    <message>
        <source>Keep Alive (s)</source>
        <translation type="vanished">Keep Alive (s)</translation>
    </message>
    <message>
        <source>Clean Session</source>
        <translation type="vanished">Czysta Sesja</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">Nazwa Użytkownika</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">Nazwa Użytkownika MQTT</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">Hasło</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">Hasło MQTT</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">Wersja</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">Tryb</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">Temat</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">np. sensors/temperature lub home/+/status</translation>
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
        <translation type="vanished">Temat Will</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">np. device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Komunikat Will</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">np. Urządzenie nieoczekiwanie rozłączone</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">Włącz SSL</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">Protokół SSL</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">Głębokość Weryfikacji</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">Tryb Weryfikacji</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Zamknij</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">Rozłącz</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">Połącz</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="205"/>
        <source>Console Only Mode</source>
        <translation>Tryb Tylko Konsoli</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="208"/>
        <source>Quick Plot Mode</source>
        <translation>Tryb Szybkiego Wykresu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="215"/>
        <source>Empty Project</source>
        <translation>Pusty Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="696"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="704"/>
        <source>Waiting for data…</source>
        <translation>Oczekiwanie na dane…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="705"/>
        <source>Connecting to device…</source>
        <translation>Łączenie z urządzeniem…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Przykład kodu</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Uzupełniacz</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Podświetlacz</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Styl</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>spacje</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>Skopiowano do Schowka</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>Odtwarzacz MDF4</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Ty</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Asystent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Wykrywanie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Wykonanie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>Zatwierdzić %1 akcji w %2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Te wywołania zostaną uruchomione razem. Rozwiń każdą kartę poniżej, aby sprawdzić argumenty.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Zatwierdź wszystkie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Odrzuć wszystkie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Kopiuj Wszystkie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Zaznacz Wszystko</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>Ty</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>Asystent</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Wykrywanie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Wykonywanie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>Uruchomione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>Oczekuje na zatwierdzenie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Zakończono</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Odrzucone</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Zablokowane</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Zatwierdź</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Odrzuć</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Zatwierdź wszystkie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Odrzuć wszystkie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Argumenty</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Wynik</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>Zatwierdzić {n} akcji w {family}?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Te wywołania zostaną wykonane razem. Rozwiń każdą kartę, aby sprawdzić argumenty.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>Nie udało się załadować README: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>Motyw</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>Parser Ramek</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>Szablon Projektu</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>Wtyczka</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>Wszystkie Typy</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>Resetuj Rozszerzenia</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Spowoduje to odinstalowanie wszystkich rozszerzeń, usunięcie wszystkich niestandardowych repozytoriów i przywrócenie domyślnych ustawień. Kontynuować?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>Wybierz Folder Repozytorium Rozszerzeń</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1017"/>
        <source>Installed (repository no longer available)</source>
        <translation>Zainstalowane (repozytorium już niedostępne)</translation>
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
        <translation>Błąd Wtyczki</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Wtyczka „%1" nie jest zainstalowana.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1335"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>Rozszerzenie „%1" nie jest wtyczką (typ: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1356"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Nie można odczytać pliku metadanych wtyczki:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Wtyczka „%1" wymaga GRPC, ale ta kompilacja nie zawiera obsługi GRPC.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Ta wtyczka używa GRPC do strumieniowania danych o wysokiej wydajności. Serwer API musi być włączony.

Czy chcesz go teraz włączyć?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1393"/>
        <source>API Server Required</source>
        <translation>Wymagany Serwer API</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1422"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Wtyczka „%1" nie ma pola 'entry' w info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Entry point not found:
%1</source>
        <translation>Nie znaleziono punktu wejścia:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Wtyczka „%1" ma nieprawidłową ścieżkę punktu wejścia.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1484"/>
        <source>Missing Dependency</source>
        <translation>Brakująca Zależność</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Ta wtyczka wymaga „%1", ale nie został znaleziony w systemie.

Czy chcesz otworzyć stronę pobierania?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>Wtyczki potrzebują serwera API do komunikacji z Serial Studio. Czy chcesz go teraz włączyć?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Wymagane Ponowne Uruchomienie</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>Nowy backend renderowania zacznie działać po ponownym uruchomieniu Serial Studio. Uruchomić ponownie, aby zastosować zmiany?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>Nie udało się załadować strony: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="152"/>
        <source>Invalid icon identifier</source>
        <translation>Nieprawidłowy identyfikator ikony</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="222"/>
        <source>Empty SVG data received</source>
        <translation>Otrzymano puste dane SVG</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Skrót Windows (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>Aplikacja macOS (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Wpis Pulpitu (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Użyj ikony .icns dla najostrzejszego rezultatu w Finderze i Docku.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Pozostaw ikonę pustą, aby dziedziczyć ikonę pliku wykonywalnego Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Umieść plik w ~/.local/share/applications/, aby udostępnić go w programie uruchamiającym aplikacje.</translation>
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
        <translation>Obraz Wektorowy lub Rastrowy (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>Licencja Pro jest wymagana do generowania skrótów.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>Nie podano ścieżki wyjściowej.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>Nie udało się zapisać pliku skrótu.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Nie można zastąpić istniejącego skrótu w %1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Nie można utworzyć struktury katalogów pakietu .app.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Nie można zapisać programu uruchamiającego pakiet: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Nie można oznaczyć programu uruchamiającego pakiet jako wykonywalny.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Nie można zapisać Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Moduł zapisu skrótów Windows nie jest dostępny na tej platformie.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Moduł zapisu skrótów Linux nie jest dostępny na tej platformie.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>Nie można zainicjować COM (wymagane do zapisu skrótów .lnk).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) nie powiodło się (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) nie powiodło się (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Zapisywanie pliku .lnk nie powiodło się (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>Moduł zapisu skrótów macOS nie jest dostępny na tej platformie.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Nie można otworzyć ścieżki skrótu do zapisu: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Skrót Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Nie można oznaczyć skrótu jako wykonywalny.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>Systemowy</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">Sprawdzać aktualizacje automatycznie?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">Czy %1 ma automatycznie sprawdzać aktualizacje? Zawsze można sprawdzić aktualizacje ręcznie w oknie „Informacje"</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>Ok</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>Zapisz</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>Zapisz wszystkie</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>Otwórz</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>Tak</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>Tak dla wszystkich</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>Nie</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>Nie dla wszystkich</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>Przerwij</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>Ponów</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>Ignoruj</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>Odrzuć</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>Zastosuj</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>Przywróć Domyślne</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>Wybierz Lokalizację Obszaru Roboczego</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>Protokół</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="70"/>
        <source>Serial Port</source>
        <translation>Port Szeregowy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="93"/>
        <source>Baud Rate</source>
        <translation>Prędkość Transmisji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="184"/>
        <source>Parity</source>
        <translation>Parzystość</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="205"/>
        <source>Data Bits</source>
        <translation>Bity Danych</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="226"/>
        <source>Stop Bits</source>
        <translation>Bity Stopu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="247"/>
        <source>Host</source>
        <translation>Host</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="257"/>
        <source>IP Address</source>
        <translation>Adres IP</translation>
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
        <translation>Adres Slave</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="313"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="361"/>
        <source>Configure Register Groups…</source>
        <translation>Konfiguruj Grupy Rejestrów…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="371"/>
        <source>Import Register Map…</source>
        <translation>Importuj Mapę Rejestrów…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="386"/>
        <source>%1 group(s) configured</source>
        <translation>Skonfigurowano grup: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="387"/>
        <source>No groups configured</source>
        <translation>Nie skonfigurowano grup</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="328"/>
        <source>Poll Interval (ms)</source>
        <translation>Interwał Odpytywania (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="333"/>
        <source>Polling interval</source>
        <translation>Interwał odpytywania</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Grupy Rejestrów Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Skonfiguruj wiele grup rejestrów, aby odpytywać różne typy rejestrów sekwencyjnie.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Dodaj Nową Grupę</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Typ Rejestru:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Adres Początkowy:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Liczba Rejestrów:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Dodaj Grupę</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Skonfigurowane Grupy</translation>
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
        <translation>Liczba</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Akcja</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Brak skonfigurowanych grup.
Dodaj grupy powyżej, aby odpytywać wiele typów rejestrów.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Łączna liczba grup: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Generuj Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Wyczyść Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Podgląd Mapy Rejestrów Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>Plik: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Przejrzyj rejestry do zaimportowania do nowego projektu Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>Rejestry</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>Nazwa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Adres</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Typ Danych</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Jednostki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>Nie znaleziono rejestrów w pliku.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Razem: %1 rejestrów w %2 grupach</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Utwórz Projekt</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>Wydawca MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>Połączono z brokerem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Brak połączenia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Testuj Połączenie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Przetestuj broker z bieżącymi ustawieniami</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Najpierw włącz publikowanie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Edytuj Skrypt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Edytuj skrypt publikatora (Lua lub JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Wczytaj Certyfikaty CA</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>Dodaj certyfikaty PEM z folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>Najpierw włącz SSL/TLS</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpoluj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="305"/>
        <source>Interpolation: %1</source>
        <translation>Interpolacja: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Pokaż Legendy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="327"/>
        <source>Show X Axis Label</source>
        <translation>Pokaż Etykietę Osi X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="338"/>
        <source>Show Y Axis Label</source>
        <translation>Pokaż Etykietę Osi Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="350"/>
        <source>Show Crosshair</source>
        <translation>Pokaż Celownik</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Resume</source>
        <translation>Wznów</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="374"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Tryb Sweep / Wyzwalania</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="386"/>
        <source>Trigger Settings</source>
        <translation>Ustawienia Wyzwalania</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="410"/>
        <source>Reset View</source>
        <translation>Resetuj Widok</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="416"/>
        <source>Axis Range Settings</source>
        <translation>Ustawienia Zakresu Osi</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Próbki</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>Bajtów na wartość</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Liczba bajtów łączonych w każdą wartość kanału.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="301"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="439"/>
        <source>Endianness</source>
        <translation>Kolejność Bajtów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="302"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="440"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>Kolejność bajtów używana przy łączeniu wartości wielobajtowych.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>Wartości ze znakiem</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Interpretuje każdą wartość jako liczbę ze znakiem w kodzie uzupełnień do dwóch.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>Tablica routingu tagów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Wpisy tag:indeks oddzielone przecinkami, np. 1:0,2:1,3:2. Tagi mogą być dziesiętne lub szesnastkowe z prefiksem 0x.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>Waliduj sumę kontrolną</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Odrzuca komunikaty z nieprawidłową sumą kontrolną Fletchera.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Odrzuca komunikaty z nieprawidłową sumą kontrolną addytywną.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>Wersja protokołu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Wybiera oczekiwany znacznik startu (0xFE dla v1, 0xFD dla v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>Waliduj CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Odrzuca ramki z nieprawidłową sumą kontrolną CRC-24Q.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>Liczba kanałów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Liczba kanałów wyjściowych (rejestrów lub cewek).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>Przesunięcie rejestru</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Przesunięcie adresu odejmowane od ech pojedynczego zapisu (40001 dla starszych map Modicon).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>Rejestry ze znakiem</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Interpretuje rejestry 16-bitowe jako wartości ze znakiem w kodzie uzupełnień do dwóch.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>Układ ładunku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>Tablica emituje każdy element w kolejności; mapa kieruje klucze przez listę kluczy.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>Klucze (tryb mapy)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Klucze mapy oddzielone przecinkami w kolejności kanałów. Używane tylko dla układu mapy.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Pola skalarne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Pola JSON oddzielone przecinkami, powtarzane w każdej generowanej ramce.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Pole tablicy próbek</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>Pole JSON zawierające zgrupowaną tablicę próbek.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Pole tablicy rekordów</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>Pole JSON zawierające tablicę obiektów rekordów.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Pola rekordu (w kolejności kanałów)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Pola rekordu oddzielone przecinkami. Pozycja każdego pola określa jego indeks kanału.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>Szerokości Kolumn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Liczby znaków dla każdego pola, oddzielone przecinkami. Pozostaw puste, aby dzielić po białych znakach.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>Przytnij Białe Znaki</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Usuwa odstępy wokół każdego wydzielonego pola.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>Klucze (w Kolejności Kanałów)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Nazwy kluczy oddzielone przecinkami. Pozycja każdego klucza określa jego indeks kanału.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>Separator Par</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>Znak między parami klucz=wartość.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>Separator Klucz-Wartość</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>Znak między kluczem a jego wartością.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>Tylko wartości numeryczne</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Ignoruje pary, których wartość nie jest liczbą.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>Tabela routingu poleceń</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Wpisy rozdzielone średnikami w formacie lista NAZWA:indeks, np. CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>Prefiks nadawcy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>Dwuliterowy identyfikator nadawcy, np. GP dla GPS lub GN dla odbiorników wielokonstelacyjnych.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Odrzuca zdania, których suma kontrolna *hh nie pasuje.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Nazwy parametrów rozdzielone przecinkami. Pozycja każdego klucza określa jego indeks kanału.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>Pola (w kolejności kanałów)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Nazwy pól rozdzielone przecinkami. Pozycja każdego pola określa jego indeks kanału.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>Tagi (w kolejności kanałów)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Nazwy tagów oddzielone przecinkami. Pozycja każdego tagu określa jego indeks kanału.</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">Bloki Rejestrów</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">Odpytywane bloki rejestrów z typowanymi, skalowanymi wpisami. Zapisane przez importer mapy rejestrów Modbus.</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">Mapa Sygnałów</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">Komunikaty CAN z ich układami bitowymi sygnałów i skalowaniem. Zapisane przez importer DBC.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Typ Gniazda</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Adres Zdalny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Port Lokalny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>Wpisz 0 dla automatycznego portu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Zdalny Port</translation>
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
        <translation>Filtruj według kanału…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Wyczyść wszystkie powiadomienia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(bez tytułu)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Brak powiadomień</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">Transformacje zestawów danych i skrypty widgetów wyjściowych mogą publikować zdarzenia za pomocą notifyInfo / notifyWarning / notifyCritical.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>Szukaj Ikon Online</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>Pobieranie nie powiodło się: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Szukaj ikon (np. temperatura, strzałka, odtwarzanie)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>Wyszukaj ikony powyżej, aby rozpocząć</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Widżety wyjściowe wymagają licencji Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Przycisk</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Wyślij polecenie po kliknięciu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Suwak</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Wyślij przeskalowane wartości numeryczne</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Przełącznik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Wysyłaj komendy włącz/wyłącz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Pole Tekstowe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Wpisuj i wysyłaj dowolne komendy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Pokrętło</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Wejście obrotowe dla wartości zadanych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Możesz konfigurować widżety wyjściowe, ale pojawiają się one na pulpicie tylko z licencją Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Duplikuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Duplikuj ten widżet wyjściowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Usuń ten widżet wyjściowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Funkcja Transmisji</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Importuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Importuj funkcję transmisji z pliku .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Szablon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Wybierz gotowy szablon funkcji transmisji</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Testuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Testuj funkcję transmisji z przykładowymi danymi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Cofnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Ponów</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Wytnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Wklej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Zaznacz Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Formatuj Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Formatuj Zaznaczenie</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Błąd Widgetu Malarskiego</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Edytor Kodu Widgetu Malarskiego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Importuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Importuj kod malarski z pliku .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Szablon</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Wybierz wbudowany szablon malowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Formatuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Przeformatuj kod malowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Testuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Otwórz podgląd na żywo z symulowanymi wartościami zestawu danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Podgląd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Wytnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Wklej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Zaznacz Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Cofnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Ponów</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Formatuj Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Formatuj Zaznaczenie</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Podgląd na Żywo Paintera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Podgląd</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Symulowane zestawy danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>Oczekiwanie na pierwszą ramkę...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Konsola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Wyczyść konsolę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">Interpoluj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="294"/>
        <source>Interpolation: %1</source>
        <translation>Interpolacja: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="307"/>
        <source>Show Area Under Plot</source>
        <translation>Pokaż Obszar Pod Wykresem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="326"/>
        <source>Show X Axis Label</source>
        <translation>Pokaż Etykietę Osi X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="337"/>
        <source>Show Y Axis Label</source>
        <translation>Pokaż Etykietę Osi Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="349"/>
        <source>Show Crosshair</source>
        <translation>Pokaż Celownik</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Resume</source>
        <translation>Wznów</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="373"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Tryb Sweep / Wyzwalania</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="385"/>
        <source>Trigger Settings</source>
        <translation>Ustawienia Wyzwalania</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="409"/>
        <source>Reset View</source>
        <translation>Resetuj Widok</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="415"/>
        <source>Axis Range Settings</source>
        <translation>Ustawienia Zakresu Osi</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="202"/>
        <source>Interpolate</source>
        <translation>Interpoluj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="220"/>
        <source>Orbit Navigation</source>
        <translation>Nawigacja Orbitalna</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="230"/>
        <source>Pan Navigation</source>
        <translation>Nawigacja Przesuwania</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="241"/>
        <source>Orthogonal View</source>
        <translation>Widok Ortogonalny</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="247"/>
        <source>Top View</source>
        <translation>Widok z Góry</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="253"/>
        <source>Left View</source>
        <translation>Widok z Lewej</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="259"/>
        <source>Front View</source>
        <translation>Widok z Przodu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="276"/>
        <source>Auto Center</source>
        <translation>Autocentrowanie</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="292"/>
        <source>Anaglyph 3D</source>
        <translation>3D Anaglif</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="306"/>
        <source>Invert Eye Positions</source>
        <translation>Odwróć Pozycje Oczu</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="71"/>
        <source>None</source>
        <translation>Brak</translation>
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
        <translation>Liniowa</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1392"/>
        <source>Time</source>
        <translation>Czas</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1415"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Przeciągnij, aby przesunąć, kliknij prawym, aby wyczyścić</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1418"/>
        <source>Click to place cursor</source>
        <translation>Kliknij, aby umieścić kursor</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1420"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Kliknij, aby umieścić drugi kursor — Przeciągnij, aby przesunąć</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Odwiedź Witrynę</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Kup Licencję</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Aktywuj</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Asystent — funkcja Pro</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>Asystent jest funkcją Serial Studio Pro. Aktywuj licencję, aby go odblokować.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Aktywuj</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>Tryb</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>Uruchom Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Nazwany Potok</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Plik Wykonywalny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/ścieżka/do/pliku/wykonywalnego</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Przeglądaj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Argumenty</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 wartość1 --arg2 wartość2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Katalog Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(opcjonalnie) /katalog/roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Ścieżka Potoku</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Wybierz Uruchomiony Proces…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Uruchom proces potomny i przechwyć jego stdout lub połącz się z nazwanym potokiem zapisywanym przez istniejący proces.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Dowiedz się więcej o nazwanych potokach</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Wybierz Uruchomiony Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Wybierz uruchomiony proces, aby uzyskać sugestię ścieżki nazwanego potoku.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Filtruj Procesy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Wpisz, aby filtrować według nazwy…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Odśwież</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Uruchomione Procesy</translation>
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
        <translation>Żaden proces nie pasuje do filtra.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Nie znaleziono uruchomionych procesów.
Kliknij Odśwież, aby zaktualizować listę.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 proces(ów)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Wybierz</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>zmodyfikowano</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="366"/>
        <source>This project is password protected</source>
        <translation>Ten projekt jest chroniony hasłem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="367"/>
        <source>Editing is available in Project mode</source>
        <translation>Edycja jest dostępna w trybie Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="378"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Wprowadź hasło, aby wprowadzić zmiany, lub otwórz inny projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="379"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Przełącz na tryb Projekt, aby załadować i edytować projekt.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="401"/>
        <source>Unlock</source>
        <translation>Odblokuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="402"/>
        <source>Switch to Project Mode</source>
        <translation>Przełącz na Tryb Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="421"/>
        <source>Open Other Project</source>
        <translation>Otwórz Inny Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="422"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="438"/>
        <source>Create New Project</source>
        <translation>Utwórz Nowy Projekt</translation>
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
        <translation>Szukaj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="349"/>
        <source>Move Up</source>
        <translation>Przenieś w Górę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="359"/>
        <source>Move Down</source>
        <translation>Przenieś w Dół</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="374"/>
        <source>Rename</source>
        <translation>Zmień Nazwę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="387"/>
        <source>Disable</source>
        <translation>Wyłącz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="387"/>
        <source>Enable</source>
        <translation>Włącz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="410"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Duplikuj Zaznaczone (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="411"/>
        <source>Duplicate</source>
        <translation>Duplikuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="429"/>
        <source>Delete Selected (%1)</source>
        <translation>Usuń Zaznaczone (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="430"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="481"/>
        <source>New Folder</source>
        <translation>Nowy Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="491"/>
        <source>New Sub-folder</source>
        <translation>Nowy Podfolder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="507"/>
        <source>Move to Folder</source>
        <translation>Przenieś do Folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="514"/>
        <source>Top Level</source>
        <translation>Najwyższy Poziom</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="547"/>
        <source>Move Here</source>
        <translation>Przenieś Tutaj</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Nowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Utwórz nowy projekt JSON</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Otwórz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Otwórz istniejący projekt JSON</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Zapisz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Zapisz bieżący projekt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Zapisz Jako</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Zapisz bieżący projekt pod nową nazwą</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">Odblokuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Zablokuj</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">Odblokuj Edytor Projektu hasłem projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Importuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Wygeneruj projekt ze schematu Protocol Buffers (.proto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Ustaw hasło i zablokuj Edytor Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Dodaj Urządzenie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Dodaj nowe źródło danych (urządzenie) do projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Akcja</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Dodaj nową akcję do projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Wyjście</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Przywróć</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Przywróć ostatnią automatyczną migawkę bieżącego projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Dodaj nowy panel sterowania wyjściem z przyciskiem</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Suwak</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Dodaj kontrolkę wyjściową suwaka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Przełącznik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Dodaj kontrolkę wyjściową przełącznika</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Pokrętło</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Dodaj kontrolkę wyjściową pokrętła</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Pole Tekstowe</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Dodaj kontrolkę wyjściową pola tekstowego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Przycisk</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Dodaj kontrolkę przycisku wyjściowego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>Dodaj ogólny zestaw danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Wykres</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>Dodaj zestaw danych wykresu 2D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>Wykres FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Dodaj wykres szybkiej transformaty Fouriera</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Wskaźnik</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Dodaj widget wskaźnika dla danych numerycznych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Wskaźnik Poziomu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Dodaj pionowy wskaźnik poziomu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>Kompas</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Dodaj widget kompasu dla danych kierunkowych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>Wskaźnik LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Dodaj wskaźnik stanu w stylu LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Grupa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Dodaj grupę kontenerową zestawu danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Kontener Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Obraz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Dodaj przeglądarkę strumienia obrazu/wideo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Widok Obrazu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="450"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Web View</source>
        <translation>Widok Internetowy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="452"/>
        <source>Add an web viewer</source>
        <translation>Dodaj przeglądarkę internetową</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="462"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="466"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Dodaj niestandardowy widget painter renderowany w JavaScript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="467"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Widgety painter wymagają licencji Pro — dodanie spowoduje powrót do siatki danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="468"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="480"/>
        <source>Table</source>
        <translation>Tabela</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Add a data table view</source>
        <translation>Dodaj widok tabeli danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Data Grid</source>
        <translation>Siatka Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="491"/>
        <source>Multi-Plot</source>
        <translation>Wykres Wielokrotny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="493"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>Dodaj wykres 2D z wieloma sygnałami</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Multiple Plot</source>
        <translation>Wykres Wielokrotny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="500"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>3D Plot</source>
        <translation>Wykres 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <source>Add a 3D plot visualization</source>
        <translation>Dodaj wizualizację wykresu 3D</translation>
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
        <translation>Dodaj grupę dla danych akcelerometru 3-osiowego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="521"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="524"/>
        <source>Gyroscope</source>
        <translation>Żyroskop</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>Dodaj grupę dla danych żyroskopu 3-osiowego</translation>
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
        <translation>Dodaj widżet mapy dla danych GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="547"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="551"/>
        <source>Assistant</source>
        <translation>Asystent</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="554"/>
        <source>Open the Assistant</source>
        <translation>Otwórz Asystenta</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="560"/>
        <source>Help Center</source>
        <translation>Centrum Pomocy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="564"/>
        <source>Open the Project Editor documentation</source>
        <translation>Otwórz dokumentację Edytora Projektu</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Podsumowanie Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>Wykryto funkcje Pro w tym projekcie.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Używane są widżety zastępcze. Kup licencję, aby odblokować pełną funkcjonalność.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Tytuł Projektu:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Projekt Bez Tytułu</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Punkty:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="145"/>
        <source>Settings</source>
        <translation>Ustawienia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="170"/>
        <source>Time Range:</source>
        <translation>Zakres Czasu:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="212"/>
        <source>Point Count:</source>
        <translation>Liczba Punktów:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="230"/>
        <source>Change-Driven Transforms:</source>
        <translation>Transformacje Sterowane Zmianą:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="239"/>
        <source>Run a dataset's transform only when one of its inputs changes. Speeds up large table-driven projects; off by default.</source>
        <translation>Uruchamia transformację zestawu danych tylko wtedy, gdy zmieni się jeden z jego wejść. Przyspiesza duże projekty oparte na tabelach; domyślnie wyłączone.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="286"/>
        <source>Source</source>
        <translation>Źródło</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="287"/>
        <source>Sources</source>
        <translation>Źródła</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="292"/>
        <source>Group</source>
        <translation>Grupa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="293"/>
        <source>Groups</source>
        <translation>Grupy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="298"/>
        <source>Dataset</source>
        <translation>Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="299"/>
        <source>Datasets</source>
        <translation>Zestawy Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="304"/>
        <source>Action</source>
        <translation>Akcja</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="305"/>
        <source>Actions</source>
        <translation>Akcje</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="393"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Kliknij dwukrotnie blok, aby go edytować. Kliknij prawym przyciskiem w dowolnym miejscu, aby dodać grupę, zbiór danych, akcję, tabelę danych lub urządzenie.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Podgląd Pliku Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Plik Proto: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Przejrzyj poniższe komunikaty, a następnie utwórz projekt. Każdy komunikat staje się grupą pulpitu; bloki kanałów tego samego typu otrzymują MultiPlot, a komunikaty mieszanych typów otrzymują DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Pokaż pola dla</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Pola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Tag</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Nazwa Pola</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Typ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Brak pól w wybranej wiadomości.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Razem: %1 wiadomości, %2 pól</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Utwórz Projekt</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Brak błędu</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Broker odrzucił połączenie z powodu nieobsługiwanej wersji protokołu. Dopasuj wersję MQTT brokera i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Broker odrzucił identyfikator klienta. Może być nieprawidłowo sformatowany, zbyt długi lub już używany. Wygeneruj go ponownie i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>Połączenie sieciowe z brokerem zostało nawiązane, ale broker jest obecnie niedostępny. Sprawdź jego status i spróbuj ponownie później.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>Nazwa użytkownika lub hasło jest nieprawidłowe lub niepoprawnie sformatowane. Sprawdź dane uwierzytelniające i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Broker odmówił połączenia z powodu niewystarczających uprawnień. Sprawdź, czy konto posiada wymagane listy ACL.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>Problem z siecią lub warstwą transportową uniemożliwił nawiązanie połączenia. Sprawdź łączność, porty i konfigurację TLS.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>Klient wykrył naruszenie protokołu MQTT i zamknął połączenie. Sprawdź zgodność brokera i klienta.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>Wystąpił nieoczekiwany błąd. Sprawdź logi brokera i konsolę aplikacji, aby uzyskać szczegóły.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>Wystąpił błąd protokołu MQTT poziomu 5. Sprawdź kod przyczyny brokera, aby uzyskać szczegóły.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Nieokreślony błąd MQTT (kod %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>Nie udało się załadować tekstu powitalnego :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="251"/>
        <source>Network error</source>
        <translation>Błąd sieci</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="254"/>
        <location filename="../../src/Licensing/Trial.cpp" line="270"/>
        <location filename="../../src/Licensing/Trial.cpp" line="302"/>
        <source>Trial Activation Error</source>
        <translation>Błąd Aktywacji Wersji Próbnej</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>Invalid server response</source>
        <translation>Nieprawidłowa odpowiedź serwera</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="268"/>
        <source>The server returned malformed data: %1</source>
        <translation>Serwer zwrócił nieprawidłowe dane: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>Unexpected server response</source>
        <translation>Nieoczekiwana odpowiedź serwera</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="300"/>
        <source>The server response is missing required fields.</source>
        <translation>W odpowiedzi serwera brakuje wymaganych pól.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Błąd Pliku Wyjścia Konsoli</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>Nie można otworzyć pliku do zapisu!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Nieprawidłowy adapter Bluetooth!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>Nieobsługiwana platforma lub system operacyjny</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>Nieobsługiwana metoda wykrywania</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>Ogólny błąd We/Wy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>Parser Ramek Wyłączony</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Parser ramek Lua dla źródła %1 przekroczył limit czasu %2 razy z rzędu i został wyłączony, aby zachować responsywność Serial Studio.

Najprawdopodobniejsza przyczyna: nieskończona pętla lub bardzo wolna operacja w skrypcie. Napraw skrypt i przeładuj projekt, aby ponownie włączyć parsowanie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>Błąd Składni Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>Kod parsera zawiera błąd:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>Błąd Wykonania Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>Kod parsera wywołał błąd:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>Brak Funkcji Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>Funkcja 'parse' nie jest zdefiniowana w skrypcie.

Upewnij się, że kod zawiera:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>Błąd Wykonania Funkcji Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>Funkcja parse zawiera błąd:

%1

Napraw błąd w ciele funkcji.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Parser ramek JavaScript dla źródła %1 przekroczył limit czasu %2 razy z rzędu i został wyłączony, aby zachować responsywność Serial Studio.

Najprawdopodobniejsza przyczyna: nieskończona pętla lub bardzo wolna operacja w skrypcie. Napraw skrypt i przeładuj projekt, aby ponownie włączyć parsowanie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>Przekroczono Limit Czasu Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>Kod parsera nie zakończył ewaluacji w ciągu %1 ms i został przerwany.

Najprawdopodobniejsza przyczyna: nieskończona pętla na najwyższym poziomie skryptu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>Błąd Składni Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>Kod parsera zawiera błąd składni w linii %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>Wystąpił Wyjątek Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>Kod parsera wywołał następujące wyjątki:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>Funkcja 'parse' nie jest zdefiniowana w skrypcie.

Upewnij się, że kod zawiera:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>Funkcja parse zawiera błąd w linii %1:

%2

Napraw błąd w ciele funkcji.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>Nieprawidłowa Deklaracja Funkcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Nie znaleziono eksportu 'parse' będącego funkcją wywoływalną.

Zdefiniuj jedną z poniższych:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>Funkcja 'parse' musi przyjmować co najmniej jeden parametr (ładunek ramki).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Nie znaleziono prawidłowej deklaracji funkcji 'parse'.

Oczekiwany format:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>Nieprawidłowy Parametr Funkcji</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Funkcja 'parse' musi mieć co najmniej jeden parametr.

Oczekiwany format:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>Przestarzała Sygnatura Funkcji</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>Funkcja 'parse' używa starego formatu dwuparametrowego: parse(%1, %2)

Ten format nie jest już obsługiwany. Zaktualizuj do nowego formatu jednoparametrowego:
function parse(%1) { ... }

Parametr separatora nie jest już potrzebny.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Critical</source>
        <translation>Krytyczny</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="191"/>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="577"/>
        <source>Project file not found</source>
        <translation>Nie znaleziono pliku projektu</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="578"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Plik projektu wskazywany przez ten skrót nie został znaleziony:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="581"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Usunąć ten skrót?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="585"/>
        <source>Delete Shortcut</source>
        <translation>Usuń Skrót</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="587"/>
        <source>Quit</source>
        <translation>Zakończ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>Czas (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>Częst.: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>Czas: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Nie ustawiono klucza API OpenAI. Otwórz Zarządzaj Kluczami, aby dodać klucz.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="236"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Nie ustawiono klucza API Anthropic. Otwórz Zarządzaj Kluczami, aby dodać klucz.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Nie ustawiono klucza API Gemini. Otwórz Zarządzaj Kluczami, aby dodać klucz.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="351"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Nie skonfigurowano adresu URL serwera modelu lokalnego. Otwórz Zarządzaj Kluczami, aby go ustawić.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Nie ustawiono klucza API DeepSeek. Otwórz Zarządzaj Kluczami, aby go dodać.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Nie ustawiono klucza API Mistral. Otwórz Zarządzaj Kluczami, aby dodać klucz.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Nie ustawiono klucza API OpenRouter. Otwórz Zarządzaj Kluczami, aby dodać klucz.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Nie ustawiono klucza API Groq. Otwórz Zarządzaj Kluczami, aby dodać klucz.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1149"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Parser ramek wykorzystuje ponad %1% czasu procesora.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1151"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio pomija ramki, aby zachować responsywność aplikacji. Uprość lub zoptymalizuj skrypt parsera ramek, aby zmniejszyć jego obciążenie.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>Oczekiwano %1, otrzymano '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Oczekiwano nazwy wyliczenia po 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>Oczekiwano nazwy oneof</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Znacznik pola '%1' poza zakresem (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Oczekiwano typu klucza w map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Oczekiwano typu wartości w map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>Oczekiwano nazwy pola mapy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>Oczekiwano znacznika pola mapy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>Oczekiwano typu pola, otrzymano '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>Oczekiwano nazwy pola po typie</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>Oczekiwano numeru znacznika pola</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Zagnieżdżenie komunikatów zbyt głębokie (limit %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>Oczekiwano nazwy komunikatu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Nieoczekiwany token '%1' w zakresie pliku</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Nieobsługiwane słowo kluczowe najwyższego poziomu '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Automatycznie (Domyślne dla Platformy)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Programowo (Tryb Awaryjny)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Wiersz %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">Ramka %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Dekoder</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">Wiersze</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 wiersz(y)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>Konfiguracja natywnego parsera nie jest prawidłowym obiektem JSON.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Nieznany szablon natywnego parsera: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Błąd Parsera Wbudowanego</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Błąd Natywnego Parsera</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append nie jest obsługiwany dla GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Otwieranie gzip zarówno do odczytu, jak i zapisu nie jest obsługiwane</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>Możesz otworzyć gzip albo do odczytu, albo do zapisu. Który to ma być?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Nie można wykonać gzopen() na pliku</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append nie jest obsługiwany dla QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite nie jest obsługiwany dla QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>Błąd API ZIP/UNZIP %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>Wygeneruj Raport PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Generuj Raport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="69"/>
        <source>Solid</source>
        <translation>Ciągła</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="70"/>
        <source>Dashed</source>
        <translation>Kreskowana</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="71"/>
        <source>Dotted</source>
        <translation>Kropkowana</translation>
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
        <translation>Letter (8,5 × 11 cali)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8,5 × 14 cali)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="104"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7,25 × 10,5 cala)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 cali)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="106"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 cali)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="120"/>
        <source>%1 — Session Report</source>
        <translation>%1 — Raport Sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="122"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="560"/>
        <source>Session Report</source>
        <translation>Raport Sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="462"/>
        <source>Branding</source>
        <translation>Branding</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="468"/>
        <source>Page</source>
        <translation>Strona</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="474"/>
        <source>Sections</source>
        <translation>Sekcje</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="480"/>
        <source>Datasets</source>
        <translation>Zestawy Danych</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="528"/>
        <source>Identity</source>
        <translation>Tożsamość</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="542"/>
        <source>Company</source>
        <translation>Firma</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="549"/>
        <source>e.g. Acme Test Systems</source>
        <translation>np. Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="553"/>
        <source>Document title</source>
        <translation>Tytuł dokumentu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="564"/>
        <source>Author</source>
        <translation>Autor</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="571"/>
        <source>Prepared by (optional)</source>
        <translation>Przygotowane przez (opcjonalnie)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="580"/>
        <source>Logo</source>
        <translation>Logo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="593"/>
        <source>File</source>
        <translation>Plik</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="604"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG lub SVG (opcjonalnie)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="606"/>
        <source>Browse…</source>
        <translation>Przeglądaj…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="609"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="650"/>
        <source>Paper</source>
        <translation>Papier</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="662"/>
        <source>Page size</source>
        <translation>Rozmiar strony</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="790"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Oznacz wartości min, maks. i średnią na wykresach</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="822"/>
        <source>Include datasets</source>
        <translation>Uwzględnij zestawy danych</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="908"/>
        <source>Loading datasets...</source>
        <translation>Ładowanie zestawów danych…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="928"/>
        <source>Select at least one dataset to include.</source>
        <translation>Wybierz co najmniej jeden zestaw danych do uwzględnienia.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="945"/>
        <source>Export HTML</source>
        <translation>Eksportuj HTML</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="778"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Strona tytułowa (logo, tytuł dokumentu, podtytuł testu)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="781"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Informacje o teście (projekt, znaczniki czasu, klasyfikacja i notatki)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="784"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Podsumowanie pomiarów (min, maks, średnia, odch. std. dla każdego parametru)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="787"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Trendy parametrów (wykres szeregów czasowych dla każdego parametru numerycznego)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="677"/>
        <source>Plot appearance</source>
        <translation>Wygląd wykresu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="691"/>
        <source>Line width</source>
        <translation>Szerokość linii</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="723"/>
        <source>Line style</source>
        <translation>Styl linii</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="763"/>
        <source>Include</source>
        <translation>Uwzględnij</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="937"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="945"/>
        <source>Export PDF</source>
        <translation>Eksportuj PDF</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Generowanie Raportu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>Przetwarzanie…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Może to potrwać kilka sekund dla sesji z wieloma parametrami. Okno zamknie się automatycznie, gdy raport będzie gotowy.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>Utracono Połączenie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>Urządzenie Niedostępne</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>Połączenie z urządzeniem zostało utracone.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio nie może połączyć się z urządzeniem.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Sprawdź kabel, zasilanie i upewnij się, że żadna inna aplikacja nie przejęła urządzenia. Możesz spróbować ponownie połączyć, przełączyć na inne urządzenie lub zakończyć.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Upewnij się, że jest podłączone, włączone i nie jest już używane przez inną aplikację. Możesz spróbować ponownie, wybrać inne urządzenie lub zakończyć.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>Zakończ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>Wybierz Inne Urządzenie</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">Zmień Konfigurację</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>Spróbuj Ponownie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>Połącz Ponownie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Wybierz właściwe urządzenie, a następnie naciśnij Połącz.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>Interfejs I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>Połącz</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="338"/>
        <source>Data Grids</source>
        <translation>Siatki Danych</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="341"/>
        <source>Multiple Data Plots</source>
        <translation>Wiele Wykresów Danych</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="344"/>
        <source>Accelerometers</source>
        <translation>Akcelerometry</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="347"/>
        <source>Gyroscopes</source>
        <translation>Żyroskopy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="350"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="353"/>
        <source>FFT Plots</source>
        <translation>Wykresy FFT</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="356"/>
        <source>LED Panels</source>
        <translation>Panele LED</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="359"/>
        <source>Data Plots</source>
        <translation>Wykresy Danych</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="362"/>
        <source>Bars</source>
        <translation>Słupki</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="365"/>
        <source>Gauges</source>
        <translation>Wskaźniki</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="368"/>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="371"/>
        <source>Clock</source>
        <translation>Zegar</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="374"/>
        <source>Stopwatch</source>
        <translation>Stoper</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="377"/>
        <source>Compasses</source>
        <translation>Kompasy</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="380"/>
        <source>Meters</source>
        <translation>Mierniki</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Termometry</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>3D Plots</source>
        <translation>Wykresy 3D</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Web Views</source>
        <translation>Widoki Internetowe</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="390"/>
        <source>Image Views</source>
        <translation>Widoki Obrazu</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="393"/>
        <source>Output Panels</source>
        <translation>Panele Wyjściowe</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="396"/>
        <source>Notifications</source>
        <translation>Powiadomienia</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="399"/>
        <source>Waterfalls</source>
        <translation>Wodospady</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="402"/>
        <source>Painter Widgets</source>
        <translation>Widżety Rysowania</translation>
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
        <translation>Systemowe</translation>
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
        <translation>Szczegóły Sesji</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Wybierz sesję, aby wyświetlić szczegóły.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Projekt:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Rozpoczęto:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Zakończono:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(w trakcie)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Ramki:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Notatki</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Dodaj notatki sesji…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Notatki są tylko do odczytu dla zakończonych sesji.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Nowy tag…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Odblokuj plik sesji, aby usunąć sesje</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Tagi</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Dodaj</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Odtwórz</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>Eksportuj CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Generuj Raport</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Sesje</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>Data</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>Ramki</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>Tagi</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>Nie znaleziono sesji.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Brak otwartego pliku sesji.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1026"/>
        <source>Select logo image</source>
        <translation>Wybierz obraz logo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1028"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Obrazy (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Open Session File</source>
        <translation>Otwórz Plik Sesji</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="430"/>
        <source>Session files (*.db)</source>
        <translation>Pliki sesji (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1225"/>
        <source>Cannot open session file</source>
        <translation>Nie można otworzyć pliku sesji</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="655"/>
        <source>Delete session from %1?</source>
        <translation>Usunąć sesję z %1?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="658"/>
        <source>Delete Session</source>
        <translation>Usuń Sesję</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1086"/>
        <source>No project data</source>
        <translation>Brak danych projektu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Wszystkie odczyty i surowe dane dla tej sesji zostaną trwale usunięte.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>Lock Session File</source>
        <translation>Zablokuj Plik Sesji</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="487"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Wybierz hasło, aby zablokować plik sesji:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="496"/>
        <source>Confirm the password:</source>
        <translation>Potwierdź hasło:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="504"/>
        <source>Passwords do not match</source>
        <translation>Hasła nie pasują</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="505"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>Wprowadzone hasła nie pasują do siebie. Plik sesji nie został zablokowany.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="541"/>
        <source>Unlock Session File</source>
        <translation>Odblokuj Plik Sesji</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="542"/>
        <source>Enter the session file password:</source>
        <translation>Wprowadź hasło pliku sesji:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="552"/>
        <source>Incorrect password</source>
        <translation>Nieprawidłowe hasło</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="553"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>Wprowadzone hasło nie pasuje do hasła zapisanego w pliku sesji.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="645"/>
        <source>Session file locked</source>
        <translation>Plik sesji zablokowany</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="646"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Odblokuj plik sesji przed usunięciem zapisanych sesji.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">Ta sesja nie zawiera osadzonego pliku projektu — pulpit przełącza się na układ szybkiego wykresu.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>Export Session to CSV</source>
        <translation>Eksportuj Sesję do CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>CSV files (*.csv)</source>
        <translation>Pliki CSV (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">Eksport Zakończony</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">Sesja wyeksportowana do:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">Przygotowywanie eksportu…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Done</source>
        <translation>Gotowe</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="962"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Failed</source>
        <translation>Niepowodzenie</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="968"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1008"/>
        <source>Report Failed</source>
        <translation>Raport Nieudany</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1009"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Nie udało się wygenerować raportu. Sprawdź ścieżkę wyjściową i spróbuj ponownie.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save PDF Report</source>
        <translation>Zapisz Raport PDF</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="860"/>
        <source>Loading session data…</source>
        <translation>Ładowanie danych sesji…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save HTML Report</source>
        <translation>Zapisz Raport HTML</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>PDF files (*.pdf)</source>
        <translation>Pliki PDF (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>HTML files (*.html)</source>
        <translation>Pliki HTML (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1087"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Ten plik sesji nie zawiera osadzonego projektu.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1096"/>
        <source>Invalid project data</source>
        <translation>Nieprawidłowe dane projektu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1097"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>Osadzony JSON projektu jest uszkodzony i nie może zostać przywrócony.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Restore Project</source>
        <translation>Przywróć Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Projekty Serial Studio (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Cannot write file</source>
        <translation>Nie można zapisać pliku</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Check file permissions and try again.</source>
        <translation>Sprawdź uprawnienia i lokalizację pliku</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="77"/>
        <source>Empty file path</source>
        <translation>Pusta ścieżka pliku</translation>
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
        <translation>Baza danych nie jest otwarta</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="263"/>
        <source>Database not open or empty label</source>
        <translation>Baza danych nie jest otwarta lub etykieta jest pusta</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="331"/>
        <source>Invalid label</source>
        <translation>Nieprawidłowa etykieta</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="598"/>
        <source>Cancelled</source>
        <translation>Anulowano</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="717"/>
        <source>Could not load session data</source>
        <translation>Nie udało się załadować danych sesji</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>Składanie raportu…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>Zapisywanie wyjścia…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>Raport Sesji</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>Projekt bez tytułu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>Przygotował</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>Wygenerowano %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>ID Testu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>Czas Trwania</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>Próbki</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>Parametry</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>Rozpoczęto</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>Zakończono</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>Projekt</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>Identyfikator testu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>Czas rozpoczęcia</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>Czas zakończenia</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>Całkowity czas trwania</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>Pobrane próbki</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>Zarejestrowane parametry</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>Klasyfikacja</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>Notatki</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>Informacje o Teście</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>Parametr</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>Jednostki</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>Minimum</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>Maksimum</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>Średnia</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>Odchylenie Std.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>Podsumowanie Pomiarów</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>kliknij kolumnę, aby posortować</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 próbek w ciągu %2 sekund</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>Połączony Widok Parametrów</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>kliknij elementy legendy, aby przełączyć sygnały</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>Trendy Parametrów</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="650"/>
        <source>Min</source>
        <translation>Min</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="651"/>
        <source>Max</source>
        <translation>Maks.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="724"/>
        <source>Page %1 of %2</source>
        <translation>Strona %1 z %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>Ładowanie silnika renderowania…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>Renderowanie wykresów…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>Generowanie PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>Otwórz Plik Sesji</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>Pliki sesji (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>Aktywne Połączenie z Urządzeniem</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Aby użyć tej funkcji, należy rozłączyć się z urządzeniem. Kontynuować?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>Nie można otworzyć pliku sesji</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>Nieznany błąd</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>Brak danych projektu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Ta sesja nie zawiera osadzonego pliku projektu — pulpit przełącza się na układ szybkiego wykresu.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>Sprawdź uprawnienia i lokalizację pliku</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">Nie znaleziono sesji</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">Ten plik nie zawiera żadnych nagranych sesji.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">Sesja nie zawiera kolumn</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">Wybrana sesja nie posiada definicji kolumn.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">Sesja nie zawiera odczytów</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">Wybrana sesja nie zawiera żadnych ramek.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Pusta ścieżka pliku</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Ten plik nie zawiera żadnych nagranych sesji.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Anulowano</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>Wybrana sesja nie zawiera definicji kolumn.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>Wybrana sesja nie zawiera żadnych ramek.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Preferencje</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="166"/>
        <source>Language</source>
        <translation>Język</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="182"/>
        <source>Theme</source>
        <translation>Motyw</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="289"/>
        <source>Workspace Folder</source>
        <translation>Folder Obszaru Roboczego</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="585"/>
        <source>Automatically Check for Updates</source>
        <translation>Automatycznie Sprawdzaj Aktualizacje</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Dashboard</source>
        <translation>Pulpit</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="402"/>
        <source>Export…</source>
        <translation>Eksportuj…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="649"/>
        <source>Data Plotting</source>
        <translation>Wykreślanie Danych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Point Count</source>
        <translation>Liczba Punktów</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="741"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Częstotliwość Odświeżania UI (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1051"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Zawsze Pokazuj Przyciski Paska Zadań</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="872"/>
        <source>Show Actions Panel</source>
        <translation>Pokaż Panel Akcji</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="336"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>Włącz Serwer API (Port 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="85"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1163"/>
        <source>Console</source>
        <translation>Konsola</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="151"/>
        <source>Appearance</source>
        <translation>Wygląd</translation>
    </message>
    <message>
        <source>Files &amp; Updates</source>
        <translation type="vanished">Pliki i Aktualizacje</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="vanished">Zaawansowane</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>Allow External API Connections</source>
        <translation>Zezwól na Zewnętrzne Połączenia API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="888"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Automatycznie Ukryj Pasek Narzędzi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <source>Taskbar</source>
        <translation>Pasek Zadań</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="470"/>
        <source>Rendering Backend</source>
        <translation>Backend Renderowania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="370"/>
        <source>API Access Token</source>
        <translation>Token Dostępu API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Startup</source>
        <translation>Uruchamianie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="202"/>
        <source>Window</source>
        <translation>Okno</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="221"/>
        <source>Custom Window Decorations</source>
        <translation>Niestandardowe Dekoracje Okna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Window Shadow</source>
        <translation>Cień Okna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="264"/>
        <source>Window decoration changes apply after restarting %1.</source>
        <translation>Zmiany dekoracji okna zostaną zastosowane po ponownym uruchomieniu %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="273"/>
        <source>Files</source>
        <translation>Pliki</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>API &amp; Plugins</source>
        <translation>API i Wtyczki</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="452"/>
        <source>Graphics</source>
        <translation>Grafika</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="508"/>
        <source>System</source>
        <translation>System</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="524"/>
        <source>Apply Performance Hints</source>
        <translation>Zastosuj Wskazówki Wydajności</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="538"/>
        <source>Keep Display Awake</source>
        <translation>Utrzymuj Wyświetlacz Aktywny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="558"/>
        <source>Performance hints raise process priority and opt out of OS power throttling. Changes take effect the next time Serial Studio starts.</source>
        <translation>Wskazówki wydajności podnoszą priorytet procesu i wyłączają ograniczanie mocy przez system operacyjny. Zmiany wchodzą w życie przy następnym uruchomieniu Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="569"/>
        <source>Updates &amp; News</source>
        <translation>Aktualizacje i Nowości</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="599"/>
        <source>Show What's New on Startup</source>
        <translation>Pokaż Co Nowego przy Uruchamianiu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="664"/>
        <source>Time Range</source>
        <translation>Zakres Czasu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Small</source>
        <translation>Mały</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Normal</source>
        <translation>Normalny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Large</source>
        <translation>Duży</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Extra Large</source>
        <translation>Bardzo Duży</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Custom</source>
        <translation>Niestandardowy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="857"/>
        <source>Layout</source>
        <translation>Układ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="904"/>
        <source>Auto-Layout Margin</source>
        <translation>Margines Automatycznego Układu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="929"/>
        <source>Auto-Layout Spacing</source>
        <translation>Odstęp Automatycznego Układu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="961"/>
        <source>Video Export</source>
        <translation>Eksport Wideo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="979"/>
        <source>Save Videos by Default</source>
        <translation>Zapisuj Wideo Domyślnie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1030"/>
        <source>Behavior</source>
        <translation>Zachowanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1066"/>
        <source>Show Search Field</source>
        <translation>Pokaż Pole Wyszukiwania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1081"/>
        <source>Auto-hide Taskbar</source>
        <translation>Automatycznie Ukryj Pasek Zadań</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1099"/>
        <source>Hide Delay (ms)</source>
        <translation>Opóźnienie Ukrywania (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1123"/>
        <source>Pinned Buttons</source>
        <translation>Przypięte Przyciski</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1141"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Przeciągnij przypięty przycisk na pasku zadań, aby zmienić jego kolejność.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1162"/>
        <source>Settings</source>
        <translation>Ustawienia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1165"/>
        <source>Clock</source>
        <translation>Zegar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1166"/>
        <source>Stopwatch</source>
        <translation>Stoper</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1167"/>
        <source>Pause / Resume</source>
        <translation>Wstrzymaj / Wznów</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1168"/>
        <source>File Transmission</source>
        <translation>Transmisja Pliku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1169"/>
        <source>AI Assistant</source>
        <translation>Asystent AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1298"/>
        <source>Display</source>
        <translation>Wyświetlanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1313"/>
        <source>Display Mode</source>
        <translation>Tryb Wyświetlania</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="778"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1326"/>
        <source>Font Family</source>
        <translation>Rodzina Czcionek</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="92"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>Notifications</source>
        <translation>Powiadomienia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="400"/>
        <source>Export Protobuf File</source>
        <translation>Eksportuj Plik Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="763"/>
        <source>Dashboard Font</source>
        <translation>Czcionka Pulpitu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="793"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1341"/>
        <source>Font Size</source>
        <translation>Rozmiar Czcionki</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">Eksport Obrazu</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">Zapisuj Obrazy Domyślnie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1358"/>
        <source>Show Timestamps</source>
        <translation>Pokaż Znaczniki Czasu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Data Transmission</source>
        <translation>Transmisja Danych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1392"/>
        <source>Line Ending</source>
        <translation>Zakończenie Linii</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1405"/>
        <source>Input Mode</source>
        <translation>Tryb Wejścia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1418"/>
        <source>Text Encoding</source>
        <translation>Kodowanie Tekstu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1431"/>
        <source>Checksum</source>
        <translation>Suma Kontrolna</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1444"/>
        <source>Echo Sent Data</source>
        <translation>Wyświetlaj Wysłane Dane</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1463"/>
        <source>Escape Codes</source>
        <translation>Kody Sterujące</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1478"/>
        <source>VT100 Emulation</source>
        <translation>Emulacja VT100</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1497"/>
        <source>ANSI Colors</source>
        <translation>Kolory ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1555"/>
        <source>Delivery</source>
        <translation>Dostarczanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1570"/>
        <source>System Notifications</source>
        <translation>Powiadomienia Systemowe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1591"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Wyświetlaj zdarzenia ostrzeżeń/krytyczne jako powiadomienia systemowe, gdy Serial Studio nie jest aktywnym oknem.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1601"/>
        <source>Application Logs</source>
        <translation>Dzienniki Aplikacji</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1616"/>
        <source>Route Warnings to Notifications</source>
        <translation>Przekieruj Ostrzeżenia do Powiadomień</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1637"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Domyślnie wyłączone — QT i QML często emitują ostrzeżenia, a włączenie tej opcji może zagłuszyć rzeczywiste alarmy. Krytyczne komunikaty są zawsze przekierowywane niezależnie od tego ustawienia.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1656"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1696"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1704"/>
        <source>Apply</source>
        <translation>Zastosuj</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Konfiguracja Urządzenia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>Serwer API Aktywny (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>Serwer API Gotowy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>Serwer API Wyłączony</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Parsowanie Ramek</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Tylko Konsola (Bez Parsowania)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Szybki Wykres (Wartości Oddzielone Przecinkami)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Parsuj przez Plik Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Zmień Plik Projektu (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Wybierz Plik Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Eksport Danych</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>Arkusz CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Nagrywanie Sesji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>Nagrywanie MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Dziennik Konsoli</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">Plik CSV</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">Baza Danych Sesji</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">Plik MDF4</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">Zrzut Konsoli</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">Utwórz Plik CSV</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">Utwórz Plik MDF4</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">Utwórz Dziennik Sesji</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">Eksportuj Dane Konsoli</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>Interfejs I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Projekt Wielourządzeniowy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Ten projekt przesyła dane z %1 niezależnych urządzeń.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Każde urządzenie ma własne ustawienia połączenia. Skonfiguruj je w Edytorze Projektu w zakładce Źródła.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Otwórz Edytor Projektu</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">Nowy Skrót</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>Wybierz Ikonę</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">Zapisz Skrót</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>Nowe Wdrożenie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="109"/>
        <source>Save Deployment</source>
        <translation>Zapisz Wdrożenie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>General</source>
        <translation>Ogólne</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Taskbar</source>
        <translation>Pasek Zadań</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="164"/>
        <source>Logging</source>
        <translation>Rejestrowanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="221"/>
        <source>Identity</source>
        <translation>Tożsamość</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="277"/>
        <source>Click to choose an icon</source>
        <translation>Kliknij, aby wybrać ikonę</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="286"/>
        <source>Name:</source>
        <translation>Nazwa:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="295"/>
        <source>Deployment Name</source>
        <translation>Nazwa Wdrożenia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="386"/>
        <source>Theme</source>
        <translation>Motyw</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="396"/>
        <source>Same as Serial Studio</source>
        <translation>Taki sam jak Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="418"/>
        <source>Actions Panel</source>
        <translation>Panel Akcji</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="429"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="638"/>
        <source>File Transmission</source>
        <translation>Transmisja Pliku</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="445"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Dwukrotne kliknięcie tego wdrożenia przenosi bezpośrednio do panelu na żywo dla tego projektu. Bez paska narzędzi ani panelu konfiguracji, tylko dane, a Serial Studio kończy działanie po odłączeniu urządzenia.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="491"/>
        <source>Visibility</source>
        <translation>Widoczność</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="506"/>
        <source>Mode</source>
        <translation>Tryb</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="515"/>
        <source>Always shown</source>
        <translation>Zawsze Widoczny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="516"/>
        <source>Auto-hide</source>
        <translation>Automatyczne Ukrywanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="517"/>
        <source>Hidden</source>
        <translation>Ukryty</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Ukrycie paska zadań usuwa przyciski minimalizacji/maksymalizacji/zamykania okna i wymusza automatyczny układ, dzięki czemu pulpit zawsze wypełnia dostępny obszar.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="536"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>Pasek zadań wysuwa się, gdy użytkownik przesunie kursor w pobliże dolnej krawędzi.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="538"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>Pasek zadań jest stale widoczny u dołu pulpitu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="551"/>
        <source>Pinned Buttons</source>
        <translation>Przypięte Przyciski</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="568"/>
        <source>Console</source>
        <translation>Konsola</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="582"/>
        <source>Notifications</source>
        <translation>Powiadomienia</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="596"/>
        <source>Clock</source>
        <translation>Zegar</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="610"/>
        <source>Stopwatch</source>
        <translation>Stoper</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="624"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="749"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Nagrania są zapisywane w folderze roboczym Serial Studio</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">Nazwa Skrótu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="304"/>
        <source>Change Icon…</source>
        <translation>Zmień Ikonę…</translation>
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
        <translation>Wybierz plik projektu, aby rozpocząć</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="371"/>
        <source>Behavior</source>
        <translation>Zachowanie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="408"/>
        <source>Fullscreen</source>
        <translation>Pełny Ekran</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">Dwukrotne kliknięcie tego skrótu przenosi bezpośrednio do panelu na żywo dla tego projektu. Bez paska narzędzi ani panelu konfiguracji, tylko dane, a Serial Studio kończy działanie po odłączeniu urządzenia.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">Osadź Projekt</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">Dwukrotne kliknięcie tego skrótu przenosi bezpośrednio do panelu na żywo dla tego projektu. Brak paska narzędzi ani panelu konfiguracji, tylko dane, a Serial Studio zamyka się natychmiast po odłączeniu urządzenia.

Włącz Osadź Projekt, aby wbudować projekt w skrót, dzięki czemu będzie działać nawet po przeniesieniu lub usunięciu oryginalnego pliku.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="688"/>
        <source>Recorders</source>
        <translation>Rejestratory</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="703"/>
        <source>CSV File</source>
        <translation>Plik CSV</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="713"/>
        <source>MDF4 File</source>
        <translation>Plik MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="723"/>
        <source>Session Database</source>
        <translation>Baza Danych Sesji</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="733"/>
        <source>Console Log</source>
        <translation>Dziennik Konsoli</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">Nagrania są zapisywane w domyślnej lokalizacji każdego modułu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="778"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="787"/>
        <source>Save</source>
        <translation>Zapisz</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>Cofnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>Ponów</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>Wytnij</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>Wklej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>Zaznacz Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>Formatuj Dokument</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>Formatuj Zaznaczenie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>Waliduj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Sprawdź, czy skrypt kompiluje się poprawnie</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">Zresetuj parametry szablonu do wartości domyślnych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>Resetuj do domyślnego skryptu parsowania</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>Otwórz</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>Importuj plik skryptu do parsowania danych</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Otwórz dokumentację pomocy dotyczącą parsowania danych</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Język:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Natywny</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>Wybierz Szablon…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>Cofnij ostatnią edycję kodu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>Ponów poprzednio cofniętą edycję</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>Wytnij zaznaczony kod do schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>Skopiuj zaznaczony kod do schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>Wklej kod ze schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>Platforma:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>Wbudowany</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">Szablon:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>Testuj z Przykładowymi Danymi</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Oceń</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Duplikuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Utwórz kopię tego źródła danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Usuń to źródło danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>Podstawowe źródło danych nie może zostać usunięte</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>Odtwarzacz Sesji</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>Ładowanie sesji…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="278"/>
        <source>Workspaces</source>
        <translation>Obszary Robocze</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="415"/>
        <source>Actions</source>
        <translation>Akcje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="438"/>
        <source>No Actions Available</source>
        <translation>Brak Dostępnych Akcji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="468"/>
        <source>Plugins</source>
        <translation>Wtyczki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="516"/>
        <source>No Plugins Installed</source>
        <translation>Brak Zainstalowanych Wtyczek</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="101"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="552"/>
        <source>Auto Layout</source>
        <translation>Automatyczny Układ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="109"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="564"/>
        <source>Full Screen</source>
        <translation>Pełny Ekran</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="115"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="576"/>
        <source>Add External Window</source>
        <translation>Dodaj Zewnętrzne Okno</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="157"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="861"/>
        <source>Help Center</source>
        <translation>Centrum Pomocy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="692"/>
        <source>Tools</source>
        <translation>Narzędzia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="821"/>
        <source>No Tools Available</source>
        <translation>Brak Dostępnych Narzędzi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="889"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Quit</source>
        <translation>Zakończ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="947"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="948"/>
        <source>Hide</source>
        <translation>Ukryj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="366"/>
        <source>New Workspace…</source>
        <translation>Nowy Obszar Roboczy…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="135"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="765"/>
        <source>Clock</source>
        <translation>Zegar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="143"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Stopwatch</source>
        <translation>Stoper</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="163"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="789"/>
        <source>Sessions</source>
        <translation>Sesje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="170"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>File Transmission</source>
        <translation>Transmisja Pliku</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="177"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="806"/>
        <source>AI Assistant</source>
        <translation>Asystent AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="352"/>
        <source>Show "%1"</source>
        <translation>Pokaż "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Pokaż Wszystkie Ukryte Obszary Robocze</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="380"/>
        <source>No Workspaces Available</source>
        <translation>Brak Dostępnych Obszarów Roboczych</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="506"/>
        <source>Manage Plugins…</source>
        <translation>Zarządzaj Wtyczkami…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="596"/>
        <source>Export</source>
        <translation>Eksportuj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="627"/>
        <source>CSV File</source>
        <translation>Plik CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="633"/>
        <source>MDF4 File</source>
        <translation>Plik MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="639"/>
        <source>Console Transcript</source>
        <translation>Zapis Konsoli</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="648"/>
        <source>Session Database</source>
        <translation>Baza Danych Sesji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="662"/>
        <source>No Export Formats Available</source>
        <translation>Brak Dostępnych Formatów Eksportu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="121"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="748"/>
        <source>Console</source>
        <translation>Konsola</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="127"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Notifications</source>
        <translation>Powiadomienia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="151"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="780"/>
        <source>Preferences</source>
        <translation>Preferencje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="936"/>
        <source>Edit…</source>
        <translation>Edytuj…</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="882"/>
        <source>Resume</source>
        <translation>Wznów</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="883"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Disconnect</source>
        <translation>Rozłącz</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Zatrzymaj</translation>
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
        <translation>Okrążenie</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Resetuj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Suma</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Brak zapisanych okrążeń</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Naciśnij Okrążenie podczas działania stopera</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="160"/>
        <source>No Data Available</source>
        <translation>Brak Dostępnych Danych</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Wartości Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Szukaj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Grupa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Zestaw Danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Jednostki</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(wirtualny)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Kopiuj kod dostępu %1 do schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Skopiowano kod dostępu zestawu danych</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>Brak zdefiniowanych zestawów danych w tym projekcie.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Żaden zestaw danych nie pasuje do wyszukiwania.</translation>
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
        <translation>Wartość</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Niestandardowa Ikona)</translation>
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
        <translation>Nie</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>Yes</source>
        <translation>Tak</translation>
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
        <translation>Dodaj Podfolder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="128"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Dodaj podfolder wewnątrz tego folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="136"/>
        <source>Add Shared Table</source>
        <translation>Dodaj Tabelę Współdzieloną</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="138"/>
        <source>Add a shared table inside this folder</source>
        <translation>Dodaj tabelę współdzieloną wewnątrz tego folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="148"/>
        <source>Rename</source>
        <translation>Zmień Nazwę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="150"/>
        <source>Rename folder</source>
        <translation>Zmień nazwę folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="158"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="160"/>
        <source>Delete folder</source>
        <translation>Usuń folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="173"/>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="174"/>
        <source>Registers</source>
        <translation>Rejestry</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="264"/>
        <source>This folder is empty. Use the toolbar to add a table or sub-folder.</source>
        <translation>Ten folder jest pusty. Użyj paska narzędzi, aby dodać tabelę lub podfolder.</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Menu Start</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Menu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Szukaj…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Ustawienia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Konsola</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Powiadomienia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Zegar</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Stoper</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>Asystent AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Wznów</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="970"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Połączono z %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="971"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Brak połączenia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="995"/>
        <source>MQTT Publisher</source>
        <translation>Wydawca MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1005"/>
        <source>Status:</source>
        <translation>Status:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Connected</source>
        <translation>Połączono</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1014"/>
        <source>Disconnected</source>
        <translation>Rozłączono</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1021"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1034"/>
        <source>Mode:</source>
        <translation>Tryb:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1047"/>
        <source>Messages sent:</source>
        <translation>Wysłane komunikaty:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1061"/>
        <source>Open MQTT Settings</source>
        <translation>Otwórz Ustawienia MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Transmisja Pliku</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">Wyszukaj widżety…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Usuń z Obszaru Roboczego</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Zaznacz Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Wyślij Dane do Urządzenia</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Pokaż Znacznik Czasu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Echo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>Emuluj VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Kolory ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Wyświetlanie: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">Czy Wiesz, Że?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">Utrzymuj firmware prosty, wysyłając surowe dane i pozwalając Serial Studio parsować je w JavaScript, Lua lub bezkodem w wbudowanych szablonach.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">Nadaj każdemu kanałowi własną funkcję do kalibracji, filtrowania lub konwersji jednostek. Przenieś obliczenia do Serial Studio i utrzymuj firmware lekki.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">Potrzebujesz wartości, której urządzenie nigdy nie wysyła? Wirtualny zestaw danych oblicza własny kanał, jak moc z napięcia i prądu, wykreślany i logowany jako dane.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">Wyłapuj zakłócenia jak oscyloskop. Wykresy osi czasu mają tryb sweep i wyzwalania, a poziom wyzwalania można przeciągnąć bezpośrednio na wykresie.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">Przestań przewijać w poszukiwaniu właściwego widgetu. Pogrupuj je we własne przestrzenie robocze i przeskakuj między nimi z wyszukiwarki paska zadań.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">Nigdy więcej nie trać przebiegu testów. Nagrywaj sesje do lokalnej bazy danych, a następnie przeglądaj, taguj i odtwarzaj je, kiedy potrzebujesz.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">Przekaż dopracowany raport zespołowi w sekundy. Eksportuj dowolną sesję do HTML lub PDF, kompletną z wykresami i statystykami min/max/średnia.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">Zamknij pętlę bez dodatkowych narzędzi. Kontrolki Wyjściowe pozwalają wysyłać komendy z powrotem do urządzenia bezpośrednio z pulpitu.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">Zbuduj wizualizację, której nikt inny nie ma. Widget Painter uruchamia własny skrypt do rysowania w pełni niestandardowej grafiki z przychodzących danych.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">Jedno narzędzie dla każdego połączenia. Serial Studio odczytuje z UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT i Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">Pomiń taniec z terminalem. Wysyłaj i odbieraj pliki przez połączenie szeregowe za pomocą wbudowanych protokołów XMODEM, YMODEM i ZMODEM.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">Masz już mapę rejestrów Modbus lub plik DBC? Wygeneruj gotowy do użycia projekt automatycznie zamiast budować go ręcznie.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">Opisz, co chcesz, i pozwól Asystentowi AI to zbudować. Może tworzyć i edytować projekty dla Ciebie w ośmiu dostawcach modeli.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">Wskazówka %1 z %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">Dowiedz Się Więcej</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">Pokaż Wskazówki przy Uruchomieniu</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">Poprzednia</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">Dalej</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Zamknij</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>Oczekuje na zatwierdzenie</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Gotowe</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Odrzucone</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Zablokowane</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>Uruchomione</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Zatwierdź</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Odrzuć</translation>
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
        <translation>Kopiuj</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Kopiuj Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Zaznacz Wszystko</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Wynik</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Edytor Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Otwórz Edytor Projektu, aby utworzyć lub zmodyfikować układ JSON</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>Odtwarzaj plik CSV tak, jakby były to dane z czujników na żywo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Preferencje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Otwórz ustawienia i preferencje aplikacji</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>Otwórz CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>Otwórz MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Sesje</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Przeglądaj, odtwarzaj i eksportuj nagrane sesje</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">Skróty</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">Utwórz skrót operatora dla bieżącego projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Rozszerzenia</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Przeglądaj i instaluj rozszerzenia</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">Konfiguruj połączenie MQTT (publikuj lub subskrybuj)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Wdróż</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">Utwórz wdrożenie operatora dla bieżącego projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Wybierz komunikację przez port szeregowy (UART)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Audio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Wybierz urządzenie wejścia audio (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Wybierz surową komunikację USB (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Sieć</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>Wybierz komunikację sieciową TCP/UDP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>Wybierz komunikację MODBUS (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>Wybierz komunikację przez urządzenie HID (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Wybierz komunikację Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>Magistrala CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>Wybierz komunikację przez magistralę CAN (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Proces</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Wybierz komunikację przez potok procesu (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Przykłady</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Przeglądaj przykładowe projekty</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Centrum Pomocy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Przeglądaj dokumentację, FAQ i wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Wyświetl szczegółową dokumentację i zadawaj pytania na DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Połącz lub rozłącz ze skonfigurowanym urządzeniem</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>O Programie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Otwórz Projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Otwórz istniejący projekt JSON</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Odtwórz plik MDF4 jak dane z czujników na żywo (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>Asystent</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>Rozmawiaj z AI, aby zbudować i edytować projekt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Zbuduj aplikację operatora dla bieżącego projektu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Pokaż informacje o aplikacji i szczegóły licencji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI Wiki i Chat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Zarządzaj licencją i aktywuj Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Rozłącz</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Połącz</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">Połącz lub rozłącz z urządzeniem lub brokerem MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Aktywuj</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Ustawienia Wyzwalania</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Utrzymuj przebieg nieruchomy, wyrównując każdy sweep do zdarzenia wyzwalającego.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Zablokuj powtarzający się sygnał w miejscu, jak przycisk Auto na oscyloskopie. Każde przejście zaczyna się w tym samym punkcie przebiegu, więc pozostaje nieruchomy zamiast przewijać się.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>Wyzwalacz</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">Tryb:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>Tryb</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>Auto</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>Normalny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Pojedynczy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto działa swobodnie, gdy nic nie przekracza poziomu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal czeka na przekroczenie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single przechwytuje jedno przejście, a następnie zatrzymuje się.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Slope:</source>
        <translation>Zbocze:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="273"/>
        <source>Trigger on a downward crossing</source>
        <translation>Wyzwalanie na zboczu opadającym</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="319"/>
        <source>Timebase:</source>
        <translation>Podstawa Czasu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="388"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Pozostaw podstawę czasu pustą, aby użyć zakresu czasu wykresu; zmniejsz, aby powiększyć szybki sygnał. Holdoff ignoruje nowe wyzwalacze przez chwilę po każdym z nich.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Sygnał:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Wartość do przekroczenia</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Zbocze:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="254"/>
        <source>Rising</source>
        <translation>Narastające</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="258"/>
        <source>Trigger on an upward crossing</source>
        <translation>Wyzwalanie na zboczu narastającym</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="269"/>
        <source>Falling</source>
        <translation>Opadające</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Nowe przejście rozpoczyna się za każdym razem, gdy sygnał przekroczy poziom w wybranym kierunku. Auto również działa swobodnie, gdy nie zostanie znalezione przekroczenie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="292"/>
        <source>Timing</source>
        <translation>Synchronizacja</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Podstawa czasu (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="332"/>
        <source>Match time range</source>
        <translation>Dopasuj zakres czasu</translation>
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
        <translation>Blokada:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="364"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">Podstawa czasu określa, ile czasu pokazuje jedno przejście; pozostaw puste, aby użyć zakresu czasu wykresu. Zmniejsz, aby powiększyć szybki sygnał. Holdoff ignoruje nowe wyzwalacze przez chwilę po każdym z nich.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="403"/>
        <source>Capture Next</source>
        <translation>Przechwyć Następny</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="405"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Uzbrój dla Jednego Kolejnego Przechwycenia Pojedynczego</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Poziom:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Poziom wyzwalania</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Blokada (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Czas blokady</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Źródło:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Ponowne Uzbrojenie</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="418"/>
        <source>Close</source>
        <translation>Zamknij</translation>
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
        <translation>Prędkość Transmisji</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="163"/>
        <source>Data Bits</source>
        <translation>Bity Danych</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="184"/>
        <source>Parity</source>
        <translation>Parzystość</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="205"/>
        <source>Stop Bits</source>
        <translation>Bity Stopu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="226"/>
        <source>Flow Control</source>
        <translation>Kontrola Przepływu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="258"/>
        <source>Auto Reconnect</source>
        <translation>Automatyczne Ponowne Łączenie</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="276"/>
        <source>Send DTR Signal</source>
        <translation>Wyślij Sygnał DTR</translation>
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
        <translation>krytyczny</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>ostrzeżenie</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Wartość %1%2 weszła w pasmo %3 (%4–%5).</translation>
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
        <translation>Konsola</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1725"/>
        <source>Notifications</source>
        <translation>Powiadomienia</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1733"/>
        <source>Clock</source>
        <translation>Zegar</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1740"/>
        <source>Stopwatch</source>
        <translation>Stoper</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1786"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1801"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Awaryjny)</translation>
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
        <translation>Nieprawidłowy</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1069"/>
        <source>Select Background Image</source>
        <translation>Wybierz Obraz Tła</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1071"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Obrazy (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>Urządzenie USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Tryb Transferu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Strumień Masowy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Zaawansowany (Masowy + Sterowanie)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Izochroniczny</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Łącz się z urządzeniami USB przy użyciu transferów masowych, sterujących lub izochronicznych. Odpowiednie dla rejestratorów danych, urządzeń z niestandardowym oprogramowaniem i instrumentów USB.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>Specyfikacje USB (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>Punkt Końcowy IN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>Punkt Końcowy OUT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>Maksymalny Rozmiar Pakietu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Transfery Sterujące Włączone</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>Wysyłanie nieprawidłowych żądań sterujących może spowodować awarię lub uszkodzenie podłączonego sprzętu. Używaj ostrożnie.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Dowiedz się więcej o transferach sterujących USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>Rozmiar pakietu powinien odpowiadać maksymalnemu rozmiarowi transferu zgłaszanemu przez punkt końcowy. Typowe wartości: 192 B (audio FS), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Pobrać aktualizację teraz?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Pobrać aktualizację teraz?&lt;br /&gt;To jest obowiązkowa aktualizacja — zamknięcie spowoduje zakończenie aplikacji.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Lista zmian:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>Wersja %1 aplikacji %2 została wydana!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Brak dostępnych aktualizacji</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Gratulacje! Używasz najnowszej wersji %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="172"/>
        <source>Add Register</source>
        <translation>Dodaj Rejestr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="175"/>
        <source>Add register</source>
        <translation>Dodaj rejestr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="182"/>
        <source>Insert Constant</source>
        <translation>Wstaw Stałą</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="185"/>
        <source>Insert constant</source>
        <translation>Wstaw stałą</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="192"/>
        <source>Import</source>
        <translation>Importuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="195"/>
        <source>Import registers from CSV</source>
        <translation>Importuj rejestry z CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="202"/>
        <source>Export</source>
        <translation>Eksportuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="205"/>
        <source>Export registers to CSV</source>
        <translation>Eksportuj rejestry do CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="217"/>
        <source>Rename</source>
        <translation>Zmień Nazwę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="220"/>
        <source>Rename table</source>
        <translation>Zmień nazwę tabeli</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="227"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="230"/>
        <source>Delete table</source>
        <translation>Usuń tabelę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="244"/>
        <source>Help</source>
        <translation>Pomoc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="249"/>
        <source>Open help documentation for shared memory</source>
        <translation>Otwórz dokumentację pomocy dla pamięci współdzielonej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="289"/>
        <source>Permissions</source>
        <translation>Uprawnienia</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="290"/>
        <source>Register Name</source>
        <translation>Nazwa Rejestru</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="291"/>
        <source>Default Value</source>
        <translation>Wartość Domyślna</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read-Only</source>
        <translation>Tylko do Odczytu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read/Write</source>
        <translation>Odczyt/zapis</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="468"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Kopiuj kod dostępu %1 do schowka</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="501"/>
        <source>Delete register</source>
        <translation>Usuń rejestr</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="518"/>
        <source>No registers.</source>
        <translation>Brak rejestrów.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="568"/>
        <source>Register access code copied</source>
        <translation>Skopiowano kod dostępu rejestru</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="237"/>
        <source>Show Colorbar</source>
        <translation>Pokaż Pasek Kolorów</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="250"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Pokaż Osie i Siatkę</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="263"/>
        <source>Show Crosshair</source>
        <translation>Pokaż Celownik</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Pause</source>
        <translation>Wstrzymaj</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Resume</source>
        <translation>Wznów</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">Wyczyść Historię</translation>
    </message>
</context>
<context>
    <name>WebView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="70"/>
        <source>Web View Unavailable</source>
        <translation>Widok Web Niedostępny</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="81"/>
        <source>This build of Serial Studio was compiled without Qt WebEngine, so web pages cannot be displayed.</source>
        <translation>Ta kompilacja Serial Studio została zbudowana bez QT WebEngine, więc strony internetowe nie mogą być wyświetlane.</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Witaj w %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio to zaawansowane narzędzie do wizualizacji w czasie rzeczywistym, stworzone dla inżynierów, studentów i twórców.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>Możesz rozpocząć w pełni funkcjonalny 14-dniowy okres próbny, aktywować program kluczem licencyjnym lub pobrać i skompilować kod źródłowy GPLv3 samodzielnie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Zakup wersji Pro wspiera autora bezpośrednio i pomaga finansować przyszły rozwój.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Samodzielna kompilacja wersji GPLv3 pomaga rozwijać społeczność i zachęca do wkładu technicznego.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Proszę czekać…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>Pozostało %1 dni okresu próbnego.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">Obecnie korzystasz z pełnej wersji próbnej %1 Pro. Jest ważna przez 14 dni do użytku osobistego, niekomercyjnego.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>Obecnie korzystasz z pełnej wersji próbnej %1 Pro. Jest ważna przez 14 dni do użytku osobistego, niekomercyjnego.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Zaktualizuj do płatnego planu, aby kontynuować korzystanie z Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Lub skompiluj kod źródłowy GPLv3, aby używać go za darmo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Aby zobaczyć dostępne plany subskrypcji, kliknij poniżej „Zaktualizuj teraz".</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Nie przypominaj mi o wersji próbnej.
Rozumiem, że po jej zakończeniu będę musiał kupić licencję lub zbudować wersję GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>Twoja wersja próbna %1 wygasła.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Okres próbny zakończony. Aby kontynuować korzystanie z %1 ze wszystkimi funkcjami Pro, zaktualizuj do płatnego planu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>Jeśli wolisz, możesz również skompilować wersję open-source na licencji GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Dziękujemy za wypróbowanie %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Zaktualizuj Teraz</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Aktywuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>Otwórz w Trybie Ograniczonym</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>Kontynuuj</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>Rozpocznij Okres Próbny</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>Co nowego w %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Lua i Wbudowane Parsery</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>Parsuj ramki w Lua 5.4 lub za pomocą bezobsługowych szablonów wbudowanych, obok JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>Asystent AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>Wbudowany asystent obsługujący ośmiu dostawców, który może tworzyć i edytować projekty za Ciebie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Device Control Loops</source>
        <translation>Pętle Sterowania Urządzeniem</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Automate your device with an Arduino-style setup() and loop() routine that can read, write, and drive the dashboard.</source>
        <translation>Automatyzuj urządzenie za pomocą procedur setup() i loop() w stylu Arduino, które mogą odczytywać, zapisywać i sterować panelem.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>Rozwinięcie i Wyzwalacz Oscyloskopu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>Rozwinięcie w stylu oscyloskopu z animowanym kursorem wyzwalacza, który można przeciągać na wykresie.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Output Controls</source>
        <translation>Sterowanie Wyjściowe</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>Transmituj z powrotem do urządzenia za pomocą widżetów sterowania i silnika pomocniczego protokołu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Dashboard Workspaces</source>
        <translation>Obszary Robocze Pulpitu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>Grupuj widżety we własne pulpity i znajdź je za pomocą wyszukiwania na pasku zadań.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Session Database &amp; Reports</source>
        <translation>Baza Danych Sesji i Raporty</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>Nagrywaj sesje do SQLITE, odtwarzaj je i eksportuj raporty HTML lub PDF.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>Operator Deployments</source>
        <translation>Wdrożenia dla Operatorów</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>Dostarcz zablokowaną kompilację jednorazowego przeznaczenia dla operatorów z trybem tylko do uruchomienia.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>New Dashboard Widgets</source>
        <translation>Nowe Widżety Pulpitu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>Tarcze wskaźników i mierników z odczytami na żywo, plus Zegar, Stoper i Wodospad.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Dataset Transforms</source>
        <translation>Transformacje Zestawu Danych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>Kalibruj, filtruj i konwertuj każdy kanał za pomocą funkcji per-dataset lub dodaj wirtualne datasety obliczające nowe kanały.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Painter Widget</source>
        <translation>Widget Painter</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>Rysuj w pełni niestandardową grafikę z przychodzących danych za pomocą własnego skryptu rysującego.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Data Tables</source>
        <translation>Tabele Danych</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>Tabele w stylu rejestrów na żywo z wirtualnymi datasetami i edytowalnymi komórkami.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Image Widget</source>
        <translation>Widget Obrazu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>Dekoduj i wyświetlaj ramki obrazu przesyłane bezpośrednio z urządzenia.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Notifications &amp; Alarms</source>
        <translation>Powiadomienia i Alarmy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>Wielopasmowe alarmy datasetów z poziomami ważności, kierowane do Centrum Powiadomień.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>Project Lock</source>
        <translation>Blokada Projektu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>Zablokuj projekt, aby oddzielić użytkowanie operatora od edycji, za pomocą kodu dostępu.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="193"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf i Przesyłanie Plików</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="194"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>Wejście i publikowanie MQTT, import Protobuf oraz transfery XMODEM / YMODEM / ZMODEM.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="240"/>
        <source>Welcome to %1!</source>
        <translation>Witaj w %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="250"/>
        <source>Here's what's new in version %1.</source>
        <translation>Oto nowości w wersji %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="423"/>
        <source>Show on Startup</source>
        <translation>Pokaż przy Uruchomieniu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="430"/>
        <source>Close</source>
        <translation>Zamknij</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">Usuń z Obszaru Roboczego</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="345"/>
        <source>Device Disconnected</source>
        <translation>Urządzenie Rozłączone</translation>
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
        <translation type="vanished">krytyczny</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">ostrzeżenie</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">Wartość %1%2 weszła w pasmo %3 (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Wartość %1%2 osiągnęła górny próg alarmowy %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Wartość %1%2 osiągnęła dolny próg alarmowy %3%2</translation>
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
        <translation>PŁD-ZAC</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="184"/>
        <source>W</source>
        <translation>ZAC</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="187"/>
        <source>NW</source>
        <translation>PŁN-ZAC</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>Wartość</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">Oczekiwanie na dane…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>Zdjęcia Satelitarne</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Zdjęcia Satelitarne z Etykietami</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>Mapa Ulic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>Mapa Topograficzna</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>Teren</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>Jasne Szare Płótno</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>Ciemne Szare Płótno</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>Dodatkowe warstwy mapy są dostępne tylko dla użytkowników Pro.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Nie możemy oferować nieograniczonego dostępu, ponieważ klucz API ArcGIS generuje rzeczywiste koszty.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Time (s)</source>
        <translation>Czas (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Samples</source>
        <translation>Próbki</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Przekroczono limit czasu skryptu transmisji po %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>Ładunek przekracza maksymalny rozmiar</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="90"/>
        <source>Time (s)</source>
        <translation>Czas (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="116"/>
        <source>Samples</source>
        <translation>Próbki</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="950"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Interwał Siatki: %1 jednostek</translation>
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
        <translation>Skala Szarości</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>Nieznany</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Edytuj Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Nowy Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Nazwa:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>Mój Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Wybierz widgety do uwzględnienia:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Filtruj widgety…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
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
        <translation>Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="127"/>
        <source>Add Sub-folder</source>
        <translation>Dodaj Podfolder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="130"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Dodaj podfolder wewnątrz tego folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="138"/>
        <source>Add Workspace</source>
        <translation>Dodaj Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="141"/>
        <source>Add a workspace inside this folder</source>
        <translation>Dodaj obszar roboczy wewnątrz tego folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="151"/>
        <source>Rename</source>
        <translation>Zmień Nazwę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="153"/>
        <source>Rename folder</source>
        <translation>Zmień nazwę folderu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="162"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="164"/>
        <source>Delete folder</source>
        <translation>Usuń folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="178"/>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="179"/>
        <source>Contents</source>
        <translation>Zawartość</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="269"/>
        <source>This folder is empty. Use the toolbar to add a workspace or sub-folder.</source>
        <translation>Ten folder jest pusty. Użyj paska narzędzi, aby dodać przestrzeń roboczą lub podfolder.</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="43"/>
        <source>Workspace</source>
        <translation>Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Add Widget</source>
        <translation>Dodaj Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="146"/>
        <source>Add widget to workspace</source>
        <translation>Dodaj widget do obszaru roboczego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="157"/>
        <source>Move Up</source>
        <translation>Przenieś w Górę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="159"/>
        <source>Move workspace up</source>
        <translation>Przenieś przestrzeń roboczą w górę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="169"/>
        <source>Move Down</source>
        <translation>Przenieś w Dół</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="171"/>
        <source>Move workspace down</source>
        <translation>Przenieś przestrzeń roboczą w dół</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="181"/>
        <source>Change Icon</source>
        <translation>Zmień Ikonę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Change workspace icon</source>
        <translation>Zmień ikonę przestrzeni roboczej</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="198"/>
        <source>Rename</source>
        <translation>Zmień Nazwę</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="200"/>
        <source>Rename workspace</source>
        <translation>Zmień nazwę obszaru roboczego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="209"/>
        <source>Delete</source>
        <translation>Usuń</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="211"/>
        <source>Delete workspace</source>
        <translation>Usuń obszar roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="233"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="239"/>
        <source>Group</source>
        <translation>Grupa</translation>
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
        <translation>(nieznany)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="303"/>
        <source>(group widget)</source>
        <translation>(widget grupy)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="353"/>
        <source>Remove widget from workspace</source>
        <translation>Usuń widget z obszaru roboczego</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">Usuń z obszaru roboczego</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="373"/>
        <source>No widgets in this workspace.</source>
        <translation>Brak widżetów w tym obszarze roboczym.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Obszary Robocze</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="124"/>
        <source>Add Folder</source>
        <translation>Dodaj Folder</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="126"/>
        <source>Add a top-level folder</source>
        <translation>Dodaj folder najwyższego poziomu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="151"/>
        <source>Customize</source>
        <translation>Dostosuj</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="153"/>
        <source>Edit workspaces manually</source>
        <translation>Edytuj obszary robocze ręcznie</translation>
    </message>
    <message>
        <source>Move Up</source>
        <translation type="vanished">Przenieś w Górę</translation>
    </message>
    <message>
        <source>Move the selected workspace up</source>
        <translation type="vanished">Przenieś wybrany obszar roboczy w górę</translation>
    </message>
    <message>
        <source>Move Down</source>
        <translation type="vanished">Przenieś w Dół</translation>
    </message>
    <message>
        <source>Move the selected workspace down</source>
        <translation type="vanished">Przenieś wybrany obszar roboczy w dół</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="135"/>
        <source>Add Workspace</source>
        <translation>Dodaj Obszar Roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="137"/>
        <source>Add workspace</source>
        <translation>Dodaj obszar roboczy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="167"/>
        <source>Cleanup</source>
        <translation>Wyczyść</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="170"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>Usuń %1 odniesień widgetów, których docelowa grupa lub zbiór danych już nie istnieje</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>No stale widget references in any workspace</source>
        <translation>Brak nieaktualnych odniesień widgetów w żadnym obszarze roboczym</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>Title</source>
        <translation>Tytuł</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="189"/>
        <source>Contents</source>
        <translation>Zawartość</translation>
    </message>
    <message>
        <source>Widgets</source>
        <translation type="vanished">Widżety</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="282"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>Brak obszarów roboczych. Dodaj jeden za pomocą paska narzędzi powyżej lub przywróć automatyczny układ.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Projekt nie zawiera odpowiednich grup -- dodaj grupę z widżetami, aby wypełnić obszary robocze.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="290"/>
        <source>Reset to Auto Layout</source>
        <translation>Przywróć Automatyczny Układ</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">Brak obszarów roboczych.</translation>
    </message>
</context>
</TS>