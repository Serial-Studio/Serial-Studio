<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="ko_KR" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="167"/>
        <source>Anthropic error</source>
        <translation>Anthropic 오류</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="280"/>
        <source>Stream parse error: %1</source>
        <translation>스트림 파싱 오류: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="327"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="330"/>
        <source>Invalid API key (%1)</source>
        <translation>유효하지 않은 API 키 (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="332"/>
        <source>Rate limited: %1</source>
        <translation>속도 제한됨: %1</translation>
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
        <translation>AI 제공자를 전환하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="345"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>다른 제공자로 전환하면 현재 대화가 지워집니다. 계속하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="348"/>
        <source>Assistant</source>
        <translation>어시스턴트</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="385"/>
        <source>AI Assistant requires a Pro license</source>
        <translation>AI 어시스턴트는 Pro 라이선스가 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Set an API key first</source>
        <translation>먼저 API 키를 설정하십시오</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="166"/>
        <source>AI Assistant requires a Pro license</source>
        <translation>AI 어시스턴트는 Pro 라이선스가 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="172"/>
        <source>AI subsystem not initialized</source>
        <translation>AI 하위 시스템이 초기화되지 않음</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="178"/>
        <source>Already busy with a previous request</source>
        <translation>이전 요청을 처리 중입니다</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="462"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>이번 턴의 도구 호출 한도에 도달했습니다. 더 이상 도구가 실행되지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1725"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>이번 턴의 도구 호출 한도에 도달했습니다. 더 이상 도구를 요청하지 마십시오. 지금까지 찾은 내용을 요약하고, 작업이 완료되지 않았다면 남은 단계를 알려 사용자가 계속 진행하도록 할 수 있습니다.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">도구 호출 한도 초과</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="912"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(모델이 빈 응답을 반환했습니다. 다른 표현으로 다시 시도하거나, 다른 모델로 전환하거나, 제공업체의 안전 필터에서 요청이 허용되는지 확인하십시오.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1009"/>
        <source>Sending request to %1...</source>
        <translation>%1에 요청 전송 중...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1021"/>
        <source>Provider returned no reply</source>
        <translation>제공업체가 응답을 반환하지 않음</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="147"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Gemini 안전 필터에 의해 프롬프트가 차단됨: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="190"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini가 응답을 생성하지 않고 중단됨: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="250"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="253"/>
        <source>Invalid API key (%1)</source>
        <translation>유효하지 않은 API 키 (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="255"/>
        <source>Rate limited: %1</source>
        <translation>속도 제한됨: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="257"/>
        <source>Invalid API key</source>
        <translation>유효하지 않은 API 키</translation>
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
        <translation>유효하지 않은 API 키 (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="326"/>
        <source>Rate limited: %1</source>
        <translation>속도 제한됨: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="328"/>
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
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Export Protobuf File</source>
        <translation>Protobuf 파일 내보내기</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="425"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers(*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="474"/>
        <source>Unable to start gRPC server</source>
        <translation>GRPC 서버를 시작할 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="475"/>
        <source>Failed to bind to %1</source>
        <translation>%1에 바인딩 실패</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="438"/>
        <source>Unable to start API TCP server</source>
        <translation>API TCP 서버를 시작할 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="483"/>
        <source>Allow External API Connections?</source>
        <translation>외부 API 연결을 허용하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="484"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>API 서버를 외부 호스트에 노출하면 네트워크의 다른 장치가 포트 7777에서 Serial Studio에 연결할 수 있습니다.

신뢰할 수 있는 네트워크에서만 활성화하십시오. 신뢰할 수 없는 클라이언트는 실시간 데이터를 읽거나 장치에 명령을 전송할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="519"/>
        <source>Unable to restart API TCP server</source>
        <translation>API TCP 서버를 재시작할 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="598"/>
        <source>Allow API device control?</source>
        <translation>API 장치 제어를 허용하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="599"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Serial Studio의 로컬 API를 사용하는 프로그램이 연결된 장치로 데이터를 전송하도록 요청하고 있습니다. API 클라이언트가 장치에 쓰기를 수행하도록 허용하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="602"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1255"/>
        <source>API server</source>
        <translation>API 서버</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1255"/>
        <source>Invalid pending connection</source>
        <translation>잘못된 대기 중인 연결</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>버전 %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Copyright © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>All Rights Reserved</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1은(는) 자유 소프트웨어입니다. Free Software Foundation이 발행한 GNU General Public License 버전 3 또는 그 이후 버전의 조건에 따라 재배포 및 수정할 수 있습니다.

%1은(는) 유용하게 사용되기를 바라며 배포되지만, 상품성 또는 특정 목적에의 적합성에 대한 묵시적 보증을 포함한 어떠한 보증도 제공하지 않습니다. 자세한 내용은 GNU General Public License를 참조하십시오.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>이 구성은 상업적 및 독점적 사용을 위해 라이선스가 부여되었습니다. 상업 라이선스 조건에 따라 비공개 소스 및 상업용 애플리케이션에서 사용할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>이 구성은 개인 및 평가 목적으로만 사용할 수 있습니다. 유효한 상업 라이선스가 활성화되지 않은 경우 상업적 사용은 금지됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>이 소프트웨어는 상품성 또는 특정 목적에의 적합성에 대한 보증을 포함하되 이에 국한되지 않는 명시적 또는 묵시적 보증 없이 '있는 그대로' 제공됩니다. 저작자는 이 소프트웨어의 사용으로 인해 발생하는 어떠한 손해에 대해서도 책임을 지지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>라이선스 관리</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>기부</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>업데이트 확인</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>License Agreement</source>
        <translation>라이선스 계약</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>Report Bug</source>
        <translation>버그 신고</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Acknowledgements</source>
        <translation>감사의 글</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="249"/>
        <source>Website</source>
        <translation>웹사이트</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="265"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="170"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="171"/>
        <source>Settings</source>
        <translation>설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="229"/>
        <source>G-FORCE</source>
        <translation>G-FORCE</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="265"/>
        <source>PITCH ↕</source>
        <translation>PITCH ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="300"/>
        <source>ROLL ↔</source>
        <translation>ROLL ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>가속도계 구성</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>가속도계 표시 범위 및 입력 단위를 구성합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>표시 범위</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>최대 G:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>최대 G 값 입력</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>입력 구성</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>입력이 이미 G 단위임</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>감사의 글</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>QT 정보…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>아이콘 변경</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>이 액션에 사용할 아이콘 변경</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>모든 설정과 함께 이 액션 복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>프로젝트에서 이 액션 삭제</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>사용 가능한 위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>행을 클릭하여 작업 공간에 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>검색</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>이름</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(전체 그룹)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>이미 작업 공간에 있음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>작업 공간에 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>사용 가능한 위젯이 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>일치하는 위젯이 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>위젯 %1개</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%2개 중 %1개 위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>알람 밴드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="70"/>
        <source>Info</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="151"/>
        <source>OK</source>
        <translation>확인</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="72"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="152"/>
        <source>Warning</source>
        <translation>경고</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Critical</source>
        <translation>심각</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="82"/>
        <source>Tachometer</source>
        <translation>타코미터</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="84"/>
        <source>Idle</source>
        <translation>유휴</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Operating</source>
        <translation>작동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Caution</source>
        <translation>주의</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Redline</source>
        <translation>레드라인</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="91"/>
        <source>Speedometer</source>
        <translation>속도계</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="93"/>
        <source>Cruise</source>
        <translation>순항</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Fast</source>
        <translation>고속</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Top Speed</source>
        <translation>최고 속도</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="99"/>
        <source>Engine Temperature</source>
        <translation>엔진 온도</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="101"/>
        <source>Cold</source>
        <translation>저온</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="143"/>
        <source>Normal</source>
        <translation>보통</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <source>Warm</source>
        <translation>고온</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Overheat</source>
        <translation>과열</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="108"/>
        <source>Pressure</source>
        <translation>압력</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="110"/>
        <source>Vacuum</source>
        <translation>진공</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>High</source>
        <translation>높음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <source>Burst</source>
        <translation>파열</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="117"/>
        <source>Battery Voltage</source>
        <translation>배터리 전압</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="119"/>
        <source>Low</source>
        <translation>낮음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Nominal</source>
        <translation>정상</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="125"/>
        <source>Fuel Level</source>
        <translation>연료 수준</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="127"/>
        <source>Empty</source>
        <translation>비어 있음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Reserve</source>
        <translation>예비</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="133"/>
        <source>Signal Strength</source>
        <translation>신호 강도</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="135"/>
        <source>No Signal</source>
        <translation>신호 없음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>Weak</source>
        <translation>약함</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Good</source>
        <translation>양호</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="141"/>
        <source>CPU / System Load</source>
        <translation>CPU / 시스템 부하</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Busy</source>
        <translation>사용 중</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Overload</source>
        <translation>과부하</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="149"/>
        <source>OK / Warning / Critical</source>
        <translation>정상 / 경고 / 심각</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="270"/>
        <source>Choose Band Color</source>
        <translation>밴드 색상 선택</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="297"/>
        <source>Presets</source>
        <translation>프리셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="320"/>
        <source>Preset</source>
        <translation>프리셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="335"/>
        <source>Choose preset…</source>
        <translation>프리셋 선택…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="557"/>
        <source>Reset to severity default</source>
        <translation>심각도 기본값으로 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="571"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>클릭하여 색상을 선택하세요. 우클릭하면 심각도 기본값으로 재설정됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="572"/>
        <source>Click to choose a custom color.</source>
        <translation>클릭하여 사용자 지정 색상을 선택하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="657"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>정의된 밴드가 없습니다. 위에서 프리셋을 선택하거나 밴드를 추가하여 시작하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="781"/>
        <source>Apply</source>
        <translation>적용</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="784"/>
        <source>Apply changes to the dataset.</source>
        <translation>데이터셋 변경 사항을 적용합니다.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">프리셋 적용</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">현재 밴드를 선택한 프리셋으로 교체하며, 이 데이터셋의 범위에 맞게 조정됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="346"/>
        <source>Range</source>
        <translation>범위</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="374"/>
        <source>Bands</source>
        <translation>밴드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="385"/>
        <source>Add Band</source>
        <translation>밴드 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="389"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>마지막 밴드에 이어서 새 밴드를 추가합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="420"/>
        <source>Min</source>
        <translation>최소값</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="426"/>
        <source>Max</source>
        <translation>최대값</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="432"/>
        <source>Severity</source>
        <translation>심각도</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="438"/>
        <source>Color</source>
        <translation>색상</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="445"/>
        <source>Label</source>
        <translation>레이블</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">사용자 지정 색상을 선택하세요.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">자동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="594"/>
        <source>(optional)</source>
        <translation>(선택 사항)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="611"/>
        <source>Move up.</source>
        <translation>위로 이동합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="630"/>
        <source>Move down.</source>
        <translation>아래로 이동합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="643"/>
        <source>Remove this band.</source>
        <translation>이 밴드를 제거합니다.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">정의된 밴드가 없습니다. 프리셋을 적용하거나 밴드를 추가하여 시작하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="674"/>
        <source>Preview</source>
        <translation>미리보기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="770"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="772"/>
        <source>Discard changes.</source>
        <translation>변경 사항 폐기.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">저장</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">데이터셋 변경 사항을 저장합니다.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>CSV vs MDF4 내보내기 - 차이점은 무엇인가요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Bar, Gauge - 각각 언제 사용하나요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="203"/>
        <source>How can I help with your project?</source>
        <translation>프로젝트에 대해 어떻게 도와드릴까요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>Set up your API key to get started</source>
        <translation>시작하려면 API 키를 설정하세요</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="216"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>구축하려는 내용을 설명하시면 소스, 그룹, 데이터셋, 프레임 파서 및 변환을 구성해 드리겠습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="219"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>채팅을 시작하려면 선택한 공급자의 API 키를 붙여넣으세요. 키는 이 컴퓨터에서 암호화되며 선택한 공급자와 통신하는 경우를 제외하고는 컴퓨터 밖으로 전송되지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="240"/>
        <source>Open API Key Setup</source>
        <translation>API 키 설정 열기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Get a key from %1</source>
        <translation>%1에서 키 받기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>List the sources in this project</source>
        <translation>이 프로젝트의 소스 목록 표시</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="121"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Serial Studio 기능 알아보기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>What can this app do for my telemetry?</source>
        <translation>이 앱이 내 텔레메트리를 위해 무엇을 할 수 있나요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>Walk me through what this project already contains</source>
        <translation>이 프로젝트에 이미 포함된 내용 안내</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="127"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>세션 데이터베이스란 무엇이며, 왜 사용하나요?</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">CSV vs MDF4 내보내기 — 차이점은 무엇인가요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="129"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>프레임 파서란 무엇이며, 언제 필요한가요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>파서에 Lua와 JavaScript 중 언제 무엇을 사용해야 하나요?</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">Plot, Bar, Gauge — 각각 언제 사용하나요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>변환과 프레임 파서의 차이점은 무엇인가요?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="135"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Arduino용 UART 소스 추가</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Set up an IMU project from scratch</source>
        <translation>IMU 프로젝트 처음부터 설정하기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Configure an MQTT subscriber</source>
        <translation>MQTT 구독자 구성</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Add a CAN bus source</source>
        <translation>CAN Bus 소스 추가</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Set up a Modbus poller</source>
        <translation>Modbus 폴러 설정</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>네트워크(TCP/UDP) 소스 추가</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Write a CSV frame parser for me</source>
        <translation>CSV 프레임 파서 작성</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Help me parse a JSON frame</source>
        <translation>JSON 프레임 파싱 지원</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>데이터셋에 EMA 스무딩 변환 추가</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Decode hexadecimal frames</source>
        <translation>16진수 프레임 디코딩</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>선형 변환으로 센서 보정</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="148"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>내 데이터에 적합한 대시보드 위젯 제안</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Build an executive overview workspace</source>
        <translation>경영진 개요 작업 공간 구축</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>사용자 정의 시각화를 위한 Painter 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>하나의 데이터셋에 대한 플롯, FFT 및 워터폴 표시</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>데이터셋을 유용한 작업 공간으로 그룹화</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="446"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Serial Studio에 무엇이든 물어보세요…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="466"/>
        <source>Clear conversation</source>
        <translation>대화 지우기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="510"/>
        <source>Stop generating</source>
        <translation>생성 중지</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="511"/>
        <source>Send message (Enter)</source>
        <translation>메시지 전송 (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="553"/>
        <source>Provider</source>
        <translation>제공자</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="586"/>
        <source>Model selection</source>
        <translation>모델 선택</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="632"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>매번 묻지 않고 편집 작업을 실행합니다. 차단된 작업은 차단 상태를 유지합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="634"/>
        <source>Auto-approve edits</source>
        <translation>편집 자동 승인</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="653"/>
        <source>Manage API keys</source>
        <translation>API 키 관리</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="674"/>
        <source>Working</source>
        <translation>작업 중</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="675"/>
        <source>Ready</source>
        <translation>준비됨</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="676"/>
        <source>  •  cache %1k tok</source>
        <translation>•  캐시 %1k tok</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="677"/>
        <source>  •  cache write %1k tok</source>
        <translation>캐시 쓰기 %1k 토큰</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>마이크가 감지되지 않음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>마이크를 연결하거나 설정을 확인하세요</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>입력 장치</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>샘플 레이트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>샘플 형식</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>채널</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>출력 장치</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">대화 상자</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">다운로드 위치에 대한 사용자 이름과 비밀번호를 입력하십시오.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">사용자 이름(&amp;U):</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">비밀번호(&amp;P):</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="47"/>
        <source>Axis Range Configuration</source>
        <translation>축 범위 구성</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="161"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>플롯 축의 표시 범위를 구성합니다. 입력 시 값이 실시간으로 업데이트됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="169"/>
        <source>X Axis</source>
        <translation>X축</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="194"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="265"/>
        <source>Minimum:</source>
        <translation>최소값:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="206"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="277"/>
        <source>Enter min value</source>
        <translation>최소값 입력</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="215"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="286"/>
        <source>Maximum:</source>
        <translation>최대값:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="227"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="298"/>
        <source>Enter max value</source>
        <translation>최대값 입력</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="242"/>
        <source>Y Axis</source>
        <translation>Y축</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="317"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="327"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>백업 복구</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>제목 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>프로젝트 로드됨</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>자동 저장</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>복원 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>데이터셋 삭제 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>그룹 삭제 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>새 프로젝트 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>프로젝트 열기 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>JSON 로드 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>템플릿 적용 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>액션 삭제 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>출력 위젯 삭제 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>데이터셋 이동 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>그룹 이동 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>작업 공간 삭제 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>모든 작업 공간 지우기 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>위젯 제거 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>작업 공간 재정렬 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>일괄 작업 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>타일 추가 전</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (외 %2개)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>프레임 파서 코드도 다르므로 교체됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>제목이 "%1"에서 "%2"(으)로 변경됩니다. 그룹 구조는 변경되지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>그룹과 데이터셋은 동일하지만 프레임 파서 코드가 다릅니다 — 복원 시 교체됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>현재 프로젝트와 동일한 그룹 및 데이터셋입니다. 복원 시 필드 수준 편집 내용이 되돌려질 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>복원 시 %1이(가) 제거되고 %2이(가) 복구됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>복원 시 %1이(가) 제거됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>복원 시 %1이(가) 복구됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>복원할 백업을 선택하세요. 현재 프로젝트는 자동으로 먼저 저장되므로 복원을 되돌릴 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>이 프로젝트의 백업이 아직 없습니다. 프로젝트를 편집하거나 저장하면 롤링 백업이 시작됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>폴더 열기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>복원</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>장치</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>서비스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>특성</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>스캔 중…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Bluetooth 어댑터가 감지되지 않음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Bluetooth 어댑터를 연결하거나 시스템 설정을 확인하세요</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>이 OS는 아직 지원되지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>QT에서 공식적으로 지원하는 즉시 이 운영 체제에서 작동하도록 Serial Studio를 업데이트할 예정입니다</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>CAN 드라이버를 찾을 수 없음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>시스템용 CAN 하드웨어 드라이버를 설치하세요</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>CAN 드라이버</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="142"/>
        <source>Interface</source>
        <translation>인터페이스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="174"/>
        <source>Bitrate</source>
        <translation>비트레이트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="238"/>
        <source>Flexible Data-Rate</source>
        <translation>Flexible Data-Rate</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="270"/>
        <source>DBC Database</source>
        <translation>DBC 데이터베이스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="274"/>
        <source>Import DBC File…</source>
        <translation>DBC 파일 가져오기…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="307"/>
        <source>No CAN Interfaces Found</source>
        <translation>CAN 인터페이스를 찾을 수 없음</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="210"/>
        <source>Select CSV file</source>
        <translation>CSV 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="212"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 파일 (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="383"/>
        <source>Device Connection Active</source>
        <translation>장치 연결 활성화됨</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="384"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>이 기능을 사용하려면 장치 연결을 해제해야 합니다. 계속하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="399"/>
        <source>Check file permissions and location</source>
        <translation>파일 권한 및 위치 확인</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="432"/>
        <source>Insufficient Data in CSV File</source>
        <translation>CSV 파일의 데이터 부족</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="399"/>
        <source>Cannot read CSV file</source>
        <translation>CSV 파일을 읽을 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="433"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>CSV 파일에는 진행하려면 최소 하나의 데이터 행이 포함되어야 합니다. 파일을 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="669"/>
        <source>Invalid CSV</source>
        <translation>잘못된 CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="670"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>CSV 파일에 데이터 또는 헤더가 포함되어 있지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="679"/>
        <source>Select a date/time column</source>
        <translation>날짜/시간 열 선택</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="679"/>
        <location filename="../../src/CSV/Player.cpp" line="691"/>
        <source>Set interval manually</source>
        <translation>간격 수동 설정</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="681"/>
        <source>CSV Date/Time Selection</source>
        <translation>CSV 날짜/시간 선택</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="682"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>날짜/시간 데이터 처리 방법 선택:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="694"/>
        <source>Set Interval</source>
        <translation>간격 설정</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>행 간 간격을 밀리초 단위로 입력하십시오:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="711"/>
        <source>Select Date/Time Column</source>
        <translation>날짜/시간 열 선택</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="712"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>날짜/시간 데이터가 포함된 열을 선택하십시오:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="722"/>
        <source>Invalid Selection</source>
        <translation>잘못된 선택</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="722"/>
        <source>The selected column is not valid.</source>
        <translation>선택한 열이 유효하지 않습니다.</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>콘솔</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="340"/>
        <source>Console Export is a Pro feature.</source>
        <translation>콘솔 내보내기는 Pro 기능입니다.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="341"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>이 기능을 사용하려면 라이선스가 필요합니다. 콘솔 내보내기를 활성화하려면 라이선스를 구매하십시오.</translation>
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
        <translation>줄 끝 없음</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="236"/>
        <source>New Line</source>
        <translation>새 줄</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="237"/>
        <source>Carriage Return</source>
        <translation>캐리지 리턴</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="238"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="248"/>
        <source>Plain Text</source>
        <translation>일반 텍스트</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="249"/>
        <source>Hexadecimal</source>
        <translation>16진수</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="271"/>
        <source>No Checksum</source>
        <translation>체크섬 없음</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="913"/>
        <source>Device %1</source>
        <translation>장치 %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>상수 삽입</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>기본</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>진공에서의 광속</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>플랑크 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>기본 전하</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>아보가드로 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>볼츠만 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>슈테판-볼츠만 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>역학</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>표준 중력</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>만유인력 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>압력</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>표준 대기압</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>해수면 기압</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>온도</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>절대 영도 (섭씨)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>물의 어는점</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>물의 끓는점 (1 atm)</translation>
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
        <translation>기체 및 유체</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>일반 기체 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>비기체 상수 (건조 공기)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>비기체 상수 (수증기)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>공기 밀도 (해수면, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>물의 밀도 (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>공기 중 음속 (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>비열비 (건조 공기)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>전자기학</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>진공 유전율</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>진공 투자율</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>수학</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>파이</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>오일러 수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>황금비</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>물리 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>SI 단위 사전 설정 값. 행을 클릭하여 %1에 삽입</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>검색</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>기호</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>이름</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>값</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>범주</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>일치하는 상수 없음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1개 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%2개 중 %1개 상수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>복구 옵션</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio가 연속적으로 예기치 않게 종료되었습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>연속 충돌: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>마지막 보고된 단계: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>감지 시각: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>복구 작업을 선택하세요. 적용 후 Serial Studio가 종료되어 다음 실행 시 초기화됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>렌더링 백엔드를 기본값으로 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>마지막으로 열었던 프로젝트 복원 건너뛰기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>모든 환경설정 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>계속 진행</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>CSV 플레이어</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>DBC 파일 미리보기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>DBC 파일: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>새 Serial Studio 프로젝트로 가져올 CAN 메시지 및 신호를 검토합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>메시지</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>메시지 이름</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>CAN ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>신호</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>DBC 파일에서 메시지를 찾을 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>총: %1개 메시지, %2개 신호</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>프로젝트 생성</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="127"/>
        <source>Dashboard %1</source>
        <translation>대시보드 %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>전송</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>전송 함수가 정의되지 않음</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="56"/>
        <source>Set Wallpaper…</source>
        <translation>배경화면 설정…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="62"/>
        <source>Clear Wallpaper</source>
        <translation>배경화면 지우기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="72"/>
        <source>Tile Windows</source>
        <translation>창 바둑판식 배열</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="91"/>
        <source>Pro features detected in this project.</source>
        <translation>이 프로젝트에서 Pro 기능이 감지되었습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="93"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>대체 위젯이 활성화되었습니다. 전체 기능을 사용하려면 라이선스를 구매하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="208"/>
        <source>Empty Workspace</source>
        <translation>빈 작업 공간</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="222"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>검색 바를 사용하여 위젯을 찾아 추가하거나, 다른 작업 공간의 위젯을 마우스 오른쪽 버튼으로 클릭하여 여기에 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="237"/>
        <source>Search Widgets</source>
        <translation>위젯 검색</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>대시보드</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="206"/>
        <source>API Server Active (%1)</source>
        <translation>API 서버 활성 (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="207"/>
        <source>API Server Ready</source>
        <translation>API 서버 준비됨</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="208"/>
        <source>API Server Off</source>
        <translation>API 서버 꺼짐</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="123"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="275"/>
        <source>Send</source>
        <translation>전송</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="263"/>
        <source>Enter command…</source>
        <translation>명령 입력…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>전송 함수가 정의되지 않음</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>명령 입력…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>전송</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>전송 함수가 정의되지 않음</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>켜짐</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>꺼짐</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>전송 함수가 정의되지 않음</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Resume</source>
        <translation>재개</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="297"/>
        <source>Awaiting data…</source>
        <translation>데이터 대기 중…</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>Import DBC File</source>
        <translation>DBC 파일 가져오기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>DBC 파일 (*.DBC);;모든 파일 (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="158"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>DBC 파일 파싱 실패: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="159"/>
        <source>Verify the file format and try again.</source>
        <translation>파일 형식을 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>DBC Import Error</source>
        <translation>DBC 가져오기 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="168"/>
        <source>DBC file contains no messages</source>
        <translation>DBC 파일에 메시지가 없음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="169"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>선택한 파일에 CAN 메시지 정의가 포함되어 있지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC Import Warning</source>
        <translation>DBC 가져오기 경고</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">임시 프로젝트 파일 생성 실패</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">애플리케이션에 임시 디렉터리에 대한 쓰기 권한이 있는지 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="223"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>%1개의 메시지와 %2개의 신호가 포함된 DBC 파일을 성공적으로 가져왔습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="216"/>
        <source>The project editor is now open for customization.</source>
        <translation>프로젝트 편집기가 사용자 지정을 위해 열렸습니다.</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">가져온 프로젝트를 불러오지 못했습니다</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">생성된 프로젝트 JSON을 불러올 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>확장 멀티플렉싱(SG_MUL_VAL_)을 사용하는 신호 %1개를 건너뛰었습니다. 단순 멀티플렉싱만 지원됩니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="228"/>
        <source>DBC Import Complete</source>
        <translation>DBC 가져오기 완료</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="250"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="58"/>
        <source>Dataset Value Transform</source>
        <translation>데이터셋 값 변환</translation>
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
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="119"/>
        <source>Language:</source>
        <translation>언어:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="122"/>
        <source>Template:</source>
        <translation>템플릿:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="101"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>원시 값 입력 (예: 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Test</source>
        <translation>테스트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="107"/>
        <source>Clear</source>
        <translation>지우기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="133"/>
        <source>Input:</source>
        <translation>입력:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="136"/>
        <source>Output:</source>
        <translation>출력:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="109"/>
        <source>Apply</source>
        <translation>적용</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="110"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="216"/>
        <source>Transform — %1</source>
        <translation>변환 — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="306"/>
        <source>Enter a value</source>
        <translation>값 입력</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>Invalid number</source>
        <translation>잘못된 숫자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="383"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>문서 서식 지정	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="384"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>선택 영역 서식 지정	Ctrl+I</translation>
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
-- 실시간 데이터셋 값을 받아 변환된 숫자를 반환하는
-- transform(value) 함수를 정의합니다. transform() 함수가
-- 정의되지 않으면 원시 값이 유지되며 프로젝트에 아무것도
-- 저장되지 않습니다.
--
-- 예제:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- 최상위 수준에서 선언된 전역 변수는 프레임 간에 유지되므로
-- 프레임 파서 스크립트와 마찬가지로 필터, 누산기 및 기타
-- 상태 유지 변환에 유용합니다:
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
-- 미리 만들어진 예제를 보려면 템플릿 드롭다운을 사용하거나
-- 테스트를 클릭하여 함수를 시험해 보십시오.
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
 * 실시간 데이터셋 값을 받아 변환된 숫자를 반환하는
 * transform(value) 함수를 정의하세요. transform() 함수가
 * 정의되지 않으면 원시 값이 유지되며 프로젝트에 아무것도
 * 저장되지 않습니다.
 *
 * 예제:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * 최상위 레벨에서 선언된 전역 변수는 프레임 간에 유지되므로
 * 필터, 누산기 및 기타 상태 유지 변환에 유용합니다.
 * 프레임 파서 스크립트와 동일합니다:
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
 * 템플릿 드롭다운에서 미리 만들어진 예제를 사용하거나
 * 테스트를 클릭하여 함수를 시험해 보세요.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="788"/>
        <source>Select Template…</source>
        <translation>템플릿 선택…</translation>
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
 * 실시간 데이터셋 값을 받아 변환된 숫자를 반환하는
 * transform(value) 함수를 정의합니다. transform() 함수가
 * 정의되지 않으면 원시 값이 유지되며 프로젝트에 저장되지
 * 않습니다.
 *
 * 예제:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * 최상위 레벨에서 선언된 전역 변수는 프레임 간에 유지되므로
 * 필터, 누산기 및 기타 상태 유지 변환에 유용합니다.
 * 프레임 파서 스크립트와 동일합니다:
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
 * 미리 만들어진 예제는 템플릿 드롭다운을 사용하거나
 * 테스트를 클릭하여 함수를 시험해 보십시오.
 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="665"/>
        <source>Engine error</source>
        <translation>엔진 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="706"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="726"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="737"/>
        <source>Error: %1</source>
        <translation>오류: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="698"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="730"/>
        <source>Error: transform() not defined</source>
        <translation>오류: transform()이 정의되지 않음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="712"/>
        <source>Error: transform() must return a number</source>
        <translation>오류: transform()은 숫자를 반환해야 함</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="965"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1083"/>
        <source>Channel %1</source>
        <translation>채널 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1095"/>
        <source>Audio Input</source>
        <translation>오디오 입력</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="974"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1100"/>
        <source>Quick Plot</source>
        <translation>빠른 플롯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="838"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>JavaScript 변환 예산 초과</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="839"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>데이터셋 변환이 %1ms 이상 소요되어, 프레임의 나머지 데이터셋은 다음 프레임까지 원시 값으로 대체되었습니다. 변환 코드를 프로파일링하거나 단순화하세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="182"/>
        <source>Frame pool exhausted</source>
        <translation>프레임 풀 소진</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="184"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>다운스트림 소비자(대시보드, CSV/MDF4 내보내기, 세션 DB, API 구독자)가 프레임을 충분히 빠르게 처리하지 못하고 있습니다. Serial Studio가 백로그가 해소될 때까지 프레임별 할당으로 전환합니다. 무거운 소비자를 비활성화하거나 데이터 속도를 줄이세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="928"/>
        <source>Device A</source>
        <translation>장치 A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="981"/>
        <source>Quick Plot Data</source>
        <translation>빠른 플롯 데이터</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="993"/>
        <source>Multiple Plots</source>
        <translation>다중 플롯</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="248"/>
        <source>None</source>
        <translation>없음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="395"/>
        <source>Invalid Hex Input</source>
        <translation>잘못된 HEX 입력</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="396"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>유효한 16진수 바이트를 입력하세요.

유효한 형식: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="430"/>
        <source>(no frames)</source>
        <translation>(프레임 없음)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="432"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>추출 결과 완전한 프레임이 생성되지 않았습니다. 시작/종료 구분자와 감지 모드를 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="440"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1개 프레임 추출됨 | %2바이트 소비됨 | %3바이트 버퍼됨 | %4개 삭제됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="631"/>
        <source>Pipeline Configuration</source>
        <translation>파이프라인 구성</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="633"/>
        <source>Pipeline Results</source>
        <translation>파이프라인 결과</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="635"/>
        <source>Detection</source>
        <translation>감지</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="636"/>
        <source>Decoder</source>
        <translation>디코더</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="637"/>
        <source>Checksum</source>
        <translation>체크섬</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="638"/>
        <source>Start Delimiter</source>
        <translation>시작 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="639"/>
        <source>End Delimiter</source>
        <translation>종료 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="640"/>
        <source>Hex Delimiters</source>
        <translation>16진수 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="642"/>
        <source>End delimiter only</source>
        <translation>종료 구분자만</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="643"/>
        <source>Start + end delimiters</source>
        <translation>시작 + 종료 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="644"/>
        <source>Start delimiter only</source>
        <translation>시작 구분자만</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="645"/>
        <source>No delimiters (whole chunk)</source>
        <translation>구분자 없음 (전체 청크)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="647"/>
        <source>Plain text (UTF-8)</source>
        <translation>일반 텍스트 (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="648"/>
        <source>Hexadecimal</source>
        <translation>16진수</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="649"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="650"/>
        <source>Binary (raw bytes)</source>
        <translation>바이너리 (원시 바이트)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="652"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="653"/>
        <source>Clear</source>
        <translation>지우기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="654"/>
        <source>Evaluate</source>
        <translation>평가</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="655"/>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="682"/>
        <source>Enter raw stream bytes here...</source>
        <translation>원시 스트림 바이트를 여기에 입력하세요...</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="656"/>
        <source>Stage</source>
        <translation>스테이지</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="632"/>
        <source>Frame Data Input</source>
        <translation>프레임 데이터 입력</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">프레임 파서 결과</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">프레임 데이터를 여기에 입력하세요…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">데이터셋 인덱스</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="656"/>
        <source>Value</source>
        <translation>값</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">위에 프레임 데이터를 입력하고, 필요한 경우 HEX 모드를 활성화한 다음 "평가"를 클릭하여 프레임 파서를 실행하세요.

예제 (텍스트): a,b,c,d,e,f
예제 (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="663"/>
        <source>Test Frame Parser</source>
        <translation>프레임 파서 테스트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="676"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>16진수 바이트 입력 (예: 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(비어 있음)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">반환된 데이터 없음</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="208"/>
        <source>Change Scripting Language?</source>
        <translation>스크립팅 언어를 변경하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="209"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>스크립팅 언어를 전환하면 현재 파서 코드가 새 언어의 해당 템플릿으로 교체됩니다.

저장하지 않은 변경 사항은 손실됩니다. 계속하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="335"/>
        <source>Select Javascript file to import</source>
        <translation>가져올 Javascript 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="335"/>
        <source>Select Lua file to import</source>
        <translation>가져올 Lua 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="369"/>
        <source>Code Validation Successful</source>
        <translation>코드 검증 성공</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="370"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>파서 코드에서 구문 오류가 감지되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="483"/>
        <source>Select Frame Parser Template</source>
        <translation>프레임 파서 템플릿 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="484"/>
        <source>Choose a template to load:</source>
        <translation>로드할 템플릿 선택:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="290"/>
        <source>Import Modbus Register Map</source>
        <translation>Modbus 레지스터 맵 가져오기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="294"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Modbus 레지스터 맵 (*.CSV *.XML *.JSON);;CSV 파일 (*.CSV);;XML 파일 (*.XML);;JSON 파일 (*.JSON);;모든 파일 (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="332"/>
        <source>No registers found</source>
        <translation>레지스터를 찾을 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="333"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>파일을 파싱할 수 없거나 레지스터 정의가 포함되어 있지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="335"/>
        <source>Modbus Import</source>
        <translation>Modbus 가져오기</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">가져온 프로젝트를 불러오지 못했습니다</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">생성된 프로젝트 JSON을 불러올 수 없습니다.</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">임시 프로젝트 파일 생성 실패</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">임시 디렉터리에 대한 쓰기 권한을 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="381"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>%2개 그룹에서 %1개 레지스터를 성공적으로 가져왔습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="383"/>
        <source>The project editor is now open for customization.</source>
        <translation>프로젝트 편집기가 사용자 지정을 위해 열렸습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="385"/>
        <source>Modbus Import Complete</source>
        <translation>Modbus 가져오기 완료</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="702"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="290"/>
        <source>Select Javascript file to import</source>
        <translation>가져올 JavaScript 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="351"/>
        <source>Select Output Widget Template</source>
        <translation>출력 위젯 템플릿 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="352"/>
        <source>Choose a template to load:</source>
        <translation>로드할 템플릿 선택:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="298"/>
        <source>Select Javascript file to import</source>
        <translation>가져올 JavaScript 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="386"/>
        <source>Select Painter Widget Template</source>
        <translation>페인터 위젯 템플릿 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Choose a template to load:</source>
        <translation>로드할 템플릿 선택:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>이 템플릿에 대한 데이터셋을 추가하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1"은(는) %2개의 데이터셋이 필요하지만 현재 그룹에는 %3개가 있습니다.

템플릿 기본값을 사용하여 %4개의 데이터셋을 추가하시겠습니까?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2155"/>
        <source>Project Information</source>
        <translation>프로젝트 정보</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2165"/>
        <source>Project Title</source>
        <translation>프로젝트 제목</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2166"/>
        <source>Untitled Project</source>
        <translation>제목 없는 프로젝트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2167"/>
        <source>Name or description of the project</source>
        <translation>프로젝트의 이름 또는 설명</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2392"/>
        <source>Frame Detection</source>
        <translation>프레임 감지</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2408"/>
        <source>Frame Detection Method</source>
        <translation>프레임 감지 방법</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2409"/>
        <source>Select how incoming data frames are identified</source>
        <translation>수신 데이터 프레임 식별 방법 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2419"/>
        <source>Hexadecimal Delimiters</source>
        <translation>16진수 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2420"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>프레임 시작/종료 시퀀스를 16진수 값으로 입력</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2436"/>
        <source>Frame Start Delimiter</source>
        <translation>프레임 시작 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2437"/>
        <source>e.g. /*</source>
        <translation>예: /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2438"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>데이터 프레임의 시작을 표시하는 시퀀스</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2450"/>
        <source>Frame End Delimiter</source>
        <translation>프레임 종료 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>e.g. */</source>
        <translation>예: */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2452"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>데이터 프레임의 끝을 표시하는 시퀀스</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2464"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>페이로드 처리 및 검증</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2475"/>
        <source>Data Conversion Method</source>
        <translation>데이터 변환 방법</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2476"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>수신된 바이너리 데이터를 파싱하기 전에 디코딩하는 방법 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2492"/>
        <source>Checksum Algorithm</source>
        <translation>체크섬 알고리즘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2493"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>프레임 검증에 사용할 체크섬 알고리즘 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2185"/>
        <source>Group Information</source>
        <translation>그룹 정보</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2195"/>
        <source>Group Title</source>
        <translation>그룹 제목</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2196"/>
        <source>Untitled Group</source>
        <translation>제목 없는 그룹</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2197"/>
        <source>Title or description of this dataset group</source>
        <translation>이 데이터셋 그룹의 제목 또는 설명</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2330"/>
        <source>Composite Widget</source>
        <translation>복합 위젯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2331"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>이 데이터셋 그룹을 시각화할 방법 선택 (선택 사항)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2247"/>
        <source>Image Configuration</source>
        <translation>이미지 구성</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3091"/>
        <source>Virtual Dataset</source>
        <translation>가상 데이터셋</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3092"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>가상 데이터셋은 변환 및 데이터 테이블에서 값을 계산하며 프레임 인덱스가 필요하지 않음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Auto-detect</source>
        <translation>자동 감지</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Manual Delimiters</source>
        <translation>수동 구분 기호</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2260"/>
        <source>Detection Mode</source>
        <translation>감지 모드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1288"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1291"/>
        <source>Frame Parser</source>
        <translation>프레임 파서</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1431"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1432"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1476"/>
        <source>Groups</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1506"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1519"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1520"/>
        <source>Shared Memory</source>
        <translation>공유 메모리</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1506"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1526"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1527"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4963"/>
        <source>Dataset Values</source>
        <translation>데이터셋 값</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1570"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1583"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1584"/>
        <source>Workspaces</source>
        <translation>작업 공간</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1622"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1625"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1626"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1675"/>
        <source>Publishing</source>
        <translation>게시 중</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1686"/>
        <source>Enable Publishing</source>
        <translation>게시 활성화</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1687"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>프레임, 원시 바이트 및 알림을 브로커에 브로드캐스트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1698"/>
        <source>Payload</source>
        <translation>페이로드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1699"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>게시할 내용 선택: 파싱된 대시보드 데이터 또는 원시 RX 바이트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1709"/>
        <source>Publish Rate (Hz)</source>
        <translation>게시 속도 (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1710"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>초당 게시 횟수 (1-30 Hz). 높은 속도는 브로커 부하를 증가시킴; 대시보드 데이터는 속도 제한되므로 느린 브로커가 프레임 파싱을 차단하지 않음.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1722"/>
        <source>Topic Base</source>
        <translation>토픽 베이스</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1723"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1724"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>프레임 및 원시 바이트 게시에 사용되는 기본 토픽</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1735"/>
        <source>Script Topic</source>
        <translation>스크립트 토픽</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1736"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1760"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>비어 있으면 기본 토픽으로 설정됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1737"/>
        <source>Topic the user script publishes to</source>
        <translation>사용자 스크립트가 게시하는 토픽</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1747"/>
        <source>Publish Notifications</source>
        <translation>알림 게시</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1748"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>대시보드 알림을 전용 토픽에 미러링</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1759"/>
        <source>Notification Topic</source>
        <translation>알림 토픽</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1761"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>대시보드 알림이 미러링되는 토픽</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1774"/>
        <source>Broker</source>
        <translation>브로커</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1784"/>
        <source>Hostname</source>
        <translation>호스트명</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1785"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1786"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>MQTT 브로커의 호스트명 또는 IP 주소</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1795"/>
        <source>Port</source>
        <translation>포트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1796"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>브로커가 노출하는 TCP 포트 (1883 평문, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1806"/>
        <source>Custom Client ID</source>
        <translation>사용자 지정 클라이언트 ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1808"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>끔: 프로젝트를 로드할 때마다 새로운 무작위 ID가 생성됩니다. 켬: 아래 ID를 사용합니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1819"/>
        <source>Client ID</source>
        <translation>클라이언트 ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1820"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>CONNECT 시 브로커로 전송되는 식별자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1833"/>
        <source>Protocol Version</source>
        <translation>프로토콜 버전</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1834"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>CONNECT 시 사용되는 MQTT 프로토콜 버전</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1843"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (초)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1844"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>유휴 상태에서 PINGREQ 패킷 간 간격(초)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1853"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1854"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>CONNECT 시 모든 영구 세션 상태 삭제</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1869"/>
        <source>Username</source>
        <translation>사용자 이름</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1870"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>브로커 인증용 사용자 이름(익명 접속 시 비워둠)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1880"/>
        <source>Password</source>
        <translation>비밀번호</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1881"/>
        <source>Password for broker authentication</source>
        <translation>브로커 인증용 비밀번호</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1892"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1902"/>
        <source>Use SSL/TLS</source>
        <translation>SSL/TLS 사용</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1903"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>TLS를 통해 브로커 연결 터널링</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1916"/>
        <source>Protocol</source>
        <translation>프로토콜</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1917"/>
        <source>Negotiated TLS protocol family</source>
        <translation>협상된 TLS 프로토콜 패밀리</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1927"/>
        <source>Peer Verify</source>
        <translation>피어 검증</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1928"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>브로커의 인증서 체인 검증 엄격도</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1938"/>
        <source>Verify Depth</source>
        <translation>검증 깊이</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1939"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>허용되는 최대 인증서 체인 길이 (0 = 무제한)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2212"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2826"/>
        <source>Device %1</source>
        <translation>장치 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2230"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2351"/>
        <source>Input Device</source>
        <translation>입력 장치</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2231"/>
        <source>Select which connected device provides data for this group</source>
        <translation>이 그룹에 데이터를 제공할 연결된 장치 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2262"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>자동 감지는 JPEG/PNG 매직 바이트를 읽고, 수동은 명시적 시작/종료 시퀀스 사용</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2272"/>
        <source>Start Sequence (Hex)</source>
        <translation>시작 시퀀스 (16진수)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2273"/>
        <source>e.g. FF D8 FF</source>
        <translation>예: FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2274"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>이미지 프레임의 시작을 표시하는 16진수 바이트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2283"/>
        <source>End Sequence (Hex)</source>
        <translation>종료 시퀀스 (16진수)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2284"/>
        <source>e.g. FF D9</source>
        <translation>예: FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2285"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>이미지 프레임의 끝을 표시하는 16진수 바이트</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">식별 정보</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2361"/>
        <source>Device Name</source>
        <translation>장치 이름</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2362"/>
        <source>Device 1</source>
        <translation>장치 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2363"/>
        <source>Human-readable name for this input device</source>
        <translation>이 입력 장치의 사람이 읽을 수 있는 이름</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2372"/>
        <source>Bus Type</source>
        <translation>버스 유형</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2373"/>
        <source>Select the hardware interface for this input device</source>
        <translation>이 입력 장치의 하드웨어 인터페이스 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Serial Port</source>
        <translation>시리얼 포트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Network Socket</source>
        <translation>네트워크 소켓</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>Audio Input</source>
        <translation>오디오 입력</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>Raw USB</source>
        <translation>Raw USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2378"/>
        <source>HID Device</source>
        <translation>HID 장치</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2378"/>
        <source>Process</source>
        <translation>프로세스</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2378"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT 구독자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2545"/>
        <source>Connection Settings</source>
        <translation>연결 설정</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2793"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3067"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4654"/>
        <source>General Information</source>
        <translation>일반 정보</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2802"/>
        <source>Action Title</source>
        <translation>액션 제목</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2804"/>
        <source>Untitled Action</source>
        <translation>제목 없는 액션</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2805"/>
        <source>Name or description of this action</source>
        <translation>이 액션의 이름 또는 설명</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Action Icon</source>
        <translation>액션 아이콘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2815"/>
        <source>Default Icon</source>
        <translation>기본 아이콘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>대시보드에서 이 액션에 표시되는 아이콘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2844"/>
        <source>Target Device</source>
        <translation>대상 장치</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2845"/>
        <source>Select which connected device this action sends data to</source>
        <translation>이 액션이 데이터를 전송할 연결된 장치 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Data Payload</source>
        <translation>데이터 페이로드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2868"/>
        <source>Send as Binary</source>
        <translation>바이너리로 전송</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2869"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>이 액션이 트리거될 때 원시 바이너리 데이터 전송</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2880"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2892"/>
        <source>Command</source>
        <translation>명령</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2881"/>
        <source>Transmit Data (Hex)</source>
        <translation>데이터 전송 (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2882"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>액션이 트리거될 때 전송할 16진수 페이로드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2893"/>
        <source>Transmit Data</source>
        <translation>데이터 전송</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2894"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>액션이 트리거될 때 전송할 텍스트 페이로드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2905"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4713"/>
        <source>Text Encoding</source>
        <translation>텍스트 인코딩</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2906"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>텍스트 페이로드 직렬화에 사용되는 문자 인코딩</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2930"/>
        <source>End-of-Line Sequence</source>
        <translation>줄 끝 시퀀스</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2931"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>메시지에 추가할 EOL 문자 (예: </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2943"/>
        <source>Execution Behavior</source>
        <translation>실행 동작</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2954"/>
        <source>Auto-Execute on Connect</source>
        <translation>연결 시 자동 실행</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2955"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>장치 연결 시 이 액션을 자동으로 트리거</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2961"/>
        <source>Timer Behavior</source>
        <translation>타이머 동작</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2970"/>
        <source>Timer Mode</source>
        <translation>타이머 모드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2973"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>이 액션이 자동으로 반복되는 시기와 방법 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2980"/>
        <source>Interval (ms)</source>
        <translation>간격 (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2984"/>
        <source>Timer Interval (ms)</source>
        <translation>타이머 간격 (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2985"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>이 액션의 반복 트리거 간 밀리초 간격</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2992"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2996"/>
        <source>Repeat Count</source>
        <translation>반복 횟수</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2997"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>각 트리거마다 명령을 전송할 횟수</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3077"/>
        <source>Untitled Dataset</source>
        <translation>제목 없는 데이터셋</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3078"/>
        <source>Dataset Title</source>
        <translation>데이터셋 제목</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3079"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>레이블 지정 및 식별에 사용되는 데이터셋 이름</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3109"/>
        <source>Hide on Dashboard</source>
        <translation>대시보드에서 숨기기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3110"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>이 데이터셋의 독립 대시보드 타일을 표시하지 않습니다. 페인터 위젯은 여전히 값을 읽을 수 있습니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3156"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>데이터셋 값 범위의 하한; 위젯과 FFT는 자체 범위가 설정되지 않은 경우 이 값을 사용합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3169"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>데이터셋 값 범위의 상한; 위젯과 FFT는 자체 범위가 설정되지 않은 경우 이 값을 사용합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3229"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>플롯의 X축을 구동할 시간 또는 데이터셋을 선택하세요</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3242"/>
        <source>Frequency Analysis</source>
        <translation>주파수 분석</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3290"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>시간(기본값) 또는 Y축을 구동하는 값을 가진 데이터셋 선택 -- 예: RPM에 바인딩하면 캠벨 다이어그램 생성</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3341"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3430"/>
        <source>Minimum Value (optional)</source>
        <translation>최대값 (선택 사항)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3342"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>데이터 정규화의 하한값; 설정하지 않으면 데이터셋 값 범위로 대체됩니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3354"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3443"/>
        <source>Maximum Value (optional)</source>
        <translation>최대값 (선택 사항)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3355"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>데이터 정규화의 상한값; 설정하지 않으면 데이터셋 값 범위로 대체됩니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3431"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>게이지 또는 막대 범위의 하한값; 설정하지 않으면 데이터셋 값 범위로 대체됩니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3444"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>게이지 또는 막대 범위의 상한값; 설정하지 않으면 데이터셋 값 범위로 대체됩니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3609"/>
        <source>Painter Widget</source>
        <translation>페인터 위젯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4964"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>모든 데이터셋의 원시 값 및 변환된 값 (읽기 전용)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4974"/>
        <source>Shared table defined in this project</source>
        <translation>이 프로젝트에 정의된 공유 테이블</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5326"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>대상 그룹 또는 데이터셋이 더 이상 존재하지 않는 위젯 참조 1개를 제거하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5327"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>대상 그룹 또는 데이터셋이 더 이상 존재하지 않는 위젯 참조 %1개를 제거하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5332"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>작업 공간 타일 배치에만 영향을 미치며, 그룹, 데이터셋 또는 데이터는 삭제되지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5335"/>
        <source>Clean Up Workspaces</source>
        <translation>작업 공간 정리</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3123"/>
        <source>Frame Index</source>
        <translation>프레임 인덱스</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3124"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>시간상 데이터셋 정렬에 사용되는 프레임 위치</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3133"/>
        <source>Measurement Unit</source>
        <translation>측정 단위</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3134"/>
        <source>Volts, Amps, etc.</source>
        <translation>볼트, 암페어 등</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3135"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>볼트 또는 암페어와 같은 측정 단위 (선택 사항)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3183"/>
        <source>Plot Settings</source>
        <translation>플롯 설정</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3206"/>
        <source>Enable Plot Widget</source>
        <translation>플롯 위젯 활성화</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3208"/>
        <source>Plot data in real-time</source>
        <translation>실시간으로 데이터 플롯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3228"/>
        <source>X-Axis Source</source>
        <translation>X축 소스</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">플롯의 X축에 사용할 데이터셋 선택</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">최소 플롯 값 (선택 사항)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">플롯 표시 범위의 하한값</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">최대 플롯 값 (선택 사항)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">플롯 표시 범위의 상한값</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">FFT 구성</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3253"/>
        <source>Enable FFT Analysis</source>
        <translation>FFT 분석 활성화</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3254"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>데이터셋의 주파수 영역 분석 수행</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3264"/>
        <source>Enable Waterfall Plot</source>
        <translation>워터폴 플롯 활성화</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3265"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>시간에 따른 주파수 콘텐츠의 스크롤 스펙트로그램 표시 (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3289"/>
        <source>Waterfall Y Axis</source>
        <translation>워터폴 Y축</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="vanished">시간(기본값) 또는 Y축을 구동하는 값을 가진 데이터셋 선택 — 예: RPM에 바인딩하면 캠벨 다이어그램 생성</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3318"/>
        <source>FFT Window Size</source>
        <translation>FFT 윈도우 크기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3319"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>각 FFT 계산 윈도우에 사용되는 샘플 수</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3330"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>FFT 샘플링 레이트 (Hz, 필수)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3331"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>FFT에 사용되는 샘플링 주파수 (Hz 단위)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">최소값 (권장)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">데이터 정규화의 하한값</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">최대값 (권장)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">데이터 정규화의 상한값</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3380"/>
        <source>Widget Settings</source>
        <translation>위젯 설정</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3403"/>
        <source>Widget</source>
        <translation>위젯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3404"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>이 데이터셋을 표시하는 데 사용할 시각적 위젯 선택</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">최소 표시 값 (필수)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">게이지 또는 막대 표시 범위의 하한값</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">최대 표시 값 (필수)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">게이지 또는 막대 표시 범위의 상한값</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3460"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3622"/>
        <source>Auto</source>
        <translation>자동</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3461"/>
        <source>Tick Count</source>
        <translation>눈금 개수</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3465"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>다이얼 눈금의 주요 눈금 개수 (0 = 위젯 크기에 자동 맞춤)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3484"/>
        <source>Label Format</source>
        <translation>레이블 포맷</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3485"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>눈금 레이블 및 값 표시에 사용되는 소수 자릿수 또는 표기법</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">값 표시기 표시</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">위젯 아래 또는 옆에 표시되는 박스형 숫자 표시 전환</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">알람 설정</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">알람 활성화</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">값이 알람 임계값을 초과하면 시각적 알람 트리거</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">하한 임계값</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">값이 이 임계값 아래로 떨어지면 시각적 알람 트리거</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">상한 임계값</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">값이 이 임계값을 초과하면 시각적 알람 트리거</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3522"/>
        <source>LED Display Settings</source>
        <translation>LED 디스플레이 설정</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3533"/>
        <source>Show in LED Panel</source>
        <translation>LED 패널에 표시</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3534"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>LED 디스플레이를 사용한 시각적 상태 모니터링 활성화</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3545"/>
        <source>LED On Threshold (required)</source>
        <translation>LED 켜짐 임계값 (필수)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3546"/>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation>값이 이 임계값 이상일 때 LED가 켜집니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Off</source>
        <translation>꺼짐</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Auto Start</source>
        <translation>자동 시작</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Start on Trigger</source>
        <translation>트리거 시 시작</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Toggle on Trigger</source>
        <translation>트리거 시 토글</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3568"/>
        <source>Repeat N Times</source>
        <translation>N회 반복</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3572"/>
        <source>Plain Text (UTF8)</source>
        <translation>일반 텍스트 (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3572"/>
        <source>Hexadecimal</source>
        <translation>16진수</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3572"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3573"/>
        <source>Binary (Direct)</source>
        <translation>바이너리 (직접)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3580"/>
        <source>No Checksum</source>
        <translation>체크섬 없음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3585"/>
        <source>End Delimiter Only</source>
        <translation>종료 구분자만</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3585"/>
        <source>Start Delimiter Only</source>
        <translation>시작 구분자만</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3586"/>
        <source>Start + End Delimiter</source>
        <translation>시작 + 종료 구분자</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3586"/>
        <source>No Delimiters</source>
        <translation>구분자 없음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Button</source>
        <translation>버튼</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Slider</source>
        <translation>슬라이더</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Toggle</source>
        <translation>토글</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Text Field</source>
        <translation>텍스트 필드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3597"/>
        <source>Knob</source>
        <translation>노브</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3602"/>
        <source>Data Grid</source>
        <translation>데이터 그리드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3603"/>
        <source>GPS Map</source>
        <translation>GPS 맵</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>Gyroscope</source>
        <translation>자이로스코프</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3605"/>
        <source>Multiple Plot</source>
        <translation>다중 플롯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3606"/>
        <source>Accelerometer</source>
        <translation>가속도계</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3607"/>
        <source>3D Plot</source>
        <translation>3D 플롯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3608"/>
        <source>Image View</source>
        <translation>이미지 뷰</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3610"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3614"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3631"/>
        <source>None</source>
        <translation>없음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3615"/>
        <source>Bar</source>
        <translation>바</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Gauge</source>
        <translation>게이지</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3617"/>
        <source>Compass</source>
        <translation>나침반</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3618"/>
        <source>Meter</source>
        <translation>미터</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">온도계</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3623"/>
        <source>Integer (0 decimals)</source>
        <translation>정수 (소수점 0자리)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3624"/>
        <source>1 decimal</source>
        <translation>소수점 1자리</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3625"/>
        <source>2 decimals</source>
        <translation>소수점 2자리</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3626"/>
        <source>3 decimals</source>
        <translation>소수점 3자리</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3627"/>
        <source>Scientific</source>
        <translation>과학적 표기법</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3632"/>
        <source>New Line (\n)</source>
        <translation>새 줄 (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3633"/>
        <source>Carriage Return (\r)</source>
        <translation>캐리지 리턴 (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3634"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <source>No</source>
        <translation>아니오</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3639"/>
        <source>Yes</source>
        <translation>예</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4664"/>
        <source>Label</source>
        <translation>레이블</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4665"/>
        <source>Display label</source>
        <translation>레이블 표시</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4675"/>
        <source>Button Icon</source>
        <translation>버튼 아이콘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4684"/>
        <source>Colorize Icon</source>
        <translation>아이콘 색상화</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4685"/>
        <source>Tint the icon with the button color</source>
        <translation>버튼 색상으로 아이콘 색조 적용</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4702"/>
        <source>Initial Value</source>
        <translation>초기값</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4714"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>transmit()가 문자열 값을 반환할 때 사용되는 문자 인코딩</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4732"/>
        <source>Value Range</source>
        <translation>값 범위</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3155"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4742"/>
        <source>Minimum Value</source>
        <translation>최소값</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3168"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4751"/>
        <source>Maximum Value</source>
        <translation>최대값</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4760"/>
        <source>Step Size</source>
        <translation>단계 크기</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="526"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="535"/>
        <source>Lock Project</source>
        <translation>프로젝트 잠금</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="527"/>
        <source>Choose a password to lock the project:</source>
        <translation>프로젝트를 잠글 비밀번호를 선택하세요:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="535"/>
        <source>Confirm the password:</source>
        <translation>비밀번호 확인:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="540"/>
        <source>Passwords do not match</source>
        <translation>비밀번호 불일치</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="541"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>입력한 두 비밀번호가 일치하지 않습니다. 프로젝트가 잠기지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="575"/>
        <source>Unlock Project</source>
        <translation>프로젝트 잠금 해제</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="576"/>
        <source>Enter the project password:</source>
        <translation>프로젝트 비밀번호를 입력하세요:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="586"/>
        <source>Incorrect password</source>
        <translation>잘못된 비밀번호</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="587"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>입력한 비밀번호가 프로젝트 파일에 저장된 비밀번호와 일치하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="619"/>
        <source>New Project</source>
        <translation>새 프로젝트</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">샘플</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1169"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>여러 데이터 소스는 Pro 라이선스가 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1170"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro는 여러 장치에 동시 연결할 수 있습니다. 이 기능을 사용하려면 업그레이드하세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1183"/>
        <source>Device %1</source>
        <translation>장치 %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(사본)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1467"/>
        <source>Do you want to save your changes?</source>
        <translation>변경 사항을 저장하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1468"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>이 프로젝트에 저장되지 않은 수정 사항이 있습니다!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="395"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="405"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="419"/>
        <source>Project error</source>
        <translation>프로젝트 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="395"/>
        <source>Project title cannot be empty!</source>
        <translation>프로젝트 제목은 비워둘 수 없습니다!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="405"/>
        <source>You need to add at least one group!</source>
        <translation>최소 하나의 그룹을 추가해야 합니다!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="419"/>
        <source>You need to add at least one dataset!</source>
        <translation>최소 하나의 데이터셋을 추가해야 합니다!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="467"/>
        <source>Your project needs a title</source>
        <translation>프로젝트에 제목이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="469"/>
        <source>Add a group to get started</source>
        <translation>시작하려면 그룹을 추가하세요</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="471"/>
        <source>Add a dataset to a group</source>
        <translation>그룹에 데이터셋을 추가하세요</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="485"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>트리 상단의 프로젝트 보기를 열고 이름을 입력하세요. 프로젝트는 언제든지 이름을 변경할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="488"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>그룹은 데이터셋을 대시보드 위젯으로 구성합니다. 위 도구 모음의 그룹 버튼을 사용하여 그룹을 생성한 다음 데이터셋을 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="492"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>데이터셋은 대시보드에 표시되는 값입니다. 트리에서 그룹을 선택하고 툴바의 데이터셋 버튼을 사용하여 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="773"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="821"/>
        <source>Time</source>
        <translation>시간</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1219"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>데이터 소스 "%1"을(를) 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1220"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>이 소스를 사용하는 그룹은 기본 소스로 이동됩니다. 이 작업은 취소할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1507"/>
        <source>Save Serial Studio Project</source>
        <translation>Serial Studio 프로젝트 저장</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1509"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2126"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Serial Studio 프로젝트 파일 (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1531"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1767"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2117"/>
        <source>Untitled Project</source>
        <translation>제목 없는 프로젝트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1777"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2278"/>
        <source>Device A</source>
        <translation>장치 A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1952"/>
        <source>Select Project File</source>
        <translation>프로젝트 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1954"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>프로젝트 파일 (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2003"/>
        <source>JSON validation error</source>
        <translation>JSON 유효성 검사 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2092"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>프로젝트가 이전 파일 형식에서 업그레이드됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2093"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>이 프로젝트는 스키마 버전 %1(으)로 저장되었습니다. 현재 버전은 %2입니다. 새 필드에는 기본값이 적용되었습니다. 업그레이드를 확정하려면 프로젝트를 저장하세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2124"/>
        <source>Save Imported Project</source>
        <translation>가져온 프로젝트 저장</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2323"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>다중 소스 프로젝트는 Pro 라이선스가 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2324"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>이 프로젝트는 여러 데이터 소스를 포함하고 있습니다. 첫 번째 소스만 로드되었습니다. 다중 소스 프로젝트를 사용하려면 Serial Studio Pro 라이선스가 필요합니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2558"/>
        <source>Workspace IDs remapped on load</source>
        <translation>워크스페이스 ID가 로드 시 재매핑됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2559"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1개의 사용자 지정 작업 공간 ID가 새로 예약된 자동 범위와 겹쳐 사용자 범위로 이동되었습니다. 프로젝트를 저장하여 이 매핑을 영구적으로 만드세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2704"/>
        <source>Legacy frame parser function updated</source>
        <translation>레거시 프레임 파서 함수 업데이트됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2705"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>프로젝트에서 'separator' 인수를 사용하는 레거시 프레임 파서 함수가 사용되었습니다. 새 형식으로 자동 마이그레이션되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2903"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>그룹 "%1"을(를) 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2904"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2955"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2990"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3757"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>이 작업은 취소할 수 없습니다. 계속하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2954"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>액션 "%1"을(를) 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2989"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>데이터셋 "%1"을(를) 삭제하시겠습니까?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (사본)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3665"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3704"/>
        <source>Output Controls</source>
        <translation>출력 제어</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3717"/>
        <source>New Button</source>
        <translation>새 버튼</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3720"/>
        <source>New Slider</source>
        <translation>새 슬라이더</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3723"/>
        <source>New Toggle</source>
        <translation>새 토글</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3726"/>
        <source>New Text Field</source>
        <translation>새 텍스트 필드</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3729"/>
        <source>New Knob</source>
        <translation>새 노브</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3756"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>출력 위젯 "%1"을(를) 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3929"/>
        <source>Group</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3948"/>
        <source>New Dataset</source>
        <translation>새 데이터셋</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3951"/>
        <source>New Plot</source>
        <translation>새 플롯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3955"/>
        <source>New FFT Plot</source>
        <translation>새 FFT 플롯</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3959"/>
        <source>New Level Indicator</source>
        <translation>새 레벨 표시기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3963"/>
        <source>New Gauge</source>
        <translation>새 게이지</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3967"/>
        <source>New Compass</source>
        <translation>새 나침반</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3973"/>
        <source>New Meter</source>
        <translation>새 미터</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">새 온도계</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3977"/>
        <source>New LED Indicator</source>
        <translation>새 LED 표시기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3981"/>
        <source>New Waterfall</source>
        <translation>새 워터폴</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4051"/>
        <source>Channel %1</source>
        <translation>채널 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4124"/>
        <source>New Action</source>
        <translation>새 액션</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4267"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>그룹 수준 위젯을 변경하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4269"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>이 그룹의 기존 데이터셋이 삭제됩니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4333"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4334"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4335"/>
        <source>Accelerometer %1</source>
        <translation>가속도계 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4350"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4350"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4350"/>
        <source>Gyro %1</source>
        <translation>자이로 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4365"/>
        <source>Latitude</source>
        <translation>위도</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4365"/>
        <source>Longitude</source>
        <translation>경도</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4365"/>
        <source>Altitude</source>
        <translation>고도</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4380"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4394"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4380"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4394"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4380"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4394"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4598"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5494"/>
        <source>Workspace</source>
        <translation>작업 공간</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4764"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4971"/>
        <source>Shared Table</source>
        <translation>공유 테이블</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4846"/>
        <source>register</source>
        <translation>레지스터</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4971"/>
        <source>New Shared Table</source>
        <translation>새 공유 테이블</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4971"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4989"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5008"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5032"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5059"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5078"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5101"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5124"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5494"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5515"/>
        <source>Name:</source>
        <translation>이름:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4989"/>
        <source>Rename Table</source>
        <translation>테이블 이름 변경</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5008"/>
        <source>Rename Group</source>
        <translation>그룹 이름 변경</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5032"/>
        <source>Rename Dataset</source>
        <translation>데이터셋 이름 변경</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5059"/>
        <source>Rename Data Source</source>
        <translation>데이터 소스 이름 변경</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5078"/>
        <source>Rename Action</source>
        <translation>액션 이름 변경</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5100"/>
        <source>New Register</source>
        <translation>새 레지스터</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5124"/>
        <source>Rename Register</source>
        <translation>레지스터 이름 변경</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5163"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5188"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6042"/>
        <source>This action cannot be undone.</source>
        <translation>이 작업은 취소할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5164"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>테이블과 함께 %1개의 레지스터가 제거됩니다. 이 작업은 취소할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5167"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5187"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6041"/>
        <source>Delete "%1"?</source>
        <translation>"%1"을(를) 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5170"/>
        <source>Delete Table</source>
        <translation>테이블 삭제</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5190"/>
        <source>Delete Register</source>
        <translation>레지스터 삭제</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5214"/>
        <source>Export Table</source>
        <translation>테이블 내보내기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5216"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5260"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 파일 (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5258"/>
        <source>Import Table</source>
        <translation>테이블 가져오기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5494"/>
        <source>New Workspace</source>
        <translation>새 작업 공간</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5515"/>
        <source>Rename Workspace</source>
        <translation>작업 공간 이름 변경</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5604"/>
        <source>Overview</source>
        <translation>개요</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5614"/>
        <source>All Data</source>
        <translation>모든 데이터</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5798"/>
        <source>Discard workspace customisations?</source>
        <translation>작업 공간 사용자 지정을 취소하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5799"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>사용자 지정을 끄면 편집 내용이 삭제되고 프로젝트의 그룹에서 작업 공간 목록이 다시 생성됩니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5802"/>
        <source>Customize Workspaces</source>
        <translation>작업 공간 사용자 지정</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6044"/>
        <source>Delete Workspace</source>
        <translation>작업 공간 삭제</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6385"/>
        <source>File save error</source>
        <translation>파일 저장 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2161"/>
        <source>File open error</source>
        <translation>파일 열기 오류</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="907"/>
        <source>Import Protocol Buffers File</source>
        <translation>Protocol Buffers 파일 가져오기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="909"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Proto 파일 (*.proto);;모든 파일 (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="944"/>
        <source>Failed to open proto file: %1</source>
        <translation>proto 파일을 열지 못했습니다: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="945"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>파일 경로와 읽기 권한을 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Protobuf Import Error</source>
        <translation>Protobuf 가져오기 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="962"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>%1번 줄에서 proto 파일 파싱 실패: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="963"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>proto3 구문만 지원됩니다. 파일 형식을 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="970"/>
        <source>Proto file contains no message definitions</source>
        <translation>Proto 파일에 메시지 정의가 없습니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="971"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>선택한 파일에 가져올 `message` 블록이 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Protobuf Import Warning</source>
        <translation>Protobuf 가져오기 경고</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">가져온 프로젝트 로드 실패</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">생성된 프로젝트 JSON을 로드할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1011"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>proto 파일에서 %1개의 메시지와 %2개의 필드를 성공적으로 가져왔습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>The project editor is now open for customization.</source>
        <translation>프로젝트 편집기가 사용자 지정을 위해 열렸습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1016"/>
        <source>Protobuf Import Complete</source>
        <translation>Protobuf 가져오기 완료</translation>
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
        <translation>잘못된 HEX 입력</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="159"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>유효한 16진수 바이트를 입력하세요.

유효한 형식: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="165"/>
        <source>No transmit function code to evaluate.</source>
        <translation>평가할 전송 함수 코드가 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="185"/>
        <source>transmit function is not callable</source>
        <translation>전송 함수를 호출할 수 없습니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="249"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="250"/>
        <source>Clear</source>
        <translation>지우기</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="251"/>
        <source>Evaluate</source>
        <translation>평가</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="252"/>
        <source>Input Value</source>
        <translation>입력 값</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="253"/>
        <source>Transmit Function Output</source>
        <translation>전송 함수 출력</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="254"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="278"/>
        <source>Enter value to transmit…</source>
        <translation>전송할 값 입력…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="255"/>
        <source>Raw string output appears here</source>
        <translation>원시 문자열 출력이 여기에 표시됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="256"/>
        <source>Hex byte output appears here</source>
        <translation>16진수 바이트 출력이 여기에 표시됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="259"/>
        <source>Test Transmit Function</source>
        <translation>전송 함수 테스트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="272"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>16진수 바이트 입력 (예: 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="383"/>
        <source>(empty) No data returned</source>
        <translation>(비어 있음) 반환된 데이터 없음</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="385"/>
        <source>0 bytes</source>
        <translation>0바이트</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="426"/>
        <source>%1 byte(s)</source>
        <translation>%1 바이트</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>공유 메모리</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>공유 테이블 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>공유 테이블 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>도움말</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>공유 메모리 도움말 문서 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>이름</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>설명</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>항목</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="269"/>
        <source>No shared tables.</source>
        <translation>공유 테이블이 없습니다.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>세션</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>열기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>세션 파일 열기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>세션 파일 닫기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>재생</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>선택한 세션을 대시보드에서 재생</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>세션을 삭제하려면 세션 파일 잠금을 해제하세요</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>선택한 세션 삭제</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>잠금 해제</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>잠금</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>삭제를 허용하려면 세션 파일 잠금을 해제하세요</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>세션 삭제를 방지하려면 비밀번호를 설정하세요</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>CSV 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>선택한 세션을 CSV로 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>PDF 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>선택한 세션에 대한 PDF 보고서 생성</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>프로젝트 복원</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>이 세션 파일에서 프로젝트 파일 복원</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>세션 로딩 중…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>작업 중…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>이 프로젝트에서 Pro 기능이 감지되었습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>대체 위젯을 사용 중입니다. 전체 기능을 잠금 해제하려면 라이선스를 구매하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>이 데이터셋에 대한 2D 플롯 시각화 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>FFT 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>주파수 콘텐츠를 시각화하기 위해 FFT 플롯 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>워터폴</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>워터폴(스펙트로그램) 플롯 전환 — FFT 설정 사용</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>바/레벨</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>이 데이터셋에 대한 바/레벨 표시기 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>게이지</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>아날로그 스타일 디스플레이를 위한 게이지 위젯 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>나침반</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>방향 데이터를 위한 나침반 위젯 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>미터</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>아날로그 미터(반원형) 위젯 전환</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">온도계</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">온도 데이터를 위한 온도계 위젯 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>이진 또는 임계값에 대한 LED 표시기 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>동작</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>알람 밴드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="304"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation>이 데이터셋의 게이지에 대해 심각도 단계별로 색상이 지정된 값 범위를 정의합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="310"/>
        <source>Transform</source>
        <translation>변환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>보정, 필터링 또는 단위 변환을 위한 값 변환 표현식 편집</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="332"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>동일한 구성으로 이 데이터셋 복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="337"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="340"/>
        <source>Delete this dataset from the group</source>
        <translation>그룹에서 이 데이터셋 삭제</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>Serial Studio 후원</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>%1 개발 후원!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio는 자원봉사자들이 지원하는 무료 오픈소스 소프트웨어입니다. 개발 활동을 지원하기 위해 기부하거나 Pro 라이선스를 구매해 주세요 :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>이 프로젝트를 공유하고, 버그를 보고하고, 새로운 기능을 제안하여 지원할 수도 있습니다!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>기부</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>Serial Studio Pro 구매</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="127"/>
        <source>Stop</source>
        <translation>중지</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="128"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>Downloading updates</source>
        <translation>업데이트 다운로드 중</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="455"/>
        <source>Time remaining</source>
        <translation>남은 시간</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <source>unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Cannot find downloaded update!</source>
        <translation>다운로드한 업데이트를 찾을 수 없습니다!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="244"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="245"/>
        <source>Download complete!</source>
        <translation>다운로드 완료!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="253"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>"확인"을 클릭하여 업데이트 설치 시작</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="255"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>업데이트를 설치하려면 애플리케이션을 종료해야 할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="246"/>
        <source>The installer opens separately</source>
        <translation>설치 프로그램이 별도로 열림</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>업데이트를 설치하려면 애플리케이션을 종료해야 할 수 있습니다. 필수 업데이트이므로 지금 종료하면 애플리케이션이 닫힙니다.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="275"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>"열기" 버튼을 클릭하여 업데이트 적용</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="288"/>
        <source>Updater</source>
        <translation>업데이트 관리자</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="292"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>다운로드를 취소하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="294"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>다운로드를 취소하시겠습니까? 필수 업데이트이므로 지금 종료하면 애플리케이션이 닫힙니다.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="349"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="356"/>
        <source>%1 bytes</source>
        <translation>%1바이트</translation>
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
        <translation>/</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="406"/>
        <source>Downloading Updates</source>
        <translation>업데이트 다운로드 중</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Time Remaining</source>
        <translation>남은 시간</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>about %1 hours</source>
        <translation>약 %1시간</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="433"/>
        <source>about one hour</source>
        <translation>약 1시간</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="441"/>
        <source>%1 minutes</source>
        <translation>%1분</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="443"/>
        <source>1 minute</source>
        <translation>1분</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="450"/>
        <source>%1 seconds</source>
        <translation>%1초</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="452"/>
        <source>1 second</source>
        <translation>1초</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">남은 시간: 0분</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">열기</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>예제 브라우저</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>뒤로</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>다운로드 및 열기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>GitHub에서 보기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>예제 검색…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="244"/>
        <source>Fetching examples…</source>
        <translation>예제 가져오는 중…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="567"/>
        <source>Loading...</source>
        <translation>로딩 중...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>No README available.</source>
        <translation>README를 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="608"/>
        <source>Copied to Clipboard</source>
        <translation>클립보드에 복사됨</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="671"/>
        <source>No screenshot available</source>
        <translation>사용 가능한 스크린샷 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="703"/>
        <source>Details</source>
        <translation>세부 정보</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="732"/>
        <source>Info</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="755"/>
        <source>Category:</source>
        <translation>범주:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="768"/>
        <source>Difficulty:</source>
        <translation>난이도:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="786"/>
        <source>Project:</source>
        <translation>프로젝트:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="828"/>
        <source>No Results Found</source>
        <translation>결과 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="839"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>철자를 확인하거나 다른 검색어를 시도하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="854"/>
        <source>%1 examples</source>
        <translation>%1개 예제</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="863"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>확장 관리자</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>새로고침</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>저장소</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>저장소 설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>뒤로</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>설치</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>제거</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>실행</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>중지</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>확장 검색…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>확장 기능 가져오는 중…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>실행 중</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>업데이트</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>설치됨</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>사용 불가</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>설명을 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>세부 정보</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>유형:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>작성자:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>버전:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>라이선스:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>미리보기 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>플러그인 출력</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>아직 출력이 없습니다. 플러그인을 실행하면 여기에 로그가 표시됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>미리보기를 사용할 수 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>저장소</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>원격 저장소 URL 또는 로컬 폴더 경로를 추가합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>로컬</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL 또는 로컬 경로…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>찾아보기…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>추가</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>결과 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>철자를 확인하거나 다른 검색어를 시도하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>사용 가능한 확장 기능 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>저장소 설정에서 저장소 URL 또는 로컬 경로를 추가한 후 새로고침하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1개의 확장 기능</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="157"/>
        <source>Interpolation: %1</source>
        <translation>보간: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="185"/>
        <source>Show Area Under Plot</source>
        <translation>플롯 아래 영역 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="203"/>
        <source>Show X Axis Label</source>
        <translation>X축 레이블 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="215"/>
        <source>Show Y Axis Label</source>
        <translation>Y축 레이블 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="233"/>
        <source>Show Crosshair</source>
        <translation>십자선 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Resume</source>
        <translation>재개</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="259"/>
        <source>Reset View</source>
        <translation>보기 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="265"/>
        <source>Axis Range Settings</source>
        <translation>축 범위 설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="294"/>
        <source>Magnitude (dB)</source>
        <translation>크기 (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="295"/>
        <source>Frequency (Hz)</source>
        <translation>주파수 (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>프로젝트 및 CSV 파일을 여기에 드롭</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>파일 전송</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>전송 프로토콜:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>파일 선택:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>파일 선택…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>전송 간격:</translation>
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
        <translation>블록 크기:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>바이트</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>타임아웃:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>최대 재시도 횟수:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>진행률: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 바이트</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>오류: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>활동 로그</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>지우기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>전송 일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>전송 재개</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>전송 중지</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>전송 시작</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>프레임 파서</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>장치 %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="395"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1402"/>
        <source>Output Panel</source>
        <translation>출력 패널</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="439"/>
        <source>Control</source>
        <translation>제어</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="462"/>
        <source>Table</source>
        <translation>테이블</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="478"/>
        <source>%1 regs</source>
        <translation>%1 레지스터</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="479"/>
        <source>empty</source>
        <translation>비어 있음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="504"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 발행자</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="905"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>이 데이터셋의 변환 코드 편집기를 엽니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1184"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1187"/>
        <source>Dataset Container</source>
        <translation>데이터셋 컨테이너</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1196"/>
        <source>Multi-Plot</source>
        <translation>다중 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1199"/>
        <source>Multiple Plot</source>
        <translation>다중 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1208"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1211"/>
        <source>Accelerometer</source>
        <translation>가속도계</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1220"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1223"/>
        <source>Gyroscope</source>
        <translation>자이로스코프</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1232"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1235"/>
        <source>GPS Map</source>
        <translation>GPS 맵</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1246"/>
        <source>3D Plot</source>
        <translation>3D 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1254"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1257"/>
        <source>Image View</source>
        <translation>이미지 뷰</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1266"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1269"/>
        <source>Painter Widget</source>
        <translation>페인터 위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1281"/>
        <source>Data Grid</source>
        <translation>데이터 그리드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1294"/>
        <source>Generic</source>
        <translation>일반</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1307"/>
        <source>Plot</source>
        <translation>플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1320"/>
        <source>FFT Plot</source>
        <translation>FFT 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1333"/>
        <source>Gauge</source>
        <translation>게이지</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1346"/>
        <source>Level Indicator</source>
        <translation>레벨 표시기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1359"/>
        <source>Compass</source>
        <translation>나침반</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1372"/>
        <source>Meter</source>
        <translation>미터</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">온도계</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1385"/>
        <source>LED Indicator</source>
        <translation>LED 표시기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1414"/>
        <source>Slider</source>
        <translation>슬라이더</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1427"/>
        <source>Toggle</source>
        <translation>토글</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1440"/>
        <source>Knob</source>
        <translation>노브</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1453"/>
        <source>Text Field</source>
        <translation>텍스트 필드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1466"/>
        <source>Button</source>
        <translation>버튼</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1490"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1565"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1652"/>
        <source>Add Group</source>
        <translation>그룹 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1505"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1667"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1712"/>
        <source>Add Dataset</source>
        <translation>데이터셋 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1519"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1594"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1681"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1933"/>
        <source>Add Output</source>
        <translation>출력 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1535"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1607"/>
        <source>Add Action</source>
        <translation>액션 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1544"/>
        <source>Add Data Source</source>
        <translation>데이터 소스 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1551"/>
        <source>Add Data Table</source>
        <translation>데이터 테이블 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1618"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1753"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1820"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1948"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1982"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2038"/>
        <source>Rename…</source>
        <translation>이름 변경…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1783"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1853"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1905"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1956"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Duplicate</source>
        <translation>복제</translation>
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
        <translation>삭제…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1697"/>
        <source>Edit Frame Parser…</source>
        <translation>프레임 파서 편집…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1739"/>
        <source>Edit Painter Code…</source>
        <translation>페인터 코드 편집…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1761"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1829"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1881"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1990"/>
        <source>Move Up</source>
        <translation>위로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1772"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1841"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1893"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2001"/>
        <source>Move Down</source>
        <translation>아래로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1809"/>
        <source>Edit Transform Code…</source>
        <translation>변환 코드 편집…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2064"/>
        <source>Edit Code…</source>
        <translation>코드 편집…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="345"/>
        <source>Action</source>
        <translation>액션</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">아직 정의된 그룹이 없습니다</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="102"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="249"/>
        <source>Undo</source>
        <translation>실행 취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="109"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="263"/>
        <source>Redo</source>
        <translation>다시 실행</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="118"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="288"/>
        <source>Cut</source>
        <translation>잘라내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="123"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="301"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="314"/>
        <source>Paste</source>
        <translation>붙여넣기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="135"/>
        <source>Select All</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="145"/>
        <source>Format Document</source>
        <translation>문서 서식 지정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="152"/>
        <source>Format Selection</source>
        <translation>선택 영역 서식 지정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="223"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="228"/>
        <source>Reset to the default parsing script</source>
        <translation>기본 파싱 스크립트로 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="236"/>
        <source>Open</source>
        <translation>열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="241"/>
        <source>Import a script file for data parsing</source>
        <translation>데이터 파싱을 위한 스크립트 파일 가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="343"/>
        <source>Open help documentation for data parsing</source>
        <translation>데이터 파싱에 대한 도움말 문서 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="365"/>
        <source>Language:</source>
        <translation>언어:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="419"/>
        <source>Select Template…</source>
        <translation>템플릿 선택…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="432"/>
        <source>Test With Sample Data</source>
        <translation>샘플 데이터로 테스트</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="439"/>
        <source>Evaluate</source>
        <translation>평가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="254"/>
        <source>Undo the last code edit</source>
        <translation>마지막 코드 편집 실행 취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="269"/>
        <source>Redo the previously undone edit</source>
        <translation>이전에 실행 취소한 편집 다시 실행</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="293"/>
        <source>Cut selected code to clipboard</source>
        <translation>선택한 코드를 클립보드로 잘라내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="306"/>
        <source>Copy selected code to clipboard</source>
        <translation>선택한 코드를 클립보드에 복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="318"/>
        <source>Paste code from clipboard</source>
        <translation>클립보드에서 코드 붙여넣기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="338"/>
        <source>Help</source>
        <translation>도움말</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>자동 중심</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>궤적 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>확대</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>축소</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>날씨 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>NASA 날씨 오버레이</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>기본 맵: %1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>이 프로젝트에서 Pro 기능이 감지되었습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>대체 위젯을 사용 중입니다. 전체 기능을 사용하려면 라이선스를 구매하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>데이터셋 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>데이터셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>현재 그룹에 일반 데이터셋 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>숫자 데이터를 시각화할 2D 플롯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>FFT 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>주파수 영역 시각화를 위한 FFT 플롯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>워터폴</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>워터폴(스펙트로그램) 플롯 추가 — FFT 설정 사용</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>바/레벨</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>스케일된 값을 표시할 바 또는 레벨 표시기 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>게이지</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>아날로그 스타일 시각화를 위한 게이지 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>나침반</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>방향 또는 각도 데이터를 표시하기 위한 나침반 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>미터</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>아날로그 미터(반원형) 위젯 추가</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">온도계</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">온도 데이터용 온도계 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>이진 상태 신호를 위한 LED 표시기 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>제어 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>버튼</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>클릭 시 명령을 전송하는 버튼 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>슬라이더</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>스케일된 숫자 값을 전송하기 위한 슬라이더 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>토글</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>켜기/끄기 명령을 위한 토글 스위치 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>텍스트 필드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>명령 입력 및 전송을 위한 텍스트 필드 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>노브</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>설정값 제어를 위한 회전 노브 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>코드 편집</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>이 페인터 위젯을 그리는 JavaScript를 편집합니다</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>현재 그룹 및 해당 콘텐츠 복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>현재 그룹 및 포함된 모든 데이터셋 삭제</translation>
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
        <translation>HID 장치</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>사용 페이지</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>사용</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>게임패드, 조이스틱, 스티어링 휠, 비행 컨트롤러 및 기타 HID 클래스 USB 장치를 연결합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>HID 사용 테이블 (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>도움말 센터</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>도움말 페이지 가져오는 중…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>검색…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>로딩 중…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="289"/>
        <source>Select a page from the sidebar</source>
        <translation>사이드바에서 페이지를 선택하세요</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="319"/>
        <source>Copied to Clipboard</source>
        <translation>클립보드에 복사됨</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="351"/>
        <source>View Online</source>
        <translation>온라인으로 보기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="370"/>
        <source>%1 pages</source>
        <translation>%1 페이지</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="379"/>
        <source>Close</source>
        <translation>닫기</translation>
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
        <translation>네트워크 소켓</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>Audio</source>
        <translation>오디오</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>USB Device</source>
        <translation>USB 장치</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>HID Device</source>
        <translation>HID 장치</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="280"/>
        <source>Process</source>
        <translation>프로세스</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="281"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT 구독자</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="631"/>
        <source>Your trial period has ended.</source>
        <translation>평가판 기간이 종료되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="632"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Serial Studio를 계속 사용하려면 라이선스를 활성화하십시오.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="764"/>
        <source>channels</source>
        <translation>채널</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="817"/>
        <source> channels</source>
        <translation>채널</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="965"/>
        <source>Unsigned 8-bit</source>
        <translation>Unsigned 8비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="966"/>
        <source>Signed 16-bit</source>
        <translation>Signed 16비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="967"/>
        <source>Signed 24-bit</source>
        <translation>Signed 24비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="968"/>
        <source>Signed 32-bit</source>
        <translation>Signed 32비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="969"/>
        <source>Float 32-bit</source>
        <translation>Float 32비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="972"/>
        <source>Mono</source>
        <translation>모노</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="973"/>
        <source>Stereo</source>
        <translation>스테레오</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1375"/>
        <source>Input Device</source>
        <translation>입력 장치</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1383"/>
        <source>Sample Rate</source>
        <translation>샘플 레이트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1391"/>
        <source>Sample Format</source>
        <translation>샘플 형식</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1399"/>
        <source>Channels</source>
        <translation>채널</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="71"/>
        <source>BLE I/O Module Error</source>
        <translation>BLE I/O 모듈 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="325"/>
        <source>Select Device</source>
        <translation>장치 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="336"/>
        <source>Select Service</source>
        <translation>서비스 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="347"/>
        <source>Select Characteristic</source>
        <translation>특성 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="514"/>
        <source>Error while configuring BLE service</source>
        <translation>BLE 서비스 구성 중 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="677"/>
        <source>Operation error</source>
        <translation>작업 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="680"/>
        <source>Characteristic write error</source>
        <translation>특성 쓰기 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="683"/>
        <source>Descriptor write error</source>
        <translation>디스크립터 쓰기 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="686"/>
        <source>Unknown error</source>
        <translation>알 수 없는 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="689"/>
        <source>Characteristic read error</source>
        <translation>특성 읽기 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="692"/>
        <source>Descriptor read error</source>
        <translation>디스크립터 읽기 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="942"/>
        <source>BLE Device</source>
        <translation>BLE 장치</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="950"/>
        <source>Service</source>
        <translation>서비스</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="969"/>
        <source>Characteristic</source>
        <translation>특성</translation>
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
        <translation>CAN Bus 사용 불가</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="274"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>이 시스템에서 CAN bus 플러그인을 찾을 수 없습니다.

macOS의 CAN bus 지원은 제한적이며 타사 하드웨어 드라이버가 필요할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="279"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>이 플랫폼에서는 CAN bus 플러그인을 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="291"/>
        <source>Invalid CAN Configuration</source>
        <translation>잘못된 CAN 구성</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="299"/>
        <source>Invalid Selection</source>
        <translation>잘못된 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="308"/>
        <source>No Devices Available</source>
        <translation>사용 가능한 장치 없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="224"/>
        <source>CAN Device Creation Failed</source>
        <translation>CAN 장치 생성 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="242"/>
        <source>CAN Connection Failed</source>
        <translation>CAN 연결 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="262"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>이 시스템에서 CAN 버스 플러그인을 찾을 수 없습니다.

Linux에서는 SOCKETCAN 커널 모듈이 로드되었는지 확인하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="268"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>이 시스템에서 CAN 버스 플러그인을 찾을 수 없습니다.

Windows에서는 CAN 하드웨어 드라이버(PEAK, VECTOR 등)를 설치하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="292"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>CAN 버스 구성이 불완전합니다. 유효한 플러그인과 인터페이스를 선택하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="300"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>선택한 플러그인 또는 인터페이스를 더 이상 사용할 수 없습니다. 목록을 새로 고침하고 다시 시도하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="309"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>플러그인 또는 인터페이스 목록이 비어 있습니다. 목록을 새로 고침하고 CAN 하드웨어가 연결되어 있는지 확인하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="225"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>CAN 버스 장치를 생성할 수 없습니다. 하드웨어와 드라이버를 확인하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="244"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>CAN 버스 장치에 연결할 수 없습니다. 하드웨어 연결 및 설정을 확인하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="582"/>
        <source>CAN Bus Error</source>
        <translation>CAN Bus 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="583"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>오류가 발생했지만 CAN 장치를 더 이상 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="590"/>
        <source>Error code: %1</source>
        <translation>오류 코드: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="593"/>
        <source>CAN Bus Communication Error</source>
        <translation>CAN Bus 통신 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="648"/>
        <source>No CAN driver selected</source>
        <translation>CAN 드라이버가 선택되지 않음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="609"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>먼저 SOCKETCAN 커널 모듈을 로드하십시오</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="612"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>먼저 가상 CAN 인터페이스를 설정하십시오</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="614"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="634"/>
        <source>No interfaces found for %1</source>
        <translation>%1에 대한 인터페이스를 찾을 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="618"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN 드라이버&lt;/a&gt;를 설치하십시오</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="622"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;VECTOR CAN 드라이버&lt;/a&gt;를 설치하십시오</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="626"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>&lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN 드라이버&lt;/a&gt; 설치</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="629"/>
        <source>Install %1 drivers</source>
        <translation>%1 드라이버 설치</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="632"/>
        <source>Install %1 drivers for macOS</source>
        <translation>macOS용 %1 드라이버 설치</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="729"/>
        <source>Plugin</source>
        <translation>플러그인</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="737"/>
        <source>Interface</source>
        <translation>인터페이스</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="745"/>
        <source>Bitrate</source>
        <translation>비트레이트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="754"/>
        <source>CAN FD</source>
        <translation>CAN FD</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="178"/>
        <source>Unknown error</source>
        <translation>알 수 없는 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="181"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>사용자가 'plugdev' 그룹에 속해 있는지 또는 udev 규칙이 이 장치에 대한 액세스 권한을 부여하는지 확인하십시오.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="185"/>
        <source>Failed to open "%1"</source>
        <translation>"%1" 열기 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="295"/>
        <source>HID Device Error</source>
        <translation>HID 장치 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="296"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>HID 장치의 연결이 끊어졌거나 치명적인 읽기 오류가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="405"/>
        <source>Select Device</source>
        <translation>장치 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="549"/>
        <source>HID Device</source>
        <translation>HID 장치</translation>
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
        <translation>TLS 1.3 이상</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 이상</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="65"/>
        <source>Any Protocol</source>
        <translation>모든 프로토콜</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>Secure Protocols Only</source>
        <translation>보안 프로토콜만</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>None</source>
        <translation>없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="70"/>
        <source>Query Peer</source>
        <translation>피어 조회</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="71"/>
        <source>Verify Peer</source>
        <translation>피어 검증</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="72"/>
        <source>Auto Verify Peer</source>
        <translation>피어 자동 검증</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="177"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>MQTT 기능은 상용 라이선스가 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="178"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation>MQTT 브로커 구독은 유효한 Serial Studio 상용 라이선스(Hobbyist 등급 이상)가 있어야 사용할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="408"/>
        <source>Use System Database</source>
        <translation>시스템 데이터베이스 사용</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="409"/>
        <source>Load From Folder…</source>
        <translation>폴더에서 불러오기…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="442"/>
        <source>Select PEM Certificates Directory</source>
        <translation>PEM 인증서 디렉터리 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Hostname</source>
        <translation>호스트명</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Port</source>
        <translation>포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="714"/>
        <source>Topic Filter</source>
        <translation>토픽 필터</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="721"/>
        <source>Client ID</source>
        <translation>클라이언트 ID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="728"/>
        <source>Username</source>
        <translation>사용자 이름</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="735"/>
        <source>Password</source>
        <translation>비밀번호</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="742"/>
        <source>MQTT Version</source>
        <translation>MQTT 버전</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="757"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (초)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="766"/>
        <source>Auto Keep Alive</source>
        <translation>자동 Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="783"/>
        <source>SSL/TLS Enabled</source>
        <translation>SSL/TLS 활성화</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>SSL Protocol</source>
        <translation>SSL 프로토콜</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="801"/>
        <source>Peer Verify Mode</source>
        <translation>피어 검증 모드</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="809"/>
        <source>Peer Verify Depth</source>
        <translation>피어 검증 깊이</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="910"/>
        <source>MQTT Subscription Error</source>
        <translation>MQTT 구독 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="911"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>토픽 "%1" 구독 실패.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>유효하지 않은 MQTT 프로토콜 버전</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>브로커가 설정된 MQTT 프로토콜 버전을 거부했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Client ID Rejected</source>
        <translation>클라이언트 ID 거부됨</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>브로커가 클라이언트 ID를 거부했습니다. 다른 식별자를 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Server Unavailable</source>
        <translation>MQTT 서버 사용 불가</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>브로커를 현재 사용할 수 없습니다. 나중에 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>Authentication Error</source>
        <translation>인증 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>제공된 자격 증명이 브로커에 의해 거부되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>Authorization Error</source>
        <translation>권한 부여 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>Account lacks permission for this operation.</source>
        <translation>계정에 이 작업을 수행할 권한이 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="958"/>
        <source>Network or Transport Error</source>
        <translation>네트워크 또는 전송 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="959"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>브로커에 연결하는 동안 네트워크/전송 계층 문제가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="962"/>
        <source>MQTT Protocol Violation</source>
        <translation>MQTT 프로토콜 위반</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="963"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>브로커가 프로토콜 위반을 보고하고 연결을 종료했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="966"/>
        <source>MQTT 5 Error</source>
        <translation>MQTT 5 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="967"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>MQTT 5 프로토콜 수준 오류가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="970"/>
        <source>MQTT Error</source>
        <translation>MQTT 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="971"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>예기치 않은 MQTT 오류가 발생했습니다.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>잘못된 시리얼 포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Modbus 초기화 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Modbus 연결 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>선택한 시리얼 포트 "%1"을(를) 더 이상 사용할 수 없습니다. 포트 목록을 새로 고침하고 다시 시도하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Modbus 장치를 생성할 수 없습니다. 시스템 구성을 확인하고 다시 시도하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>"%1"에 연결할 수 없습니다. 연결 설정을 확인하세요.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="566"/>
        <source>None</source>
        <translation>없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="567"/>
        <source>Even</source>
        <translation>짝수</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="568"/>
        <source>Odd</source>
        <translation>홀수</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="569"/>
        <source>Space</source>
        <translation>스페이스</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="570"/>
        <source>Mark</source>
        <translation>마크</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="622"/>
        <source>Holding Registers (0x03)</source>
        <translation>홀딩 레지스터 (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="623"/>
        <source>Input Registers (0x04)</source>
        <translation>입력 레지스터 (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="624"/>
        <source>Coils (0x01)</source>
        <translation>코일 (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="625"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>이산 입력 (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="811"/>
        <source>No register groups configured</source>
        <translation>레지스터 그룹이 구성되지 않음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="812"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>프로젝트를 생성하기 전에 하나 이상의 레지스터 그룹을 추가하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="814"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="827"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="852"/>
        <source>Modbus Project Generator</source>
        <translation>Modbus 프로젝트 생성기</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">임시 프로젝트 파일 생성 실패</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">임시 디렉터리에 대한 쓰기 권한을 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="824"/>
        <source>Failed to load generated project</source>
        <translation>생성된 프로젝트 로드 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="825"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>생성된 프로젝트 JSON을 로드할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="847"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>%1개 그룹과 %2개 데이터셋으로 프로젝트를 성공적으로 생성했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>The project editor is now open for customization.</source>
        <translation>프로젝트 편집기가 사용자 지정을 위해 열렸습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="866"/>
        <source>Modbus Project</source>
        <translation>Modbus 프로젝트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="872"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="894"/>
        <source>Holding Registers</source>
        <translation>Holding Register</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="895"/>
        <source>Input Registers</source>
        <translation>입력 레지스터</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="896"/>
        <source>Coils</source>
        <translation>코일</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="897"/>
        <source>Discrete Inputs</source>
        <translation>이산 입력</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="912"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="927"/>
        <source>Register %1</source>
        <translation>레지스터 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="935"/>
        <source>Coil %1</source>
        <translation>코일 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="935"/>
        <source>Discrete %1</source>
        <translation>이산 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1354"/>
        <source>Error code: %1</source>
        <translation>오류 코드: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1357"/>
        <source>Modbus Communication Error</source>
        <translation>Modbus 통신 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1370"/>
        <source>Select Port</source>
        <translation>포트 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1527"/>
        <source>Protocol</source>
        <translation>프로토콜</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1535"/>
        <source>Slave Address</source>
        <translation>슬레이브 주소</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1544"/>
        <source>Poll Interval (ms)</source>
        <translation>폴링 간격 (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1585"/>
        <source>Host / IP</source>
        <translation>호스트 / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1592"/>
        <source>Port</source>
        <translation>포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1607"/>
        <source>Serial Port</source>
        <translation>시리얼 포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1615"/>
        <source>Baud Rate</source>
        <translation>보레이트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1623"/>
        <source>Parity</source>
        <translation>패리티</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1631"/>
        <source>Data Bits</source>
        <translation>데이터 비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1639"/>
        <source>Stop Bits</source>
        <translation>스톱 비트</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="495"/>
        <source>Network socket error</source>
        <translation>네트워크 소켓 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="512"/>
        <source>Socket Type</source>
        <translation>소켓 유형</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="520"/>
        <source>Remote Address</source>
        <translation>원격 주소</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="528"/>
        <source>TCP Port</source>
        <translation>TCP 포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="537"/>
        <source>UDP Local Port</source>
        <translation>UDP 로컬 포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="546"/>
        <source>UDP Remote Port</source>
        <translation>UDP 원격 포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="555"/>
        <source>UDP Multicast</source>
        <translation>UDP 멀티캐스트</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="190"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="234"/>
        <source>Failed to start process</source>
        <translation>프로세스 시작 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="191"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>실행 파일 "%1"을(를) PATH에서 찾을 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="379"/>
        <source>Select Executable</source>
        <translation>실행 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="404"/>
        <source>Select Working Directory</source>
        <translation>작업 디렉터리 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="430"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Named Pipe / FIFO 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="529"/>
        <source>The process crashed.</source>
        <translation>프로세스가 충돌했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="530"/>
        <source>Exit code: %1</source>
        <translation>종료 코드: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="533"/>
        <source>Process "%1" stopped</source>
        <translation>프로세스 "%1" 중지됨</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="552"/>
        <source>Unknown error</source>
        <translation>알 수 없는 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="553"/>
        <source>Process Error</source>
        <translation>프로세스 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="567"/>
        <source>Pipe Error</source>
        <translation>Pipe 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="567"/>
        <source>Could not open named pipe: %1</source>
        <translation>Named Pipe를 열 수 없음: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="781"/>
        <source>Mode</source>
        <translation>모드</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="784"/>
        <source>Launch Process</source>
        <translation>프로세스 실행</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="784"/>
        <source>Named Pipe</source>
        <translation>명명된 파이프</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="789"/>
        <source>Executable</source>
        <translation>실행 파일</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="796"/>
        <source>Arguments</source>
        <translation>인수</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="803"/>
        <source>Working Directory</source>
        <translation>작업 디렉터리</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="810"/>
        <source>Pipe Path</source>
        <translation>파이프 경로</translation>
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
        <translation>없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="352"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="752"/>
        <source>Select Port</source>
        <translation>포트 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="396"/>
        <source>Even</source>
        <translation>짝수</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="397"/>
        <source>Odd</source>
        <translation>홀수</translation>
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
        <translation>"%1"은(는) 유효한 경로가 아닙니다</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="573"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>사용자 정의 시리얼 장치를 등록하려면 다른 경로를 입력하십시오</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="852"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>지정한 장치를 찾을 수 없습니다. 연결을 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="859"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>알 수 없는 오류가 발생했습니다. 장치를 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="861"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>장치가 열려 있지 않습니다. 이 작업을 시도하기 전에 장치를 여십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="263"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>시리얼 포트 "%1"에 연결하지 못했습니다</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>Critical error on serial port "%1"</source>
        <translation>시리얼 포트 "%1"에서 치명적 오류 발생</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>Unknown error</source>
        <translation>알 수 없는 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="851"/>
        <source>No error occurred.</source>
        <translation>오류가 발생하지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="853"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>권한이 거부되었습니다. 애플리케이션이 장치에 대한 필요한 액세스 권한을 가지고 있는지 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="854"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>장치를 열지 못했습니다. 이미 사용 중이거나 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="855"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>장치에 데이터를 쓰는 중 오류가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="856"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>장치에서 데이터를 읽는 중 오류가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="857"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>치명적인 리소스 오류가 발생했습니다. 장치가 연결 해제되었거나 더 이상 액세스할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="858"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>요청한 작업은 이 장치에서 지원되지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="860"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>작업 시간이 초과되었습니다. 장치가 응답하지 않을 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1029"/>
        <source>Serial Port</source>
        <translation>시리얼 포트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1037"/>
        <source>Baud Rate</source>
        <translation>보레이트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1045"/>
        <source>Parity</source>
        <translation>패리티</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1053"/>
        <source>Data Bits</source>
        <translation>데이터 비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1061"/>
        <source>Stop Bits</source>
        <translation>스톱 비트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1069"/>
        <source>Flow Control</source>
        <translation>흐름 제어</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1077"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1084"/>
        <source>Auto-Reconnect</source>
        <translation>자동 재연결</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="185"/>
        <source>USB Error</source>
        <translation>USB 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>USB 하위 시스템을 초기화하지 못했습니다. 시스템에 libusb가 설치되어 있는지 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="223"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="240"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="632"/>
        <source>USB Device Error</source>
        <translation>USB 장치 오류</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>USB 장치를 열 수 없습니다: %1.

Linux에서는 장치 노드에 대한 읽기/쓰기 권한이 있는지 확인하십시오(udev 규칙 추가 또는 root로 실행). macOS에서는 커널 드라이버를 먼저 분리해야 할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="186"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>USB 장치가 선택되지 않았습니다. 장치를 선택한 후 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="193"/>
        <source>Unknown Device</source>
        <translation>알 수 없는 장치</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="198"/>
        <source>Failed to open "%1"</source>
        <translation>"%1"을(를) 열지 못했습니다</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="241"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>USB 장치에서 인터페이스 %1을(를) 요청하지 못했습니다.

다른 드라이버나 애플리케이션이 이미 사용 중일 수 있습니다. Linux에서는 커널 드라이버(예: cdc_acm)를 언로드하거나 udev 규칙을 추가해 보십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="404"/>
        <source>Select Device</source>
        <translation>장치 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="423"/>
        <source>Select IN Endpoint</source>
        <translation>IN 엔드포인트 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="434"/>
        <source>None (Read-only)</source>
        <translation>없음(읽기 전용)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="509"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>고급 USB 제어 전송을 활성화하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="510"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>벌크 전송 외에 제어 전송을 활성화합니다. 잘못된 제어 요청을 전송하면 연결된 하드웨어가 손상되거나 충돌할 수 있습니다. 용도를 정확히 알고 있는 경우에만 활성화하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="514"/>
        <source>Advanced USB Mode</source>
        <translation>고급 USB 모드</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="633"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>USB 장치의 연결이 끊어졌거나 치명적인 읽기 오류가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="772"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>이 장치에서 아이소크로너스 IN 엔드포인트를 찾을 수 없지만 벌크 엔드포인트는 사용 가능합니다.

전송 모드를 "벌크 스트림"으로 전환한 후 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="777"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>이 장치에서 벌크 IN 엔드포인트를 찾을 수 없지만 아이소크로너스 엔드포인트는 사용 가능합니다.

전송 모드를 "아이소크로너스"로 전환한 후 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="781"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>이 장치에서 사용 가능한 IN 엔드포인트를 찾을 수 없습니다.

장치가 활성 구성에서 데이터 엔드포인트를 노출하지 않거나 특정 드라이버가 필요할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1212"/>
        <source>USB Device</source>
        <translation>USB 장치</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1220"/>
        <source>Transfer Mode</source>
        <translation>전송 모드</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Bulk Stream</source>
        <translation>벌크 스트림</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Advanced Control</source>
        <translation>고급 제어</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Isochronous</source>
        <translation>동기식</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1228"/>
        <source>IN Endpoint</source>
        <translation>IN 엔드포인트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1236"/>
        <source>OUT Endpoint</source>
        <translation>OUT 엔드포인트</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1244"/>
        <source>ISO Packet Size</source>
        <translation>ISO 패킷 크기</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="214"/>
        <source>No file selected…</source>
        <translation>파일이 선택되지 않음…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="249"/>
        <source>Plain Text</source>
        <translation>일반 텍스트</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="250"/>
        <source>Raw Binary</source>
        <translation>Raw 바이너리</translation>
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
        <translation>전송할 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="295"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>파일 선택됨: %1 (%2바이트)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="298"/>
        <source>Error opening file: %1</source>
        <translation>파일 열기 오류: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="389"/>
        <source>Starting %1 transfer…</source>
        <translation>%1 전송 시작 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="624"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="644"/>
        <source>Transmission complete</source>
        <translation>전송 완료</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="626"/>
        <source>Plain text transmission complete</source>
        <translation>일반 텍스트 전송 완료</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="646"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>원시 바이너리 전송 완료 (%1바이트)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="670"/>
        <source>Transfer complete</source>
        <translation>전송 완료</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="671"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>전송 성공적으로 완료됨 (%1바이트)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="673"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="674"/>
        <source>Transfer failed: %1</source>
        <translation>전송 실패: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="758"/>
        <source>%1 B/s</source>
        <translation>%1 B/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="760"/>
        <source>%1 KB/s</source>
        <translation>%1 KB/s</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="762"/>
        <source>%1 MB/s</source>
        <translation>%1 MB/s</translation>
    </message>
</context>
<context>
    <name>IO::FrameReader</name>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="298"/>
        <source>Frames dropped</source>
        <translation>프레임 손실</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="300"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>수신 데이터가 Serial Studio의 처리 속도보다 빠르게 도착하고 있습니다; %1개의 프레임이 손실되었습니다. 데이터 전송률을 줄이거나 부하가 큰 소비자를 비활성화하십시오.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="85"/>
        <source>Cannot open file: %1</source>
        <translation>파일을 열 수 없음: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="114"/>
        <source>Transfer cancelled</source>
        <translation>전송 취소됨</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="115"/>
        <source>Transfer cancelled by user</source>
        <translation>사용자가 전송을 취소함</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="95"/>
        <source>Waiting for receiver…</source>
        <translation>수신기 대기 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="211"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>수신기 준비 완료 (CRC 모드), 데이터 전송 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="144"/>
        <source>Too many retries, transfer aborted</source>
        <translation>재시도 횟수 초과, 전송 중단됨</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="145"/>
        <source>Maximum retries exceeded</source>
        <translation>최대 재시도 횟수 초과</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="149"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>NAK 수신, 블록 %1 재시도 중 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="158"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="396"/>
        <source>Failed to seek in file</source>
        <translation>파일 탐색 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="168"/>
        <source>Transfer cancelled by receiver</source>
        <translation>수신자가 전송 취소</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="169"/>
        <source>Receiver cancelled the transfer</source>
        <translation>수신자가 전송을 취소했습니다</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="183"/>
        <source>Transfer complete</source>
        <translation>전송 완료</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="310"/>
        <source>File read returned more data than requested</source>
        <translation>파일 읽기가 요청된 것보다 많은 데이터를 반환했습니다</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="326"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>블록 %1 전송 중 (%2바이트)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="339"/>
        <source>Sending EOT…</source>
        <translation>EOT 전송 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="386"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>타임아웃, 재시도 중 (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="381"/>
        <source>Transfer timed out</source>
        <translation>전송 시간 초과</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="382"/>
        <source>Timeout: no response from receiver</source>
        <translation>시간 초과: 수신기 응답 없음</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="64"/>
        <source>Cannot open file: %1</source>
        <translation>파일을 열 수 없음: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="98"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="176"/>
        <source>Transfer cancelled by receiver</source>
        <translation>수신기가 전송을 취소함</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="99"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="177"/>
        <source>Receiver cancelled the transfer</source>
        <translation>수신기가 전송을 취소함</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="76"/>
        <source>Waiting for receiver…</source>
        <translation>수신기 대기 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="135"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="331"/>
        <source>Sending first EOT…</source>
        <translation>첫 번째 EOT 전송 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="153"/>
        <source>Too many retries, transfer aborted</source>
        <translation>재시도 횟수 초과, 전송 중단됨</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="154"/>
        <source>Maximum retries exceeded</source>
        <translation>최대 재시도 횟수 초과</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="158"/>
        <source>NAK received, retrying block %1</source>
        <translation>NAK 수신됨, 블록 %1 재시도 중</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="164"/>
        <source>Failed to seek in file</source>
        <translation>파일 탐색 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="228"/>
        <source>Sending second EOT…</source>
        <translation>두 번째 EOT 전송 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="312"/>
        <source>Sending end-of-batch marker…</source>
        <translation>배치 종료 마커 전송 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="252"/>
        <source>Transfer complete</source>
        <translation>전송 완료</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="297"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>파일 헤더 전송 중: %1 (%2바이트)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="346"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>블록 %1 전송 중 (%2/%3바이트)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="88"/>
        <source>Cannot open file: %1</source>
        <translation>파일을 열 수 없음: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="106"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>ZMODEM에 비해 파일이 너무 큼 (%1바이트, 제한 4GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="133"/>
        <source>Transfer cancelled</source>
        <translation>전송 취소됨</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="134"/>
        <source>Transfer cancelled by user</source>
        <translation>사용자가 전송을 취소함</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="279"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>16진수 헤더 CRC 불일치, 프레임 폐기</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="466"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>파일 정보 전송 중: %1 (%2바이트)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="482"/>
        <source>Failed to seek to offset %1</source>
        <translation>오프셋 %1로 이동 실패</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="513"/>
        <source>File read returned more data than requested</source>
        <translation>파일 읽기가 요청된 것보다 많은 데이터 반환</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="601"/>
        <source>Receiver requests data from offset %1</source>
        <translation>수신기가 오프셋 %1부터 데이터 요청</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="610"/>
        <source>Receiver skipped the file</source>
        <translation>수신기가 파일 건너뜀</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="623"/>
        <source>Too many errors, transfer aborted</source>
        <translation>오류 과다, 전송 중단됨</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="624"/>
        <source>Maximum retries exceeded</source>
        <translation>최대 재시도 횟수 초과</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="439"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>ZRQINIT 전송 중, 수신기 대기 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="541"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>파일 데이터 전송 완료, 확인 대기 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="552"/>
        <source>Sending ZFIN…</source>
        <translation>ZFIN 전송 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="589"/>
        <source>Receiver ready, sending file info…</source>
        <translation>수신기 준비 완료, 파일 정보 전송 중…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>NAK 수신, 재시도 중 (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="650"/>
        <source>Transfer complete</source>
        <translation>전송 완료</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="661"/>
        <source>Transfer cancelled by receiver</source>
        <translation>수신기가 전송을 취소함</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="662"/>
        <source>Receiver cancelled the transfer</source>
        <translation>수신기가 전송을 취소함</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="671"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="672"/>
        <source>Receiver reported a file error</source>
        <translation>수신기가 파일 오류를 보고함</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="879"/>
        <source>Transfer timed out</source>
        <translation>전송 타임아웃</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="880"/>
        <source>Timeout: no response from receiver</source>
        <translation>타임아웃: 수신기 응답 없음</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="884"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>타임아웃, 재시도 중 (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>아이콘 선택</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>온라인 검색…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>확인</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>보통</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>그레이스케일</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>고대비</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>선명</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>야간 투시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>적외선</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>딥 블루</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>앰버</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>이미지 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>내보내기 폴더 열기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>확대</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>축소</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>십자선 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>이미지 대기 중…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="24"/>
        <source>API Keys</source>
        <translation>API 키</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="48"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. 기본값은 Claude Haiku 4.5(백만 토큰당 입력 $1 / 출력 $5)입니다. Sonnet 4.6 및 OPUS 4.7도 사용 가능합니다. 스트리밍, 도구 사용, 확장 사고 및 프롬프트 캐싱을 지원합니다.</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions. 기본값은 GPT-4o mini(백만 토큰당 입력 $0.15 / 출력 $0.60)입니다. GPT-4o, GPT-4 Turbo 및 o1-mini도 사용 가능합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="58"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. 기본값은 Gemini 2.0 Flash이며, 넉넉한 무료 티어를 제공합니다(속도 제한 적용). Gemini 1.5 Pro 및 Gemini 1.5 Flash도 사용 가능합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="101"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>자체 API 키를 사용하세요. 키는 머신별 키로 암호화되어 저장되며, 선택한 제공업체와 통신할 때를 제외하고는 컴퓨터 외부로 전송되지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>Key set</source>
        <translation>키 설정됨</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="168"/>
        <source>No key</source>
        <translation>키 없음</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">키가 등록되어 있습니다 — 새 키를 붙여넣어 교체하세요</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="53"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. 기본값은 빠르고 비용 효율적인 에이전트 작업을 위한 GPT-5 mini입니다. GPT-5.2는 더 강력한 범용 옵션이며, GPT-5.2 Chat은 현재 ChatGPT에서 사용되는 모델을 추적합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="62"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. OpenAI 호환 API. 기본값은 deepseek-chat(V3)입니다. deepseek-reasoner(R1)도 사용 가능합니다. 도구 사용에 있어 가장 저렴한 클라우드 옵션인 경우가 많습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="66"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. 하나의 키로 Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen 등의 약 200개 모델을 사용할 수 있습니다. 무료 계층 모델(:free 접미사)은 속도 제한이 있지만 추가 결제가 필요하지 않습니다. 나머지는 사용량 기반 요금제입니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="71"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. 매우 빠른 Llama, Mixtral, Gemma, DeepSeek-R1 distill, Qwen 모델을 위한 하드웨어 가속 추론(LPU)을 제공합니다. 일일 토큰 제한이 있는 넉넉한 무료 계층을 제공합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="75"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. 기본값은 Mistral Large입니다. Codestral은 코드 완성을 대상으로 하고, Pixtral은 비전을 처리하며, Ministral 모델은 엣지/저지연 사용에 최적화되어 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="79"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>로컬 모델 서버. OpenAI 호환 엔드포인트(Ollama, llama.cpp의 llama-server, LM Studio, vLLM)와 작동합니다. 모든 데이터가 사용자 시스템에 남습니다. 모델 목록은 서버에서 실시간으로 조회됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>키가 등록되어 있습니다 -- 새 키를 붙여넣어 교체하세요</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="206"/>
        <source>Paste your API key here</source>
        <translation>API 키를 여기에 붙여넣으세요</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Hide key</source>
        <translation>키 숨기기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="214"/>
        <source>Show key while typing</source>
        <translation>입력 중 키 표시</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Get key</source>
        <translation>키 가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="226"/>
        <source>Open the provider's console to create a new key</source>
        <translation>공급자의 콘솔을 열어 새 키 생성</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="237"/>
        <source>Save</source>
        <translation>저장</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="255"/>
        <source>Remove the stored key for %1</source>
        <translation>%1에 저장된 키 제거</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="279"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Install Ollama</source>
        <translation>Ollama 설치</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="284"/>
        <source>Open the Ollama download page</source>
        <translation>Ollama 다운로드 페이지 열기</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="295"/>
        <source>Apply</source>
        <translation>적용</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="310"/>
        <source>Re-query the model list</source>
        <translation>모델 목록 다시 조회</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="358"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>아직 구성된 API 키가 없습니다. 시작하려면 키를 추가하세요.</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">아직 구성된 API 키가 없습니다. 시작하려면 위에서 하나 이상 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="361"/>
        <source>One provider is ready.</source>
        <translation>공급자 1개 준비됨.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="363"/>
        <source>%1 providers are ready.</source>
        <translation>공급자 %1개 준비됨.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>라이선스</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>잠시 기다려 주세요…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Serial Studio Pro 활성화</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>MQTT, 3D 플로팅 등 Pro 기능을 잠금 해제하려면 아래에 라이선스 키를 붙여넣으세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>라이선스에는 5개의 기기 활성화가 포함됩니다.
월간, 연간 및 평생 플랜이 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>라이선스 키를 여기에 붙여넣으세요…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>붙여넣기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>제품</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>라이선스 소유자</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>라이선스 소유자 이메일</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>장치 사용량</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1개 장치 사용 중 (무제한 플랜)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>%2개 중 %1개 장치 사용됨</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>장치 ID</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>라이선스 키</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>고객 포털</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>라이선스 구매</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>활성화</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>비활성화</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="546"/>
        <source>There was an issue validating your license.</source>
        <translation>라이선스 유효성 검사 중 문제가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="564"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="745"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="870"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>제공된 라이선스 키는 Serial Studio에 속하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="565"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>공식 Serial Studio 스토어에서 라이선스를 구매했는지 다시 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="576"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="754"/>
        <source>This license key was activated on a different device.</source>
        <translation>이 라이선스 키는 다른 장치에서 활성화되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="577"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>먼저 해당 장치에서 비활성화하거나 지원팀에 문의하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="588"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="765"/>
        <source>This license is not currently active.</source>
        <translation>이 라이선스는 현재 활성 상태가 아닙니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="589"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="766"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>만료되었거나 비활성화되었을 수 있습니다(상태: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="599"/>
        <source>Something went wrong on the server.</source>
        <translation>서버에서 문제가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="600"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="776"/>
        <source>No activation ID was returned.</source>
        <translation>활성화 ID가 반환되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="610"/>
        <source>Could not validate your license at this time.</source>
        <translation>현재 라이선스를 유효성 검사할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="611"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="785"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="881"/>
        <source>Try again later.</source>
        <translation>나중에 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="871"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>공식 Serial Studio 스토어에서 라이선스를 구매했는지 다시 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="775"/>
        <source>Something went wrong on the server…</source>
        <translation>서버에서 문제가 발생했습니다…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="629"/>
        <source>%1 %2</source>
        <translation>%1 %2</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="631"/>
        <source>%1 (%2)</source>
        <translation>%1 (%2)</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="682"/>
        <source>Your license has been successfully activated.</source>
        <translation>라이선스가 성공적으로 활성화되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="683"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Serial Studio를 후원해 주셔서 감사합니다!
이제 모든 프리미엄 기능을 사용할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>There was an issue activating your license.</source>
        <translation>라이선스 활성화 중 문제가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="784"/>
        <source>Could not activate your license at this time.</source>
        <translation>현재 라이선스를 활성화할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="861"/>
        <source>There was an issue deactivating your license.</source>
        <translation>라이선스 비활성화 중 문제가 발생했습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="880"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>현재 라이선스를 비활성화할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="891"/>
        <source>Your license has been deactivated.</source>
        <translation>라이선스가 비활성화되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="892"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Pro 기능에 대한 액세스가 제거되었습니다.
Serial Studio를 지원해 주셔서 다시 한번 감사드립니다!</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="638"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>MDF4 내보내기는 Pro 기능입니다.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="639"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>이 기능을 사용하려면 라이선스가 필요합니다. MDF4 내보내기를 활성화하려면 라이선스를 구매하십시오.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="363"/>
        <source>Select MDF4 file</source>
        <translation>MDF4 파일 선택</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="365"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>MDF4 파일 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="389"/>
        <source>Disconnect from device?</source>
        <translation>장치에서 연결을 끊으시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="390"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>MDF4 파일을 열기 전에 현재 장치에서 연결을 끊어야 합니다.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="407"/>
        <source>Cannot open MDF4 file</source>
        <translation>MDF4 파일을 열 수 없습니다</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="408"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>파일이 손상되었거나 지원되지 않는 형식일 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="415"/>
        <source>Invalid MDF4 file</source>
        <translation>잘못된 MDF4 파일</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="416"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>파일 구조 읽기 실패. 파일이 손상되었을 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="433"/>
        <source>No data in file</source>
        <translation>파일에 데이터 없음</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="434"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>MDF4 파일에 측정 데이터가 없습니다.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>호스트명</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>예: broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>포트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>토픽 필터</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>예: sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>클라이언트 ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>재생성</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>사용자 이름</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>비밀번호</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>버전</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (초)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>SSL/TLS 사용</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>SSL 프로토콜</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>피어 검증</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>검증 깊이</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>CA 인증서</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>폴더에서 불러오기…</translation>
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
        <translation type="vanished">TLS 1.3 이상</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 이상</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">모든 프로토콜</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">보안 프로토콜만</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">없음</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">피어 조회</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">피어 검증</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">피어 자동 검증</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">시스템 데이터베이스 사용</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">MQTT 구독자</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">MQTT 발행자</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">MQTT 기능은 상용 라이선스가 필요합니다</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">폴더에서 불러오기…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">MQTT 브로커 연결은 유효한 Serial Studio 상용 라이선스(Hobbyist 등급 이상)가 있어야 사용할 수 있습니다.

이 기능을 사용하려면 라이선스를 활성화하거나 스토어를 방문하십시오.</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">MQTT 토픽 누락</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">발행자로 연결하기 전에 토픽을 지정해야 합니다.</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">구성 오류</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">MQTT 토픽이 설정되지 않음</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">토픽이 구성될 때까지 메시지를 수신할 수 없습니다.</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">구성 경고</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">유효하지 않은 MQTT 토픽</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">토픽 "%1"이(가) 유효하지 않습니다.</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">PEM 인증서 디렉터리 선택</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">구독 오류</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">토픽 "%1" 구독 실패.</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">유효하지 않은 MQTT 프로토콜 버전</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">MQTT 브로커가 지원되지 않는 프로토콜 버전으로 인해 연결을 거부했습니다. 클라이언트와 브로커가 동일한 프로토콜 버전을 지원하는지 확인하십시오.</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">클라이언트 ID 거부됨</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">브로커가 클라이언트 ID를 거부했습니다. 형식이 잘못되었거나, 너무 길거나, 이미 사용 중일 수 있습니다. 다른 클라이언트 ID를 사용해 보십시오.</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">MQTT 서버 사용 불가</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">네트워크 연결이 설정되었으나 브로커를 현재 사용할 수 없습니다. 브로커 상태를 확인하고 나중에 다시 시도하십시오.</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">인증 오류</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">제공된 사용자 이름 또는 비밀번호가 올바르지 않거나 형식이 잘못되었습니다. 자격 증명을 다시 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">권한 부여 오류</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">MQTT 브로커가 권한 부족으로 인해 연결을 거부했습니다. 계정에 필요한 액세스 권한이 있는지 확인하십시오.</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">네트워크 또는 전송 오류</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">네트워크 또는 전송 계층 문제가 발생하여 예기치 않은 연결 실패가 발생했습니다. 네트워크 연결 및 브로커 설정을 확인하십시오.</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">MQTT 프로토콜 위반</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">클라이언트가 MQTT 프로토콜 위반을 감지하여 연결을 종료했습니다. MQTT 구현의 준수 여부를 확인하십시오.</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">알 수 없는 오류</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">예기치 않은 오류가 발생했습니다. 자세한 내용은 로그를 확인하거나 애플리케이션을 재시작하십시오.</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">MQTT 5 오류</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">MQTT 프로토콜 레벨 5 오류가 발생했습니다. 자세한 내용은 브로커 로그 또는 이유 코드를 확인하십시오.</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">MQTT 인증 실패</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">인증 실패: %1.</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">확장 인증이 필요하지만 MQTT 5.0이 활성화되지 않았습니다.</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">알 수 없음</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">MQTT 인증 필요</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">MQTT 브로커가 "%1" 방식을 사용한 인증을 요구합니다.

필요한 자격 증명을 제공하십시오.</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">MQTT 사용자 이름 입력</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">사용자 이름:</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">MQTT 비밀번호 입력</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">비밀번호:</translation>
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
        <translation>TLS 1.3 이상</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="822"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 이상</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="823"/>
        <source>Any Protocol</source>
        <translation>모든 프로토콜</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="824"/>
        <source>Secure Protocols Only</source>
        <translation>보안 프로토콜만</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="826"/>
        <source>None</source>
        <translation>없음</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="827"/>
        <source>Query Peer</source>
        <translation>피어 조회</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="828"/>
        <source>Verify Peer</source>
        <translation>피어 검증</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="829"/>
        <source>Auto Verify Peer</source>
        <translation>피어 자동 검증</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1150"/>
        <source>Raw RX Data</source>
        <translation>원시 RX 데이터</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1151"/>
        <source>Custom Script</source>
        <translation>사용자 정의 스크립트</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1152"/>
        <source>Dashboard Data (CSV)</source>
        <translation>대시보드 데이터 (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1153"/>
        <source>Dashboard Data (JSON)</source>
        <translation>대시보드 데이터 (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1313"/>
        <source>MQTT publisher unavailable</source>
        <translation>MQTT 게시자를 사용할 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1314"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>MQTT 게시 기능을 사용하려면 유효한 상용 라이선스가 필요합니다.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1316"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1890"/>
        <source>MQTT Test Connection</source>
        <translation>MQTT 연결 테스트</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1336"/>
        <source>Select PEM Certificates Directory</source>
        <translation>PEM 인증서 디렉터리 선택</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1887"/>
        <source>MQTT broker reachable</source>
        <translation>MQTT 브로커 연결 가능</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1887"/>
        <source>MQTT broker unreachable</source>
        <translation>MQTT 브로커 연결 불가</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1901"/>
        <source>MQTT broker connection failed</source>
        <translation>MQTT 브로커 연결 실패</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1901"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 발행자</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="49"/>
        <source>MQTT Publisher Script</source>
        <translation>MQTT 발행자 스크립트</translation>
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
        <translation>샘플 프레임 바이트 (텍스트 또는 16진수)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="97"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Test</source>
        <translation>테스트</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Clear</source>
        <translation>지우기</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="101"/>
        <source>Apply</source>
        <translation>적용</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="111"/>
        <source>Language:</source>
        <translation>언어:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="114"/>
        <source>Template:</source>
        <translation>템플릿:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="125"/>
        <source>Frame:</source>
        <translation>프레임:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="129"/>
        <source>Output:</source>
        <translation>출력:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="269"/>
        <source>Enter a frame</source>
        <translation>프레임 입력</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="276"/>
        <source>Invalid hex</source>
        <translation>잘못된 16진수</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="359"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>문서 서식 지정	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>선택 영역 서식 지정	Ctrl+I</translation>
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
-- 파싱된 프레임 하나의 원시 바이트를 수신하고
-- 브로커에 게시할 페이로드를 반환하거나
-- 이 프레임을 건너뛰려면 nil을 반환하는
-- mqtt(frame) 함수를 정의합니다.
--
-- frame 인수는 Frame Parser 스크립트가 보는 것과 동일합니다:
-- FrameReader 구분 기호 사이의 바이트를 담고 있는 Lua 문자열입니다.
--
-- 예제:
--   function mqtt(frame)
--     return frame    -- 그대로 전달
--   end
--
-- 미리 만들어진 예제를 보려면 Template 드롭다운을 사용하세요.
--
</translation>
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
 * 파싱된 프레임 하나의 원시 바이트를 수신하고
 * 브로커에 게시할 페이로드를 반환하거나
 * 이 프레임을 건너뛰려면 null을 반환하는
 * mqtt(frame) 함수를 정의합니다.
 *
 * frame 인수는 Frame Parser 스크립트가 보는 것과 동일합니다:
 * FrameReader 구분 기호 사이의 바이트를 담고 있는 문자열입니다.
 *
 * 예제:
 *   function mqtt(frame) {
 *     return frame;   // 그대로 전달
 *   }
 *
 * 미리 만들어진 예제를 보려면 Template 드롭다운을 사용하세요.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="602"/>
        <source>Script is empty</source>
        <translation>스크립트가 비어 있음</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="609"/>
        <source>Lua engine error</source>
        <translation>Lua 엔진 오류</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="631"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="645"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="683"/>
        <source>Error: %1</source>
        <translation>오류: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="639"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="675"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt()가 정의되지 않음</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="656"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- 프레임 건너뜀)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <source>(non-string return)</source>
        <translation>(비문자열 반환)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- 프레임 건너뜀)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="766"/>
        <source>Select Template…</source>
        <translation>템플릿 선택…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="696"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>연결을 테스트하기 전에 브로커 호스트명과 포트를 구성하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="733"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>%1:%2에 성공적으로 연결되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="744"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>브로커에 도달하지 못한 채 5초 후 시간 초과되었습니다.</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">MQTT 설정</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT는 Pro 기능입니다</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">라이선스를 활성화하거나 스토어를 방문하여 MQTT 지원을 활성화하세요.</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">일반</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">인증</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">MQTT 옵션</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">SSL 속성</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">호스트</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">포트</translation>
    </message>
    <message>
        <source>Client ID</source>
        <translation type="vanished">클라이언트 ID</translation>
    </message>
    <message>
        <source>Keep Alive (s)</source>
        <translation type="vanished">Keep Alive (초)</translation>
    </message>
    <message>
        <source>Clean Session</source>
        <translation type="vanished">Clean Session</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">사용자 이름</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">MQTT 사용자 이름</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">비밀번호</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">MQTT 비밀번호</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">버전</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">모드</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">토픽</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">예: sensors/temperature 또는 home/+/status</translation>
    </message>
    <message>
        <source>Will Retain</source>
        <translation type="vanished">Will 보관</translation>
    </message>
    <message>
        <source>Will QoS</source>
        <translation type="vanished">Will QOS</translation>
    </message>
    <message>
        <source>Will Topic</source>
        <translation type="vanished">Will 토픽</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">예: device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will 메시지</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">예: Device unexpectedly disconnected</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">SSL 활성화</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">SSL 프로토콜</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">검증 깊이</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">검증 모드</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">닫기</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">연결 해제</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">연결</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="184"/>
        <source>Console Only Mode</source>
        <translation>콘솔 전용 모드</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="187"/>
        <source>Quick Plot Mode</source>
        <translation>빠른 플롯 모드</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="194"/>
        <source>Empty Project</source>
        <translation>빈 프로젝트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="659"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="667"/>
        <source>Waiting for data…</source>
        <translation>데이터 대기 중…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="668"/>
        <source>Connecting to device…</source>
        <translation>장치 연결 중…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>코드 샘플</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>자동 완성</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>구문 강조</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>스타일</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>공백</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="35"/>
        <source>Copied to Clipboard</source>
        <translation>클립보드에 복사됨</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>MDF4 플레이어</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>사용자</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>탐색</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>실행</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>%2에서 %1개 작업을 승인하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>이 호출들은 함께 실행됩니다. 각 카드를 펼쳐서 인수를 확인하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>모두 승인</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>모두 거부</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>모두 복사</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>모두 선택</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="62"/>
        <source>You</source>
        <translation>사용자</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>Assistant</source>
        <translation>어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <source>Discovery</source>
        <translation>검색</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Execution</source>
        <translation>실행</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Running</source>
        <translation>실행 중</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Awaiting approval</source>
        <translation>승인 대기 중</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Done</source>
        <translation>완료</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Denied</source>
        <translation>거부됨</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Blocked</source>
        <translation>차단됨</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Approve</source>
        <translation>승인</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Deny</source>
        <translation>거부</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Approve all</source>
        <translation>모두 승인</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Deny all</source>
        <translation>모두 거부</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Arguments</source>
        <translation>인수</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Result</source>
        <translation>결과</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>{family}에서 {n}개 작업을 승인하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>이 호출들은 함께 실행됩니다. 각 카드를 펼쳐 인수를 확인하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="294"/>
        <source>Failed to load README: %1</source>
        <translation>README 로드 실패: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Theme</source>
        <translation>테마</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Frame Parser</source>
        <translation>프레임 파서</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Project Template</source>
        <translation>프로젝트 템플릿</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>Plugin</source>
        <translation>플러그인</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="259"/>
        <source>All Types</source>
        <translation>모든 유형</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="487"/>
        <source>Reset Extensions</source>
        <translation>확장 기능 재설정</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="488"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>모든 확장 기능을 제거하고, 모든 사용자 저장소를 삭제하며, 기본 설정을 복원합니다. 계속하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="531"/>
        <source>Select Extension Repository Folder</source>
        <translation>확장 기능 저장소 폴더 선택</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1064"/>
        <source>Installed (repository no longer available)</source>
        <translation>설치됨 (저장소를 더 이상 사용할 수 없음)</translation>
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
        <translation>플러그인 오류</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>플러그인 "%1"이(가) 설치되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1389"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>확장 기능 "%1"은(는) 플러그인이 아닙니다 (유형: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1410"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>플러그인 메타데이터 파일을 읽을 수 없음:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>플러그인 "%1"은(는) GRPC가 필요하지만 이 빌드에는 GRPC 지원이 포함되어 있지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>이 플러그인은 고성능 데이터 스트리밍을 위해 GRPC를 사용합니다. API 서버를 활성화해야 합니다.

지금 활성화하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1447"/>
        <source>API Server Required</source>
        <translation>API 서버 필요</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1476"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>플러그인 "%1"의 info.json에 'entry' 필드가 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1486"/>
        <source>Entry point not found:
%1</source>
        <translation>진입점을 찾을 수 없음:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1495"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>플러그인 "%1"의 진입점 경로가 유효하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1538"/>
        <source>Missing Dependency</source>
        <translation>종속성 누락</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1539"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>이 플러그인은 "%1"이(가) 필요하지만 시스템에서 찾을 수 없습니다.

다운로드 페이지를 여시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1444"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>플러그인이 Serial Studio와 통신하려면 API 서버가 필요합니다. 지금 활성화하시겠습니까?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>다시 시작 필요</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>새로운 렌더링 백엔드는 Serial Studio를 다시 시작한 후 적용됩니다. 지금 다시 시작하여 변경 사항을 적용하시겠습니까?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="320"/>
        <source>Failed to load page: %1</source>
        <translation>페이지 로드 실패: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="154"/>
        <source>Invalid icon identifier</source>
        <translation>유효하지 않은 아이콘 식별자</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="228"/>
        <source>Empty SVG data received</source>
        <translation>빈 SVG 데이터 수신됨</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Windows 바로 가기 (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>macOS 애플리케이션 (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>데스크톱 항목 (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Finder 및 Dock에서 가장 선명한 결과를 얻으려면 .icns 아이콘을 사용하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>아이콘을 비워 두면 Serial Studio 실행 파일 아이콘이 상속됩니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>애플리케이션 런처에 표시하려면 파일을 ~/.local/share/applications/ 아래에 배치하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Apple 아이콘 이미지 (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Windows 아이콘 (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>벡터 또는 래스터 이미지 (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="215"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>바로 가기를 생성하려면 Pro 라이선스가 필요합니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="220"/>
        <source>No output path was provided.</source>
        <translation>출력 경로가 제공되지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="263"/>
        <source>Failed to write shortcut file.</source>
        <translation>바로 가기 파일 쓰기 실패.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>%1의 기존 바로 가기를 교체할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>.app 번들 디렉터리 레이아웃을 생성할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>번들 런처를 쓸 수 없음: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>번들 런처를 실행 가능으로 표시할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Info.plist를 쓸 수 없음: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="272"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="141"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Windows 바로가기 생성기는 이 플랫폼에서 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="286"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="200"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Linux 바로가기 생성기는 이 플랫폼에서 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>COM을 초기화할 수 없음(.lnk 바로가기 작성에 필요).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) 실패(HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="154"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) 실패(HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="164"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>.lnk 파일 저장 실패(HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="186"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="155"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>macOS 바로가기 생성기는 이 플랫폼에서 사용할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>바로가기 경로를 쓰기용으로 열 수 없음: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Serial Studio 바로가기</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>바로가기를 실행 가능으로 표시할 수 없음.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="426"/>
        <source>System</source>
        <translation>시스템</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">자동으로 업데이트를 확인하시겠습니까?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">%1이(가) 자동으로 업데이트를 확인하도록 하시겠습니까? "정보" 대화 상자에서 언제든지 수동으로 업데이트를 확인할 수 있습니다</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="170"/>
        <source>Ok</source>
        <translation>확인</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="172"/>
        <source>Save</source>
        <translation>저장</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="174"/>
        <source>Save all</source>
        <translation>모두 저장</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="176"/>
        <source>Open</source>
        <translation>열기</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="178"/>
        <source>Yes</source>
        <translation>예</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="180"/>
        <source>Yes to all</source>
        <translation>모두 예</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="182"/>
        <source>No</source>
        <translation>아니요</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="184"/>
        <source>No to all</source>
        <translation>모두 아니요</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="186"/>
        <source>Abort</source>
        <translation>중단</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="188"/>
        <source>Retry</source>
        <translation>재시도</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="190"/>
        <source>Ignore</source>
        <translation>무시</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="192"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="194"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="196"/>
        <source>Discard</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="198"/>
        <source>Help</source>
        <translation>도움말</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="200"/>
        <source>Apply</source>
        <translation>적용</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="202"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="204"/>
        <source>Restore defaults</source>
        <translation>기본값 복원</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="236"/>
        <source>Select Workspace Location</source>
        <translation>작업 공간 위치 선택</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>프로토콜</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="72"/>
        <source>Serial Port</source>
        <translation>시리얼 포트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="97"/>
        <source>Baud Rate</source>
        <translation>보레이트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="201"/>
        <source>Parity</source>
        <translation>패리티</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="222"/>
        <source>Data Bits</source>
        <translation>데이터 비트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="245"/>
        <source>Stop Bits</source>
        <translation>정지 비트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="268"/>
        <source>Host</source>
        <translation>호스트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="278"/>
        <source>IP Address</source>
        <translation>IP 주소</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="292"/>
        <source>Port</source>
        <translation>포트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="301"/>
        <source>TCP Port</source>
        <translation>TCP 포트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="329"/>
        <source>Slave Address</source>
        <translation>슬레이브 주소</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="334"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="382"/>
        <source>Configure Register Groups…</source>
        <translation>레지스터 그룹 구성…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="392"/>
        <source>Import Register Map…</source>
        <translation>레지스터 맵 가져오기…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="407"/>
        <source>%1 group(s) configured</source>
        <translation>%1개 그룹 구성됨</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="408"/>
        <source>No groups configured</source>
        <translation>구성된 그룹 없음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="349"/>
        <source>Poll Interval (ms)</source>
        <translation>폴링 간격 (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="354"/>
        <source>Polling interval</source>
        <translation>폴링 간격</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Modbus 레지스터 그룹</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>여러 레지스터 그룹을 구성하여 다양한 레지스터 유형을 순차적으로 폴링합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>새 그룹 추가</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>레지스터 유형:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>시작 주소:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>레지스터 개수:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>그룹 추가</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>구성된 그룹</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>유형</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>시작</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>개수</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>액션</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>제거</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>구성된 그룹이 없습니다.
여러 레지스터 유형을 폴링하려면 위에서 그룹을 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>총 그룹 수: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>프로젝트 생성</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>모두 지우기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Modbus 레지스터 맵 미리보기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>파일: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>새 Serial Studio 프로젝트로 가져올 레지스터를 검토하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>레지스터</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>이름</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>주소</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>유형</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>데이터 타입</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>단위</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>파일에서 레지스터를 찾을 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>총 %2개 그룹의 레지스터 %1개</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>프로젝트 생성</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 발행자</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>브로커에 연결됨</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>연결되지 않음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>연결 테스트</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>현재 설정으로 브로커 탐색</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>먼저 게시 활성화</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>스크립트 편집</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>게시자 스크립트 편집 (Lua 또는 JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>CA 인증서 로드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>폴더에서 PEM 인증서 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>먼저 SSL/TLS 활성화</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">보간</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="234"/>
        <source>Interpolation: %1</source>
        <translation>보간: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">범례 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="256"/>
        <source>Show X Axis Label</source>
        <translation>X축 레이블 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="267"/>
        <source>Show Y Axis Label</source>
        <translation>Y축 레이블 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="279"/>
        <source>Show Crosshair</source>
        <translation>십자선 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="286"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="286"/>
        <source>Resume</source>
        <translation>재개</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="303"/>
        <source>Sweep / Trigger Mode</source>
        <translation>스윕 / 트리거 모드</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="312"/>
        <source>Trigger Settings</source>
        <translation>트리거 설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="336"/>
        <source>Reset View</source>
        <translation>보기 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="342"/>
        <source>Axis Range Settings</source>
        <translation>축 범위 설정</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">샘플</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>소켓 유형</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>원격 주소</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>로컬 포트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>자동 포트는 0 입력</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>원격 포트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="219"/>
        <source>Multicast</source>
        <translation>멀티캐스트</translation>
    </message>
</context>
<context>
    <name>NotificationLog</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="162"/>
        <source>Filter by channel…</source>
        <translation>채널별 필터링…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>모든 알림 지우기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(제목 없음)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>아직 알림이 없습니다</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">데이터셋 변환 및 출력 위젯 스크립트는 notifyInfo / notifyWarning / notifyCritical을 통해 여기에 이벤트를 게시할 수 있습니다.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>온라인 아이콘 검색</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>다운로드 실패: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>아이콘 검색 (예: temperature, arrow, play)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>검색</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>시작하려면 위에서 아이콘을 검색하세요</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>확인</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>출력 위젯은 Pro 라이선스가 필요합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>버튼</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>클릭 시 명령 전송</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>슬라이더</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>스케일된 숫자 값 전송</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>토글</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>On/Off 명령 전송</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>텍스트 필드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>임의의 명령 입력 및 전송</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>노브</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>설정값 입력용 회전 입력</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>출력 위젯을 구성할 수 있지만, Pro 라이선스가 있어야 대시보드에 표시됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>이 출력 위젯 복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>이 출력 위젯 삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>전송 함수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>.js 파일에서 전송 함수 가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>템플릿</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>사전 구축된 전송 함수 템플릿 선택</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>테스트</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>샘플 입력으로 전송 함수 테스트</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>실행 취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>다시 실행</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>잘라내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>붙여넣기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>문서 서식 지정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>선택 영역 서식 지정</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>페인터 위젯 오류</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>페인터 위젯 코드 편집기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>.js 파일에서 페인터 코드를 가져옵니다</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>템플릿</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>내장 페인터 템플릿 선택</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>포맷</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>페인터 코드 재포맷</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>테스트</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>시뮬레이션된 데이터셋 값으로 실시간 미리보기 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>미리보기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>잘라내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>붙여넣기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>실행 취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>다시 실행</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>문서 서식 지정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>선택 영역 서식 지정</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Painter 실시간 미리보기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>미리보기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>시뮬레이션된 데이터셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>런타임 정상</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>첫 번째 프레임 대기 중...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>콘솔</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>콘솔 지우기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">보간</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="231"/>
        <source>Interpolation: %1</source>
        <translation>보간: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="244"/>
        <source>Show Area Under Plot</source>
        <translation>플롯 아래 영역 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="262"/>
        <source>Show X Axis Label</source>
        <translation>X축 레이블 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="273"/>
        <source>Show Y Axis Label</source>
        <translation>Y축 레이블 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="285"/>
        <source>Show Crosshair</source>
        <translation>십자선 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="292"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="292"/>
        <source>Resume</source>
        <translation>재개</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="309"/>
        <source>Sweep / Trigger Mode</source>
        <translation>스윕 / 트리거 모드</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="318"/>
        <source>Trigger Settings</source>
        <translation>트리거 설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="342"/>
        <source>Reset View</source>
        <translation>보기 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="348"/>
        <source>Axis Range Settings</source>
        <translation>축 범위 설정</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="203"/>
        <source>Interpolate</source>
        <translation>보간</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="221"/>
        <source>Orbit Navigation</source>
        <translation>궤도 탐색</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="231"/>
        <source>Pan Navigation</source>
        <translation>패닝 탐색</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="242"/>
        <source>Orthogonal View</source>
        <translation>직교 보기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="248"/>
        <source>Top View</source>
        <translation>상단 보기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="254"/>
        <source>Left View</source>
        <translation>좌측 보기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="260"/>
        <source>Front View</source>
        <translation>정면 보기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="277"/>
        <source>Auto Center</source>
        <translation>자동 중심</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="293"/>
        <source>Anaglyph 3D</source>
        <translation>애너글리프 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="307"/>
        <source>Invert Eye Positions</source>
        <translation>눈 위치 반전</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="59"/>
        <source>None</source>
        <translation>없음</translation>
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
        <translation>선형</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1260"/>
        <source>Time</source>
        <translation>시간</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1283"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — 드래그하여 이동, 우클릭하여 지우기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1286"/>
        <source>Click to place cursor</source>
        <translation>클릭하여 커서 배치</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1288"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>클릭하여 두 번째 커서 배치 — 드래그하여 이동</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>웹사이트 방문</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>라이선스 구매</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>활성화</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>어시스턴트 — Pro 기능</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>어시스턴트는 Serial Studio Pro 기능입니다. 라이선스를 활성화하여 잠금 해제하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>활성화</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>모드</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>프로세스 실행</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>명명된 파이프</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>실행 파일</translation>
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
        <translation>찾아보기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>인수</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 value1 --arg2 value2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>작업 디렉터리</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(선택 사항) /working/directory</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>파이프 경로</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>실행 중인 프로세스 선택…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>자식 프로세스를 실행하여 stdout을 캡처하거나, 기존 프로세스가 작성한 명명된 파이프에 연결합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>명명된 파이프에 대해 알아보기</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="55"/>
        <source>Select Running Process</source>
        <translation>실행 중인 프로세스 선택</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="206"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>실행 중인 프로세스를 선택하여 명명된 파이프 경로 제안을 생성합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="212"/>
        <source>Filter Processes</source>
        <translation>프로세스 필터링</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="226"/>
        <source>Type to filter by name…</source>
        <translation>이름으로 필터링하려면 입력하세요…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="230"/>
        <source>Refresh</source>
        <translation>새로고침</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="238"/>
        <source>Running Processes</source>
        <translation>실행 중인 프로세스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="276"/>
        <source>Process</source>
        <translation>프로세스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="282"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="375"/>
        <source>No processes match the filter.</source>
        <translation>필터와 일치하는 프로세스가 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="376"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>실행 중인 프로세스를 찾을 수 없습니다.
새로고침을 클릭하여 목록을 업데이트하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="392"/>
        <source>%1 process(es)</source>
        <translation>%1개 프로세스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="396"/>
        <source>Select</source>
        <translation>선택</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="402"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="42"/>
        <source>modified</source>
        <translation>수정됨</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="322"/>
        <source>This project is password protected</source>
        <translation>이 프로젝트는 비밀번호로 보호되어 있습니다</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="323"/>
        <source>Editing is available in Project mode</source>
        <translation>프로젝트 모드에서 편집 가능</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="334"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>변경하려면 비밀번호를 입력하거나 다른 프로젝트를 여십시오.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="335"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>프로젝트를 불러오고 편집하려면 프로젝트 모드로 전환하십시오.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="357"/>
        <source>Unlock</source>
        <translation>잠금 해제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="358"/>
        <source>Switch to Project Mode</source>
        <translation>프로젝트 모드로 전환</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="377"/>
        <source>Open Other Project</source>
        <translation>다른 프로젝트 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="378"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="394"/>
        <source>Create New Project</source>
        <translation>새 프로젝트 생성</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>프로젝트 구조</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>검색</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>위로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>아래로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>선택 항목 복제(%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>선택 항목 삭제(%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>새로 만들기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>새 JSON 프로젝트 생성</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>기존 JSON 프로젝트 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>저장</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>현재 프로젝트 저장</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>다른 이름으로 저장</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>현재 프로젝트를 새 이름으로 저장</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">잠금 해제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>잠금</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">프로젝트 비밀번호로 프로젝트 편집기 잠금 해제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Protocol Buffers(.proto) 스키마에서 프로젝트 생성</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>비밀번호를 설정하고 프로젝트 편집기 잠금</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>장치 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>프로젝트에 새 데이터 소스(장치) 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>액션</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>프로젝트에 새 액션 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>출력</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>복원</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>현재 프로젝트의 최근 자동 스냅샷 복원</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>버튼이 있는 새 출력 제어 패널 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>슬라이더</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>출력 슬라이더 컨트롤 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>토글</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>출력 토글 컨트롤 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>노브</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>출력 노브 컨트롤 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>텍스트 필드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>출력 텍스트 필드 컨트롤 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>버튼</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>출력 버튼 컨트롤 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>데이터셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>일반 데이터셋 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>2D 플롯 데이터셋 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>FFT 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>고속 푸리에 변환 플롯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>게이지</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>숫자 데이터용 게이지 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>레벨 표시기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>수직 막대 레벨 표시기 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>나침반</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>방향 데이터를 위한 나침반 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>LED 표시기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>LED 스타일 상태 표시기 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>데이터셋 컨테이너 그룹 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>데이터셋 컨테이너</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>이미지</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>이미지/비디오 스트림 뷰어 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>이미지 뷰</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="458"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>사용자 정의 JavaScript 렌더링 Painter 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="459"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Painter 위젯은 Pro 라이선스가 필요합니다 — 추가 시 데이터 그리드로 대체됩니다</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="460"/>
        <source>Painter Widget</source>
        <translation>Painter 위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="472"/>
        <source>Table</source>
        <translation>테이블</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="475"/>
        <source>Add a data table view</source>
        <translation>데이터 테이블 뷰 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="477"/>
        <source>Data Grid</source>
        <translation>데이터 그리드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Multi-Plot</source>
        <translation>다중 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>여러 신호가 포함된 2D 플롯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="487"/>
        <source>Multiple Plot</source>
        <translation>다중 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="492"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="497"/>
        <source>3D Plot</source>
        <translation>3D 플롯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Add a 3D plot visualization</source>
        <translation>3D 플롯 시각화 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="507"/>
        <source>Accelerometer</source>
        <translation>가속도계</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>3축 가속도계 데이터를 위한 그룹 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="516"/>
        <source>Gyroscope</source>
        <translation>자이로스코프</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="517"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>3축 자이로스코프 데이터를 위한 그룹 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="522"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="527"/>
        <source>GPS Map</source>
        <translation>GPS 맵</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a map widget for GPS data</source>
        <translation>GPS 데이터를 위한 맵 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="539"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="543"/>
        <source>Assistant</source>
        <translation>어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="546"/>
        <source>Open the Assistant</source>
        <translation>어시스턴트 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="552"/>
        <source>Help Center</source>
        <translation>도움말 센터</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="556"/>
        <source>Open the Project Editor documentation</source>
        <translation>프로젝트 편집기 문서 열기</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>프로젝트 요약</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>이 프로젝트에서 Pro 기능이 감지되었습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>대체 위젯을 사용 중입니다. 전체 기능을 잠금 해제하려면 라이선스를 구매하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>프로젝트 제목:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>제목 없는 프로젝트</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">포인트:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>시간 범위:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="235"/>
        <source>Source</source>
        <translation>소스</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="236"/>
        <source>Sources</source>
        <translation>소스</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="241"/>
        <source>Group</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="242"/>
        <source>Groups</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="247"/>
        <source>Dataset</source>
        <translation>데이터셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="248"/>
        <source>Datasets</source>
        <translation>데이터셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Action</source>
        <translation>액션</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="254"/>
        <source>Actions</source>
        <translation>작업</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="342"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>블록을 더블 클릭하여 편집합니다. 아무 곳이나 우클릭하여 그룹, 데이터셋, 액션, 데이터 테이블 또는 장치를 추가합니다.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Protocol Buffers 파일 미리보기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Proto 파일: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>아래 메시지를 확인한 후 프로젝트를 생성합니다. 모든 메시지는 대시보드 그룹이 되며, 동일 유형 채널 블록은 MultiPlot으로, 혼합 유형 메시지는 DataGrid로 표시됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>필드 표시 대상</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>필드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>태그</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>필드 이름</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>유형</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>선택한 메시지에 필드가 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>총: %1개 메시지, %2개 필드</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>프로젝트 생성</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>오류 없음</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>브로커가 지원되지 않는 프로토콜 버전으로 인해 연결을 거부했습니다. 브로커의 MQTT 버전과 일치시킨 후 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>브로커가 클라이언트 ID를 거부했습니다. 형식이 잘못되었거나, 너무 길거나, 이미 사용 중일 수 있습니다. 재생성한 후 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>네트워크가 브로커에 도달했으나 브로커를 현재 사용할 수 없습니다. 상태를 확인하고 나중에 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>사용자 이름 또는 비밀번호가 올바르지 않거나 형식이 잘못되었습니다. 자격 증명을 다시 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>브로커가 권한 부족으로 인해 연결을 거부했습니다. 계정에 필요한 ACL이 있는지 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>네트워크 또는 전송 계층 문제로 인해 연결이 실패했습니다. 연결, 포트 및 TLS 구성을 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>클라이언트가 MQTT 프로토콜 위반을 감지하여 연결을 종료했습니다. 브로커와 클라이언트 호환성을 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>예기치 않은 오류가 발생했습니다. 자세한 내용은 브로커 로그 및 애플리케이션 콘솔을 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>MQTT 5 프로토콜 레벨 오류가 발생했습니다. 자세한 내용은 브로커의 이유 코드를 확인하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>지정되지 않은 MQTT 오류 (코드 %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="234"/>
        <source>Failed to load welcome text :(</source>
        <translation>환영 텍스트를 불러오지 못했습니다 :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="264"/>
        <source>Network error</source>
        <translation>네트워크 오류</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <location filename="../../src/Licensing/Trial.cpp" line="284"/>
        <location filename="../../src/Licensing/Trial.cpp" line="317"/>
        <source>Trial Activation Error</source>
        <translation>평가판 활성화 오류</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="281"/>
        <source>Invalid server response</source>
        <translation>잘못된 서버 응답</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="282"/>
        <source>The server returned malformed data: %1</source>
        <translation>서버가 잘못된 형식의 데이터를 반환했습니다: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="314"/>
        <source>Unexpected server response</source>
        <translation>예상치 못한 서버 응답</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="315"/>
        <source>The server response is missing required fields.</source>
        <translation>서버 응답에 필수 필드가 누락되었습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="168"/>
        <source>Console Output File Error</source>
        <translation>콘솔 출력 파일 오류</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="169"/>
        <source>Cannot open file for writing!</source>
        <translation>쓰기용 파일을 열 수 없습니다!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="805"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>잘못된 Bluetooth 어댑터!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="808"/>
        <source>Unsuported platform or operating system</source>
        <translation>지원되지 않는 플랫폼 또는 운영 체제</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="811"/>
        <source>Unsupported discovery method</source>
        <translation>지원되지 않는 검색 방법</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="814"/>
        <source>General I/O error</source>
        <translation>일반 I/O 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="275"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="255"/>
        <source>Frame Parser Disabled</source>
        <translation>프레임 파서 비활성화됨</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="276"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>소스 %1의 Lua 프레임 파서가 %2개 프레임 연속으로 시간 초과되어 Serial Studio의 응답성을 유지하기 위해 비활성화되었습니다.

가장 가능성 높은 원인: 스크립트 본문의 무한 루프 또는 매우 느린 작업. 스크립트를 수정하고 프로젝트를 다시 로드하여 파싱을 재활성화하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="321"/>
        <source>Lua Syntax Error</source>
        <translation>Lua 구문 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>The parser code contains an error:

%1</source>
        <translation>파서 코드에 오류가 있습니다:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="362"/>
        <source>Lua Runtime Error</source>
        <translation>Lua 런타임 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="363"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>파서 코드가 오류를 발생시켰습니다:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="384"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>Missing Parse Function</source>
        <translation>Parse 함수 누락</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="385"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>스크립트에 'parse' 함수가 정의되지 않았습니다.

코드에 다음이 포함되어 있는지 확인하십시오:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="447"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="533"/>
        <source>Parse Function Runtime Error</source>
        <translation>Parse 함수 런타임 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="448"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>parse 함수에 오류가 있습니다:

%1

함수 본문의 오류를 수정하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="256"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>소스 %1의 JavaScript 프레임 파서가 %2개 프레임 연속으로 시간 초과되어 Serial Studio의 응답성을 유지하기 위해 비활성화되었습니다.

가장 가능성 높은 원인: 스크립트 본문의 무한 루프 또는 매우 느린 작업. 스크립트를 수정하고 프로젝트를 다시 로드하여 파싱을 재활성화하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="418"/>
        <source>JavaScript Timed Out</source>
        <translation>JavaScript 시간 초과</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>파서 코드가 %1 ms 내에 평가를 완료하지 못하고 중단되었습니다.

가장 가능성 높은 원인: 스크립트 최상위 레벨의 무한 루프입니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>JavaScript 구문 오류</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>파서 코드의 %1번째 줄에 구문 오류가 있습니다:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>JavaScript Exception Occurred</source>
        <translation>JavaScript 예외 발생</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="454"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>파서 코드에서 다음 예외가 발생했습니다:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="480"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>스크립트에 'parse' 함수가 정의되지 않았습니다.

코드에 다음을 포함해야 합니다:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="534"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>parse 함수의 %1번째 줄에 오류가 있습니다:

%2

함수 본문의 오류를 수정하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="637"/>
        <source>Invalid Function Declaration</source>
        <translation>잘못된 함수 선언</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="638"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>호출 가능한 'parse' 내보내기를 찾을 수 없습니다.

다음 중 하나를 정의하세요:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="654"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>'parse' 함수는 최소 하나의 매개변수(프레임 페이로드)를 받아야 합니다.</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">유효한 'parse' 함수 선언을 찾을 수 없습니다.

예상 형식:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="653"/>
        <source>Invalid Function Parameter</source>
        <translation>잘못된 함수 매개변수</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">'parse' 함수는 최소 하나의 매개변수를 가져야 합니다.

예상 형식:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="618"/>
        <source>Deprecated Function Signature</source>
        <translation>더 이상 사용되지 않는 함수 서명</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="619"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>'parse' 함수가 이전 두 매개변수 형식을 사용합니다: parse(%1, %2)

이 형식은 더 이상 지원되지 않습니다. 새로운 단일 매개변수 형식으로 업데이트하십시오:
function parse(%1) { ... }

구분 기호 매개변수는 더 이상 필요하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="170"/>
        <source>Critical</source>
        <translation>심각</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="170"/>
        <source>Warning</source>
        <translation>경고</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="525"/>
        <source>Project file not found</source>
        <translation>프로젝트 파일을 찾을 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="526"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>이 바로 가기가 참조하는 프로젝트 파일을 찾을 수 없습니다:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="529"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>이 바로 가기를 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="533"/>
        <source>Delete Shortcut</source>
        <translation>바로 가기 삭제</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="535"/>
        <source>Quit</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1070"/>
        <source>Time (s)</source>
        <translation>시간 (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1132"/>
        <source>Freq: %1</source>
        <translation>주파수: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1136"/>
        <source>Time: −%1</source>
        <translation>시간: −%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="327"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>OpenAI API 키가 설정되지 않았습니다. 키 관리를 열어 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="199"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Anthropic API 키가 설정되지 않았습니다. 키 관리를 열어 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="282"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Gemini API 키가 설정되지 않았습니다. 키 관리를 열어 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="321"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>로컬 모델 서버 URL이 구성되지 않았습니다. 키 관리를 열어 설정하세요.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="141"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>DeepSeek API 키가 설정되지 않았습니다. 키 관리를 열어 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="164"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Mistral API 키가 설정되지 않았습니다. 키 관리를 열어 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="177"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>OpenRouter API 키가 설정되지 않았습니다. 키 관리를 열어 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="148"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Groq API 키가 설정되지 않았습니다. 키 관리를 열어 추가하세요.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="649"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>프레임 파서가 CPU 시간의 %1% 이상을 사용하고 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="651"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio가 애플리케이션 응답성을 유지하기 위해 프레임을 드롭하고 있습니다. 프레임 파서 스크립트를 단순화하거나 최적화하여 작업 부하를 줄이십시오.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="388"/>
        <source>Expected %1, got '%2'</source>
        <translation>%1이(가) 예상되었으나 '%2'을(를) 받았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="437"/>
        <source>Expected enum name after 'enum'</source>
        <translation>'enum' 뒤에 열거형 이름이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="451"/>
        <source>Expected oneof name</source>
        <translation>oneof 이름이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="479"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>필드 태그 '%1'이(가) 범위를 벗어났습니다 (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="497"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>map&lt;&gt;에 키 타입이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="505"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>map&lt;&gt;에 값 타입이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="513"/>
        <source>Expected map field name</source>
        <translation>맵 필드 이름이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="525"/>
        <source>Expected map field tag</source>
        <translation>맵 필드 태그가 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="557"/>
        <source>Expected field type, got '%1'</source>
        <translation>필드 타입이 필요하지만 '%1'을(를) 받았습니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="577"/>
        <source>Expected field name after type</source>
        <translation>타입 뒤에 필드 이름이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="587"/>
        <source>Expected field tag number</source>
        <translation>필드 태그 번호가 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>메시지 중첩이 너무 깊습니다 (제한 %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="640"/>
        <source>Expected message name</source>
        <translation>메시지 이름이 필요합니다</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="722"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>파일 범위에서 예기치 않은 토큰 '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="768"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>지원되지 않는 최상위 키워드 '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>자동(플랫폼 기본값)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>소프트웨어(대체)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="93"/>
        <source>Row %1</source>
        <translation>행 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="98"/>
        <source>[%1]</source>
        <translation>[%1]</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="112"/>
        <source>Frame %1</source>
        <translation>프레임 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="119"/>
        <source>Decoder</source>
        <translation>디코더</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="123"/>
        <source>Rows</source>
        <translation>행</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="124"/>
        <source>%1 row(s)</source>
        <translation>%1개 행</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append는 GZIP에서 지원되지 않습니다</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>읽기와 쓰기를 동시에 수행하도록 gzip을 여는 것은 지원되지 않습니다</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>gzip은 읽기 또는 쓰기 중 하나로만 열 수 있습니다. 어느 것입니까?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>파일을 gzopen()할 수 없음</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append는 QuaZIODevice에서 지원되지 않습니다</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite는 QuaZIODevice에서 지원되지 않습니다</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>ZIP/UNZIP API 오류 %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>PDF 보고서 생성</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>보고서 생성</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>실선</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>파선</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>점선</translation>
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
        <translation>%1 — 세션 보고서</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="279"/>
        <source>Session Report</source>
        <translation>세션 보고서</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="187"/>
        <source>Branding</source>
        <translation>브랜딩</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="193"/>
        <source>Page</source>
        <translation>페이지</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="199"/>
        <source>Sections</source>
        <translation>섹션</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="247"/>
        <source>Identity</source>
        <translation>식별 정보</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="261"/>
        <source>Company</source>
        <translation>회사</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="268"/>
        <source>e.g. Acme Test Systems</source>
        <translation>예: Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="272"/>
        <source>Document title</source>
        <translation>문서 제목</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="283"/>
        <source>Author</source>
        <translation>작성자</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="290"/>
        <source>Prepared by (optional)</source>
        <translation>작성자 (선택 사항)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="299"/>
        <source>Logo</source>
        <translation>로고</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="312"/>
        <source>File</source>
        <translation>파일</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="323"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG 또는 SVG (선택 사항)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="325"/>
        <source>Browse…</source>
        <translation>찾아보기…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="328"/>
        <source>Clear</source>
        <translation>지우기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="369"/>
        <source>Paper</source>
        <translation>용지</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="381"/>
        <source>Page size</source>
        <translation>페이지 크기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="509"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>플롯에 최소값, 최대값 및 평균값 주석 표시</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export HTML</source>
        <translation>HTML 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="497"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>표지 (로고, 문서 제목, 테스트 부제목)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="500"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>테스트 정보 (프로젝트, 타임스탬프, 분류 및 메모)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="503"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>측정 요약 (매개변수별 최소값, 최대값, 평균, 표준 편차)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="506"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>매개변수 추세 (숫자 매개변수별 시계열 차트)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="396"/>
        <source>Plot appearance</source>
        <translation>플롯 모양</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="410"/>
        <source>Line width</source>
        <translation>선 두께</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="442"/>
        <source>Line style</source>
        <translation>선 스타일</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="482"/>
        <source>Include</source>
        <translation>포함</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="532"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export PDF</source>
        <translation>PDF 내보내기</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>보고서 생성 중</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>작업 중…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>매개변수가 많은 세션의 경우 몇 초가 걸릴 수 있습니다. 보고서가 준비되면 창이 자동으로 닫힙니다.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Connection Lost</source>
        <translation>연결 끊김</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="42"/>
        <source>Device Unavailable</source>
        <translation>장치 사용 불가</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>The connection to your device was lost.</source>
        <translation>장치와의 연결이 끊어졌습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="97"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio가 장치에 연결할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>케이블, 전원을 확인하고 다른 애플리케이션이 장치를 점유하지 않았는지 확인하세요. 재연결을 시도하거나 다른 장치로 전환하거나 종료할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>장치가 연결되어 있고 전원이 켜져 있으며 다른 앱에서 사용 중이 아닌지 확인하십시오. 다시 시도하거나 다른 장치를 선택하거나 종료할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>다른 장치 선택</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">재구성</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>다시 시도</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>재연결</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>올바른 장치를 선택한 후 연결을 누르세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>I/O 인터페이스: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>연결</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="336"/>
        <source>Data Grids</source>
        <translation>데이터 그리드</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="339"/>
        <source>Multiple Data Plots</source>
        <translation>다중 데이터 플롯</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="342"/>
        <source>Accelerometers</source>
        <translation>가속도계</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="345"/>
        <source>Gyroscopes</source>
        <translation>자이로스코프</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="348"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="351"/>
        <source>FFT Plots</source>
        <translation>FFT 플롯</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="354"/>
        <source>LED Panels</source>
        <translation>LED 패널</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="357"/>
        <source>Data Plots</source>
        <translation>데이터 플롯</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="360"/>
        <source>Bars</source>
        <translation>바</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="363"/>
        <source>Gauges</source>
        <translation>게이지</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="366"/>
        <source>Terminal</source>
        <translation>터미널</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="369"/>
        <source>Clock</source>
        <translation>시계</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="372"/>
        <source>Stopwatch</source>
        <translation>스톱워치</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="375"/>
        <source>Compasses</source>
        <translation>나침반</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="378"/>
        <source>Meters</source>
        <translation>미터</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">온도계</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="381"/>
        <source>3D Plots</source>
        <translation>3D 플롯</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="385"/>
        <source>Image Views</source>
        <translation>이미지 뷰</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="388"/>
        <source>Output Panels</source>
        <translation>출력 패널</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="391"/>
        <source>Notifications</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="394"/>
        <source>Waterfalls</source>
        <translation>워터폴</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="397"/>
        <source>Painter Widgets</source>
        <translation>페인터 위젯</translation>
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
        <translation>시스템</translation>
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
        <translation>Shift-JIS</translation>
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
        <translation>세션 세부 정보</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>세부 정보를 보려면 세션을 선택하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>프로젝트:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>시작:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>종료:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(진행 중)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>프레임:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>메모</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>세션 메모 추가…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>완료된 세션의 메모는 읽기 전용입니다.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>새 태그…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>세션을 삭제하려면 세션 파일 잠금을 해제하세요</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>태그</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>추가</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>재생</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>CSV 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>보고서 생성</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>세션</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>검색</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>날짜</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>프레임</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>태그</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>세션을 찾을 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>열린 세션 파일이 없습니다.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="997"/>
        <source>Select logo image</source>
        <translation>로고 이미지 선택</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="999"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>이미지 (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="417"/>
        <source>Open Session File</source>
        <translation>세션 파일 열기</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="419"/>
        <source>Session files (*.db)</source>
        <translation>세션 파일 (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1197"/>
        <source>Cannot open session file</source>
        <translation>세션 파일을 열 수 없습니다</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="646"/>
        <source>Delete session from %1?</source>
        <translation>%1의 세션을 삭제하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="649"/>
        <source>Delete Session</source>
        <translation>세션 삭제</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1058"/>
        <source>No project data</source>
        <translation>프로젝트 데이터 없음</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="647"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>이 세션의 모든 측정값과 원시 데이터가 영구적으로 제거됩니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="476"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="485"/>
        <source>Lock Session File</source>
        <translation>세션 파일 잠금</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="477"/>
        <source>Choose a password to lock the session file:</source>
        <translation>세션 파일을 잠글 비밀번호를 선택하세요:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <source>Confirm the password:</source>
        <translation>비밀번호 확인:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="494"/>
        <source>Passwords do not match</source>
        <translation>비밀번호 불일치</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>입력한 두 비밀번호가 일치하지 않습니다. 세션 파일이 잠기지 않았습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="531"/>
        <source>Unlock Session File</source>
        <translation>세션 파일 잠금 해제</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="532"/>
        <source>Enter the session file password:</source>
        <translation>세션 파일 비밀번호를 입력하세요:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="542"/>
        <source>Incorrect password</source>
        <translation>잘못된 비밀번호</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="543"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>입력한 비밀번호가 세션 파일에 저장된 비밀번호와 일치하지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="636"/>
        <source>Session file locked</source>
        <translation>세션 파일 잠김</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="637"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>기록된 세션을 삭제하기 전에 세션 파일의 잠금을 해제하세요.</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">이 세션에는 내장된 프로젝트 파일이 없습니다 — 대시보드가 빠른 플롯 레이아웃으로 대체됩니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="771"/>
        <source>Export Session to CSV</source>
        <translation>세션을 CSV로 내보내기</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="771"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 파일 (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">내보내기 완료</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">세션 내보내기 완료:
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">내보내기 준비 중…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="969"/>
        <source>Done</source>
        <translation>완료</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="933"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="969"/>
        <source>Failed</source>
        <translation>실패</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="939"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="979"/>
        <source>Report Failed</source>
        <translation>보고서 실패</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="941"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="980"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>보고서를 생성할 수 없습니다. 출력 경로를 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="867"/>
        <source>Save PDF Report</source>
        <translation>PDF 보고서 저장</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="847"/>
        <source>Loading session data…</source>
        <translation>세션 데이터 로드 중…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="867"/>
        <source>Save HTML Report</source>
        <translation>HTML 보고서 저장</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="868"/>
        <source>PDF files (*.pdf)</source>
        <translation>PDF 파일 (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="868"/>
        <source>HTML files (*.html)</source>
        <translation>HTML 파일 (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1059"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>이 세션 파일에는 내장된 프로젝트가 포함되어 있지 않습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1068"/>
        <source>Invalid project data</source>
        <translation>잘못된 프로젝트 데이터</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1069"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>내장된 프로젝트 JSON이 손상되어 복원할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1079"/>
        <source>Restore Project</source>
        <translation>프로젝트 복원</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1079"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Serial Studio 프로젝트 (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1087"/>
        <source>Cannot write file</source>
        <translation>파일을 쓸 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1087"/>
        <source>Check file permissions and try again.</source>
        <translation>파일 권한을 확인하고 다시 시도하십시오.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="75"/>
        <source>Empty file path</source>
        <translation>파일 경로가 비어 있음</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="169"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="224"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="285"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="356"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="381"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="409"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="449"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="639"/>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="704"/>
        <source>Database not open</source>
        <translation>데이터베이스가 열려 있지 않음</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="262"/>
        <source>Database not open or empty label</source>
        <translation>데이터베이스가 열려 있지 않거나 레이블이 비어 있음</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="330"/>
        <source>Invalid label</source>
        <translation>잘못된 레이블</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="598"/>
        <source>Cancelled</source>
        <translation>취소됨</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="711"/>
        <source>Could not load session data</source>
        <translation>세션 데이터를 불러올 수 없음</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="212"/>
        <source>Assembling report…</source>
        <translation>보고서 생성 중…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="221"/>
        <source>Writing output…</source>
        <translation>출력 작성 중…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="293"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="357"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="731"/>
        <source>Session Report</source>
        <translation>세션 보고서</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="360"/>
        <source>Untitled project</source>
        <translation>제목 없는 프로젝트</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="368"/>
        <source>Prepared by</source>
        <translation>작성자</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="371"/>
        <source>Generated on %1</source>
        <translation>%1에 생성됨</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="394"/>
        <source>Test ID</source>
        <translation>테스트 ID</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="396"/>
        <source>Duration</source>
        <translation>지속 시간</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="398"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="515"/>
        <source>Samples</source>
        <translation>샘플</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="400"/>
        <source>Parameters</source>
        <translation>매개변수</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="402"/>
        <source>Started</source>
        <translation>시작됨</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="404"/>
        <source>Ended</source>
        <translation>종료됨</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="440"/>
        <source>Project</source>
        <translation>프로젝트</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="442"/>
        <source>Test identifier</source>
        <translation>테스트 식별자</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="443"/>
        <source>Start time</source>
        <translation>시작 시간</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="444"/>
        <source>End time</source>
        <translation>종료 시간</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="445"/>
        <source>Total duration</source>
        <translation>총 지속 시간</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Samples acquired</source>
        <translation>획득한 샘플</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="447"/>
        <source>Parameters logged</source>
        <translation>기록된 매개변수</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="465"/>
        <source>Classification</source>
        <translation>분류</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="473"/>
        <source>Notes</source>
        <translation>비고</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="481"/>
        <source>Test Information</source>
        <translation>테스트 정보</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="504"/>
        <source>Parameter</source>
        <translation>매개변수</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="507"/>
        <source>Units</source>
        <translation>단위</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="516"/>
        <source>Minimum</source>
        <translation>최소값</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="517"/>
        <source>Maximum</source>
        <translation>최대값</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="518"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="680"/>
        <source>Mean</source>
        <translation>평균</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="519"/>
        <source>Std. Deviation</source>
        <translation>표준 편차</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="565"/>
        <source>Measurement Summary</source>
        <translation>측정 요약</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="566"/>
        <source>click a column to sort</source>
        <translation>열을 클릭하여 정렬</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="592"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%2초 동안 %1개 샘플</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="611"/>
        <source>Combined Parameter View</source>
        <translation>통합 파라미터 보기</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="612"/>
        <source>click legend items to toggle signals</source>
        <translation>범례 항목을 클릭하여 신호 전환</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="620"/>
        <source>Parameter Trends</source>
        <translation>파라미터 추세</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="678"/>
        <source>Min</source>
        <translation>최소값</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="679"/>
        <source>Max</source>
        <translation>최대값</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="758"/>
        <source>Page %1 of %2</source>
        <translation>%2 중 %1 페이지</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="828"/>
        <source>Loading rendering engine…</source>
        <translation>렌더링 엔진 로드 중…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="850"/>
        <source>Rendering charts…</source>
        <translation>차트 렌더링 중…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="895"/>
        <source>Generating PDF…</source>
        <translation>PDF 생성 중…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="259"/>
        <source>Open Session File</source>
        <translation>세션 파일 열기</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Session files (*.db)</source>
        <translation>세션 파일 (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="337"/>
        <source>Device Connection Active</source>
        <translation>장치 연결 활성화됨</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="338"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>이 기능을 사용하려면 장치 연결을 해제해야 합니다. 계속하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="388"/>
        <location filename="../../src/Sessions/Player.cpp" line="470"/>
        <source>Cannot open session file</source>
        <translation>세션 파일을 열 수 없음</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="389"/>
        <source>Unknown error</source>
        <translation>알 수 없는 오류</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="407"/>
        <source>No project data</source>
        <translation>프로젝트 데이터 없음</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="408"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>이 세션에는 내장된 프로젝트 파일이 없습니다 — 대시보드가 빠른 플롯 레이아웃으로 대체됩니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="471"/>
        <source>Check file permissions and try again.</source>
        <translation>파일 권한을 확인하고 다시 시도하십시오.</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">세션을 찾을 수 없음</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">이 파일에는 녹화된 세션이 없습니다.</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">세션에 열이 없음</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">선택한 세션에 열 정의가 없습니다.</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">세션에 판독값이 없음</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">선택한 세션에 프레임이 없습니다.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>빈 파일 경로</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>이 파일에는 녹화된 세션이 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>취소됨</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>선택한 세션에 열 정의가 없습니다.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>선택한 세션에 프레임이 없습니다.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>환경설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>일반</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="159"/>
        <source>Language</source>
        <translation>언어</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="175"/>
        <source>Theme</source>
        <translation>테마</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="210"/>
        <source>Workspace Folder</source>
        <translation>작업 공간 폴더</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Automatically Check for Updates</source>
        <translation>자동으로 업데이트 확인</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Dashboard</source>
        <translation>대시보드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="386"/>
        <source>Export…</source>
        <translation>내보내기…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="435"/>
        <source>Data Plotting</source>
        <translation>데이터 플로팅</translation>
    </message>
    <message>
        <source>Point Count</source>
        <translation type="vanished">포인트 수</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="500"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>UI 새로고침 레이트 (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="737"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>작업 표시줄 버튼 항상 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="624"/>
        <source>Show Actions Panel</source>
        <translation>작업 패널 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>API 서버 활성화 (포트 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="849"/>
        <source>Console</source>
        <translation>콘솔</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="144"/>
        <source>Appearance</source>
        <translation>모양</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="194"/>
        <source>Files &amp; Updates</source>
        <translation>파일 및 업데이트</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="255"/>
        <source>Advanced</source>
        <translation>고급</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="338"/>
        <source>Allow External API Connections</source>
        <translation>외부 API 연결 허용</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="303"/>
        <source>Auto-Hide Toolbar</source>
        <translation>도구 모음 자동 숨기기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Taskbar</source>
        <translation>작업 표시줄</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="270"/>
        <source>Rendering Backend</source>
        <translation>렌더링 백엔드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>API Access Token</source>
        <translation>API 액세스 토큰</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="450"/>
        <source>Time Range</source>
        <translation>시간 범위</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Small</source>
        <translation>작게</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Normal</source>
        <translation>보통</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Large</source>
        <translation>크게</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Extra Large</source>
        <translation>매우 크게</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Custom</source>
        <translation>사용자 지정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="609"/>
        <source>Layout</source>
        <translation>레이아웃</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="647"/>
        <source>Video Export</source>
        <translation>비디오 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="665"/>
        <source>Save Videos by Default</source>
        <translation>기본적으로 비디오 저장</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Behavior</source>
        <translation>동작</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="752"/>
        <source>Show Search Field</source>
        <translation>검색 필드 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="767"/>
        <source>Auto-hide Taskbar</source>
        <translation>작업 표시줄 자동 숨기기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="785"/>
        <source>Hide Delay (ms)</source>
        <translation>숨기기 지연 시간 (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="809"/>
        <source>Pinned Buttons</source>
        <translation>고정된 버튼</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="827"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>작업 표시줄에서 고정된 버튼을 드래그하여 순서를 변경합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="848"/>
        <source>Settings</source>
        <translation>설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="851"/>
        <source>Clock</source>
        <translation>시계</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="852"/>
        <source>Stopwatch</source>
        <translation>스톱워치</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="853"/>
        <source>Pause / Resume</source>
        <translation>일시정지 / 재개</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="854"/>
        <source>File Transmission</source>
        <translation>파일 전송</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="855"/>
        <source>AI Assistant</source>
        <translation>AI 어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="984"/>
        <source>Display</source>
        <translation>디스플레이</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="999"/>
        <source>Display Mode</source>
        <translation>디스플레이 모드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="537"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1012"/>
        <source>Font Family</source>
        <translation>글꼴 패밀리</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="86"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="850"/>
        <source>Notifications</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="384"/>
        <source>Export Protobuf File</source>
        <translation>Protobuf 파일 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="522"/>
        <source>Dashboard Font</source>
        <translation>대시보드 글꼴</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="552"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1027"/>
        <source>Font Size</source>
        <translation>글꼴 크기</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">이미지 내보내기</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">기본적으로 이미지 저장</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1044"/>
        <source>Show Timestamps</source>
        <translation>타임스탬프 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1063"/>
        <source>Data Transmission</source>
        <translation>데이터 전송</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1078"/>
        <source>Line Ending</source>
        <translation>줄 끝</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1091"/>
        <source>Input Mode</source>
        <translation>입력 모드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1104"/>
        <source>Text Encoding</source>
        <translation>텍스트 인코딩</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1117"/>
        <source>Checksum</source>
        <translation>체크섬</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1130"/>
        <source>Echo Sent Data</source>
        <translation>전송 데이터 에코</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1149"/>
        <source>Escape Codes</source>
        <translation>이스케이프 코드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>VT100 Emulation</source>
        <translation>VT100 에뮬레이션</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1183"/>
        <source>ANSI Colors</source>
        <translation>ANSI 색상</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1241"/>
        <source>Delivery</source>
        <translation>전달</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1256"/>
        <source>System Notifications</source>
        <translation>시스템 알림</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1277"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Serial Studio가 포그라운드 창이 아닐 때 경고/치명적 이벤트를 OS 데스크톱 알림으로 표시합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1287"/>
        <source>Application Logs</source>
        <translation>애플리케이션 로그</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1302"/>
        <source>Route Warnings to Notifications</source>
        <translation>경고를 알림으로 라우팅</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1323"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>기본적으로 비활성화 — QT와 QML은 경고를 자주 발생시키며 이 옵션을 활성화하면 실제 알람이 묻힐 수 있습니다. 중요 메시지는 이 설정과 관계없이 항상 전달됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1342"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1385"/>
        <source>Apply</source>
        <translation>적용</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>장치 설정</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>API 서버 활성 (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>API 서버 준비됨</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>API 서버 꺼짐</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>프레임 파싱</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>콘솔 전용 (파싱 없음)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>빠른 플롯 (쉼표로 구분된 값)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>프로젝트 파일을 통한 파싱</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>프로젝트 파일 변경 (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>프로젝트 파일 선택</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>데이터 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>CSV 스프레드시트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>세션 기록</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>MDF4 기록</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>콘솔 로그</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">CSV 파일</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">세션 데이터베이스</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">MDF4 파일</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">콘솔 덤프</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">CSV 파일 생성</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">MDF4 파일 생성</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">세션 로그 생성</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">콘솔 데이터 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>I/O 인터페이스: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>다중 장치 프로젝트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>이 프로젝트는 %1개의 독립 장치에서 데이터를 스트리밍합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>각 장치는 고유한 연결 설정을 가집니다. 프로젝트 편집기의 소스 탭에서 구성하십시오.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>프로젝트 편집기 열기</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">새 바로가기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="95"/>
        <source>Choose an Icon</source>
        <translation>아이콘 선택</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">바로가기 저장</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="24"/>
        <source>New Deployment</source>
        <translation>새 배포</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="108"/>
        <source>Save Deployment</source>
        <translation>배포 저장</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="148"/>
        <source>General</source>
        <translation>일반</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="154"/>
        <source>Taskbar</source>
        <translation>작업 표시줄</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="160"/>
        <source>Logging</source>
        <translation>로깅</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="217"/>
        <source>Identity</source>
        <translation>식별 정보</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="273"/>
        <source>Click to choose an icon</source>
        <translation>아이콘을 선택하려면 클릭</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="282"/>
        <source>Name:</source>
        <translation>이름:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="291"/>
        <source>Deployment Name</source>
        <translation>배포 이름</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="382"/>
        <source>Theme</source>
        <translation>테마</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="392"/>
        <source>Same as Serial Studio</source>
        <translation>Serial Studio와 동일</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="414"/>
        <source>Actions Panel</source>
        <translation>작업 패널</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="425"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="634"/>
        <source>File Transmission</source>
        <translation>파일 전송</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="441"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>이 배포를 더블 클릭하면 이 프로젝트의 실시간 대시보드로 바로 이동합니다. 툴바나 설정 창 없이 데이터만 표시되며, 장치 연결이 끊어지는 즉시 Serial Studio가 종료됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="487"/>
        <source>Visibility</source>
        <translation>표시 여부</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="502"/>
        <source>Mode</source>
        <translation>모드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="511"/>
        <source>Always shown</source>
        <translation>항상 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="512"/>
        <source>Auto-hide</source>
        <translation>자동 숨기기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="513"/>
        <source>Hidden</source>
        <translation>숨김</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="528"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>작업 표시줄을 숨기면 창 최소화/최대화/닫기 버튼이 제거되고 자동 레이아웃이 강제 적용되어 대시보드가 항상 사용 가능한 영역을 채웁니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>사용자가 커서를 화면 하단 가장자리 근처로 이동하면 작업 표시줄이 슬라이드되어 나타납니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="534"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>작업 표시줄이 대시보드 하단에 항상 표시됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="547"/>
        <source>Pinned Buttons</source>
        <translation>고정된 버튼</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="564"/>
        <source>Console</source>
        <translation>콘솔</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="578"/>
        <source>Notifications</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="592"/>
        <source>Clock</source>
        <translation>시계</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="606"/>
        <source>Stopwatch</source>
        <translation>스톱워치</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="620"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="745"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>녹화는 Serial Studio 작업 공간 폴더에 저장됩니다</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">바로가기 이름</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="300"/>
        <source>Change Icon…</source>
        <translation>아이콘 변경…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="317"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="335"/>
        <source>Project</source>
        <translation>프로젝트</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="345"/>
        <source>Choose a project file to begin</source>
        <translation>시작할 프로젝트 파일 선택</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="367"/>
        <source>Behavior</source>
        <translation>동작</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="404"/>
        <source>Fullscreen</source>
        <translation>전체 화면</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">이 바로 가기를 더블 클릭하면 이 프로젝트의 실시간 대시보드로 바로 이동합니다. 툴바나 설정 창 없이 데이터만 표시되며, 장치 연결이 끊어지는 즉시 Serial Studio가 종료됩니다.</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">프로젝트 포함</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">이 바로가기를 더블클릭하면 해당 프로젝트의 실시간 대시보드로 바로 이동합니다. 툴바나 설정 패널 없이 데이터만 표시되며, 장치 연결이 해제되는 즉시 Serial Studio가 종료됩니다.

프로젝트 포함을 활성화하면 프로젝트가 바로가기에 내장되어 원본 파일이 이동되거나 삭제되어도 계속 작동합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="684"/>
        <source>Recorders</source>
        <translation>레코더</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="699"/>
        <source>CSV File</source>
        <translation>CSV 파일</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="709"/>
        <source>MDF4 File</source>
        <translation>MDF4 파일</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="719"/>
        <source>Session Database</source>
        <translation>세션 데이터베이스</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="729"/>
        <source>Console Log</source>
        <translation>콘솔 로그</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">녹화는 각 모듈의 기본 위치에 저장됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="774"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="783"/>
        <source>Save</source>
        <translation>저장</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="110"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="242"/>
        <source>Undo</source>
        <translation>실행 취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="117"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="253"/>
        <source>Redo</source>
        <translation>다시 실행</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="272"/>
        <source>Cut</source>
        <translation>잘라내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="131"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="282"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="292"/>
        <source>Paste</source>
        <translation>붙여넣기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="143"/>
        <source>Select All</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="153"/>
        <source>Format Document</source>
        <translation>문서 서식 지정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="160"/>
        <source>Format Selection</source>
        <translation>선택 영역 서식 지정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="227"/>
        <source>Reset to the default parsing script</source>
        <translation>기본 파싱 스크립트로 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="232"/>
        <source>Open</source>
        <translation>열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="237"/>
        <source>Import a script file for data parsing</source>
        <translation>데이터 파싱을 위한 스크립트 파일 가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="315"/>
        <source>Open help documentation for data parsing</source>
        <translation>데이터 파싱 도움말 문서 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Language:</source>
        <translation>언어:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="378"/>
        <source>Select Template…</source>
        <translation>템플릿 선택…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Undo the last code edit</source>
        <translation>마지막 코드 편집 실행 취소</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="259"/>
        <source>Redo the previously undone edit</source>
        <translation>이전에 실행 취소한 편집 다시 실행</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="277"/>
        <source>Cut selected code to clipboard</source>
        <translation>선택한 코드를 클립보드로 잘라내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="287"/>
        <source>Copy selected code to clipboard</source>
        <translation>선택한 코드를 클립보드에 복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="296"/>
        <source>Paste code from clipboard</source>
        <translation>클립보드에서 코드 붙여넣기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="310"/>
        <source>Help</source>
        <translation>도움말</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="387"/>
        <source>Test With Sample Data</source>
        <translation>샘플 데이터로 테스트</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="394"/>
        <source>Evaluate</source>
        <translation>평가</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>복제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>이 데이터 소스의 사본 생성</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>이 데이터 소스 제거</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>기본 데이터 소스는 제거할 수 없습니다</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>세션 플레이어</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>세션 로드 중…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="276"/>
        <source>Workspaces</source>
        <translation>작업 공간</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="406"/>
        <source>Actions</source>
        <translation>작업</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="429"/>
        <source>No Actions Available</source>
        <translation>사용 가능한 작업 없음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="459"/>
        <source>Plugins</source>
        <translation>플러그인</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="507"/>
        <source>No Plugins Installed</source>
        <translation>설치된 플러그인 없음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="99"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="543"/>
        <source>Auto Layout</source>
        <translation>자동 레이아웃</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="107"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="555"/>
        <source>Full Screen</source>
        <translation>전체 화면</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="113"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="568"/>
        <source>Add External Window</source>
        <translation>외부 창 추가</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="155"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="853"/>
        <source>Help Center</source>
        <translation>도움말 센터</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="684"/>
        <source>Tools</source>
        <translation>도구</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="813"/>
        <source>No Tools Available</source>
        <translation>사용 가능한 도구 없음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="881"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Quit</source>
        <translation>종료</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="939"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Hide</source>
        <translation>숨기기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>New Workspace…</source>
        <translation>새 작업 공간…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Clock</source>
        <translation>시계</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="141"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="764"/>
        <source>Stopwatch</source>
        <translation>스톱워치</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="781"/>
        <source>Sessions</source>
        <translation>세션</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="168"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="790"/>
        <source>File Transmission</source>
        <translation>파일 전송</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="175"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>AI Assistant</source>
        <translation>AI 어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="343"/>
        <source>Show "%1"</source>
        <translation>"%1" 표시</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="348"/>
        <source>Show All Hidden Workspaces</source>
        <translation>숨겨진 모든 작업 공간 표시</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="372"/>
        <source>No Workspaces Available</source>
        <translation>사용 가능한 작업 공간 없음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="497"/>
        <source>Manage Plugins…</source>
        <translation>플러그인 관리…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="588"/>
        <source>Export</source>
        <translation>내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="619"/>
        <source>CSV File</source>
        <translation>CSV 파일</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="625"/>
        <source>MDF4 File</source>
        <translation>MDF4 파일</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="631"/>
        <source>Console Transcript</source>
        <translation>콘솔 기록</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="640"/>
        <source>Session Database</source>
        <translation>세션 데이터베이스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="654"/>
        <source>No Export Formats Available</source>
        <translation>사용 가능한 내보내기 형식 없음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="119"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="740"/>
        <source>Console</source>
        <translation>콘솔</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="125"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="749"/>
        <source>Notifications</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="149"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Preferences</source>
        <translation>환경설정</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="928"/>
        <source>Edit…</source>
        <translation>편집…</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="874"/>
        <source>Resume</source>
        <translation>재개</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Disconnect</source>
        <translation>연결 해제</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="195"/>
        <source>Stop</source>
        <translation>중지</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="195"/>
        <source>Start</source>
        <translation>시작</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="205"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>Lap</source>
        <translation>랩</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="221"/>
        <source>Reset</source>
        <translation>재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="271"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Total</source>
        <translation>합계</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="380"/>
        <source>No laps recorded</source>
        <translation>기록된 랩 없음</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="388"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>스톱워치 실행 중 랩 버튼을 누르세요</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>사용 가능한 데이터 없음</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>데이터셋 값</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>검색</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>데이터셋</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>단위</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(가상)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>액세스 코드 %1을 클립보드에 복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>데이터셋 액세스 코드 복사됨</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>이 프로젝트에 정의된 데이터셋이 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>검색 조건과 일치하는 데이터셋이 없습니다.</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>매개변수</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>값</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(사용자 지정 아이콘)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>자동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="806"/>
        <source>No</source>
        <translation>아니요</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="806"/>
        <source>Yes</source>
        <translation>예</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>시작 메뉴</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>메뉴</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>검색…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>설정</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>콘솔</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>시계</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>스톱워치</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>AI 어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>재개</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: %1에 연결됨</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: 연결되지 않음</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 발행자</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>상태:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>연결됨</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>연결 해제됨</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1000"/>
        <source>Broker:</source>
        <translation>브로커:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Mode:</source>
        <translation>모드:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1026"/>
        <source>Messages sent:</source>
        <translation>전송된 메시지:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>MQTT 설정 열기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>파일 전송</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">위젯 검색…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">작업 공간에서 제거</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>지우기</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>장치로 데이터 전송</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>타임스탬프 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>에코</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>VT-100 에뮬레이션</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>ANSI 색상</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>표시: %1</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>승인 대기 중</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>완료</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>오류</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>거부됨</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>차단됨</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>실행 중</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>승인</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>거부</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>인수</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>복사</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>모두 복사</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>모두 선택</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>결과</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>프로젝트 편집기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>프로젝트 편집기를 열어 JSON 레이아웃을 생성하거나 수정합니다</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>CSV 파일을 실시간 센서 데이터처럼 재생</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>환경설정</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>애플리케이션 설정 및 환경설정 열기</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>CSV 열기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>MDF4 열기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>세션</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>기록된 세션 탐색, 재생 및 내보내기</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">단축키</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">현재 프로젝트에 대한 운영자 단축키 생성</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>확장 기능</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>확장 기능 탐색 및 설치</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">MQTT 연결 구성 (게시 또는 구독)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>배포</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">현재 프로젝트에 대한 운영자 배포 빌드</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>시리얼 포트(UART) 통신 선택</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>오디오</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>오디오 입력 장치 선택 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Raw USB 통신 선택 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>네트워크</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>TCP/UDP 네트워크 통신 선택</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>MODBUS 통신 선택 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>HID 장치 통신 선택 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Bluetooth Low Energy 통신 선택</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>CAN Bus 통신 선택 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>프로세스</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>프로세스 파이프 통신 선택 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>예제</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>예제 프로젝트 탐색</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>도움말 센터</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>문서, FAQ 및 위키 탐색</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>DeepWiki에서 자세한 문서 보기 및 질문하기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>구성된 장치에 연결하거나 연결 해제</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>정보</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>프로젝트 열기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>기존 JSON 프로젝트 열기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>MDF4 파일을 실시간 센서 데이터처럼 재생 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>어시스턴트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>AI와 대화하여 프로젝트를 구축하고 편집</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>현재 프로젝트에 대한 운영자 앱 빌드</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>애플리케이션 정보 및 라이선스 세부 정보 표시</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI 위키 및 채팅</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>라이선스 관리 및 Serial Studio Pro 활성화</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>연결 해제</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>연결</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">장치 또는 MQTT 브로커에 연결하거나 연결 해제</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>활성화</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>트리거 설정</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">각 스윕을 트리거 이벤트에 정렬하여 파형을 정지 상태로 유지합니다.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">오실로스코프의 Auto 버튼처럼 반복되는 신호를 고정합니다. 각 스윕이 파형의 동일한 지점에서 시작하므로 스크롤되지 않고 정지 상태를 유지합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Trigger</source>
        <translation>트리거</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">모드:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="96"/>
        <source>Mode</source>
        <translation>모드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="121"/>
        <source>Auto</source>
        <translation>자동</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="121"/>
        <source>Normal</source>
        <translation>보통</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="121"/>
        <source>Single</source>
        <translation>단일</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="144"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto는 레벨을 교차하지 않을 때 자유 실행됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="145"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal은 교차를 기다립니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="146"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single은 한 번의 스윕을 캡처한 후 중지합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="224"/>
        <source>Slope:</source>
        <translation>슬로프:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="256"/>
        <source>Trigger on a downward crossing</source>
        <translation>하강 교차 시 트리거</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="302"/>
        <source>Timebase:</source>
        <translation>타임베이스:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="367"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>타임베이스를 비워 두면 플롯의 시간 범위를 사용합니다. 빠른 신호를 확대하려면 낮추십시오. Holdoff는 각 트리거 후 잠시 동안 새 트리거를 무시합니다.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">신호:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="216"/>
        <source>Value to cross</source>
        <translation>교차 값</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">에지:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="237"/>
        <source>Rising</source>
        <translation>상승</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Trigger on an upward crossing</source>
        <translation>상승 교차 시 트리거</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="252"/>
        <source>Falling</source>
        <translation>하강</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">신호가 선택한 방향으로 레벨을 교차할 때마다 새 스윕이 시작됩니다. Auto는 교차가 발견되지 않을 때도 자유 실행됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="275"/>
        <source>Timing</source>
        <translation>타이밍</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">타임베이스 (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="315"/>
        <source>Match time range</source>
        <translation>시간 범위 일치</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="327"/>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="354"/>
        <source>ms</source>
        <translation>ms</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="333"/>
        <source>Holdoff:</source>
        <translation>홀드오프:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="346"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">타임베이스는 한 스윕이 표시하는 시간을 설정합니다. 비워 두면 플롯의 시간 범위를 사용합니다. 빠른 신호를 확대하려면 낮추십시오. Holdoff는 각 트리거 후 잠시 동안 새 트리거를 무시합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="382"/>
        <source>Capture Next</source>
        <translation>다음 캡처</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="384"/>
        <source>Arm for one more single-shot capture</source>
        <translation>단일 샷 캡처 1회 재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="203"/>
        <source>Level:</source>
        <translation>레벨:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">트리거 레벨</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">홀드오프 (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">홀드오프 시간</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="183"/>
        <source>Source:</source>
        <translation>소스:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">재설정</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="397"/>
        <source>Close</source>
        <translation>닫기</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>COM 포트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="81"/>
        <source>Baud Rate</source>
        <translation>보레이트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="174"/>
        <source>Data Bits</source>
        <translation>데이터 비트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="195"/>
        <source>Parity</source>
        <translation>패리티</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="216"/>
        <source>Stop Bits</source>
        <translation>스톱 비트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="237"/>
        <source>Flow Control</source>
        <translation>흐름 제어</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="269"/>
        <source>Auto Reconnect</source>
        <translation>자동 재연결</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="287"/>
        <source>Send DTR Signal</source>
        <translation>DTR 신호 전송</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1181"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1916"/>
        <source>Console</source>
        <translation>콘솔</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1268"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1928"/>
        <source>Notifications</source>
        <translation>알림</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1354"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1940"/>
        <source>Clock</source>
        <translation>시계</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1439"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1951"/>
        <source>Stopwatch</source>
        <translation>스톱워치</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2004"/>
        <location filename="../../src/UI/Dashboard.cpp" line="2020"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (대체)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2046"/>
        <source>LED Panel (%1)</source>
        <translation>LED 패널 (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="148"/>
        <source>Invalid</source>
        <translation>유효하지 않음</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1098"/>
        <source>Select Background Image</source>
        <translation>배경 이미지 선택</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1100"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>이미지 (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>USB 장치</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>전송 모드</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>벌크 스트림</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>고급 (벌크 + 제어)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>동기식</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>벌크, 제어 또는 동기식 전송을 사용하여 USB 장치에 연결합니다. 데이터 로거, 커스텀 펌웨어 장치 및 USB 계측기에 적합합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>USB 사양 (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>IN 엔드포인트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>OUT 엔드포인트</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>최대 패킷 크기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>제어 전송 활성화됨</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>잘못된 제어 요청을 전송하면 연결된 하드웨어가 손상되거나 충돌할 수 있습니다. 주의하여 사용하십시오.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>USB 제어 전송에 대해 알아보기</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>패킷 크기는 엔드포인트에서 보고하는 최대 전송 크기와 일치해야 합니다. 일반적인 값: 192 B (FS 오디오), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>지금 업데이트를 다운로드하시겠습니까?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>지금 업데이트를 다운로드하시겠습니까?&lt;br /&gt;필수 업데이트이므로 지금 종료하면 애플리케이션이 닫힙니다.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;변경 로그:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>%2 버전 %1이 출시되었습니다!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>현재 사용 가능한 업데이트가 없습니다</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>축하합니다! %1의 최신 버전을 실행 중입니다</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="166"/>
        <source>Add Register</source>
        <translation>레지스터 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add register</source>
        <translation>레지스터 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="176"/>
        <source>Insert Constant</source>
        <translation>상수 삽입</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert constant</source>
        <translation>상수 삽입</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="186"/>
        <source>Import</source>
        <translation>가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import registers from CSV</source>
        <translation>CSV에서 레지스터 가져오기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="196"/>
        <source>Export</source>
        <translation>내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export registers to CSV</source>
        <translation>레지스터를 CSV로 내보내기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="211"/>
        <source>Rename</source>
        <translation>이름 변경</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename table</source>
        <translation>테이블 이름 변경</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="221"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete table</source>
        <translation>테이블 삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="238"/>
        <source>Help</source>
        <translation>도움말</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="243"/>
        <source>Open help documentation for shared memory</source>
        <translation>공유 메모리 도움말 문서 열기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="283"/>
        <source>Permissions</source>
        <translation>권한</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="284"/>
        <source>Register Name</source>
        <translation>레지스터 이름</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="285"/>
        <source>Default Value</source>
        <translation>기본값</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read-Only</source>
        <translation>읽기 전용</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read/Write</source>
        <translation>읽기/쓰기</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="462"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>액세스 코드 %1 클립보드에 복사</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="495"/>
        <source>Delete register</source>
        <translation>레지스터 삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="512"/>
        <source>No registers.</source>
        <translation>레지스터가 없습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="562"/>
        <source>Register access code copied</source>
        <translation>레지스터 액세스 코드 복사됨</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="232"/>
        <source>Show Colorbar</source>
        <translation>색상 막대 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="245"/>
        <source>Show Axes &amp; Grid</source>
        <translation>축 및 격자 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="258"/>
        <source>Show Crosshair</source>
        <translation>십자선 표시</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Pause</source>
        <translation>일시정지</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Resume</source>
        <translation>재개</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">기록 지우기</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>%1에 오신 것을 환영합니다!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio는 엔지니어, 학생, 메이커를 위해 제작된 강력한 실시간 시각화 도구입니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>완전한 기능을 갖춘 14일 평가판을 시작하거나, 라이선스 키로 활성화하거나, GPLv3 소스 코드를 직접 다운로드하여 컴파일할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Pro 구매는 개발자를 직접 지원하고 향후 개발 자금을 마련하는 데 도움이 됩니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>GPLv3 버전을 직접 빌드하면 커뮤니티 성장에 도움이 되고 기술적 기여를 장려합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>잠시 기다려 주세요…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>평가판 남은 기간: %1일.</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">현재 %1 Pro의 모든 기능을 갖춘 평가판을 사용 중입니다. 개인 비상업적 용도로 14일간 유효합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>현재 %1 Pro의 모든 기능을 갖춘 평가판을 사용 중입니다. 개인 비상업적 용도로 14일간 유효합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Serial Studio Pro를 계속 사용하려면 유료 플랜으로 업그레이드하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>또는 GPLv3 소스 코드를 컴파일하여 무료로 사용할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>사용 가능한 구독 플랜을 보려면 아래 "지금 업그레이드"를 클릭하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>평가판에 대해 다시 알리지 않음.
평가판 종료 시 라이선스를 구매하거나 GPLv3 버전을 빌드해야 함을 이해합니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>%1 평가판이 만료되었습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>평가판 기간이 종료되었습니다. 모든 Pro 기능과 함께 %1을 계속 사용하려면 유료 플랜으로 업그레이드하세요.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>원하시면 GPLv3 라이선스에 따라 오픈 소스 버전을 컴파일할 수도 있습니다.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>%1을 사용해 주셔서 감사합니다!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>지금 업그레이드</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>활성화</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>제한 모드로 열기</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>계속</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>평가판 시작</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">작업 공간에서 제거</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="331"/>
        <source>Device Disconnected</source>
        <translation>장치 연결 해제됨</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="332"/>
        <source>Alarm</source>
        <translation>알람</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="333"/>
        <source>critical</source>
        <translation>심각</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="333"/>
        <source>warning</source>
        <translation>경고</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="337"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>값 %1%2이(가) %3 밴드(%4–%5)에 진입했습니다.</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">값 %1%2이(가) 상한 알람 %3%2에 도달</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">값 %1%2이(가) 하한 알람 %3%2에 도달</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="343"/>
        <source>Alarms</source>
        <translation>알람</translation>
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
        <translation>NE</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="164"/>
        <source>E</source>
        <translation>E</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="167"/>
        <source>SE</source>
        <translation>SE</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="170"/>
        <source>S</source>
        <translation>S</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="173"/>
        <source>SW</source>
        <translation>남서</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="176"/>
        <source>W</source>
        <translation>서</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="179"/>
        <source>NW</source>
        <translation>북서</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="128"/>
        <source>Title</source>
        <translation>제목</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="129"/>
        <source>Value</source>
        <translation>값</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">데이터 대기 중…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Satellite Imagery</source>
        <translation>위성 이미지</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Satellite Imagery with Labels</source>
        <translation>레이블 포함 위성 이미지</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Street Map</source>
        <translation>거리 지도</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Topographic Map</source>
        <translation>지형도</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Terrain</source>
        <translation>지형</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Light Gray Canvas</source>
        <translation>밝은 회색 캔버스</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="97"/>
        <source>Dark Gray Canvas</source>
        <translation>어두운 회색 캔버스</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="97"/>
        <source>National Geographic</source>
        <translation>내셔널 지오그래픽</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="409"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>추가 지도 레이어는 Pro 사용자만 이용할 수 있습니다.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="410"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>ArcGIS API 키에 실제 비용이 발생하므로 무제한 액세스를 제공할 수 없습니다.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="102"/>
        <source>Time (s)</source>
        <translation>시간 (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="102"/>
        <source>Samples</source>
        <translation>샘플</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="175"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>전송 스크립트가 %1 ms 후 시간 초과됨</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="194"/>
        <source>Payload exceeds maximum size</source>
        <translation>페이로드가 최대 크기를 초과함</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="87"/>
        <source>Time (s)</source>
        <translation>시간 (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="115"/>
        <source>Samples</source>
        <translation>샘플</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="1046"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>그리드 간격: %1 단위</translation>
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
        <translation>그레이스케일</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="428"/>
        <source>Unknown</source>
        <translation>알 수 없음</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>작업 공간 편집</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>새 작업 공간</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>이름:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>내 작업 공간</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>포함할 위젯 선택:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>위젯 필터링…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>취소</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="309"/>
        <source>OK</source>
        <translation>확인</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="41"/>
        <source>Workspace</source>
        <translation>작업 공간</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>작업 공간에 위젯 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>이름 변경</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>작업 공간 이름 변경</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>작업 공간 삭제</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>그룹</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="178"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="184"/>
        <source>Widget</source>
        <translation>위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="179"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="185"/>
        <source>Type</source>
        <translation>유형</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="229"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="267"/>
        <source>(unknown)</source>
        <translation>(알 수 없음)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(그룹 위젯)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>작업 공간에서 위젯 제거</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">작업 공간에서 제거</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>이 작업 공간에 위젯이 없습니다.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>작업 공간</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>사용자 지정</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>작업 공간 수동 편집</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>위로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>선택한 작업 공간을 위로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>아래로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>선택한 작업 공간을 아래로 이동</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>작업 공간 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>작업 공간 추가</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>정리</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>대상 그룹 또는 데이터셋이 더 이상 존재하지 않는 위젯 참조 %1개 제거</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>모든 작업 공간에 오래된 위젯 참조 없음</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="203"/>
        <source>Title</source>
        <translation>제목</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="204"/>
        <source>Widgets</source>
        <translation>위젯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="276"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>작업 공간이 없습니다. 위 도구 모음에서 추가하거나 자동 레이아웃으로 재설정하십시오.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>프로젝트에 적합한 그룹이 없습니다 -- 작업 공간을 채우려면 위젯이 있는 그룹을 추가하십시오.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>자동 레이아웃으로 재설정</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">작업 공간이 없습니다.</translation>
    </message>
</context>
</TS>