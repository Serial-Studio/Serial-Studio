<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="167"/>
        <source>Anthropic error</source>
        <translation>Anthropic 错误</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="280"/>
        <source>Stream parse error: %1</source>
        <translation>流解析错误：%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="327"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="330"/>
        <source>Invalid API key (%1)</source>
        <translation>无效的 API 密钥（%1）</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="332"/>
        <source>Rate limited: %1</source>
        <translation>速率受限：%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="334"/>
        <source>Anthropic %1: %2</source>
        <translation>Anthropic %1：%2</translation>
    </message>
</context>
<context>
    <name>AI::Assistant</name>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="344"/>
        <source>Switch AI provider?</source>
        <translation>切换 AI 提供商？</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="345"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>切换到其他提供商将清除当前对话。是否继续？</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="348"/>
        <source>Assistant</source>
        <translation>助手</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="385"/>
        <source>AI Assistant requires a Pro license</source>
        <translation>AI 助手需要 Pro 许可证</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="390"/>
        <source>Set an API key first</source>
        <translation>请先设置 API 密钥</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="166"/>
        <source>AI Assistant requires a Pro license</source>
        <translation>AI 助手需要 Pro 许可证</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="172"/>
        <source>AI subsystem not initialized</source>
        <translation>AI 子系统未初始化</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="178"/>
        <source>Already busy with a previous request</source>
        <translation>正在处理先前的请求</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="462"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>已达到本轮工具调用预算;不会再运行更多工具。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1725"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>您已达到本轮工具调用预算。请勿请求更多工具。总结目前发现的内容,如果任务未完成,请说明剩余步骤以便用户告知您继续。</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">工具调用预算已超限</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="912"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(模型返回了空响应。请尝试重新表述、切换到其他模型，或检查请求是否被提供商的安全过滤器拦截。)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1009"/>
        <source>Sending request to %1...</source>
        <translation>正在向 %1 发送请求…</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1021"/>
        <source>Provider returned no reply</source>
        <translation>提供商未返回回复</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="147"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>提示被 Gemini 安全过滤器拦截:%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="190"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini 停止且未生成响应:%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="250"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="253"/>
        <source>Invalid API key (%1)</source>
        <translation>API 密钥无效 (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="255"/>
        <source>Rate limited: %1</source>
        <translation>速率受限:%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="257"/>
        <source>Invalid API key</source>
        <translation>API 密钥无效</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="259"/>
        <source>Gemini %1: %2</source>
        <translation>Gemini %1:%2</translation>
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
        <translation>API 密钥无效 (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="326"/>
        <source>Rate limited: %1</source>
        <translation>速率受限:%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="328"/>
        <source>%1 %2: %3</source>
        <translation>%1 %2: %3</translation>
    </message>
    <message>
        <source>OpenAI %1: %2</source>
        <translation type="vanished">OpenAI %1:%2</translation>
    </message>
</context>
<context>
    <name>API::GRPC::GRPCServer</name>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Export Protobuf File</source>
        <translation>导出 Protobuf 文件</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="425"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="474"/>
        <source>Unable to start gRPC server</source>
        <translation>无法启动 GRPC 服务器</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="475"/>
        <source>Failed to bind to %1</source>
        <translation>绑定到 %1 失败</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="438"/>
        <source>Unable to start API TCP server</source>
        <translation>无法启动 API TCP 服务器</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="483"/>
        <source>Allow External API Connections?</source>
        <translation>允许外部 API 连接？</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="484"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>将 API 服务器暴露给外部主机允许网络上的其他设备通过端口 7777 连接到 Serial Studio。

仅在受信任的网络上启用此功能。不受信任的客户端可能读取实时数据或向设备发送命令。</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="519"/>
        <source>Unable to restart API TCP server</source>
        <translation>无法重启 API TCP 服务器</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="598"/>
        <source>Allow API device control?</source>
        <translation>允许 API 设备控制?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="599"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>使用 Serial Studio 本地 API 的程序正在请求向已连接设备发送数据。是否允许 API 客户端写入设备?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="602"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1255"/>
        <source>API server</source>
        <translation>API 服务器</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1255"/>
        <source>Invalid pending connection</source>
        <translation>待处理连接无效</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>版本 %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>版权所有 © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>保留所有权利</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 是自由软件：您可以根据自由软件基金会发布的 GNU 通用公共许可证的条款重新分发和/或修改它；可以选择许可证的第 3 版，或（根据您的选择）任何更高版本。

%1 的分发希望它能有用，但不提供任何保证；甚至不提供适销性或特定用途适用性的默示保证。有关详细信息，请参阅 GNU 通用公共许可证。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>此配置已获得商业和专有使用许可。它可以在闭源和商业应用程序中使用，但须遵守商业许可证的条款。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>此配置仅供个人和评估使用。除非激活有效的商业许可证，否则禁止商业使用。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>本软件按"原样"提供，不提供任何明示或默示的保证，包括但不限于适销性或特定用途适用性的保证。在任何情况下，作者均不对因使用本软件而产生的任何损害承担责任。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>管理许可证</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>捐赠</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>检查更新</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>License Agreement</source>
        <translation>许可协议</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>Report Bug</source>
        <translation>报告错误</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Acknowledgements</source>
        <translation>致谢</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Benchmark</source>
        <translation>基准测试</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="258"/>
        <source>Website</source>
        <translation>网站</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="274"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="170"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="171"/>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="229"/>
        <source>G-FORCE</source>
        <translation>G力</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="265"/>
        <source>PITCH ↕</source>
        <translation>俯仰 ↕</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="300"/>
        <source>ROLL ↔</source>
        <translation>横滚 ↔</translation>
    </message>
</context>
<context>
    <name>AccelerometerConfigDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="35"/>
        <source>Accelerometer Configuration</source>
        <translation>加速度计配置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>配置加速度计显示范围和输入单位。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>显示范围</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>最大 G 值：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>输入最大 G 值</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>输入配置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>输入已为 G 单位</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="35"/>
        <source>Acknowledgements</source>
        <translation>致谢</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="76"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="87"/>
        <source>About Qt…</source>
        <translation>关于 QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>更改图标</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>更改此操作使用的图标</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>复制此操作及其所有设置</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>从项目中删除此操作</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>添加控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>可用控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>单击行以将其添加到工作区。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>搜索</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>（整个组）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>已在工作区中</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>添加到工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>无可用组件。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>无匹配组件。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 个组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 / %2 个组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>报警区段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="70"/>
        <source>Info</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="151"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="72"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="152"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="73"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="153"/>
        <source>Critical</source>
        <translation>严重</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="82"/>
        <source>Tachometer</source>
        <translation>转速表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="84"/>
        <source>Idle</source>
        <translation>空闲</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Operating</source>
        <translation>运行</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Caution</source>
        <translation>注意</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Redline</source>
        <translation>红线</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="91"/>
        <source>Speedometer</source>
        <translation>速度计</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="93"/>
        <source>Cruise</source>
        <translation>巡航</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Fast</source>
        <translation>快速</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Top Speed</source>
        <translation>最高速度</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="99"/>
        <source>Engine Temperature</source>
        <translation>引擎温度</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="101"/>
        <source>Cold</source>
        <translation>冷</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="143"/>
        <source>Normal</source>
        <translation>正常</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <source>Warm</source>
        <translation>温暖</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Overheat</source>
        <translation>过热</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="108"/>
        <source>Pressure</source>
        <translation>压强</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="110"/>
        <source>Vacuum</source>
        <translation>真空</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>High</source>
        <translation>高</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <source>Burst</source>
        <translation>爆发</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="117"/>
        <source>Battery Voltage</source>
        <translation>电池电压</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="119"/>
        <source>Low</source>
        <translation>低</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Nominal</source>
        <translation>标称</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="125"/>
        <source>Fuel Level</source>
        <translation>燃油液位</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="127"/>
        <source>Empty</source>
        <translation>空</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Reserve</source>
        <translation>备用</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="133"/>
        <source>Signal Strength</source>
        <translation>信号强度</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="135"/>
        <source>No Signal</source>
        <translation>无信号</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>Weak</source>
        <translation>弱</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Good</source>
        <translation>良好</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="141"/>
        <source>CPU / System Load</source>
        <translation>CPU / 系统负载</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Busy</source>
        <translation>繁忙</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Overload</source>
        <translation>过载</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="149"/>
        <source>OK / Warning / Critical</source>
        <translation>正常 / 警告 / 严重</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="270"/>
        <source>Choose Band Color</source>
        <translation>选择波段颜色</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="297"/>
        <source>Presets</source>
        <translation>预设</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="320"/>
        <source>Preset</source>
        <translation>预设</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="335"/>
        <source>Choose preset…</source>
        <translation>选择预设…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="557"/>
        <source>Reset to severity default</source>
        <translation>重置为严重性默认值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="571"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>单击选择颜色。右键单击重置为严重性默认值。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="572"/>
        <source>Click to choose a custom color.</source>
        <translation>单击选择自定义颜色。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="657"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>未定义区段。选择上方预设或添加区段以开始。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="781"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="784"/>
        <source>Apply changes to the dataset.</source>
        <translation>应用对数据集的更改。</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">应用预设</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">用所选预设替换当前波段，并缩放至此数据集的范围。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="346"/>
        <source>Range</source>
        <translation>范围</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="374"/>
        <source>Bands</source>
        <translation>波段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="385"/>
        <source>Add Band</source>
        <translation>添加波段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="389"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>添加一个从最后一个波段延续的新波段。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="420"/>
        <source>Min</source>
        <translation>最小值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="426"/>
        <source>Max</source>
        <translation>最大值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="432"/>
        <source>Severity</source>
        <translation>严重程度</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="438"/>
        <source>Color</source>
        <translation>颜色</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="445"/>
        <source>Label</source>
        <translation>标签</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">选择自定义颜色。</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">自动</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="594"/>
        <source>(optional)</source>
        <translation>(可选)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="611"/>
        <source>Move up.</source>
        <translation>上移。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="630"/>
        <source>Move down.</source>
        <translation>下移。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="643"/>
        <source>Remove this band.</source>
        <translation>移除此区段。</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">未定义区段。应用预设或添加区段以开始。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="674"/>
        <source>Preview</source>
        <translation>预览</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="770"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="772"/>
        <source>Discard changes.</source>
        <translation>放弃更改。</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">保存</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">保存对数据集的更改。</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>助手</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="128"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>CSV 与 MDF4 导出 - 有什么区别？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="131"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>图表、柱状图和仪表 - 何时使用每种类型？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="203"/>
        <source>How can I help with your project?</source>
        <translation>我能如何帮助您的项目?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="204"/>
        <source>Set up your API key to get started</source>
        <translation>设置您的 API 密钥以开始使用</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="216"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>描述您想要构建的内容,我将为您配置数据源、组、数据集、帧解析器和转换。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="219"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>要开始对话,请为所选提供商粘贴 API 密钥。密钥在本机加密,除了与您选择的提供商通信外,不会离开您的计算机。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="240"/>
        <source>Open API Key Setup</source>
        <translation>打开 API 密钥设置</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Get a key from %1</source>
        <translation>从 %1 获取密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="124"/>
        <source>List the sources in this project</source>
        <translation>列出此项目中的数据源</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="121"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>帮我了解 Serial Studio 的功能</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="122"/>
        <source>What can this app do for my telemetry?</source>
        <translation>这个应用能为我的遥测做什么？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="123"/>
        <source>Walk me through what this project already contains</source>
        <translation>带我了解这个项目已包含的内容</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="127"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>什么是会话数据库，为什么要使用它？</translation>
    </message>
    <message>
        <source>CSV vs MDF4 export — what is the difference?</source>
        <translation type="vanished">CSV 与 MDF4 导出 — 有什么区别？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="129"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>什么是帧解析器，什么时候需要使用？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="130"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>何时应使用 Lua 而非 JavaScript 作为解析器？</translation>
    </message>
    <message>
        <source>Plot, Bar, and Gauge — when to use each?</source>
        <translation type="vanished">图表、柱状图和仪表 — 何时使用每种类型？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="132"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>变换与帧解析器之间有什么区别？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="135"/>
        <source>Add a UART source for an Arduino</source>
        <translation>为 Arduino 添加 UART 数据源</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="136"/>
        <source>Set up an IMU project from scratch</source>
        <translation>从头开始设置 IMU 项目</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="137"/>
        <source>Configure an MQTT subscriber</source>
        <translation>配置 MQTT 订阅者</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="138"/>
        <source>Add a CAN bus source</source>
        <translation>添加 CAN 总线数据源</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="139"/>
        <source>Set up a Modbus poller</source>
        <translation>设置 Modbus 轮询器</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="140"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>添加网络 (TCP/UDP) 数据源</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="141"/>
        <source>Write a CSV frame parser for me</source>
        <translation>为我编写 CSV 帧解析器</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="142"/>
        <source>Help me parse a JSON frame</source>
        <translation>帮我解析 JSON 帧</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="143"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>为数据集添加 EMA 平滑变换</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="144"/>
        <source>Decode hexadecimal frames</source>
        <translation>解码十六进制帧</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="145"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>使用线性变换校准传感器</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="148"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>为我的数据推荐仪表板部件</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="149"/>
        <source>Build an executive overview workspace</source>
        <translation>构建管理概览工作区</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="150"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>添加绘图器组件以实现自定义可视化</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="151"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>为单个数据集显示绘图、FFT 和瀑布图</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="152"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>将我的数据集分组到有用的工作区</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="446"/>
        <source>Ask Serial Studio anything…</source>
        <translation>向 Serial Studio 提问任何问题…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="466"/>
        <source>Clear conversation</source>
        <translation>清除对话</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="510"/>
        <source>Stop generating</source>
        <translation>停止生成</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="511"/>
        <source>Send message (Enter)</source>
        <translation>发送消息 (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="553"/>
        <source>Provider</source>
        <translation>提供商</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="586"/>
        <source>Model selection</source>
        <translation>模型选择</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="632"/>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation>执行编辑操作时不再逐次询问。已阻止的操作保持阻止状态。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="634"/>
        <source>Auto-approve edits</source>
        <translation>自动批准编辑</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="653"/>
        <source>Manage API keys</source>
        <translation>管理 API 密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="674"/>
        <source>Working</source>
        <translation>处理中</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="675"/>
        <source>Ready</source>
        <translation>就绪</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="676"/>
        <source>  •  cache %1k tok</source>
        <translation>•  缓存 %1k tok</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="677"/>
        <source>  •  cache write %1k tok</source>
        <translation>缓存写入 %1k 令牌</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>未检测到麦克风</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>连接麦克风或检查设置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>输入设备</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>采样率</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>采样格式</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>声道</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>输出设备</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">对话框</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">请提供下载位置的用户名和密码。</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">用户名(&amp;U)：</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">密码(&amp;P)：</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="47"/>
        <source>Axis Range Configuration</source>
        <translation>坐标轴范围配置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="161"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>配置绘图坐标轴的可见范围。输入时数值实时更新。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="169"/>
        <source>X Axis</source>
        <translation>X 轴</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="194"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="265"/>
        <source>Minimum:</source>
        <translation>最小值：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="206"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="277"/>
        <source>Enter min value</source>
        <translation>输入最小值</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="215"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="286"/>
        <source>Maximum:</source>
        <translation>最大值：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="227"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="298"/>
        <source>Enter max value</source>
        <translation>输入最大值</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="242"/>
        <source>Y Axis</source>
        <translation>Y 轴</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="317"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="327"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>恢复备份</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>无标题</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>项目已加载</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>自动保存</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>恢复前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>删除数据集前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>删除分组前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>新建项目前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>打开项目前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>加载 JSON 前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>应用模板前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>删除动作前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>删除输出组件前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>移动数据集之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>移动组之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>删除工作区之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>清除所有工作区之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>移除控件之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>重新排序工作区之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>批量操作之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>添加磁贴之前</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1(还有 %2 个)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>帧解析器代码也不同，将被替换。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>标题从"%1"更改为"%2"。组结构未更改。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>组和数据集相同，但帧解析器代码不同 — 恢复将替换它。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>与当前项目的组和数据集相同。恢复仍可能还原字段级编辑。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>恢复将移除 %1 并恢复 %2。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>恢复将移除 %1。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>恢复将恢复 %1。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>选择要恢复的备份。当前项目会先自动保存,因此恢复操作可逆。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>此项目尚无备份。编辑或保存项目以启动滚动备份。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>打开文件夹</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>恢复</translation>
    </message>
</context>
<context>
    <name>Benchmark</name>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="32"/>
        <source>Benchmark</source>
        <translation>基准测试</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="62"/>
        <source>%1 frames/s</source>
        <translation>%1 帧/秒</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="66"/>
        <source>%1 s</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="71"/>
        <source>n/a</source>
        <translation>不适用</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="73"/>
        <source>Pass</source>
        <translation>通过</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="73"/>
        <source>Fail</source>
        <translation>失败</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="94"/>
        <source>Hotpath Benchmark</source>
        <translation>热路径基准测试</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="105"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>测量此计算机通过 Serial Studio 数据管道提取、解析和可视化帧的速度。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="143"/>
        <source>The interface will freeze while running</source>
        <translation>运行时界面将冻结</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="154"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>每个阶段在主线程上全速运行,因此窗口会停止响应直到完成。基准测试结束时会自动重新加载当前项目。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="173"/>
        <source>Frames per phase:</source>
        <translation>每阶段帧数:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="187"/>
        <source>Minimum duration:</source>
        <translation>最短持续时间:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="225"/>
        <source>Running %1...</source>
        <translation>运行中 %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="226"/>
        <source>Preparing...</source>
        <translation>准备中...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="269"/>
        <source>Pipeline</source>
        <translation>流水线</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="281"/>
        <source>Throughput</source>
        <translation>吞吐量</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="293"/>
        <source>Time</source>
        <translation>时间</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="305"/>
        <source>Result</source>
        <translation>结果</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="398"/>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation>通过/失败仅适用于数据管道和解析器阶段(数据管道 1024 K 帧/秒;数值型:Lua 256 K,JavaScript 128 K;混合型:Lua 128 K,JavaScript 64 K)。导出和仪表板阶段仅供参考。</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">通过/失败仅适用于解析器阶段(Lua 目标 256 K 帧/秒,JavaScript 128 K)。导出和仪表板阶段仅供参考。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="412"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="421"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="431"/>
        <source>Running...</source>
        <translation>运行中…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="431"/>
        <source>Run Benchmark</source>
        <translation>运行基准测试</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>Data pipeline</source>
        <translation>数据管道</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">Lua 解析器</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">JavaScript 解析器</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + 数据导出</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + 仪表板</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>Lua parser (numeric)</source>
        <translation>Lua 解析器(数值型)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="184"/>
        <source>JavaScript parser (numeric)</source>
        <translation>JavaScript 解析器(数值型)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="185"/>
        <source>Lua parser (mixed)</source>
        <translation>Lua 解析器(混合型)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="186"/>
        <source>JavaScript parser (mixed)</source>
        <translation>JavaScript 解析器(混合型)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="187"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + 数据导出(混合型)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="188"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + 仪表板(数值型)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="189"/>
        <source>100 K frames</source>
        <translation>100K 帧</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="189"/>
        <source>250 K frames</source>
        <translation>250 K 帧</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="189"/>
        <source>500 K frames</source>
        <translation>500 K 帧</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="189"/>
        <source>1 M frames</source>
        <translation>1 M 帧</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="190"/>
        <source>1 second</source>
        <translation>1 秒</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="190"/>
        <source>2 seconds</source>
        <translation>2 秒</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="190"/>
        <source>5 seconds</source>
        <translation>5 秒</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="190"/>
        <source>10 seconds</source>
        <translation>10 秒</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">正在显示仪表板…</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>设备</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>服务</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>特征</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>正在扫描…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>未检测到蓝牙适配器</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>连接蓝牙适配器或检查系统设置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>尚不支持此操作系统。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>一旦 QT 正式支持此操作系统，我们将更新 Serial Studio 以支持它</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>未找到 CAN 驱动程序</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>为您的系统安装 CAN 硬件驱动程序</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>CAN 驱动程序</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="142"/>
        <source>Interface</source>
        <translation>接口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="174"/>
        <source>Bitrate</source>
        <translation>比特率</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="238"/>
        <source>Flexible Data-Rate</source>
        <translation>灵活数据速率</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="270"/>
        <source>DBC Database</source>
        <translation>DBC 数据库</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="274"/>
        <source>Import DBC File…</source>
        <translation>导入 DBC 文件…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="307"/>
        <source>No CAN Interfaces Found</source>
        <translation>未找到 CAN 接口</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="210"/>
        <source>Select CSV file</source>
        <translation>选择 CSV 文件</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="212"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 文件 (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="383"/>
        <source>Device Connection Active</source>
        <translation>设备连接活动</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="384"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>要使用此功能,必须断开与设备的连接。是否继续?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="399"/>
        <source>Check file permissions and location</source>
        <translation>检查文件权限和位置</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="432"/>
        <source>Insufficient Data in CSV File</source>
        <translation>CSV 文件中数据不足</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="399"/>
        <source>Cannot read CSV file</source>
        <translation>无法读取 CSV 文件</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="433"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>CSV 文件必须至少包含一个数据行才能继续。请检查文件并重试。</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="669"/>
        <source>Invalid CSV</source>
        <translation>CSV 无效</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="670"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>CSV 文件不包含任何数据或标题。</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="679"/>
        <source>Select a date/time column</source>
        <translation>选择日期/时间列</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="679"/>
        <location filename="../../src/CSV/Player.cpp" line="691"/>
        <source>Set interval manually</source>
        <translation>手动设置间隔</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="681"/>
        <source>CSV Date/Time Selection</source>
        <translation>CSV 日期/时间选择</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="682"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>选择如何处理日期/时间数据：</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="694"/>
        <source>Set Interval</source>
        <translation>设置间隔</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>请输入行间隔（毫秒）：</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="711"/>
        <source>Select Date/Time Column</source>
        <translation>选择日期/时间列</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="712"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>请选择包含日期/时间数据的列：</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="722"/>
        <source>Invalid Selection</source>
        <translation>选择无效</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="722"/>
        <source>The selected column is not valid.</source>
        <translation>所选列无效。</translation>
    </message>
</context>
<context>
    <name>Console</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Console.qml" line="32"/>
        <source>Console</source>
        <translation>控制台</translation>
    </message>
</context>
<context>
    <name>Console::Export</name>
    <message>
        <location filename="../../src/Console/Export.cpp" line="340"/>
        <source>Console Export is a Pro feature.</source>
        <translation>控制台导出是 Pro 功能。</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="341"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>此功能需要许可证。请购买许可证以启用控制台导出。</translation>
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
        <translation>无行结束符</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="236"/>
        <source>New Line</source>
        <translation>换行</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="237"/>
        <source>Carriage Return</source>
        <translation>回车</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="238"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="248"/>
        <source>Plain Text</source>
        <translation>纯文本</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="249"/>
        <source>Hexadecimal</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="271"/>
        <source>No Checksum</source>
        <translation>无校验和</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="913"/>
        <source>Device %1</source>
        <translation>设备 %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>插入常量</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>基本常量</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>真空光速</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>普朗克常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>基本电荷</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>阿伏伽德罗常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>玻尔兹曼常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>斯特藩-玻尔兹曼常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>力学</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>标准重力加速度</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>万有引力常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>压强</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>标准大气压</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>海平面气压</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>温度</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>绝对零度（摄氏度）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>水的冰点</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>水的沸点（1 atm）</translation>
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
        <translation>气体和流体</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>通用气体常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>比气体常数（干空气）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>比气体常数（水蒸气）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>空气密度（海平面，15°C）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>水密度 (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>空气中的声速 (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>热容比 (干燥空气)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>电磁学</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>真空介电常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>真空磁导率</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>数学</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>圆周率</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>欧拉数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>黄金比例</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>物理常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>SI 单位预设值。单击行以将其插入到 %1。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>搜索</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>符号</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>类别</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>无匹配常数。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 个常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 / %2 个常数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>恢复选项</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio 已连续多次意外关闭。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>连续崩溃次数：%1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>上次报告的阶段：%1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>检测时间：%1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>请选择恢复操作。应用后 Serial Studio 将退出，以便下次启动时恢复正常。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>重置渲染后端为默认值</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>跳过恢复上次打开的项目</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>重置所有首选项</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>仍然继续</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="36"/>
        <source>CSV Player</source>
        <translation>CSV 播放器</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>DBC 文件预览</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>DBC 文件：%1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>查看要导入到新 Serial Studio 项目中的 CAN 消息和信号。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>消息</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>消息名称</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>CAN ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>信号</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>DBC 文件中未找到消息。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>总计：%1 条消息，%2 个信号</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>创建项目</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="127"/>
        <source>Dashboard %1</source>
        <translation>仪表板 %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>未定义传输函数</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="56"/>
        <source>Set Wallpaper…</source>
        <translation>设置壁纸…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="62"/>
        <source>Clear Wallpaper</source>
        <translation>清除壁纸</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="72"/>
        <source>Tile Windows</source>
        <translation>平铺窗口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="91"/>
        <source>Pro features detected in this project.</source>
        <translation>检测到此项目中的 Pro 功能。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="93"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>后备控件已激活。购买许可证以获得完整功能。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="208"/>
        <source>Empty Workspace</source>
        <translation>空工作区</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="222"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>使用搜索栏查找并添加控件,或右键单击其他工作区中的控件以将其添加到此处。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="237"/>
        <source>Search Widgets</source>
        <translation>搜索控件</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>仪表板</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="206"/>
        <source>API Server Active (%1)</source>
        <translation>API 服务器活动 (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="207"/>
        <source>API Server Ready</source>
        <translation>API 服务器就绪</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="208"/>
        <source>API Server Off</source>
        <translation>API 服务器关闭</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="123"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="275"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="263"/>
        <source>Enter command…</source>
        <translation>输入命令…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>未定义传输函数</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>输入命令…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>发送</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>未定义传输函数</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>开</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>关</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>未定义传输函数</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="86"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="87"/>
        <source>Resume</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="297"/>
        <source>Awaiting data…</source>
        <translation>等待数据…</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>Import DBC File</source>
        <translation>导入 DBC 文件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="124"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>DBC 文件 (*.DBC);;所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="158"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>解析 DBC 文件失败：%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="159"/>
        <source>Verify the file format and try again.</source>
        <translation>请验证文件格式并重试。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>DBC Import Error</source>
        <translation>DBC 导入错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="168"/>
        <source>DBC file contains no messages</source>
        <translation>DBC 文件不包含任何消息</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="169"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>所选文件不包含任何 CAN 消息定义。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC Import Warning</source>
        <translation>DBC 导入警告</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">创建临时项目文件失败</translation>
    </message>
    <message>
        <source>Check if the application has write permissions to the temporary directory.</source>
        <translation type="vanished">请检查应用程序是否具有临时目录的写入权限。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="223"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>成功导入 DBC 文件，包含 %1 条消息和 %2 个信号。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="216"/>
        <source>The project editor is now open for customization.</source>
        <translation>项目编辑器现已打开，可进行自定义。</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">加载导入的项目失败</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">无法加载生成的项目 JSON。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>已跳过 %1 个使用扩展复用 (SG_MUL_VAL_) 的信号;仅支持简单复用。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="228"/>
        <source>DBC Import Complete</source>
        <translation>DBC 导入完成</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="250"/>
        <source>CAN Bus</source>
        <translation>CAN 总线</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="58"/>
        <source>Dataset Value Transform</source>
        <translation>数据集数值转换</translation>
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
        <translation>语言：</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="122"/>
        <source>Template:</source>
        <translation>模板：</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="101"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>输入原始值（例如 1024）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Test</source>
        <translation>测试</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="107"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="133"/>
        <source>Input:</source>
        <translation>输入:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="136"/>
        <source>Output:</source>
        <translation>输出:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="109"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="110"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="216"/>
        <source>Transform — %1</source>
        <translation>变换 — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="306"/>
        <source>Enter a value</source>
        <translation>输入一个值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>Invalid number</source>
        <translation>无效数字</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="383"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>格式化文档	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="384"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>格式化选区	Ctrl+I</translation>
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
-- 定义一个 transform(value) 函数,接收实时数据集读数
-- 并返回转换后的数值。如果未定义 transform() 函数,
-- 则保留原始值,且不会随项目保存任何内容。
--
-- 示例:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- 在顶层声明的全局变量在帧之间保持,这对于滤波器、
-- 累加器以及其他任何有状态的变换非常有用,就像帧
-- 解析器脚本一样:
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
-- 使用模板下拉菜单获取现成示例,或点击测试以试用
-- 您的函数。
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
 * 定义一个 transform(value) 函数,接收实时数据集读数
 * 并返回转换后的数值。如果未定义 transform() 函数,
 * 则保留原始值,且不会随项目保存任何内容。
 *
 * 示例:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * 在顶层声明的全局变量在帧之间持久存在,这对于滤波器、
 * 累加器和其他任何有状态转换非常有用 -- 就像帧解析器
 * 脚本一样:
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
 * 使用模板下拉菜单获取现成示例,或点击测试来
 * 试用您的函数。
 */
</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="788"/>
        <source>Select Template…</source>
        <translation>选择模板…</translation>
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
 * 定义一个 transform(value) 函数，接收实时数据集读数
 * 并返回转换后的数值。如果未定义 transform() 函数，
 * 则保留原始值，且不会随项目保存任何内容。
 *
 * 示例：
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * 在顶层声明的全局变量在帧之间持久存在，这对于滤波器、
 * 累加器和其他任何有状态转换非常有用——就像帧解析器
 * 脚本一样：
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
 * 使用模板下拉菜单获取现成示例，或单击测试以试用您的函数。
 */

</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="665"/>
        <source>Engine error</source>
        <translation>引擎错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="706"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="726"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="737"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="698"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="730"/>
        <source>Error: transform() not defined</source>
        <translation>错误：未定义 transform()</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="712"/>
        <source>Error: transform() must return a number</source>
        <translation>错误：transform() 必须返回数值</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="973"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1091"/>
        <source>Channel %1</source>
        <translation>通道 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1103"/>
        <source>Audio Input</source>
        <translation>音频输入</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="982"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1108"/>
        <source>Quick Plot</source>
        <translation>快速绘图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="846"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>JavaScript 转换超出预算</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="847"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>某个数据集转换耗时超过 %1 毫秒；该帧中其余数据集已回退为原始值，直到下一个帧。请对转换代码进行分析或简化。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="190"/>
        <source>Frame pool exhausted</source>
        <translation>帧池已耗尽</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="192"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>下游消费者（仪表盘、CSV/MDF4 导出、会话数据库或 API 订阅者）处理帧的速度不够快。Serial Studio 正回退为每帧分配，直到积压清除。请禁用高负载消费者或降低数据速率。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="936"/>
        <source>Device A</source>
        <translation>设备 A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="989"/>
        <source>Quick Plot Data</source>
        <translation>快速绘图数据</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1001"/>
        <source>Multiple Plots</source>
        <translation>多重绘图</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="248"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="395"/>
        <source>Invalid Hex Input</source>
        <translation>十六进制输入无效</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="396"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>请输入有效的十六进制字节。

有效格式：01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="430"/>
        <source>(no frames)</source>
        <translation>(无帧)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="432"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>提取未生成完整帧。请检查起始/结束分隔符和检测模式。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="440"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>已提取 %1 帧 | 已消耗 %2 字节 | 已缓冲 %3 字节 | 已丢弃 %4</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="631"/>
        <source>Pipeline Configuration</source>
        <translation>管道配置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="633"/>
        <source>Pipeline Results</source>
        <translation>处理流程结果</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="635"/>
        <source>Detection</source>
        <translation>检测</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="636"/>
        <source>Decoder</source>
        <translation>解码器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="637"/>
        <source>Checksum</source>
        <translation>校验和</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="638"/>
        <source>Start Delimiter</source>
        <translation>起始定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="639"/>
        <source>End Delimiter</source>
        <translation>结束定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="640"/>
        <source>Hex Delimiters</source>
        <translation>十六进制定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="642"/>
        <source>End delimiter only</source>
        <translation>仅结束定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="643"/>
        <source>Start + end delimiters</source>
        <translation>起始 + 结束定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="644"/>
        <source>Start delimiter only</source>
        <translation>仅起始定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="645"/>
        <source>No delimiters (whole chunk)</source>
        <translation>无分隔符（整块）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="647"/>
        <source>Plain text (UTF-8)</source>
        <translation>纯文本（UTF-8）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="648"/>
        <source>Hexadecimal</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="649"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="650"/>
        <source>Binary (raw bytes)</source>
        <translation>二进制（原始字节）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="652"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="653"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="654"/>
        <source>Evaluate</source>
        <translation>评估</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="655"/>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="682"/>
        <source>Enter raw stream bytes here...</source>
        <translation>在此输入原始流字节...</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="656"/>
        <source>Stage</source>
        <translation>暂存</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="632"/>
        <source>Frame Data Input</source>
        <translation>帧数据输入</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">帧解析器结果</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">在此输入帧数据…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">数据集索引</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="656"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">在上方输入帧数据,如需要可启用 HEX 模式,然后单击"评估"以运行帧解析器。

示例(文本):a,b,c,d,e,f
示例(HEX):48 65 6C 6C 6F</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="663"/>
        <source>Test Frame Parser</source>
        <translation>测试帧解析器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="676"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>输入十六进制字节(例如:01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(空)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">未返回数据</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="208"/>
        <source>Change Scripting Language?</source>
        <translation>更改脚本语言?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="209"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>切换脚本语言会将当前解析器代码替换为新语言的等效模板。

任何未保存的更改都将丢失。是否继续?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="335"/>
        <source>Select Javascript file to import</source>
        <translation>选择要导入的 JavaScript 文件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="335"/>
        <source>Select Lua file to import</source>
        <translation>选择要导入的 Lua 文件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="369"/>
        <source>Code Validation Successful</source>
        <translation>代码验证成功</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="370"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>解析器代码中未检测到语法错误。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="483"/>
        <source>Select Frame Parser Template</source>
        <translation>选择帧解析器模板</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="484"/>
        <source>Choose a template to load:</source>
        <translation>选择要加载的模板:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="290"/>
        <source>Import Modbus Register Map</source>
        <translation>导入 Modbus 寄存器映射</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="294"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Modbus 寄存器映射 (*.CSV *.XML *.JSON);;CSV 文件 (*.CSV);;XML 文件 (*.XML);;JSON 文件 (*.JSON);;所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="332"/>
        <source>No registers found</source>
        <translation>未找到寄存器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="333"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>无法解析该文件或文件不包含寄存器定义。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="335"/>
        <source>Modbus Import</source>
        <translation>Modbus 导入</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">加载导入的项目失败</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">无法加载生成的项目 JSON。</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">创建临时项目文件失败</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">检查临时目录的写入权限。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="381"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>已成功导入 %1 个寄存器到 %2 个组。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="383"/>
        <source>The project editor is now open for customization.</source>
        <translation>项目编辑器现已打开以供自定义。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="385"/>
        <source>Modbus Import Complete</source>
        <translation>Modbus 导入完成</translation>
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
        <translation>选择要导入的 JavaScript 文件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="351"/>
        <source>Select Output Widget Template</source>
        <translation>选择输出控件模板</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="352"/>
        <source>Choose a template to load:</source>
        <translation>选择要加载的模板:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="298"/>
        <source>Select Javascript file to import</source>
        <translation>选择要导入的 JavaScript 文件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="386"/>
        <source>Select Painter Widget Template</source>
        <translation>选择绘图控件模板</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Choose a template to load:</source>
        <translation>选择要加载的模板:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>为此模板添加数据集?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" 需要 %2 个数据集;当前组有 %3 个。

是否使用模板默认值添加 %4 个数据集?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2155"/>
        <source>Project Information</source>
        <translation>项目信息</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2165"/>
        <source>Project Title</source>
        <translation>项目标题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2166"/>
        <source>Untitled Project</source>
        <translation>无标题项目</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2167"/>
        <source>Name or description of the project</source>
        <translation>项目的名称或描述</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2392"/>
        <source>Frame Detection</source>
        <translation>帧检测</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2408"/>
        <source>Frame Detection Method</source>
        <translation>帧检测方法</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2409"/>
        <source>Select how incoming data frames are identified</source>
        <translation>选择如何识别传入的数据帧</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2419"/>
        <source>Hexadecimal Delimiters</source>
        <translation>十六进制分隔符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2420"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>以十六进制值输入帧起始/结束序列</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2436"/>
        <source>Frame Start Delimiter</source>
        <translation>帧起始分隔符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2437"/>
        <source>e.g. /*</source>
        <translation>例如 /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2438"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>标记数据帧开始的序列</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2450"/>
        <source>Frame End Delimiter</source>
        <translation>帧结束分隔符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2451"/>
        <source>e.g. */</source>
        <translation>例如 */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2452"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>标记数据帧结束的序列</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2464"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>有效载荷处理与验证</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2475"/>
        <source>Data Conversion Method</source>
        <translation>数据转换方法</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2476"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>选择解析前如何解码传入的二进制数据</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2492"/>
        <source>Checksum Algorithm</source>
        <translation>校验和算法</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2493"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>选择用于验证帧的校验和算法</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2185"/>
        <source>Group Information</source>
        <translation>组信息</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2195"/>
        <source>Group Title</source>
        <translation>组标题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2196"/>
        <source>Untitled Group</source>
        <translation>未命名组</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2197"/>
        <source>Title or description of this dataset group</source>
        <translation>此数据集组的标题或描述</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2330"/>
        <source>Composite Widget</source>
        <translation>复合控件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2331"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>选择此数据集组的可视化方式（可选）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2247"/>
        <source>Image Configuration</source>
        <translation>图像配置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3091"/>
        <source>Virtual Dataset</source>
        <translation>虚拟数据集</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3092"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>虚拟数据集通过变换和数据表计算其值，不需要帧索引</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Auto-detect</source>
        <translation>自动检测</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Manual Delimiters</source>
        <translation>手动分隔符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2260"/>
        <source>Detection Mode</source>
        <translation>检测模式</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1288"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1291"/>
        <source>Frame Parser</source>
        <translation>帧解析器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1431"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1432"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1476"/>
        <source>Groups</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1506"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1519"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1520"/>
        <source>Shared Memory</source>
        <translation>共享内存</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1506"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1526"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1527"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4963"/>
        <source>Dataset Values</source>
        <translation>数据集值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1570"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1583"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1584"/>
        <source>Workspaces</source>
        <translation>工作区</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1622"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1625"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1626"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 发布者</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1675"/>
        <source>Publishing</source>
        <translation>发布中</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1686"/>
        <source>Enable Publishing</source>
        <translation>启用发布</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1687"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>向代理广播帧、原始字节和通知</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1698"/>
        <source>Payload</source>
        <translation>有效载荷</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1699"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>选择发布内容：已解析的仪表板数据或原始 RX 字节</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1709"/>
        <source>Publish Rate (Hz)</source>
        <translation>发布速率 (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1710"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>每秒发布次数 (1-30 Hz)。较高速率会增加代理负载；仪表板数据受速率限制，因此缓慢的代理不会阻塞帧解析。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1722"/>
        <source>Topic Base</source>
        <translation>主题基础</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1723"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/device</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1724"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>用于帧和原始字节发布的基础主题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1735"/>
        <source>Script Topic</source>
        <translation>脚本主题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1736"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1760"/>
        <source>Defaults to Topic Base when empty</source>
        <translation>为空时默认为基础主题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1737"/>
        <source>Topic the user script publishes to</source>
        <translation>用户脚本发布到的主题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1747"/>
        <source>Publish Notifications</source>
        <translation>发布通知</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1748"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>将仪表板通知镜像到专用主题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1759"/>
        <source>Notification Topic</source>
        <translation>通知主题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1761"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>仪表板通知镜像到的主题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1774"/>
        <source>Broker</source>
        <translation>代理</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1784"/>
        <source>Hostname</source>
        <translation>主机名</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1785"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1786"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>MQTT 代理的主机名或 IP 地址</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1795"/>
        <source>Port</source>
        <translation>端口</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1796"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>代理公开的 TCP 端口（1883 明文，8883 TLS）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1806"/>
        <source>Custom Client ID</source>
        <translation>自定义客户端 ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1808"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>关闭：每次加载项目时生成新的随机 ID。开启：使用下方的 ID。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1819"/>
        <source>Client ID</source>
        <translation>客户端 ID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1820"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>连接时发送给代理的标识符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1833"/>
        <source>Protocol Version</source>
        <translation>协议版本</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1834"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>CONNECT 时使用的 MQTT 协议版本</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1843"/>
        <source>Keep Alive (s)</source>
        <translation>保持连接 (秒)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1844"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>空闲时 PINGREQ 数据包之间的间隔秒数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1853"/>
        <source>Clean Session</source>
        <translation>清除会话</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1854"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>CONNECT 时丢弃任何持久会话状态</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1869"/>
        <source>Username</source>
        <translation>用户名</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1870"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>用于 Broker 身份验证的用户名（留空表示匿名）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1880"/>
        <source>Password</source>
        <translation>密码</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1881"/>
        <source>Password for broker authentication</source>
        <translation>用于 Broker 身份验证的密码</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1892"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1902"/>
        <source>Use SSL/TLS</source>
        <translation>使用 SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1903"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>通过 TLS 隧道传输 Broker 连接</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1916"/>
        <source>Protocol</source>
        <translation>协议</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1917"/>
        <source>Negotiated TLS protocol family</source>
        <translation>协商的 TLS 协议族</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1927"/>
        <source>Peer Verify</source>
        <translation>对等方验证</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1928"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Broker 证书链的验证严格程度</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1938"/>
        <source>Verify Depth</source>
        <translation>验证深度</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1939"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>接受的最大证书链长度(0 = 无限制)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2212"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2826"/>
        <source>Device %1</source>
        <translation>设备 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2230"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2351"/>
        <source>Input Device</source>
        <translation>输入设备</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2231"/>
        <source>Select which connected device provides data for this group</source>
        <translation>选择为此组提供数据的已连接设备</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2262"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>自动检测读取 JPEG/PNG 魔术字节;手动使用显式起始/结束序列</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2272"/>
        <source>Start Sequence (Hex)</source>
        <translation>起始序列(十六进制)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2273"/>
        <source>e.g. FF D8 FF</source>
        <translation>例如 FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2274"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>标记图像帧起始的十六进制字节</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2283"/>
        <source>End Sequence (Hex)</source>
        <translation>结束序列(十六进制)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2284"/>
        <source>e.g. FF D9</source>
        <translation>例如 FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2285"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>标记图像帧结束的十六进制字节</translation>
    </message>
    <message>
        <source>Identity</source>
        <translation type="vanished">标识</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2361"/>
        <source>Device Name</source>
        <translation>设备名称</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2362"/>
        <source>Device 1</source>
        <translation>设备 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2363"/>
        <source>Human-readable name for this input device</source>
        <translation>此输入设备的易读名称</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2372"/>
        <source>Bus Type</source>
        <translation>总线类型</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2373"/>
        <source>Select the hardware interface for this input device</source>
        <translation>选择此输入设备的硬件接口</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Serial Port</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Network Socket</source>
        <translation>网络套接字</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2375"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>Audio Input</source>
        <translation>音频输入</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>CAN Bus</source>
        <translation>CAN 总线</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2377"/>
        <source>Raw USB</source>
        <translation>原始 USB</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2378"/>
        <source>HID Device</source>
        <translation>HID 设备</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2378"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2378"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT 订阅者</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2545"/>
        <source>Connection Settings</source>
        <translation>连接设置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2793"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3067"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4654"/>
        <source>General Information</source>
        <translation>常规信息</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2802"/>
        <source>Action Title</source>
        <translation>操作标题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2804"/>
        <source>Untitled Action</source>
        <translation>未命名操作</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2805"/>
        <source>Name or description of this action</source>
        <translation>此操作的名称或描述</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Action Icon</source>
        <translation>操作图标</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2815"/>
        <source>Default Icon</source>
        <translation>默认图标</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>仪表板中为此操作显示的图标</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2844"/>
        <source>Target Device</source>
        <translation>目标设备</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2845"/>
        <source>Select which connected device this action sends data to</source>
        <translation>选择此操作将数据发送到哪个已连接设备</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Data Payload</source>
        <translation>数据负载</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2868"/>
        <source>Send as Binary</source>
        <translation>以二进制发送</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2869"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>触发此操作时发送原始二进制数据</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2880"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2892"/>
        <source>Command</source>
        <translation>命令</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2881"/>
        <source>Transmit Data (Hex)</source>
        <translation>传输数据（十六进制）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2882"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>触发操作时发送的十六进制负载</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2893"/>
        <source>Transmit Data</source>
        <translation>传输数据</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2894"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>触发操作时发送的文本负载</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2905"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4713"/>
        <source>Text Encoding</source>
        <translation>文本编码</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2906"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>用于序列化文本负载的字符编码</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2930"/>
        <source>End-of-Line Sequence</source>
        <translation>行尾序列</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2931"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>附加到消息的行尾字符（例如 </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2943"/>
        <source>Execution Behavior</source>
        <translation>执行行为</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2954"/>
        <source>Auto-Execute on Connect</source>
        <translation>连接时自动执行</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2955"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>设备连接时自动触发此操作</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2961"/>
        <source>Timer Behavior</source>
        <translation>定时器行为</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2970"/>
        <source>Timer Mode</source>
        <translation>定时器模式</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2973"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>选择此操作应何时以及如何自动重复</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2980"/>
        <source>Interval (ms)</source>
        <translation>间隔（毫秒）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2984"/>
        <source>Timer Interval (ms)</source>
        <translation>定时器间隔（毫秒）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2985"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>每次重复触发此操作之间的毫秒数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2992"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2996"/>
        <source>Repeat Count</source>
        <translation>重复次数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2997"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>每次触发时发送命令的次数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3077"/>
        <source>Untitled Dataset</source>
        <translation>未命名数据集</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3078"/>
        <source>Dataset Title</source>
        <translation>数据集标题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3079"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>数据集的名称，用于标记和识别</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3109"/>
        <source>Hide on Dashboard</source>
        <translation>在仪表板上隐藏</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3110"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>隐藏此数据集的独立仪表板磁贴;绘图控件仍可读取其值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3156"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>数据集值范围的下限；当小部件和 FFT 自身范围未设置时，将回退到此值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3169"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>数据集值范围的上限；当小部件和 FFT 自身范围未设置时，将回退到此值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3229"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>选择时间或数据集以驱动绘图中的 X 轴</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3242"/>
        <source>Frequency Analysis</source>
        <translation>频率分析</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3290"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>选择时间(默认)或任何数据集的值来驱动 Y 轴 -- 绑定到例如 RPM 时可生成坎贝尔图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3341"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3430"/>
        <source>Minimum Value (optional)</source>
        <translation>最大值（可选）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3342"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>数据归一化的下限；未设置时回退到数据集值范围</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3354"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3443"/>
        <source>Maximum Value (optional)</source>
        <translation>最大值（可选）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3355"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>数据归一化的上限；未设置时回退到数据集值范围</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3431"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>仪表或条形图范围的下限；未设置时回退到数据集值范围</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3444"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>仪表或条形图范围的上限；未设置时回退到数据集值范围</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3609"/>
        <source>Painter Widget</source>
        <translation>绘图控件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4964"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>每个数据集的原始值和转换值（只读）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4974"/>
        <source>Shared table defined in this project</source>
        <translation>此项目中定义的共享表</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5326"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>移除 1 个目标组或数据集已不存在的控件引用?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5327"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>移除 %1 个目标组或数据集已不存在的控件引用?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5332"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>此操作仅影响工作区磁贴布局;不会删除任何组、数据集或数据。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5335"/>
        <source>Clean Up Workspaces</source>
        <translation>清理工作区</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3123"/>
        <source>Frame Index</source>
        <translation>帧索引</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3124"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>用于在时间上对齐数据集的帧位置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3133"/>
        <source>Measurement Unit</source>
        <translation>测量单位</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3134"/>
        <source>Volts, Amps, etc.</source>
        <translation>伏特、安培等</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3135"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>测量单位，例如伏特或安培（可选）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3183"/>
        <source>Plot Settings</source>
        <translation>绘图设置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3206"/>
        <source>Enable Plot Widget</source>
        <translation>启用绘图控件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3208"/>
        <source>Plot data in real-time</source>
        <translation>实时绘制数据</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3228"/>
        <source>X-Axis Source</source>
        <translation>X 轴数据源</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">选择用于绘图 X 轴的数据集</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">最小绘图值（可选）</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">绘图显示范围的下限</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">最大绘图值（可选）</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">绘图显示范围的上限</translation>
    </message>
    <message>
        <source>FFT Configuration</source>
        <translation type="vanished">FFT 配置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3253"/>
        <source>Enable FFT Analysis</source>
        <translation>启用 FFT 分析</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3254"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>对数据集执行频域分析</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3264"/>
        <source>Enable Waterfall Plot</source>
        <translation>启用瀑布图控件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3265"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>显示频率内容随时间变化的滚动频谱图 (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3289"/>
        <source>Waterfall Y Axis</source>
        <translation>瀑布图 Y 轴</translation>
    </message>
    <message>
        <source>Choose Time (default) or any dataset whose value drives the Y axis — produces a Campbell diagram when bound to e.g. RPM</source>
        <translation type="vanished">选择时间(默认)或任何数据集的值来驱动 Y 轴 — 绑定到例如 RPM 时可生成坎贝尔图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3318"/>
        <source>FFT Window Size</source>
        <translation>FFT 窗口大小</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3319"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>每次 FFT 计算窗口使用的采样数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3330"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>FFT 采样率（Hz，必填）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3331"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>用于 FFT 的采样频率（单位：Hz）</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">最小值（推荐）</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">数据归一化的下限</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">最大值（推荐）</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">数据归一化的上限</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3380"/>
        <source>Widget Settings</source>
        <translation>控件设置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3403"/>
        <source>Widget</source>
        <translation>控件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3404"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>选择用于显示此数据集的可视化控件</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">最小显示值（必填）</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">仪表或条形图显示范围的下限</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">最大显示值（必填）</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">仪表或条形图显示范围的上限</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3460"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3622"/>
        <source>Auto</source>
        <translation>自动</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3461"/>
        <source>Tick Count</source>
        <translation>刻度数量</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3465"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>刻度盘上的主刻度数量（0 = 自动适应组件大小）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3484"/>
        <source>Label Format</source>
        <translation>标签格式</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3485"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>刻度标签和数值显示使用的小数位数或记数法</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">显示数值指示器</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">切换显示在组件下方或旁边的带框数值读数</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">报警设置</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">启用报警</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">当数值超出报警阈值时触发视觉报警</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">低阈值</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">当数值低于此阈值时触发视觉报警</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">高阈值</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">当数值超过此阈值时触发视觉报警</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3522"/>
        <source>LED Display Settings</source>
        <translation>LED 显示设置</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3533"/>
        <source>Show in LED Panel</source>
        <translation>在 LED 面板中显示</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3534"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>使用 LED 显示启用视觉状态监控</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3545"/>
        <source>LED On Threshold (required)</source>
        <translation>LED 开启阈值(必填)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3546"/>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation>当值达到或超过此阈值时 LED 点亮</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Off</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Auto Start</source>
        <translation>自动启动</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Start on Trigger</source>
        <translation>触发时启动</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Toggle on Trigger</source>
        <translation>触发时切换</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3568"/>
        <source>Repeat N Times</source>
        <translation>重复 N 次</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3572"/>
        <source>Plain Text (UTF8)</source>
        <translation>纯文本 (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3572"/>
        <source>Hexadecimal</source>
        <translation>十六进制</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3572"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3573"/>
        <source>Binary (Direct)</source>
        <translation>二进制（直接）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3580"/>
        <source>No Checksum</source>
        <translation>无校验和</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3585"/>
        <source>End Delimiter Only</source>
        <translation>仅结束定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3585"/>
        <source>Start Delimiter Only</source>
        <translation>仅起始定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3586"/>
        <source>Start + End Delimiter</source>
        <translation>起始 + 结束定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3586"/>
        <source>No Delimiters</source>
        <translation>无定界符</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Button</source>
        <translation>按钮</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Slider</source>
        <translation>滑块</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Toggle</source>
        <translation>开关</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3596"/>
        <source>Text Field</source>
        <translation>文本字段</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3597"/>
        <source>Knob</source>
        <translation>旋钮</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3602"/>
        <source>Data Grid</source>
        <translation>数据网格</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3603"/>
        <source>GPS Map</source>
        <translation>GPS 地图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3604"/>
        <source>Gyroscope</source>
        <translation>陀螺仪</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3605"/>
        <source>Multiple Plot</source>
        <translation>多重绘图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3606"/>
        <source>Accelerometer</source>
        <translation>加速度计</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3607"/>
        <source>3D Plot</source>
        <translation>3D 绘图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3608"/>
        <source>Image View</source>
        <translation>图像视图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3610"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3614"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3631"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3615"/>
        <source>Bar</source>
        <translation>条形图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Gauge</source>
        <translation>仪表盘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3617"/>
        <source>Compass</source>
        <translation>罗盘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3618"/>
        <source>Meter</source>
        <translation>仪表</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">温度计</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3623"/>
        <source>Integer (0 decimals)</source>
        <translation>整数（0 位小数）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3624"/>
        <source>1 decimal</source>
        <translation>1 位小数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3625"/>
        <source>2 decimals</source>
        <translation>2 位小数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3626"/>
        <source>3 decimals</source>
        <translation>3 位小数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3627"/>
        <source>Scientific</source>
        <translation>科学计数法</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3632"/>
        <source>New Line (\n)</source>
        <translation>换行符 (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3633"/>
        <source>Carriage Return (\r)</source>
        <translation>回车符 (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3634"/>
        <source>CRLF (\r\n)</source>
        <translation>回车换行符 (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3639"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4664"/>
        <source>Label</source>
        <translation>标签</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4665"/>
        <source>Display label</source>
        <translation>显示标签</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4675"/>
        <source>Button Icon</source>
        <translation>按钮图标</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4684"/>
        <source>Colorize Icon</source>
        <translation>图标着色</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4685"/>
        <source>Tint the icon with the button color</source>
        <translation>使用按钮颜色为图标着色</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4702"/>
        <source>Initial Value</source>
        <translation>初始值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4714"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>transmit() 返回字符串值时使用的字符编码</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4732"/>
        <source>Value Range</source>
        <translation>数值范围</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3155"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4742"/>
        <source>Minimum Value</source>
        <translation>最小值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3168"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4751"/>
        <source>Maximum Value</source>
        <translation>最大值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4760"/>
        <source>Step Size</source>
        <translation>步进大小</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="526"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="535"/>
        <source>Lock Project</source>
        <translation>锁定项目</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="527"/>
        <source>Choose a password to lock the project:</source>
        <translation>选择密码以锁定项目：</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="535"/>
        <source>Confirm the password:</source>
        <translation>确认密码：</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="540"/>
        <source>Passwords do not match</source>
        <translation>密码不匹配</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="541"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>两次输入的密码不匹配。项目未锁定。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="575"/>
        <source>Unlock Project</source>
        <translation>解锁项目</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="576"/>
        <source>Enter the project password:</source>
        <translation>输入项目密码：</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="586"/>
        <source>Incorrect password</source>
        <translation>密码错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="587"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>输入的密码与项目文件中存储的密码不匹配。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="619"/>
        <source>New Project</source>
        <translation>新建项目</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">采样</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1169"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>多数据源需要 Pro 许可证</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1170"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro 允许同时连接多个设备。请升级以解锁此功能。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1183"/>
        <source>Device %1</source>
        <translation>设备 %1</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(副本)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1467"/>
        <source>Do you want to save your changes?</source>
        <translation>要保存更改吗?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1468"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>此项目中有未保存的修改!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="395"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="405"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="419"/>
        <source>Project error</source>
        <translation>项目错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="395"/>
        <source>Project title cannot be empty!</source>
        <translation>项目标题不能为空!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="405"/>
        <source>You need to add at least one group!</source>
        <translation>至少需要添加一个组!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="419"/>
        <source>You need to add at least one dataset!</source>
        <translation>至少需要添加一个数据集!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="467"/>
        <source>Your project needs a title</source>
        <translation>项目需要标题</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="469"/>
        <source>Add a group to get started</source>
        <translation>添加组以开始</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="471"/>
        <source>Add a dataset to a group</source>
        <translation>向组添加数据集</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="485"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>打开树顶部的项目视图并输入名称。可随时重命名项目。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="488"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>组将数据集组织到仪表板小部件中。使用上方工具栏中的"组"按钮创建一个,然后向其添加数据集。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="492"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>数据集是显示在仪表板上的值。在树中选择一个组，然后使用工具栏中的"数据集"按钮添加数据集。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="773"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="821"/>
        <source>Time</source>
        <translation>时间</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1219"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>是否删除数据源"%1"？</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1220"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>使用此数据源的组将移至默认数据源。此操作无法撤销。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1507"/>
        <source>Save Serial Studio Project</source>
        <translation>保存 Serial Studio 项目</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1509"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2132"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Serial Studio 项目文件 (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1531"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1767"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2123"/>
        <source>Untitled Project</source>
        <translation>未命名项目</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1777"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2284"/>
        <source>Device A</source>
        <translation>设备 A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1952"/>
        <source>Select Project File</source>
        <translation>选择项目文件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1954"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>项目文件 (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2003"/>
        <source>JSON validation error</source>
        <translation>JSON 验证错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2098"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>项目已从早期文件格式升级</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2099"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>此项目保存时的架构版本为 %1；当前版本为 %2。所有新字段已应用默认值。请保存项目以锁定升级。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2130"/>
        <source>Save Imported Project</source>
        <translation>保存导入的项目</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2329"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>多源项目需要 Pro 许可证</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2330"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>此项目包含多个数据源。仅加载了第一个源。使用多源项目需要 Serial Studio Pro 许可证。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2564"/>
        <source>Workspace IDs remapped on load</source>
        <translation>工作区 ID 已在加载时重新映射</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2565"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 个自定义工作区 ID 与新的保留自动范围重叠，已移动到用户范围。保存项目以使重新映射生效。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2710"/>
        <source>Legacy frame parser function updated</source>
        <translation>旧版帧解析器函数已更新</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2711"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>您的项目使用了带有"separator"参数的旧版帧解析器函数。已自动迁移到新格式。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2909"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>是否删除组"%1"？</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2910"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2961"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2996"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3763"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>此操作无法撤销。是否继续？</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2960"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>是否删除操作"%1"？</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2995"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>是否删除数据集"%1"？</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1（副本）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3671"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3710"/>
        <source>Output Controls</source>
        <translation>输出控件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3723"/>
        <source>New Button</source>
        <translation>新建按钮</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3726"/>
        <source>New Slider</source>
        <translation>新建滑块</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3729"/>
        <source>New Toggle</source>
        <translation>新建开关</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3732"/>
        <source>New Text Field</source>
        <translation>新建文本字段</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3735"/>
        <source>New Knob</source>
        <translation>新建旋钮</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3762"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>是否删除输出控件"%1"？</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3935"/>
        <source>Group</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3954"/>
        <source>New Dataset</source>
        <translation>新建数据集</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3957"/>
        <source>New Plot</source>
        <translation>新建绘图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3961"/>
        <source>New FFT Plot</source>
        <translation>新建 FFT 绘图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3965"/>
        <source>New Level Indicator</source>
        <translation>新建电平指示器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3969"/>
        <source>New Gauge</source>
        <translation>新建仪表</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3973"/>
        <source>New Compass</source>
        <translation>新建罗盘</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3979"/>
        <source>New Meter</source>
        <translation>新建仪表</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">新建温度计</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3983"/>
        <source>New LED Indicator</source>
        <translation>新建 LED 指示器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3987"/>
        <source>New Waterfall</source>
        <translation>新建瀑布图</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4057"/>
        <source>Channel %1</source>
        <translation>通道 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4130"/>
        <source>New Action</source>
        <translation>新建操作</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4273"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>确定要更改组级别部件吗？</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4275"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>此组的现有数据集将被删除</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4339"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4340"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4341"/>
        <source>Accelerometer %1</source>
        <translation>加速度计 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4356"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4356"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4356"/>
        <source>Gyro %1</source>
        <translation>陀螺仪 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4371"/>
        <source>Latitude</source>
        <translation>纬度</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4371"/>
        <source>Longitude</source>
        <translation>经度</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4371"/>
        <source>Altitude</source>
        <translation>海拔</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4386"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4400"/>
        <source>X</source>
        <translation>X</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4386"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4400"/>
        <source>Y</source>
        <translation>Y</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4386"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4400"/>
        <source>Z</source>
        <translation>Z</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4604"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5500"/>
        <source>Workspace</source>
        <translation>工作区</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4770"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4977"/>
        <source>Shared Table</source>
        <translation>共享表</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4852"/>
        <source>register</source>
        <translation>寄存器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4977"/>
        <source>New Shared Table</source>
        <translation>新建共享表</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4977"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4995"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5014"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5038"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5065"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5084"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5107"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5130"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5500"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5521"/>
        <source>Name:</source>
        <translation>名称:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4995"/>
        <source>Rename Table</source>
        <translation>重命名表</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5014"/>
        <source>Rename Group</source>
        <translation>重命名组</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5038"/>
        <source>Rename Dataset</source>
        <translation>重命名数据集</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5065"/>
        <source>Rename Data Source</source>
        <translation>重命名数据源</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5084"/>
        <source>Rename Action</source>
        <translation>重命名操作</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5106"/>
        <source>New Register</source>
        <translation>新建寄存器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5130"/>
        <source>Rename Register</source>
        <translation>重命名寄存器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5169"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5194"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6048"/>
        <source>This action cannot be undone.</source>
        <translation>此操作无法撤销。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5170"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>这将删除 %1 个寄存器以及表格。此操作无法撤销。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5173"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5193"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6047"/>
        <source>Delete "%1"?</source>
        <translation>删除"%1"？</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5176"/>
        <source>Delete Table</source>
        <translation>删除表格</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5196"/>
        <source>Delete Register</source>
        <translation>删除寄存器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5220"/>
        <source>Export Table</source>
        <translation>导出表格</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5222"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5266"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 文件 (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5264"/>
        <source>Import Table</source>
        <translation>导入表格</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5500"/>
        <source>New Workspace</source>
        <translation>新建工作区</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5521"/>
        <source>Rename Workspace</source>
        <translation>重命名工作区</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5610"/>
        <source>Overview</source>
        <translation>概览</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5620"/>
        <source>All Data</source>
        <translation>所有数据</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5804"/>
        <source>Discard workspace customisations?</source>
        <translation>放弃工作区自定义设置?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5805"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>关闭自定义将丢弃您的编辑内容,并根据项目的组重新构建工作区列表。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5808"/>
        <source>Customize Workspaces</source>
        <translation>自定义工作区</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6050"/>
        <source>Delete Workspace</source>
        <translation>删除工作区</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6391"/>
        <source>File save error</source>
        <translation>文件保存错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2167"/>
        <source>File open error</source>
        <translation>文件打开错误</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="907"/>
        <source>Import Protocol Buffers File</source>
        <translation>导入 Protocol Buffers 文件</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="909"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Proto 文件 (*.proto);;所有文件 (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="944"/>
        <source>Failed to open proto file: %1</source>
        <translation>无法打开 proto 文件：%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="945"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>请验证文件路径和读取权限,然后重试。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Protobuf Import Error</source>
        <translation>Protobuf 导入错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="962"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>解析 proto 文件第 %1 行失败:%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="963"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>仅支持 proto3 语法。请验证文件格式并重试。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="970"/>
        <source>Proto file contains no message definitions</source>
        <translation>Proto 文件不包含消息定义</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="971"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>所选文件没有可导入的 `message` 块。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Protobuf Import Warning</source>
        <translation>Protobuf 导入警告</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">加载导入的项目失败</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">无法加载生成的项目 JSON。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1011"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>已成功从 proto 文件导入 %1 个消息和 %2 个字段。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>The project editor is now open for customization.</source>
        <translation>项目编辑器现已打开以供自定义。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1016"/>
        <source>Protobuf Import Complete</source>
        <translation>Protobuf 导入完成</translation>
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
        <translation>十六进制输入无效</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="159"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>请输入有效的十六进制字节。

有效格式：01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="165"/>
        <source>No transmit function code to evaluate.</source>
        <translation>没有可评估的传输函数代码。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="185"/>
        <source>transmit function is not callable</source>
        <translation>传输函数不可调用</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="249"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="250"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="251"/>
        <source>Evaluate</source>
        <translation>评估</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="252"/>
        <source>Input Value</source>
        <translation>输入值</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="253"/>
        <source>Transmit Function Output</source>
        <translation>传输函数输出</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="254"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="278"/>
        <source>Enter value to transmit…</source>
        <translation>输入要传输的值…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="255"/>
        <source>Raw string output appears here</source>
        <translation>原始字符串输出显示在此处</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="256"/>
        <source>Hex byte output appears here</source>
        <translation>十六进制字节输出显示在此处</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="259"/>
        <source>Test Transmit Function</source>
        <translation>测试传输函数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="272"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>输入十六进制字节(例如 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="383"/>
        <source>(empty) No data returned</source>
        <translation>(空)未返回数据</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="385"/>
        <source>0 bytes</source>
        <translation>0 字节</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="426"/>
        <source>%1 byte(s)</source>
        <translation>%1 字节</translation>
    </message>
</context>
<context>
    <name>DataTablesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="33"/>
        <source>Shared Memory</source>
        <translation>共享内存</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="149"/>
        <source>Add Shared Table</source>
        <translation>添加共享表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="151"/>
        <source>Add shared table</source>
        <translation>添加共享表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="160"/>
        <source>Help</source>
        <translation>帮助</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="165"/>
        <source>Open help documentation for shared memory</source>
        <translation>打开共享内存帮助文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="174"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="175"/>
        <source>Description</source>
        <translation>描述</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="176"/>
        <source>Entries</source>
        <translation>条目</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="274"/>
        <source>No shared tables.</source>
        <translation>无共享表。</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>会话</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>打开会话文件</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>关闭会话文件</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>回放</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>在仪表板上回放选定的会话</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>解锁会话文件以删除会话</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>删除选定的会话</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>解锁</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>锁定</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>解锁会话文件以允许删除</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>设置密码以防止会话被删除</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>导出 CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>将选定会话导出为 CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>导出 PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>为选定会话生成 PDF 报告</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>恢复项目</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>从此会话文件恢复项目文件</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>加载会话中…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>处理中…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="109"/>
        <source>Pro features detected in this project.</source>
        <translation>此项目中检测到 Pro 功能。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="111"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>正在使用备用组件。购买许可证以解锁完整功能。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="139"/>
        <source>Plots</source>
        <translation>图表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="144"/>
        <source>Plot</source>
        <translation>绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="148"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>切换此数据集的 2D 绘图可视化</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="160"/>
        <source>FFT Plot</source>
        <translation>FFT 绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>切换 FFT 绘图以可视化频率内容</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="175"/>
        <source>Waterfall</source>
        <translation>瀑布图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="179"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>切换瀑布图(频谱图)绘制 — 使用 FFT 设置</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="196"/>
        <source>Widgets</source>
        <translation>组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="202"/>
        <source>Bar/Level</source>
        <translation>条形图/电平</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="206"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>切换此数据集的条形图/电平指示器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="217"/>
        <source>Gauge</source>
        <translation>仪表盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="222"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>切换仪表盘控件以显示模拟样式</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="234"/>
        <source>Compass</source>
        <translation>罗盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="238"/>
        <source>Toggle compass widget for directional data</source>
        <translation>切换罗盘控件以显示方向数据</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="249"/>
        <source>Meter</source>
        <translation>仪表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="254"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>切换模拟仪表(半圆弧)控件</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">温度计</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">切换温度计控件以显示温度数据</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="265"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="270"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>切换 LED 指示器以显示二进制或阈值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="290"/>
        <source>Behavior</source>
        <translation>行为</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="295"/>
        <source>Alarm Bands</source>
        <translation>报警区间</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="304"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation>为此数据集的仪表定义带有严重性等级的彩色数值范围。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="310"/>
        <source>Transform</source>
        <translation>变换</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>编辑值变换表达式以进行校准、滤波或单位转换</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="332"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>复制此数据集及其配置</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="337"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="340"/>
        <source>Delete this dataset from the group</source>
        <translation>从组中删除此数据集</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="37"/>
        <source>Support Serial Studio</source>
        <translation>支持 Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="86"/>
        <source>Support the development of %1!</source>
        <translation>支持 %1 的开发！</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="97"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio 是由志愿者支持的免费开源软件。请考虑捐赠或获取 Pro 许可证以支持开发工作 :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="110"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>您也可以通过分享、报告错误和提出新功能来支持此项目！</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="124"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="135"/>
        <source>Donate</source>
        <translation>捐赠</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="150"/>
        <source>Get Serial Studio Pro</source>
        <translation>获取 Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="127"/>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="128"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="362"/>
        <source>Downloading updates</source>
        <translation>正在下载更新</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="455"/>
        <source>Time remaining</source>
        <translation>剩余时间</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="129"/>
        <source>unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="228"/>
        <source>Cannot find downloaded update!</source>
        <translation>找不到已下载的更新!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="244"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="245"/>
        <source>Download complete!</source>
        <translation>下载完成!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="253"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>单击"确定"开始安装更新</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="255"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>要安装更新，可能需要退出应用程序。</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="246"/>
        <source>The installer opens separately</source>
        <translation>安装程序将单独打开</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>要安装更新，可能需要退出应用程序。这是强制更新，现在退出将关闭应用程序。</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="275"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>单击"打开"按钮应用更新</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="288"/>
        <source>Updater</source>
        <translation>更新程序</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="292"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>确定要取消下载吗？</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="294"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>确定要取消下载吗？这是强制更新，现在退出将关闭应用程序</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="349"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="356"/>
        <source>%1 bytes</source>
        <translation>%1 字节</translation>
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
        <translation>正在下载更新</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Time Remaining</source>
        <translation>剩余时间</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="407"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="431"/>
        <source>about %1 hours</source>
        <translation>大约 %1 小时</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="433"/>
        <source>about one hour</source>
        <translation>大约一小时</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="441"/>
        <source>%1 minutes</source>
        <translation>%1 分钟</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="443"/>
        <source>1 minute</source>
        <translation>1 分钟</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="450"/>
        <source>%1 seconds</source>
        <translation>%1 秒</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="452"/>
        <source>1 second</source>
        <translation>1 秒</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">剩余时间：0 分钟</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">打开</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="34"/>
        <source>Examples Browser</source>
        <translation>示例浏览器</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="120"/>
        <source>Back</source>
        <translation>返回</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="152"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="173"/>
        <source>Download &amp;&amp; Open</source>
        <translation>下载并打开</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="188"/>
        <source>View on GitHub</source>
        <translation>在 GitHub 上查看</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="91"/>
        <source>Search in Examples…</source>
        <translation>在示例中搜索…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="244"/>
        <source>Fetching examples…</source>
        <translation>正在获取示例…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="567"/>
        <source>Loading...</source>
        <translation>加载中...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>No README available.</source>
        <translation>无可用 README。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="608"/>
        <source>Copied to Clipboard</source>
        <translation>已复制到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="671"/>
        <source>No screenshot available</source>
        <translation>无可用截图</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="703"/>
        <source>Details</source>
        <translation>详情</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="732"/>
        <source>Info</source>
        <translation>信息</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="755"/>
        <source>Category:</source>
        <translation>类别：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="768"/>
        <source>Difficulty:</source>
        <translation>难度：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="786"/>
        <source>Project:</source>
        <translation>项目：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="828"/>
        <source>No Results Found</source>
        <translation>未找到结果</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="839"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>请检查拼写或尝试其他搜索词。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="854"/>
        <source>%1 examples</source>
        <translation>%1 个示例</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="863"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="32"/>
        <source>Extension Manager</source>
        <translation>扩展管理器</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="119"/>
        <source>Refresh</source>
        <translation>刷新</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="133"/>
        <source>Repos</source>
        <translation>仓库</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="163"/>
        <source>Repository Settings</source>
        <translation>仓库设置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="175"/>
        <source>Back</source>
        <translation>返回</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="216"/>
        <source>Install</source>
        <translation>安装</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="233"/>
        <source>Uninstall</source>
        <translation>卸载</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="260"/>
        <source>Run</source>
        <translation>运行</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="284"/>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="318"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="78"/>
        <source>Search extensions…</source>
        <translation>搜索扩展…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="368"/>
        <source>Fetching extensions…</source>
        <translation>正在获取扩展…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="605"/>
        <source>Running</source>
        <translation>运行中</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Update</source>
        <translation>更新</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="627"/>
        <source>Installed</source>
        <translation>已安装</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="644"/>
        <source>Unavailable</source>
        <translation>不可用</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="823"/>
        <source>No description available.</source>
        <translation>无可用描述。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="864"/>
        <source>Details</source>
        <translation>详细信息</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="885"/>
        <source>Type:</source>
        <translation>类型：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="898"/>
        <source>Author:</source>
        <translation>作者：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="910"/>
        <source>Version:</source>
        <translation>版本：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="922"/>
        <source>License:</source>
        <translation>许可证：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="983"/>
        <source>No preview</source>
        <translation>无预览</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1012"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>插件输出</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1042"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>尚无输出。运行插件以在此处查看其日志。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1077"/>
        <source>No preview available</source>
        <translation>无可用预览</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1121"/>
        <source>Repositories</source>
        <translation>仓库</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1134"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>添加远程仓库的 URL 或本地文件夹路径。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1171"/>
        <source>LOCAL</source>
        <translation>本地</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1228"/>
        <source>URL or local path…</source>
        <translation>URL 或本地路径…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1259"/>
        <source>Browse…</source>
        <translation>浏览…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1236"/>
        <source>Add</source>
        <translation>添加</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1296"/>
        <source>No Results Found</source>
        <translation>未找到结果</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1307"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>请检查拼写或尝试其他搜索词。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1331"/>
        <source>No Extensions Available</source>
        <translation>无可用扩展</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1342"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>在仓库设置中添加仓库 URL 或本地路径,然后刷新。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1357"/>
        <source>%1 extensions</source>
        <translation>%1 个扩展</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1366"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="157"/>
        <source>Interpolation: %1</source>
        <translation>插值:%1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="185"/>
        <source>Show Area Under Plot</source>
        <translation>显示曲线下方区域</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="203"/>
        <source>Show X Axis Label</source>
        <translation>显示 X 轴标签</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="215"/>
        <source>Show Y Axis Label</source>
        <translation>显示 Y 轴标签</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="233"/>
        <source>Show Crosshair</source>
        <translation>显示十字准线</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="240"/>
        <source>Resume</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="259"/>
        <source>Reset View</source>
        <translation>重置视图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="265"/>
        <source>Axis Range Settings</source>
        <translation>坐标轴范围设置</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="294"/>
        <source>Magnitude (dB)</source>
        <translation>幅度 (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="295"/>
        <source>Frequency (Hz)</source>
        <translation>频率 (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>将项目和 CSV 文件拖放到此处</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="34"/>
        <source>File Transmission</source>
        <translation>文件传输</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="102"/>
        <source>Transfer Protocol:</source>
        <translation>传输协议:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="135"/>
        <source>File Selection:</source>
        <translation>文件选择:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="152"/>
        <source>Select File…</source>
        <translation>选择文件…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="170"/>
        <source>Transmission Interval:</source>
        <translation>传输间隔：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="196"/>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="272"/>
        <source>msecs</source>
        <translation>毫秒</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="206"/>
        <source>Block Size:</source>
        <translation>块大小：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="234"/>
        <source>bytes</source>
        <translation>字节</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="244"/>
        <source>Timeout:</source>
        <translation>超时：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="282"/>
        <source>Max Retries:</source>
        <translation>最大重试次数：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="340"/>
        <source>Progress: %1%</source>
        <translation>进度：%1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="373"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 字节</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="381"/>
        <source>Errors: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="461"/>
        <source>Activity Log</source>
        <translation>活动日志</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="465"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Pause Transmission</source>
        <translation>暂停传输</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="420"/>
        <source>Resume Transmission</source>
        <translation>恢复传输</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Stop Transmission</source>
        <translation>停止传输</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="424"/>
        <source>Begin Transmission</source>
        <translation>开始传输</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="159"/>
        <source>Frame Parser</source>
        <translation>帧解析器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="166"/>
        <source>Device %1</source>
        <translation>设备 %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="395"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1402"/>
        <source>Output Panel</source>
        <translation>输出面板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="439"/>
        <source>Control</source>
        <translation>控制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="462"/>
        <source>Table</source>
        <translation>表格</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="478"/>
        <source>%1 regs</source>
        <translation>%1 个寄存器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="479"/>
        <source>empty</source>
        <translation>空</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="504"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 发布者</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="905"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>打开此数据集的转换代码编辑器。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1184"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1187"/>
        <source>Dataset Container</source>
        <translation>数据集容器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1196"/>
        <source>Multi-Plot</source>
        <translation>多重绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1199"/>
        <source>Multiple Plot</source>
        <translation>多重绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1208"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1211"/>
        <source>Accelerometer</source>
        <translation>加速度计</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1220"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1223"/>
        <source>Gyroscope</source>
        <translation>陀螺仪</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1232"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1235"/>
        <source>GPS Map</source>
        <translation>GPS 地图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1243"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1246"/>
        <source>3D Plot</source>
        <translation>3D 绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1254"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1257"/>
        <source>Image View</source>
        <translation>图像视图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1266"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1269"/>
        <source>Painter Widget</source>
        <translation>绘图控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1278"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1281"/>
        <source>Data Grid</source>
        <translation>数据网格</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1294"/>
        <source>Generic</source>
        <translation>通用</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1307"/>
        <source>Plot</source>
        <translation>绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1320"/>
        <source>FFT Plot</source>
        <translation>FFT 绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1333"/>
        <source>Gauge</source>
        <translation>仪表盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1346"/>
        <source>Level Indicator</source>
        <translation>电平指示器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1359"/>
        <source>Compass</source>
        <translation>罗盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1372"/>
        <source>Meter</source>
        <translation>仪表</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">温度计</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1385"/>
        <source>LED Indicator</source>
        <translation>LED 指示灯</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1414"/>
        <source>Slider</source>
        <translation>滑块</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1427"/>
        <source>Toggle</source>
        <translation>开关</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1440"/>
        <source>Knob</source>
        <translation>旋钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1453"/>
        <source>Text Field</source>
        <translation>文本字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1466"/>
        <source>Button</source>
        <translation>按钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1490"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1565"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1652"/>
        <source>Add Group</source>
        <translation>添加组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1505"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1580"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1667"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1712"/>
        <source>Add Dataset</source>
        <translation>添加数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1519"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1594"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1681"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1933"/>
        <source>Add Output</source>
        <translation>添加输出</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1535"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1607"/>
        <source>Add Action</source>
        <translation>添加动作</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1544"/>
        <source>Add Data Source</source>
        <translation>添加数据源</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1551"/>
        <source>Add Data Table</source>
        <translation>添加数据表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1618"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1753"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1820"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1948"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1982"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2038"/>
        <source>Rename…</source>
        <translation>重命名…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1626"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1783"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1853"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1905"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1956"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Duplicate</source>
        <translation>复制</translation>
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
        <translation>删除…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1697"/>
        <source>Edit Frame Parser…</source>
        <translation>编辑帧解析器…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1739"/>
        <source>Edit Painter Code…</source>
        <translation>编辑绘图器代码…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1761"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1829"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1881"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1990"/>
        <source>Move Up</source>
        <translation>上移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1772"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1841"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1893"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2001"/>
        <source>Move Down</source>
        <translation>下移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1809"/>
        <source>Edit Transform Code…</source>
        <translation>编辑转换代码…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2064"/>
        <source>Edit Code…</source>
        <translation>编辑代码…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="222"/>
        <source>Group</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="345"/>
        <source>Action</source>
        <translation>操作</translation>
    </message>
    <message>
        <source>No groups defined yet</source>
        <translation type="vanished">尚未定义组</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="102"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="249"/>
        <source>Undo</source>
        <translation>撤销</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="109"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="263"/>
        <source>Redo</source>
        <translation>重做</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="118"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="288"/>
        <source>Cut</source>
        <translation>剪切</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="123"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="301"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="314"/>
        <source>Paste</source>
        <translation>粘贴</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="135"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="145"/>
        <source>Format Document</source>
        <translation>格式化文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="152"/>
        <source>Format Selection</source>
        <translation>格式化选区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="223"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="228"/>
        <source>Reset to the default parsing script</source>
        <translation>重置为默认解析脚本</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="236"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="241"/>
        <source>Import a script file for data parsing</source>
        <translation>导入用于数据解析的脚本文件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="343"/>
        <source>Open help documentation for data parsing</source>
        <translation>打开数据解析帮助文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="365"/>
        <source>Language:</source>
        <translation>语言：</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="419"/>
        <source>Select Template…</source>
        <translation>选择模板…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="432"/>
        <source>Test With Sample Data</source>
        <translation>使用示例数据测试</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="439"/>
        <source>Evaluate</source>
        <translation>评估</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="254"/>
        <source>Undo the last code edit</source>
        <translation>撤销上次代码编辑</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="269"/>
        <source>Redo the previously undone edit</source>
        <translation>重做上次撤销的编辑</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="293"/>
        <source>Cut selected code to clipboard</source>
        <translation>剪切选定代码到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="306"/>
        <source>Copy selected code to clipboard</source>
        <translation>复制选定代码到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="318"/>
        <source>Paste code from clipboard</source>
        <translation>从剪贴板粘贴代码</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FrameParserView.qml" line="338"/>
        <source>Help</source>
        <translation>帮助</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>自动居中</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>绘制轨迹</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>放大</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>缩小</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>显示天气</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>NASA 天气叠加层</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>底图:%1</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="97"/>
        <source>Pro features detected in this project.</source>
        <translation>项目中检测到 Pro 功能。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="99"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>使用备用控件。购买许可证以解锁完整功能。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="129"/>
        <source>Add Dataset</source>
        <translation>添加数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="136"/>
        <source>Dataset</source>
        <translation>数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Add a generic dataset to the current group</source>
        <translation>向当前组添加通用数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="146"/>
        <source>Plot</source>
        <translation>绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="150"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>添加 2D 绘图以可视化数值数据</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="158"/>
        <source>FFT Plot</source>
        <translation>FFT 绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="163"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>添加 FFT 绘图以进行频域可视化</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="169"/>
        <source>Waterfall</source>
        <translation>瀑布图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="174"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>添加瀑布图(频谱图)绘制 — 使用 FFT 设置</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="180"/>
        <source>Bar/Level</source>
        <translation>条形图/电平</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="184"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>添加条形图或电平指示器以显示缩放值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="190"/>
        <source>Gauge</source>
        <translation>仪表盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="195"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>添加仪表盘控件以实现模拟式可视化</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="202"/>
        <source>Compass</source>
        <translation>罗盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="206"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>添加罗盘以显示方向或角度数据</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="212"/>
        <source>Meter</source>
        <translation>仪表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="216"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>添加模拟仪表（半弧形）组件</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">温度计</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">添加温度计组件以显示温度数据</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="223"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="228"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>添加 LED 指示器以显示二进制状态信号</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="241"/>
        <source>Add Control</source>
        <translation>添加控制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="247"/>
        <source>Button</source>
        <translation>按钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="251"/>
        <source>Add a button that sends a command on click</source>
        <translation>添加按钮以在单击时发送命令</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="257"/>
        <source>Slider</source>
        <translation>滑块</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="261"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>添加滑块以发送缩放的数值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="267"/>
        <source>Toggle</source>
        <translation>开关</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="271"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>添加用于开/关命令的开关控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="278"/>
        <source>Text Field</source>
        <translation>文本字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>添加用于输入和发送命令的文本字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="287"/>
        <source>Knob</source>
        <translation>旋钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="291"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>添加用于设定点控制的旋转旋钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="310"/>
        <source>Edit Code</source>
        <translation>编辑代码</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="314"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>编辑绘制此绘图器组件的 JavaScript 代码</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="327"/>
        <source>Duplicate</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="331"/>
        <source>Duplicate the current group and its contents</source>
        <translation>复制当前组及其内容</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="336"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="341"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>删除当前组及其包含的所有数据集</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="376"/>
        <source>ROLL ↔</source>
        <translation>横滚 ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="404"/>
        <source>YAW ↻</source>
        <translation>偏航 ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="432"/>
        <source>PITCH ↕</source>
        <translation>俯仰 ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>HID 设备</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>用途页</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>用途</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>连接游戏手柄、操纵杆、方向盘、飞行控制器和其他 HID 类 USB 设备。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>HID 用途表 (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="33"/>
        <source>Help Center</source>
        <translation>帮助中心</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="95"/>
        <source>Fetching help pages…</source>
        <translation>正在获取帮助页面…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="129"/>
        <source>Search…</source>
        <translation>搜索…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="247"/>
        <source>Loading…</source>
        <translation>加载中…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="289"/>
        <source>Select a page from the sidebar</source>
        <translation>从侧边栏选择页面</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="319"/>
        <source>Copied to Clipboard</source>
        <translation>已复制到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="351"/>
        <source>View Online</source>
        <translation>在线查看</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="370"/>
        <source>%1 pages</source>
        <translation>%1 页</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="379"/>
        <source>Close</source>
        <translation>关闭</translation>
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
        <translation>网络套接字</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>Audio</source>
        <translation>音频</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>CAN Bus</source>
        <translation>CAN 总线</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>USB Device</source>
        <translation>USB 设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>HID Device</source>
        <translation>HID 设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="280"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="281"/>
        <source>MQTT Subscriber</source>
        <translation>MQTT 订阅者</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="631"/>
        <source>Your trial period has ended.</source>
        <translation>试用期已结束。</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="632"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>要继续使用 Serial Studio，请激活许可证。</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="764"/>
        <source>channels</source>
        <translation>声道</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="817"/>
        <source> channels</source>
        <translation>声道</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="965"/>
        <source>Unsigned 8-bit</source>
        <translation>无符号 8 位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="966"/>
        <source>Signed 16-bit</source>
        <translation>有符号 16 位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="967"/>
        <source>Signed 24-bit</source>
        <translation>有符号 24 位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="968"/>
        <source>Signed 32-bit</source>
        <translation>有符号 32 位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="969"/>
        <source>Float 32-bit</source>
        <translation>浮点 32 位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="972"/>
        <source>Mono</source>
        <translation>单声道</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="973"/>
        <source>Stereo</source>
        <translation>立体声</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1375"/>
        <source>Input Device</source>
        <translation>输入设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1383"/>
        <source>Sample Rate</source>
        <translation>采样率</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1391"/>
        <source>Sample Format</source>
        <translation>采样格式</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1399"/>
        <source>Channels</source>
        <translation>声道</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="71"/>
        <source>BLE I/O Module Error</source>
        <translation>BLE I/O 模块错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="325"/>
        <source>Select Device</source>
        <translation>选择设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="336"/>
        <source>Select Service</source>
        <translation>选择服务</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="347"/>
        <source>Select Characteristic</source>
        <translation>选择特征</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="514"/>
        <source>Error while configuring BLE service</source>
        <translation>配置 BLE 服务时出错</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="677"/>
        <source>Operation error</source>
        <translation>操作错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="680"/>
        <source>Characteristic write error</source>
        <translation>特征写入错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="683"/>
        <source>Descriptor write error</source>
        <translation>描述符写入错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="686"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="689"/>
        <source>Characteristic read error</source>
        <translation>特征读取错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="692"/>
        <source>Descriptor read error</source>
        <translation>描述符读取错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="942"/>
        <source>BLE Device</source>
        <translation>BLE 设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="950"/>
        <source>Service</source>
        <translation>服务</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="969"/>
        <source>Characteristic</source>
        <translation>特征</translation>
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
        <translation>CAN 总线不可用</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="274"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>系统中未找到 CAN 总线插件。

macOS 上的 CAN 总线支持有限,可能需要第三方硬件驱动程序。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="279"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>此平台上没有可用的 CAN 总线插件。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="291"/>
        <source>Invalid CAN Configuration</source>
        <translation>CAN 配置无效</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="299"/>
        <source>Invalid Selection</source>
        <translation>选择无效</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="308"/>
        <source>No Devices Available</source>
        <translation>无可用设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="224"/>
        <source>CAN Device Creation Failed</source>
        <translation>CAN 设备创建失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="242"/>
        <source>CAN Connection Failed</source>
        <translation>CAN 连接失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="262"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>系统中未找到 CAN 总线插件。

在 Linux 上,请确保已加载 SOCKETCAN 内核模块。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="268"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>系统中未找到 CAN 总线插件。

在 Windows 上,请安装 CAN 硬件驱动程序(PEAK、VECTOR 等)。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="292"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>CAN 总线配置不完整。请选择有效的插件和接口。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="300"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>所选插件或接口不再可用。请刷新列表并重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="309"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>插件或接口列表为空。请刷新列表并确保 CAN 硬件已连接。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="225"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>无法创建 CAN 总线设备。请检查硬件和驱动程序。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="244"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>无法连接到 CAN 总线设备。请检查硬件连接和设置。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="582"/>
        <source>CAN Bus Error</source>
        <translation>CAN 总线错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="583"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>发生错误，但 CAN 设备已不可用。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="590"/>
        <source>Error code: %1</source>
        <translation>错误代码：%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="593"/>
        <source>CAN Bus Communication Error</source>
        <translation>CAN 总线通信错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="648"/>
        <source>No CAN driver selected</source>
        <translation>未选择 CAN 驱动</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="609"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>请先加载 SOCKETCAN 内核模块</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="612"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>请先设置虚拟 CAN 接口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="614"/>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="634"/>
        <source>No interfaces found for %1</source>
        <translation>未找到 %1 的接口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="618"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>安装 &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN 驱动&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="622"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>安装 &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;VECTOR CAN 驱动&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="626"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>安装 &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN 驱动程序&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="629"/>
        <source>Install %1 drivers</source>
        <translation>安装 %1 驱动程序</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="632"/>
        <source>Install %1 drivers for macOS</source>
        <translation>安装 macOS 版 %1 驱动程序</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="729"/>
        <source>Plugin</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="737"/>
        <source>Interface</source>
        <translation>接口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CANBus.cpp" line="745"/>
        <source>Bitrate</source>
        <translation>比特率</translation>
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
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="181"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>请检查用户是否在 'plugdev' 组中，或是否有 udev 规则授予对此设备的访问权限。

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="185"/>
        <source>Failed to open "%1"</source>
        <translation>打开"%1"失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="295"/>
        <source>HID Device Error</source>
        <translation>HID 设备错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="296"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>HID 设备已断开连接或遇到严重读取错误。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="405"/>
        <source>Select Device</source>
        <translation>选择设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="549"/>
        <source>HID Device</source>
        <translation>HID 设备</translation>
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
        <translation>TLS 1.3 或更高版本</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 或更高版本</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="65"/>
        <source>Any Protocol</source>
        <translation>任意协议</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>Secure Protocols Only</source>
        <translation>仅安全协议</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="70"/>
        <source>Query Peer</source>
        <translation>查询对等方</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="71"/>
        <source>Verify Peer</source>
        <translation>验证对等方</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="72"/>
        <source>Auto Verify Peer</source>
        <translation>自动验证对等方</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="177"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>MQTT 功能需要商业许可证</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="178"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation>订阅 MQTT 代理仅适用于有效的 Serial Studio 商业许可证(Hobbyist 层级或更高)。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="408"/>
        <source>Use System Database</source>
        <translation>使用系统数据库</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="409"/>
        <source>Load From Folder…</source>
        <translation>从文件夹加载…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="442"/>
        <source>Select PEM Certificates Directory</source>
        <translation>选择 PEM 证书目录</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Hostname</source>
        <translation>主机名</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Port</source>
        <translation>端口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="714"/>
        <source>Topic Filter</source>
        <translation>主题过滤器</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="721"/>
        <source>Client ID</source>
        <translation>客户端 ID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="728"/>
        <source>Username</source>
        <translation>用户名</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="735"/>
        <source>Password</source>
        <translation>密码</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="742"/>
        <source>MQTT Version</source>
        <translation>MQTT 版本</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Clean Session</source>
        <translation>清除会话</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="757"/>
        <source>Keep Alive (s)</source>
        <translation>保持连接 (秒)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="766"/>
        <source>Auto Keep Alive</source>
        <translation>自动保持连接</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="783"/>
        <source>SSL/TLS Enabled</source>
        <translation>启用 SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>SSL Protocol</source>
        <translation>SSL 协议</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="801"/>
        <source>Peer Verify Mode</source>
        <translation>对等方验证模式</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="809"/>
        <source>Peer Verify Depth</source>
        <translation>对等方验证深度</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="910"/>
        <source>MQTT Subscription Error</source>
        <translation>MQTT 订阅错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="911"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>订阅主题"%1"失败。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>MQTT 协议版本无效</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>代理服务器拒绝了配置的 MQTT 协议版本。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Client ID Rejected</source>
        <translation>客户端 ID 被拒绝</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>代理服务器拒绝了客户端 ID。请尝试使用其他标识符。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Server Unavailable</source>
        <translation>MQTT 服务器不可用</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>代理服务器当前不可用。请稍后重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>Authentication Error</source>
        <translation>身份验证错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>代理服务器拒绝了提供的凭据。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>Authorization Error</source>
        <translation>授权错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>Account lacks permission for this operation.</source>
        <translation>账户缺少此操作的权限。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="958"/>
        <source>Network or Transport Error</source>
        <translation>网络或传输错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="959"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>连接代理服务器时发生网络/传输层问题。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="962"/>
        <source>MQTT Protocol Violation</source>
        <translation>MQTT 协议违规</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="963"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>代理服务器报告协议违规并关闭了连接。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="966"/>
        <source>MQTT 5 Error</source>
        <translation>MQTT 5 错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="967"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>发生了 MQTT 5 协议级错误。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="970"/>
        <source>MQTT Error</source>
        <translation>MQTT 错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="971"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>发生了意外的 MQTT 错误。</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>串口无效</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Modbus 初始化失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Modbus 连接失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>所选串口"%1"不再可用。请刷新端口列表并重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>无法创建 Modbus 设备。请检查系统配置并重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>无法连接到"%1"。请检查连接设置。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="566"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="567"/>
        <source>Even</source>
        <translation>偶</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="568"/>
        <source>Odd</source>
        <translation>奇</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="569"/>
        <source>Space</source>
        <translation>空格</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="570"/>
        <source>Mark</source>
        <translation>标记</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="622"/>
        <source>Holding Registers (0x03)</source>
        <translation>保持寄存器 (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="623"/>
        <source>Input Registers (0x04)</source>
        <translation>输入寄存器 (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="624"/>
        <source>Coils (0x01)</source>
        <translation>线圈 (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="625"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>离散输入 (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="811"/>
        <source>No register groups configured</source>
        <translation>未配置寄存器组</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="812"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>生成项目前请至少添加一个寄存器组。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="814"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="827"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="852"/>
        <source>Modbus Project Generator</source>
        <translation>Modbus 项目生成器</translation>
    </message>
    <message>
        <source>Failed to create temporary project file</source>
        <translation type="vanished">创建临时项目文件失败</translation>
    </message>
    <message>
        <source>Check write permissions to the temporary directory.</source>
        <translation type="vanished">请检查临时目录的写入权限。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="824"/>
        <source>Failed to load generated project</source>
        <translation>加载生成的项目失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="825"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>无法加载生成的项目 JSON。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="847"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>已成功生成包含 %1 个组和 %2 个数据集的项目。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>The project editor is now open for customization.</source>
        <translation>项目编辑器现已打开以供自定义。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="866"/>
        <source>Modbus Project</source>
        <translation>Modbus 项目</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="872"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="894"/>
        <source>Holding Registers</source>
        <translation>保持寄存器</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="895"/>
        <source>Input Registers</source>
        <translation>输入寄存器</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="896"/>
        <source>Coils</source>
        <translation>线圈</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="897"/>
        <source>Discrete Inputs</source>
        <translation>离散输入</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="912"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="927"/>
        <source>Register %1</source>
        <translation>寄存器 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="935"/>
        <source>Coil %1</source>
        <translation>线圈 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="935"/>
        <source>Discrete %1</source>
        <translation>离散 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1354"/>
        <source>Error code: %1</source>
        <translation>错误代码:%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1357"/>
        <source>Modbus Communication Error</source>
        <translation>Modbus 通信错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1370"/>
        <source>Select Port</source>
        <translation>选择端口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1527"/>
        <source>Protocol</source>
        <translation>协议</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1535"/>
        <source>Slave Address</source>
        <translation>从站地址</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1544"/>
        <source>Poll Interval (ms)</source>
        <translation>轮询间隔 (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1585"/>
        <source>Host / IP</source>
        <translation>主机 / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1592"/>
        <source>Port</source>
        <translation>端口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1607"/>
        <source>Serial Port</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1615"/>
        <source>Baud Rate</source>
        <translation>波特率</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1623"/>
        <source>Parity</source>
        <translation>校验位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1631"/>
        <source>Data Bits</source>
        <translation>数据位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1639"/>
        <source>Stop Bits</source>
        <translation>停止位</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="495"/>
        <source>Network socket error</source>
        <translation>网络套接字错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="512"/>
        <source>Socket Type</source>
        <translation>套接字类型</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="520"/>
        <source>Remote Address</source>
        <translation>远程地址</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="528"/>
        <source>TCP Port</source>
        <translation>TCP 端口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="537"/>
        <source>UDP Local Port</source>
        <translation>UDP 本地端口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="546"/>
        <source>UDP Remote Port</source>
        <translation>UDP 远程端口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="555"/>
        <source>UDP Multicast</source>
        <translation>UDP 组播</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Process</name>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="190"/>
        <location filename="../../src/IO/Drivers/Process.cpp" line="234"/>
        <source>Failed to start process</source>
        <translation>启动进程失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="191"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>在 PATH 中未找到可执行文件"%1"。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="379"/>
        <source>Select Executable</source>
        <translation>选择可执行文件</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="404"/>
        <source>Select Working Directory</source>
        <translation>选择工作目录</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="430"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>选择命名管道 / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="529"/>
        <source>The process crashed.</source>
        <translation>进程已崩溃。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="530"/>
        <source>Exit code: %1</source>
        <translation>退出代码：%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="533"/>
        <source>Process "%1" stopped</source>
        <translation>进程"%1"已停止</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="552"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="553"/>
        <source>Process Error</source>
        <translation>进程错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="567"/>
        <source>Pipe Error</source>
        <translation>管道错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="567"/>
        <source>Could not open named pipe: %1</source>
        <translation>无法打开命名管道：%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="781"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="784"/>
        <source>Launch Process</source>
        <translation>启动进程</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="784"/>
        <source>Named Pipe</source>
        <translation>命名管道</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="789"/>
        <source>Executable</source>
        <translation>可执行文件</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="796"/>
        <source>Arguments</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="803"/>
        <source>Working Directory</source>
        <translation>工作目录</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="810"/>
        <source>Pipe Path</source>
        <translation>管道路径</translation>
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
        <translation>无</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="352"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="752"/>
        <source>Select Port</source>
        <translation>选择端口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="396"/>
        <source>Even</source>
        <translation>偶校验</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="397"/>
        <source>Odd</source>
        <translation>奇校验</translation>
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
        <translation>"%1" 不是有效路径</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="573"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>请输入其他路径以注册自定义串行设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="852"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>找不到指定的设备。请检查连接并重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="859"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>发生未知错误。请检查设备并重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="861"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>设备未打开。请在执行此操作前打开设备。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="263"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>连接到串行端口 "%1" 失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>Critical error on serial port "%1"</source>
        <translation>串口"%1"发生严重错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="851"/>
        <source>No error occurred.</source>
        <translation>未发生错误。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="853"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>权限被拒绝。请确保应用程序具有访问设备的必要权限。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="854"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>打开设备失败。设备可能已被占用或不可用。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="855"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>向设备写入数据时发生错误。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="856"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>从设备读取数据时发生错误。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="857"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>发生严重资源错误。设备可能已断开连接或不再可访问。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="858"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>此设备不支持所请求的操作。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="860"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>操作超时。设备可能未响应。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1029"/>
        <source>Serial Port</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1037"/>
        <source>Baud Rate</source>
        <translation>波特率</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1045"/>
        <source>Parity</source>
        <translation>校验位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1053"/>
        <source>Data Bits</source>
        <translation>数据位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1061"/>
        <source>Stop Bits</source>
        <translation>停止位</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1069"/>
        <source>Flow Control</source>
        <translation>流控制</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1077"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1084"/>
        <source>Auto-Reconnect</source>
        <translation>自动重连</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="185"/>
        <source>USB Error</source>
        <translation>USB 错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>初始化 USB 子系统失败。请检查系统中是否有 libusb。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="223"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="240"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="632"/>
        <source>USB Device Error</source>
        <translation>USB 设备错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>无法打开 USB 设备:%1。

在 Linux 上,请确保您对设备节点具有读/写权限(添加 udev 规则或以 root 身份运行)。在 macOS 上,可能需要先分离内核驱动程序。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="186"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>未选择 USB 设备。请选择设备后重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="193"/>
        <source>Unknown Device</source>
        <translation>未知设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="198"/>
        <source>Failed to open "%1"</source>
        <translation>打开"%1"失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="241"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>无法声明 USB 设备上的接口 %1。

另一个驱动程序或应用程序可能已打开它。在 Linux 上,请尝试卸载内核驱动程序(例如 cdc_acm)或添加 udev 规则。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="404"/>
        <source>Select Device</source>
        <translation>选择设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="423"/>
        <source>Select IN Endpoint</source>
        <translation>选择 IN 端点</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="434"/>
        <source>None (Read-only)</source>
        <translation>无(只读)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="509"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>启用高级 USB 控制传输？</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="510"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>这将在批量传输之外启用控制传输。发送不正确的控制请求可能会导致连接的硬件崩溃或损坏。仅在了解相关操作的情况下启用此功能。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="514"/>
        <source>Advanced USB Mode</source>
        <translation>高级 USB 模式</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="633"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>USB 设备已断开连接或遇到严重读取错误。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="772"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>此设备上未找到同步 IN 端点，但批量端点可用。

将传输模式切换为"批量流"并重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="777"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>此设备上未找到批量 IN 端点，但同步端点可用。

将传输模式切换为"同步"并重试。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="781"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>此设备上未找到可用的 IN 端点。

该设备可能未在其活动配置中公开数据端点，或者可能需要特定驱动程序。</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1212"/>
        <source>USB Device</source>
        <translation>USB 设备</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1220"/>
        <source>Transfer Mode</source>
        <translation>传输模式</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Bulk Stream</source>
        <translation>批量流</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Advanced Control</source>
        <translation>高级控制</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1223"/>
        <source>Isochronous</source>
        <translation>同步</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1228"/>
        <source>IN Endpoint</source>
        <translation>IN 端点</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1236"/>
        <source>OUT Endpoint</source>
        <translation>OUT 端点</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1244"/>
        <source>ISO Packet Size</source>
        <translation>ISO 数据包大小</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="214"/>
        <source>No file selected…</source>
        <translation>未选择文件…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="249"/>
        <source>Plain Text</source>
        <translation>纯文本</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="250"/>
        <source>Raw Binary</source>
        <translation>原始二进制</translation>
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
        <translation>选择要传输的文件</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="295"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>已选择文件：%1（%2 字节）</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="298"/>
        <source>Error opening file: %1</source>
        <translation>打开文件时出错：%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="389"/>
        <source>Starting %1 transfer…</source>
        <translation>正在开始 %1 传输…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="624"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="644"/>
        <source>Transmission complete</source>
        <translation>传输完成</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="626"/>
        <source>Plain text transmission complete</source>
        <translation>纯文本传输完成</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="646"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>原始二进制传输完成（%1 字节）</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="670"/>
        <source>Transfer complete</source>
        <translation>传输完成</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="671"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>传输成功完成（%1 字节）</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="673"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="674"/>
        <source>Transfer failed: %1</source>
        <translation>传输失败：%1</translation>
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
        <translation>帧丢弃</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="300"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>传入数据的速度超过 Serial Studio 的处理能力；已丢弃 %1 个帧。请降低数据速率或禁用占用资源较多的组件。</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="85"/>
        <source>Cannot open file: %1</source>
        <translation>无法打开文件：%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="114"/>
        <source>Transfer cancelled</source>
        <translation>传输已取消</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="115"/>
        <source>Transfer cancelled by user</source>
        <translation>传输已被用户取消</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="95"/>
        <source>Waiting for receiver…</source>
        <translation>等待接收方…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="211"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>接收方就绪（CRC 模式），正在发送数据…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="144"/>
        <source>Too many retries, transfer aborted</source>
        <translation>重试次数过多，传输已中止</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="145"/>
        <source>Maximum retries exceeded</source>
        <translation>超过最大重试次数</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="149"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>收到 NAK，正在重试数据块 %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="158"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="396"/>
        <source>Failed to seek in file</source>
        <translation>文件定位失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="168"/>
        <source>Transfer cancelled by receiver</source>
        <translation>传输已被接收方取消</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="169"/>
        <source>Receiver cancelled the transfer</source>
        <translation>接收方取消了传输</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="183"/>
        <source>Transfer complete</source>
        <translation>传输完成</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="310"/>
        <source>File read returned more data than requested</source>
        <translation>文件读取返回的数据超过请求量</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="326"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>正在发送数据块 %1（%2 字节）</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="339"/>
        <source>Sending EOT…</source>
        <translation>正在发送 EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="386"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>超时，正在重试 (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="381"/>
        <source>Transfer timed out</source>
        <translation>传输超时</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="382"/>
        <source>Timeout: no response from receiver</source>
        <translation>超时：接收方无响应</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="64"/>
        <source>Cannot open file: %1</source>
        <translation>无法打开文件：%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="98"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="176"/>
        <source>Transfer cancelled by receiver</source>
        <translation>接收方取消传输</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="99"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="177"/>
        <source>Receiver cancelled the transfer</source>
        <translation>接收方已取消传输</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="76"/>
        <source>Waiting for receiver…</source>
        <translation>等待接收方…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="135"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="331"/>
        <source>Sending first EOT…</source>
        <translation>发送第一个 EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="153"/>
        <source>Too many retries, transfer aborted</source>
        <translation>重试次数过多，传输已中止</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="154"/>
        <source>Maximum retries exceeded</source>
        <translation>超出最大重试次数</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="158"/>
        <source>NAK received, retrying block %1</source>
        <translation>收到 NAK，正在重试数据块 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="164"/>
        <source>Failed to seek in file</source>
        <translation>文件定位失败</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="228"/>
        <source>Sending second EOT…</source>
        <translation>正在发送第二个 EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="312"/>
        <source>Sending end-of-batch marker…</source>
        <translation>正在发送批次结束标记…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="252"/>
        <source>Transfer complete</source>
        <translation>传输完成</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="297"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>正在发送文件头：%1（%2 字节）</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="346"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>正在发送数据块 %1（%2/%3 字节）</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="88"/>
        <source>Cannot open file: %1</source>
        <translation>无法打开文件：%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="106"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>文件对于 ZMODEM 过大（%1 字节，限制 4 GiB）。</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="133"/>
        <source>Transfer cancelled</source>
        <translation>传输已取消</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="134"/>
        <source>Transfer cancelled by user</source>
        <translation>用户已取消传输</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="279"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>十六进制头 CRC 不匹配，丢弃帧</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="466"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>正在发送文件信息：%1（%2 字节）</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="482"/>
        <source>Failed to seek to offset %1</source>
        <translation>无法定位到偏移量 %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="513"/>
        <source>File read returned more data than requested</source>
        <translation>文件读取返回的数据超过请求量</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="601"/>
        <source>Receiver requests data from offset %1</source>
        <translation>接收方请求从偏移量 %1 开始的数据</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="610"/>
        <source>Receiver skipped the file</source>
        <translation>接收方跳过了该文件</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="623"/>
        <source>Too many errors, transfer aborted</source>
        <translation>错误过多，传输已中止</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="624"/>
        <source>Maximum retries exceeded</source>
        <translation>超过最大重试次数</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="439"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>正在发送 ZRQINIT，等待接收方…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="541"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>文件数据已发送，等待确认…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="552"/>
        <source>Sending ZFIN…</source>
        <translation>正在发送 ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="589"/>
        <source>Receiver ready, sending file info…</source>
        <translation>接收方就绪，正在发送文件信息…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>收到 NAK，正在重试 (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="650"/>
        <source>Transfer complete</source>
        <translation>传输完成</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="661"/>
        <source>Transfer cancelled by receiver</source>
        <translation>接收方取消传输</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="662"/>
        <source>Receiver cancelled the transfer</source>
        <translation>接收方取消了传输</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="671"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="672"/>
        <source>Receiver reported a file error</source>
        <translation>接收方报告文件错误</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="879"/>
        <source>Transfer timed out</source>
        <translation>传输超时</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="880"/>
        <source>Timeout: no response from receiver</source>
        <translation>超时：接收方无响应</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="884"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>超时，正在重试 (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="42"/>
        <source>Select Icon</source>
        <translation>选择图标</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="119"/>
        <source>Search Online…</source>
        <translation>在线搜索…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="132"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="144"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>正常</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>灰度</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>高对比度</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>鲜艳</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>夜视</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>红外</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>深蓝色</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>琥珀色</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>导出图像</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>打开导出文件夹</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>放大</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>缩小</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>显示十字准线</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>等待图像…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="24"/>
        <source>API Keys</source>
        <translation>API 密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="48"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude。默认为 Claude Haiku 4.5(每百万令牌 $1 输入 / $5 输出)。也可使用 Sonnet 4.6 和 OPUS 4.7。支持流式传输、工具使用、扩展思考和提示缓存。</translation>
    </message>
    <message>
        <source>OpenAI Chat Completions. The default is GPT-4o mini ($0.15 input / $0.60 output per million tokens). GPT-4o, GPT-4 Turbo, and o1-mini are also available.</source>
        <translation type="vanished">OpenAI Chat Completions。默认为 GPT-4o mini(每百万令牌 $0.15 输入 / $0.60 输出)。也可使用 GPT-4o、GPT-4 Turbo 和 o1-mini。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="58"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini。默认为 Gemini 2.0 Flash,提供慷慨的免费额度(受速率限制)。也可使用 Gemini 1.5 Pro 和 Gemini 1.5 Flash。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="101"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>使用您自己的 API 密钥。密钥在本地使用每台机器的密钥加密存储,除了与您选择的提供商通信外,不会离开您的计算机。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>Key set</source>
        <translation>密钥已设置</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="168"/>
        <source>No key</source>
        <translation>无密钥</translation>
    </message>
    <message>
        <source>A key is on file — paste a new one to replace it</source>
        <translation type="vanished">已存储密钥 — 粘贴新密钥以替换</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="53"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions。默认为 GPT-5 mini,适用于快速、成本敏感的代理工作。GPT-5.2 是更强大的通用选项,GPT-5.2 Chat 跟踪 ChatGPT 当前使用的模型。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="62"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek。兼容 OpenAI API。默认为 deepseek-chat (V3)。deepseek-reasoner (R1) 也可用。通常是工具调用最便宜的云选项。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="66"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter。一个密钥,约 200 个模型,来自 Anthropic、OpenAI、Google、Meta、Mistral、DeepSeek、Qwen 等。免费层模型(后缀 :free)有速率限制但无需额外付费。其余模型按使用量付费。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="71"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq。硬件加速推理(LPU),为 Llama、Mixtral、Gemma、DeepSeek-R1 distill 和 Qwen 模型提供极快速度。慷慨的免费层,每日令牌限额。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="75"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral。默认为 Mistral Large。Codestral 针对代码补全,Pixtral 处理视觉,Ministral 模型针对边缘/低延迟使用进行调优。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="79"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>本地模型服务器。兼容任何 OpenAI 端点 -- Ollama、llama.cpp 的 llama-server、LM Studio 或 vLLM。数据不离开本机。模型列表从服务器实时查询。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>已存储密钥 -- 粘贴新密钥以替换</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="206"/>
        <source>Paste your API key here</source>
        <translation>在此粘贴您的 API 密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Hide key</source>
        <translation>隐藏密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="214"/>
        <source>Show key while typing</source>
        <translation>输入时显示密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Get key</source>
        <translation>获取密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="226"/>
        <source>Open the provider's console to create a new key</source>
        <translation>打开提供商的控制台以创建新密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="237"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="255"/>
        <source>Remove the stored key for %1</source>
        <translation>移除 %1 的已存储密钥</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="279"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Install Ollama</source>
        <translation>安装 Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="284"/>
        <source>Open the Ollama download page</source>
        <translation>打开 Ollama 下载页面</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="295"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="310"/>
        <source>Re-query the model list</source>
        <translation>重新查询模型列表</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="358"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>尚未配置 API 密钥。请添加密钥以开始使用。</translation>
    </message>
    <message>
        <source>No API keys configured yet. Add at least one above to get started.</source>
        <translation type="vanished">尚未配置 API 密钥。请在上方至少添加一个以开始使用。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="361"/>
        <source>One provider is ready.</source>
        <translation>一个提供商已就绪。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="363"/>
        <source>%1 providers are ready.</source>
        <translation>%1 个提供商已就绪。</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>许可证管理</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>请稍候…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>激活 Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>在下方粘贴您的许可证密钥以解锁 Pro 功能，如 MQTT、3D 绘图等。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>您的许可证包含 5 个设备激活。
计划包括月度、年度和终身选项。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>在此粘贴您的许可证密钥…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="332"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="381"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>粘贴</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="338"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="387"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="234"/>
        <source>Product</source>
        <translation>产品</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="241"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="252"/>
        <source>Licensee</source>
        <translation>许可证持有人</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="271"/>
        <source>Licensee E-Mail</source>
        <translation>被许可人电子邮件</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="288"/>
        <source>Device Usage</source>
        <translation>设备使用情况</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="296"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>已使用 %1 台设备（无限制计划）</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 of %2 devices used</source>
        <translation>已使用 %1 / %2 台设备</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="307"/>
        <source>Device ID</source>
        <translation>设备 ID</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="354"/>
        <source>License Key</source>
        <translation>许可证密钥</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="409"/>
        <source>Customer Portal</source>
        <translation>客户门户</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="420"/>
        <source>Buy License</source>
        <translation>购买许可证</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="427"/>
        <source>Activate</source>
        <translation>激活</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="437"/>
        <source>Deactivate</source>
        <translation>停用</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="546"/>
        <source>There was an issue validating your license.</source>
        <translation>验证许可证时出现问题。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="564"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="745"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="870"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>您提供的许可证密钥不属于 Serial Studio。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="565"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>请仔细检查您是否从 Serial Studio 官方商店购买了许可证。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="576"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="754"/>
        <source>This license key was activated on a different device.</source>
        <translation>此许可证密钥已在其他设备上激活。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="577"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="755"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>请先在该设备上停用或联系支持寻求帮助。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="588"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="765"/>
        <source>This license is not currently active.</source>
        <translation>此许可证当前未激活。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="589"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="766"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>可能已过期或已停用(状态：%1)。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="599"/>
        <source>Something went wrong on the server.</source>
        <translation>服务器发生错误。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="600"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="776"/>
        <source>No activation ID was returned.</source>
        <translation>未返回激活 ID。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="610"/>
        <source>Could not validate your license at this time.</source>
        <translation>目前无法验证许可证。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="611"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="785"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="881"/>
        <source>Try again later.</source>
        <translation>请稍后重试。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="746"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="871"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>请仔细检查您是否从 Serial Studio 官方商店购买了许可证。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="775"/>
        <source>Something went wrong on the server…</source>
        <translation>服务器出现问题…</translation>
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
        <translation>许可证已成功激活。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="683"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>感谢您支持 Serial Studio！
现在可以使用所有高级功能。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="737"/>
        <source>There was an issue activating your license.</source>
        <translation>激活许可证时出现问题。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="784"/>
        <source>Could not activate your license at this time.</source>
        <translation>目前无法激活许可证。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="861"/>
        <source>There was an issue deactivating your license.</source>
        <translation>停用许可证时出现问题。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="880"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>当前无法停用您的许可证。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="891"/>
        <source>Your license has been deactivated.</source>
        <translation>您的许可证已停用。</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="892"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Pro 功能访问权限已移除。
再次感谢您对 Serial Studio 的支持！</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="638"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>MDF4 导出是 Pro 功能。</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="639"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>此功能需要许可证。请购买许可证以启用 MDF4 导出。</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="363"/>
        <source>Select MDF4 file</source>
        <translation>选择 MDF4 文件</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="365"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>MDF4 文件 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="389"/>
        <source>Disconnect from device?</source>
        <translation>断开设备连接？</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="390"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>打开 MDF4 文件前必须断开当前设备连接。</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="407"/>
        <source>Cannot open MDF4 file</source>
        <translation>无法打开 MDF4 文件</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="408"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>文件可能已损坏或格式不受支持。</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="415"/>
        <source>Invalid MDF4 file</source>
        <translation>无效的 MDF4 文件</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="416"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>读取文件结构失败。文件可能已损坏。</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="433"/>
        <source>No data in file</source>
        <translation>文件中无数据</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="434"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>MDF4 文件不包含测量数据。</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>主机名</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>例如 broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>端口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>主题过滤器</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>例如 sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>客户端 ID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>重新生成</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>用户名</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>密码</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>版本</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="187"/>
        <source>Keep Alive (s)</source>
        <translation>保持连接 (秒)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="206"/>
        <source>Clean Session</source>
        <translation>清除会话</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="232"/>
        <source>Use SSL/TLS</source>
        <translation>使用 SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>SSL 协议</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>对等方验证</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>验证深度</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>CA 证书</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>从文件夹加载…</translation>
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
        <translation type="vanished">TLS 1.3 或更高版本</translation>
    </message>
    <message>
        <source>DTLS 1.2 or Later</source>
        <translation type="vanished">DTLS 1.2 或更高版本</translation>
    </message>
    <message>
        <source>Any Protocol</source>
        <translation type="vanished">任意协议</translation>
    </message>
    <message>
        <source>Secure Protocols Only</source>
        <translation type="vanished">仅安全协议</translation>
    </message>
    <message>
        <source>None</source>
        <translation type="vanished">无</translation>
    </message>
    <message>
        <source>Query Peer</source>
        <translation type="vanished">查询对等方</translation>
    </message>
    <message>
        <source>Verify Peer</source>
        <translation type="vanished">验证对等方</translation>
    </message>
    <message>
        <source>Auto Verify Peer</source>
        <translation type="vanished">自动验证对等方</translation>
    </message>
    <message>
        <source>Use System Database</source>
        <translation type="vanished">使用系统数据库</translation>
    </message>
    <message>
        <source>MQTT Subscriber</source>
        <translation type="vanished">MQTT 订阅者</translation>
    </message>
    <message>
        <source>MQTT Publisher</source>
        <translation type="vanished">MQTT 发布者</translation>
    </message>
    <message>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation type="vanished">MQTT 功能需要商业许可证</translation>
    </message>
    <message>
        <source>Load From Folder…</source>
        <translation type="vanished">从文件夹加载…</translation>
    </message>
    <message>
        <source>Connecting to MQTT brokers is only available with a valid Serial Studio commercial license (Hobbyist tier or above).

To unlock this feature, please activate your license or visit the store.</source>
        <translation type="vanished">连接到 MQTT 代理仅适用于拥有有效 Serial Studio 商业许可证（Hobbyist 层级或更高）的用户。

要解锁此功能，请激活您的许可证或访问商店。</translation>
    </message>
    <message>
        <source>Missing MQTT Topic</source>
        <translation type="vanished">缺少 MQTT 主题</translation>
    </message>
    <message>
        <source>You must specify a topic before connecting as a publisher.</source>
        <translation type="vanished">作为发布者连接前必须指定主题。</translation>
    </message>
    <message>
        <source>Configuration Error</source>
        <translation type="vanished">配置错误</translation>
    </message>
    <message>
        <source>MQTT Topic Not Set</source>
        <translation type="vanished">MQTT 主题未设置</translation>
    </message>
    <message>
        <source>You won't receive any messages until a topic is configured.</source>
        <translation type="vanished">配置主题之前不会接收任何消息。</translation>
    </message>
    <message>
        <source>Configuration Warning</source>
        <translation type="vanished">配置警告</translation>
    </message>
    <message>
        <source>Invalid MQTT Topic</source>
        <translation type="vanished">MQTT 主题无效</translation>
    </message>
    <message>
        <source>The topic "%1" is not valid.</source>
        <translation type="vanished">主题"%1"无效。</translation>
    </message>
    <message>
        <source>Select PEM Certificates Directory</source>
        <translation type="vanished">选择 PEM 证书目录</translation>
    </message>
    <message>
        <source>Subscription Error</source>
        <translation type="vanished">订阅错误</translation>
    </message>
    <message>
        <source>Failed to subscribe to topic "%1".</source>
        <translation type="vanished">订阅主题"%1"失败。</translation>
    </message>
    <message>
        <source>Invalid MQTT Protocol Version</source>
        <translation type="vanished">MQTT 协议版本无效</translation>
    </message>
    <message>
        <source>The MQTT broker rejected the connection due to an unsupported protocol version. Ensure that your client and broker support the same protocol version.</source>
        <translation type="vanished">MQTT 代理因不支持的协议版本拒绝了连接。请确保客户端和代理支持相同的协议版本。</translation>
    </message>
    <message>
        <source>Client ID Rejected</source>
        <translation type="vanished">客户端 ID 被拒绝</translation>
    </message>
    <message>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Try using a different client ID.</source>
        <translation type="vanished">代理拒绝了客户端 ID。它可能格式错误、过长或已被使用。请尝试使用不同的客户端 ID。</translation>
    </message>
    <message>
        <source>MQTT Server Unavailable</source>
        <translation type="vanished">MQTT 服务器不可用</translation>
    </message>
    <message>
        <source>The network connection was established, but the broker is currently unavailable. Verify the broker status and try again later.</source>
        <translation type="vanished">网络连接已建立，但 Broker 当前不可用。请验证 Broker 状态并稍后重试。</translation>
    </message>
    <message>
        <source>Authentication Error</source>
        <translation type="vanished">身份验证错误</translation>
    </message>
    <message>
        <source>The username or password provided is incorrect or malformed. Double-check your credentials and try again.</source>
        <translation type="vanished">提供的用户名或密码不正确或格式错误。请仔细检查您的凭据并重试。</translation>
    </message>
    <message>
        <source>Authorization Error</source>
        <translation type="vanished">授权错误</translation>
    </message>
    <message>
        <source>The MQTT broker denied the connection due to insufficient permissions. Ensure that your account has the necessary access rights.</source>
        <translation type="vanished">MQTT Broker 因权限不足拒绝了连接。请确保您的账户具有必要的访问权限。</translation>
    </message>
    <message>
        <source>Network or Transport Error</source>
        <translation type="vanished">网络或传输错误</translation>
    </message>
    <message>
        <source>A network or transport layer issue occurred, causing an unexpected connection failure. Check your network connection and broker settings.</source>
        <translation type="vanished">发生网络或传输层问题，导致意外连接失败。请检查您的网络连接和 Broker 设置。</translation>
    </message>
    <message>
        <source>MQTT Protocol Violation</source>
        <translation type="vanished">MQTT 协议违规</translation>
    </message>
    <message>
        <source>The client detected a violation of the MQTT protocol and closed the connection. Check your MQTT implementation for compliance.</source>
        <translation type="vanished">客户端检测到违反 MQTT 协议的行为并关闭了连接。请检查您的 MQTT 实现是否符合规范。</translation>
    </message>
    <message>
        <source>Unknown Error</source>
        <translation type="vanished">未知错误</translation>
    </message>
    <message>
        <source>An unexpected error occurred. Check the logs for more details or restart the application.</source>
        <translation type="vanished">发生意外错误。请检查日志以获取更多详细信息或重启应用程序。</translation>
    </message>
    <message>
        <source>MQTT 5 Error</source>
        <translation type="vanished">MQTT 5 错误</translation>
    </message>
    <message>
        <source>An MQTT protocol level 5 error occurred. Check the broker logs or reason codes for more details.</source>
        <translation type="vanished">发生 MQTT 协议级别 5 错误。请检查 Broker 日志或原因代码以获取更多详细信息。</translation>
    </message>
    <message>
        <source>MQTT Authentication Failed</source>
        <translation type="vanished">MQTT 身份验证失败</translation>
    </message>
    <message>
        <source>Authentication failed: %1.</source>
        <translation type="vanished">身份验证失败：%1。</translation>
    </message>
    <message>
        <source>Extended authentication is required, but MQTT 5.0 is not enabled.</source>
        <translation type="vanished">需要扩展身份验证，但未启用 MQTT 5.0。</translation>
    </message>
    <message>
        <source>Unknown</source>
        <translation type="vanished">未知</translation>
    </message>
    <message>
        <source>MQTT Authentication Required</source>
        <translation type="vanished">需要 MQTT 身份验证</translation>
    </message>
    <message>
        <source>The MQTT broker requires authentication using method: "%1".

Please provide the necessary credentials.</source>
        <translation type="vanished">MQTT Broker 需要使用方法"%1"进行身份验证。

请提供必要的凭据。</translation>
    </message>
    <message>
        <source>Enter MQTT Username</source>
        <translation type="vanished">输入 MQTT 用户名</translation>
    </message>
    <message>
        <source>Username:</source>
        <translation type="vanished">用户名：</translation>
    </message>
    <message>
        <source>Enter MQTT Password</source>
        <translation type="vanished">输入 MQTT 密码</translation>
    </message>
    <message>
        <source>Password:</source>
        <translation type="vanished">密码：</translation>
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
        <translation>TLS 1.3 或更高版本</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="822"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 或更高版本</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="823"/>
        <source>Any Protocol</source>
        <translation>任意协议</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="824"/>
        <source>Secure Protocols Only</source>
        <translation>仅安全协议</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="826"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="827"/>
        <source>Query Peer</source>
        <translation>查询对等方</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="828"/>
        <source>Verify Peer</source>
        <translation>验证对等方</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="829"/>
        <source>Auto Verify Peer</source>
        <translation>自动验证对等方</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1150"/>
        <source>Raw RX Data</source>
        <translation>原始 RX 数据</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1151"/>
        <source>Custom Script</source>
        <translation>自定义脚本</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1152"/>
        <source>Dashboard Data (CSV)</source>
        <translation>仪表板数据 (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1153"/>
        <source>Dashboard Data (JSON)</source>
        <translation>仪表板数据 (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1313"/>
        <source>MQTT publisher unavailable</source>
        <translation>MQTT 发布器不可用</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1314"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>使用 MQTT 发布功能需要有效的商业许可证。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1316"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1890"/>
        <source>MQTT Test Connection</source>
        <translation>MQTT 测试连接</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1336"/>
        <source>Select PEM Certificates Directory</source>
        <translation>选择 PEM 证书目录</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1887"/>
        <source>MQTT broker reachable</source>
        <translation>MQTT 代理可达</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1887"/>
        <source>MQTT broker unreachable</source>
        <translation>MQTT 代理不可达</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1901"/>
        <source>MQTT broker connection failed</source>
        <translation>MQTT 代理连接失败</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1901"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 发布者</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="49"/>
        <source>MQTT Publisher Script</source>
        <translation>MQTT 发布者脚本</translation>
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
        <translation>示例帧字节(文本或十六进制)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="97"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Test</source>
        <translation>测试</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="101"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="111"/>
        <source>Language:</source>
        <translation>语言：</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="114"/>
        <source>Template:</source>
        <translation>模板：</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="125"/>
        <source>Frame:</source>
        <translation>帧：</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="129"/>
        <source>Output:</source>
        <translation>输出：</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="269"/>
        <source>Enter a frame</source>
        <translation>输入帧</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="276"/>
        <source>Invalid hex</source>
        <translation>无效的十六进制</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="359"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>格式化文档	Ctrl+Shift+I</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>格式化选区	Ctrl+I</translation>
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
-- 定义一个 mqtt(frame) 函数，接收一个已解析帧的原始字节
-- 并返回要发布到代理的有效载荷，或返回 nil 以跳过此帧。
--
-- frame 参数与帧解析器脚本中看到的一致：
-- 一个 Lua 字符串，包含 FrameReader 分隔符之间的字节。
--
-- 示例：
--   function mqtt(frame)
--     return frame    -- 原样转发
--   end
--
-- 使用模板下拉菜单查看现成示例。
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
 * 定义一个 mqtt(frame) 函数，接收一个已解析帧的原始字节
 * 并返回要发布到代理的有效载荷，或返回 null 以跳过此帧。
 *
 * frame 参数与帧解析器脚本中看到的一致：
 * 一个字符串，包含 FrameReader 分隔符之间的字节。
 *
 * 示例：
 *   function mqtt(frame) {
 *     return frame;   // 原样转发
 *   }
 *
 * 使用模板下拉菜单查看现成示例。
 */

</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="602"/>
        <source>Script is empty</source>
        <translation>脚本为空</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="609"/>
        <source>Lua engine error</source>
        <translation>Lua 引擎错误</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="631"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="645"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="683"/>
        <source>Error: %1</source>
        <translation>错误：%1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="639"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="675"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() 未定义</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="656"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- 已跳过帧)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <source>(non-string return)</source>
        <translation>(非字符串返回)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- 跳过帧)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="766"/>
        <source>Select Template…</source>
        <translation>选择模板…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="696"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>测试连接前请先配置代理主机名和端口。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="733"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>已成功连接到 %1:%2。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="744"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>5 秒后超时，未能连接到代理。</translation>
    </message>
</context>
<context>
    <name>MQTTConfiguration</name>
    <message>
        <source>MQTT Setup</source>
        <translation type="vanished">MQTT 设置</translation>
    </message>
    <message>
        <source>MQTT is a Pro Feature</source>
        <translation type="vanished">MQTT 是 Pro 功能</translation>
    </message>
    <message>
        <source>Activate your license or visit the store to unlock MQTT support.</source>
        <translation type="vanished">激活许可证或访问商店以解锁 MQTT 支持。</translation>
    </message>
    <message>
        <source>General</source>
        <translation type="vanished">常规</translation>
    </message>
    <message>
        <source>Authentication</source>
        <translation type="vanished">身份验证</translation>
    </message>
    <message>
        <source>MQTT Options</source>
        <translation type="vanished">MQTT 选项</translation>
    </message>
    <message>
        <source>SSL Properties</source>
        <translation type="vanished">SSL 属性</translation>
    </message>
    <message>
        <source>Host</source>
        <translation type="vanished">主机</translation>
    </message>
    <message>
        <source>Port</source>
        <translation type="vanished">端口</translation>
    </message>
    <message>
        <source>Client ID</source>
        <translation type="vanished">客户端 ID</translation>
    </message>
    <message>
        <source>Keep Alive (s)</source>
        <translation type="vanished">保持连接 (秒)</translation>
    </message>
    <message>
        <source>Clean Session</source>
        <translation type="vanished">清除会话</translation>
    </message>
    <message>
        <source>Username</source>
        <translation type="vanished">用户名</translation>
    </message>
    <message>
        <source>MQTT Username</source>
        <translation type="vanished">MQTT 用户名</translation>
    </message>
    <message>
        <source>Password</source>
        <translation type="vanished">密码</translation>
    </message>
    <message>
        <source>MQTT Password</source>
        <translation type="vanished">MQTT 密码</translation>
    </message>
    <message>
        <source>Version</source>
        <translation type="vanished">版本</translation>
    </message>
    <message>
        <source>Mode</source>
        <translation type="vanished">模式</translation>
    </message>
    <message>
        <source>Topic</source>
        <translation type="vanished">主题</translation>
    </message>
    <message>
        <source>e.g. sensors/temperature or home/+/status</source>
        <translation type="vanished">例如 sensors/temperature 或 home/+/status</translation>
    </message>
    <message>
        <source>Will Retain</source>
        <translation type="vanished">Will 保留</translation>
    </message>
    <message>
        <source>Will QoS</source>
        <translation type="vanished">Will QOS</translation>
    </message>
    <message>
        <source>Will Topic</source>
        <translation type="vanished">Will 主题</translation>
    </message>
    <message>
        <source>e.g. device/alerts/offline</source>
        <translation type="vanished">例如 device/alerts/offline</translation>
    </message>
    <message>
        <source>Will Message</source>
        <translation type="vanished">Will 消息</translation>
    </message>
    <message>
        <source>e.g. Device unexpectedly disconnected</source>
        <translation type="vanished">例如设备意外断开连接</translation>
    </message>
    <message>
        <source>Enable SSL</source>
        <translation type="vanished">启用 SSL</translation>
    </message>
    <message>
        <source>SSL Protocol</source>
        <translation type="vanished">SSL 协议</translation>
    </message>
    <message>
        <source>Verify Depth</source>
        <translation type="vanished">验证深度</translation>
    </message>
    <message>
        <source>Verify Mode</source>
        <translation type="vanished">验证模式</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">关闭</translation>
    </message>
    <message>
        <source>Disconnect</source>
        <translation type="vanished">断开连接</translation>
    </message>
    <message>
        <source>Connect</source>
        <translation type="vanished">连接</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="184"/>
        <source>Console Only Mode</source>
        <translation>仅控制台模式</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="187"/>
        <source>Quick Plot Mode</source>
        <translation>快速绘图模式</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="194"/>
        <source>Empty Project</source>
        <translation>空项目</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="683"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="691"/>
        <source>Waiting for data…</source>
        <translation>等待数据…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="692"/>
        <source>Connecting to device…</source>
        <translation>正在连接设备…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>代码示例</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>补全器</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>高亮器</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>样式</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>个空格</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="35"/>
        <source>Copied to Clipboard</source>
        <translation>已复制到剪贴板</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="24"/>
        <source>MDF4 Player</source>
        <translation>MDF4 播放器</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>您</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>助手</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>发现</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>执行</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>批准 %2 中的 %1 个操作？</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>这些调用将一起运行。展开下方每个卡片以检查参数。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>全部批准</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>全部拒绝</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>全部复制</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="62"/>
        <source>You</source>
        <translation>您</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>Assistant</source>
        <translation>助手</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <source>Discovery</source>
        <translation>发现</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Execution</source>
        <translation>执行</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Running</source>
        <translation>运行中</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Awaiting approval</source>
        <translation>等待批准</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Done</source>
        <translation>完成</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Denied</source>
        <translation>已拒绝</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Blocked</source>
        <translation>已阻止</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Approve</source>
        <translation>批准</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Deny</source>
        <translation>拒绝</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Approve all</source>
        <translation>全部批准</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Deny all</source>
        <translation>全部拒绝</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Arguments</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Result</source>
        <translation>结果</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>批准 {family} 中的 {n} 个操作?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>这些调用将一起运行。展开每个卡片以检查参数。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="294"/>
        <source>Failed to load README: %1</source>
        <translation>加载 README 失败：%1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Theme</source>
        <translation>主题</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Frame Parser</source>
        <translation>帧解析器</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Project Template</source>
        <translation>项目模板</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>Plugin</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="259"/>
        <source>All Types</source>
        <translation>所有类型</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="487"/>
        <source>Reset Extensions</source>
        <translation>重置扩展</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="488"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>此操作将卸载所有扩展、移除所有自定义仓库并恢复默认设置。是否继续？</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="531"/>
        <source>Select Extension Repository Folder</source>
        <translation>选择扩展仓库文件夹</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1064"/>
        <source>Installed (repository no longer available)</source>
        <translation>已安装（仓库不再可用）</translation>
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
        <translation>插件错误</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>插件"%1"未安装。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1389"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>扩展"%1"不是插件（类型：%2）。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1410"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>无法读取插件元数据文件：
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>插件"%1"需要 GRPC 支持，但当前构建版本不包含 GRPC 支持。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>此插件使用 GRPC 进行高性能数据流传输。需要启用 API 服务器。

是否立即启用?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1447"/>
        <source>API Server Required</source>
        <translation>需要 API 服务器</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1476"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>插件"%1"的 info.json 中没有"entry"字段。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1486"/>
        <source>Entry point not found:
%1</source>
        <translation>未找到入口点:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1495"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>插件"%1"的入口点路径无效。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1538"/>
        <source>Missing Dependency</source>
        <translation>缺少依赖项</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1539"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>此插件需要"%1",但在系统中未找到。

是否打开下载页面?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1444"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>插件需要 API 服务器才能与 Serial Studio 通信。是否立即启用?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>需要重启</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>新的渲染后端将在重启 Serial Studio 后生效。现在重启以应用更改吗？</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="320"/>
        <source>Failed to load page: %1</source>
        <translation>加载页面失败:%1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="154"/>
        <source>Invalid icon identifier</source>
        <translation>无效的图标标识符</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="228"/>
        <source>Empty SVG data received</source>
        <translation>收到空 SVG 数据</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Windows 快捷方式 (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>macOS 应用程序 (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>桌面条目 (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>使用 .icns 图标可在 Finder 和程序坞中获得最清晰的效果。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>留空图标将继承 Serial Studio 可执行文件图标。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>将文件放置在 ~/.local/share/applications/ 下以在应用程序启动器中显示。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Apple 图标图像 (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Windows 图标 (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>矢量或光栅图像 (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="215"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>需要 Pro 许可证才能生成快捷方式。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="220"/>
        <source>No output path was provided.</source>
        <translation>未提供输出路径。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="263"/>
        <source>Failed to write shortcut file.</source>
        <translation>写入快捷方式文件失败。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>无法替换 %1 处的现有快捷方式。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>无法创建 .app 包目录布局。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>无法写入包启动器：%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>无法将包启动器标记为可执行。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>无法写入 Info.plist：%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="272"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="141"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>此平台不支持 Windows 快捷方式写入器。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="286"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="200"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>此平台不支持 Linux 快捷方式写入器。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>无法初始化 COM（写入 .lnk 快捷方式所需）。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) 失败（HRESULT 0x%1）。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="154"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) 失败（HRESULT 0x%1）。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="164"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>保存 .lnk 文件失败（HRESULT 0x%1）。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="186"/>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="155"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>此平台不支持 macOS 快捷方式写入器。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>无法打开快捷方式路径进行写入：%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Serial Studio 快捷方式</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>无法将快捷方式标记为可执行。</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="426"/>
        <source>System</source>
        <translation>系统</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <source>Check for updates automatically?</source>
        <translation type="vanished">自动检查更新?</translation>
    </message>
    <message>
        <source>Should %1 automatically check for updates? You can always check for updates manually from the "About" dialog</source>
        <translation type="vanished">%1 是否自动检查更新?您始终可以从"关于"对话框手动检查更新</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="170"/>
        <source>Ok</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="172"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="174"/>
        <source>Save all</source>
        <translation>全部保存</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="176"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="178"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="180"/>
        <source>Yes to all</source>
        <translation>全部选是</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="182"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="184"/>
        <source>No to all</source>
        <translation>全部否</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="186"/>
        <source>Abort</source>
        <translation>中止</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="188"/>
        <source>Retry</source>
        <translation>重试</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="190"/>
        <source>Ignore</source>
        <translation>忽略</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="192"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="194"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="196"/>
        <source>Discard</source>
        <translation>放弃</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="198"/>
        <source>Help</source>
        <translation>帮助</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="200"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="202"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="204"/>
        <source>Restore defaults</source>
        <translation>恢复默认值</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="267"/>
        <source>Select Workspace Location</source>
        <translation>选择工作区位置</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>协议</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="72"/>
        <source>Serial Port</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="97"/>
        <source>Baud Rate</source>
        <translation>波特率</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="201"/>
        <source>Parity</source>
        <translation>校验位</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="222"/>
        <source>Data Bits</source>
        <translation>数据位</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="245"/>
        <source>Stop Bits</source>
        <translation>停止位</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="268"/>
        <source>Host</source>
        <translation>主机</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="278"/>
        <source>IP Address</source>
        <translation>IP 地址</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="292"/>
        <source>Port</source>
        <translation>端口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="301"/>
        <source>TCP Port</source>
        <translation>TCP 端口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="329"/>
        <source>Slave Address</source>
        <translation>从站地址</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="334"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="382"/>
        <source>Configure Register Groups…</source>
        <translation>配置寄存器组…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="392"/>
        <source>Import Register Map…</source>
        <translation>导入寄存器映射…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="407"/>
        <source>%1 group(s) configured</source>
        <translation>已配置 %1 个组</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="408"/>
        <source>No groups configured</source>
        <translation>未配置组</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="349"/>
        <source>Poll Interval (ms)</source>
        <translation>轮询间隔 (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="354"/>
        <source>Polling interval</source>
        <translation>轮询间隔</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Modbus 寄存器组</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>配置多个寄存器组以按顺序轮询不同的寄存器类型。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>添加新组</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>寄存器类型：</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>起始地址：</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>寄存器数量：</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>添加组</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>已配置的组</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>起始地址</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>数量</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>操作</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>移除</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>未配置组。
在上方添加组以轮询多种寄存器类型。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>总组数：%1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>生成项目</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>全部清除</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Modbus 寄存器映射预览</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>文件:%1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>查看要导入到新 Serial Studio 项目中的寄存器。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>寄存器</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>名称</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>地址</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>数据类型</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>单位</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>文件中未找到寄存器。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>总计：%2 个组中的 %1 个寄存器</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>创建项目</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 发布者</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>已连接到代理</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>未连接</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>测试连接</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>使用当前设置探测代理</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>请先启用发布</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>编辑脚本</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>编辑发布者脚本（Lua 或 JavaScript）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>加载 CA 证书</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>从文件夹添加 PEM 证书</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>请先启用 SSL/TLS</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">插值</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="249"/>
        <source>Interpolation: %1</source>
        <translation>插值:%1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">显示图例</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="271"/>
        <source>Show X Axis Label</source>
        <translation>显示 X 轴标签</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="282"/>
        <source>Show Y Axis Label</source>
        <translation>显示 Y 轴标签</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="294"/>
        <source>Show Crosshair</source>
        <translation>显示十字准线</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="301"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="301"/>
        <source>Resume</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="318"/>
        <source>Sweep / Trigger Mode</source>
        <translation>扫描 / 触发模式</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="330"/>
        <source>Trigger Settings</source>
        <translation>触发设置</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="354"/>
        <source>Reset View</source>
        <translation>重置视图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="360"/>
        <source>Axis Range Settings</source>
        <translation>坐标轴范围设置</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">采样</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>套接字类型</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>远程地址</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>本地端口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>输入 0 以自动分配端口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>远程端口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="219"/>
        <source>Multicast</source>
        <translation>组播</translation>
    </message>
</context>
<context>
    <name>NotificationLog</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="162"/>
        <source>Filter by channel…</source>
        <translation>按通道筛选…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>清除所有通知</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(无标题)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>暂无通知</translation>
    </message>
    <message>
        <source>Dataset transforms and output widget scripts can post events here via notifyInfo / notifyWarning / notifyCritical.</source>
        <translation type="vanished">数据集转换和输出组件脚本可以通过 notifyInfo / notifyWarning / notifyCritical 在此发布事件。</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="42"/>
        <source>Search Online Icons</source>
        <translation>搜索在线图标</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="72"/>
        <source>Download failed: %1</source>
        <translation>下载失败:%1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="97"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>搜索图标(例如 temperature、arrow、play)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="109"/>
        <source>Search</source>
        <translation>搜索</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="148"/>
        <source>Search for icons above to get started</source>
        <translation>在上方搜索图标以开始</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="249"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="259"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>输出组件需要 Pro 许可证。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>按钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>点击时发送命令</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>滑块</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>发送缩放的数值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>开关</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>发送开/关命令</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>文本字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>输入并发送任意命令</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>旋钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>用于设定点的旋转输入</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>您可以配置输出组件，但它们仅在拥有 Pro 许可证时显示在仪表板上。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>复制此输出组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>删除此输出组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>传输函数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>从 .js 文件导入传输函数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>模板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>选择预置的传输函数模板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>测试</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>使用示例输入测试传输函数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>撤销</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>重做</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>剪切</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>粘贴</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>格式化文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>格式化选区</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>绘图器组件错误</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>绘图器组件代码编辑器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>从 .js 文件导入绘图器代码</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>模板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>选择内置绘图模板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>格式化</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>重新格式化绘图代码</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>测试</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>使用模拟数据集值打开实时预览</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>预览</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>剪切</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>粘贴</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>撤销</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>重做</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>格式化文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>格式化选区</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>绘图器实时预览</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>预览</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>模拟数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>运行时正常</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>等待第一帧...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>控制台</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>清空控制台</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <source>Interpolate</source>
        <translation type="vanished">插值</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="245"/>
        <source>Interpolation: %1</source>
        <translation>插值:%1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="258"/>
        <source>Show Area Under Plot</source>
        <translation>显示曲线下方区域</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="276"/>
        <source>Show X Axis Label</source>
        <translation>显示 X 轴标签</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="287"/>
        <source>Show Y Axis Label</source>
        <translation>显示 Y 轴标签</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="299"/>
        <source>Show Crosshair</source>
        <translation>显示十字准线</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="306"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="306"/>
        <source>Resume</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="323"/>
        <source>Sweep / Trigger Mode</source>
        <translation>扫描 / 触发模式</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="335"/>
        <source>Trigger Settings</source>
        <translation>触发设置</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="359"/>
        <source>Reset View</source>
        <translation>重置视图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="365"/>
        <source>Axis Range Settings</source>
        <translation>坐标轴范围设置</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="203"/>
        <source>Interpolate</source>
        <translation>插值</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="221"/>
        <source>Orbit Navigation</source>
        <translation>轨道导航</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="231"/>
        <source>Pan Navigation</source>
        <translation>平移导航</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="242"/>
        <source>Orthogonal View</source>
        <translation>正交视图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="248"/>
        <source>Top View</source>
        <translation>顶视图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="254"/>
        <source>Left View</source>
        <translation>左视图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="260"/>
        <source>Front View</source>
        <translation>前视图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="277"/>
        <source>Auto Center</source>
        <translation>自动居中</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="293"/>
        <source>Anaglyph 3D</source>
        <translation>浮雕 3D</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="307"/>
        <source>Invert Eye Positions</source>
        <translation>反转眼睛位置</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="59"/>
        <source>None</source>
        <translation>无</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="62"/>
        <source>ZOH</source>
        <translation>零阶保持</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="65"/>
        <source>Stem</source>
        <translation>茎状图</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="67"/>
        <source>Linear</source>
        <translation>线性</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1326"/>
        <source>Time</source>
        <translation>时间</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1349"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — 拖动以移动，右键单击以清除</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1352"/>
        <source>Click to place cursor</source>
        <translation>单击以放置光标</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1354"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>单击以放置第二个光标 — 拖动以移动</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>访问网站</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>购买许可证</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>激活</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>助手 — Pro 功能</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>助手是 Serial Studio Pro 功能。激活您的许可证以解锁该功能。</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>激活</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>启动进程</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>命名管道</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>可执行文件</translation>
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
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 value1 --arg2 value2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>工作目录</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(可选) /working/directory</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>管道路径</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>选择运行中的进程…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>启动子进程并捕获其 stdout，或连接到现有进程写入的命名管道。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>了解命名管道</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>选择运行中的进程</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>选择一个运行中的进程以派生命名管道路径建议。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>筛选进程</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>按名称筛选…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>刷新</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>运行中的进程</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>没有进程匹配筛选条件。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>未找到运行中的进程。
点击刷新以更新列表。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 个进程</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>选择</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="42"/>
        <source>modified</source>
        <translation>已修改</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="322"/>
        <source>This project is password protected</source>
        <translation>此项目受密码保护</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="323"/>
        <source>Editing is available in Project mode</source>
        <translation>项目模式下可进行编辑</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="334"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>输入密码以进行更改，或打开其他项目。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="335"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>切换到项目模式以加载和编辑项目。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="357"/>
        <source>Unlock</source>
        <translation>解锁</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="358"/>
        <source>Switch to Project Mode</source>
        <translation>切换到项目模式</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="377"/>
        <source>Open Other Project</source>
        <translation>打开其他项目</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="378"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="394"/>
        <source>Create New Project</source>
        <translation>创建新项目</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>项目结构</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>搜索</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="169"/>
        <source>Move Up</source>
        <translation>上移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="174"/>
        <source>Move Down</source>
        <translation>下移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="185"/>
        <source>Duplicate Selected (%1)</source>
        <translation>复制所选（%1）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="186"/>
        <source>Duplicate</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="199"/>
        <source>Delete Selected (%1)</source>
        <translation>删除所选（%1）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="200"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>新建</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>创建新的 JSON 项目</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>打开现有 JSON 项目</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>保存当前项目</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>另存为</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>以新名称保存当前项目</translation>
    </message>
    <message>
        <source>Unlock</source>
        <translation type="vanished">解锁</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>锁定</translation>
    </message>
    <message>
        <source>Unlock the Project Editor with the project password</source>
        <translation type="vanished">使用项目密码解锁项目编辑器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>从 Protocol Buffers (.proto) 架构生成项目</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>设置密码并锁定项目编辑器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>添加设备</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>向项目添加新数据源（设备）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>操作</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>向项目添加新操作</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>输出</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>恢复当前项目最近的自动快照</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>添加一个带按钮的新输出控制面板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>滑块</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>添加一个输出滑块控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>开关</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>添加一个输出开关控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>旋钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>添加一个输出旋钮控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>文本字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>添加一个输出文本字段控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>按钮</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>添加输出按钮控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>添加通用数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>添加 2D 绘图数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>FFT 绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>添加快速傅里叶变换绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>仪表盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>添加数值数据仪表盘组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>电平指示器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>添加垂直条形电平指示器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>罗盘</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>添加罗盘控件以显示方向数据</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>LED 指示器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>添加 LED 样式状态指示器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>添加数据集容器组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>数据集容器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>图像</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>添加图像/视频流查看器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>图像视图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Painter</source>
        <translation>绘图器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="458"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>添加自定义 JavaScript 渲染的绘图器组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="459"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>绘图器组件需要 Pro 许可证 — 添加后将回退为数据网格</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="460"/>
        <source>Painter Widget</source>
        <translation>绘图器组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="472"/>
        <source>Table</source>
        <translation>表格</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="475"/>
        <source>Add a data table view</source>
        <translation>添加数据表格视图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="477"/>
        <source>Data Grid</source>
        <translation>数据网格</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Multi-Plot</source>
        <translation>多重绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>添加包含多个信号的 2D 绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="487"/>
        <source>Multiple Plot</source>
        <translation>多重绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="492"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="497"/>
        <source>3D Plot</source>
        <translation>3D 绘图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Add a 3D plot visualization</source>
        <translation>添加 3D 绘图可视化</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="507"/>
        <source>Accelerometer</source>
        <translation>加速度计</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>添加用于 3 轴加速度计数据的组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="516"/>
        <source>Gyroscope</source>
        <translation>陀螺仪</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="517"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>添加用于 3 轴陀螺仪数据的组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="522"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="527"/>
        <source>GPS Map</source>
        <translation>GPS 地图</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a map widget for GPS data</source>
        <translation>添加用于 GPS 数据的地图组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="539"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="543"/>
        <source>Assistant</source>
        <translation>助手</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="546"/>
        <source>Open the Assistant</source>
        <translation>打开助手</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="552"/>
        <source>Help Center</source>
        <translation>帮助中心</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="556"/>
        <source>Open the Project Editor documentation</source>
        <translation>打开项目编辑器文档</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>项目摘要</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>项目中检测到 Pro 功能。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>正在使用备用组件。购买许可证以解锁完整功能。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>项目标题:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>未命名项目</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">点数:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="149"/>
        <source>Time Range:</source>
        <translation>时间范围:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="235"/>
        <source>Source</source>
        <translation>源</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="236"/>
        <source>Sources</source>
        <translation>源</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="241"/>
        <source>Group</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="242"/>
        <source>Groups</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="247"/>
        <source>Dataset</source>
        <translation>数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="248"/>
        <source>Datasets</source>
        <translation>数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="253"/>
        <source>Action</source>
        <translation>操作</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="254"/>
        <source>Actions</source>
        <translation>操作</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="342"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>双击块进行编辑。右键单击任意位置以添加组、数据集、动作、数据表或设备。</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Protocol Buffers 文件预览</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Proto 文件：%1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>浏览下方的消息，然后创建项目。每条消息将成为一个仪表板组；相同类型的通道块将获得多重绘图，混合类型的消息将获得数据网格。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>显示字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>标签</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>字段名称</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>所选消息中没有字段。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>总计：%1 条消息，%2 个字段</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>创建项目</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>无错误</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>代理因不支持的协议版本拒绝了连接。请匹配代理的 MQTT 版本后重试。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>代理拒绝了客户端 ID。它可能格式错误、过长或已被使用。请重新生成后重试。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>网络已到达 Broker，但 Broker 当前不可用。请验证其状态并稍后重试。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>用户名或密码不正确或格式错误。请仔细检查凭据并重试。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Broker 因权限不足拒绝了连接。请验证账户具有所需的 ACL。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>网络或传输层问题阻止了连接。请检查连接性、端口和 TLS 配置。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>客户端检测到违反 MQTT 协议的行为并关闭了连接。请验证 Broker 和客户端的兼容性。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>发生意外错误。请检查 Broker 日志和应用程序控制台以获取详细信息。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>发生 MQTT 5 协议级别错误。请检查 Broker 的原因代码以获取详细信息。</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>未指定的 MQTT 错误（代码 %1）。</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="234"/>
        <source>Failed to load welcome text :(</source>
        <translation>加载欢迎文本失败 :(</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="264"/>
        <source>Network error</source>
        <translation>网络错误</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <location filename="../../src/Licensing/Trial.cpp" line="284"/>
        <location filename="../../src/Licensing/Trial.cpp" line="317"/>
        <source>Trial Activation Error</source>
        <translation>试用激活错误</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="281"/>
        <source>Invalid server response</source>
        <translation>无效的服务器响应</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="282"/>
        <source>The server returned malformed data: %1</source>
        <translation>服务器返回格式错误的数据：%1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="314"/>
        <source>Unexpected server response</source>
        <translation>意外的服务器响应</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="315"/>
        <source>The server response is missing required fields.</source>
        <translation>服务器响应缺少必需字段。</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="168"/>
        <source>Console Output File Error</source>
        <translation>控制台输出文件错误</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="169"/>
        <source>Cannot open file for writing!</source>
        <translation>无法打开文件进行写入！</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="805"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>蓝牙适配器无效！</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="808"/>
        <source>Unsuported platform or operating system</source>
        <translation>不支持的平台或操作系统</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="811"/>
        <source>Unsupported discovery method</source>
        <translation>不支持的发现方法</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="814"/>
        <source>General I/O error</source>
        <translation>常规 I/O 错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="275"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="255"/>
        <source>Frame Parser Disabled</source>
        <translation>帧解析器已禁用</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="276"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>数据源 %1 的 Lua 帧解析器连续 %2 帧超时，已被禁用以保持 Serial Studio 响应。

最可能的原因：脚本主体中存在无限循环或极慢的操作。请修复脚本并重新加载项目以重新启用解析。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="324"/>
        <source>Lua Syntax Error</source>
        <translation>Lua 语法错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="325"/>
        <source>The parser code contains an error:

%1</source>
        <translation>解析器代码包含错误：

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="375"/>
        <source>Lua Runtime Error</source>
        <translation>Lua 运行时错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="376"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>解析器代码触发了错误：

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="397"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>Missing Parse Function</source>
        <translation>缺少解析函数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="398"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>脚本中未定义 'parse' 函数。

请确保您的代码包含：
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="460"/>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="533"/>
        <source>Parse Function Runtime Error</source>
        <translation>解析函数运行时错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="461"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>解析函数包含错误：

%1

请修复函数体中的错误。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="256"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>数据源 %1 的 JavaScript 帧解析器连续 %2 帧超时，已被禁用以保持 Serial Studio 响应。

最可能的原因：脚本主体中存在无限循环或极慢的操作。请修复脚本并重新加载项目以重新启用解析。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="418"/>
        <source>JavaScript Timed Out</source>
        <translation>JavaScript 超时</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>解析器代码未能在 %1 毫秒内完成评估并被中断。

最可能的原因:脚本顶层存在无限循环。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>JavaScript 语法错误</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>解析器代码在第 %1 行包含语法错误：

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>JavaScript Exception Occurred</source>
        <translation>JavaScript 异常发生</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="454"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>解析器代码触发了以下异常：

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="480"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>脚本中未定义 'parse' 函数。

请确保您的代码包含：
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="534"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>解析函数在第 %1 行包含错误：

%2

请修复函数体中的错误。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="637"/>
        <source>Invalid Function Declaration</source>
        <translation>函数声明无效</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="638"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>未找到可调用的 'parse' 导出。

请定义以下之一：
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="654"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>'parse' 函数必须接受至少一个参数（即 frame 有效载荷）。</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">未找到有效的 'parse' 函数声明。

预期格式：
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="653"/>
        <source>Invalid Function Parameter</source>
        <translation>无效的函数参数</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">'parse' 函数必须至少有一个参数。

预期格式：
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="618"/>
        <source>Deprecated Function Signature</source>
        <translation>已弃用的函数签名</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="619"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>'parse' 函数使用旧的双参数格式：parse(%1, %2)

不再支持此格式。请更新为新的单参数格式：
function parse(%1) { ... }

不再需要分隔符参数。</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="182"/>
        <source>Critical</source>
        <translation>严重</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="182"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="523"/>
        <source>Project file not found</source>
        <translation>未找到项目文件</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="524"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>此快捷方式引用的项目文件无法找到：

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="527"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>是否删除此快捷方式？</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="531"/>
        <source>Delete Shortcut</source>
        <translation>删除快捷方式</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="533"/>
        <source>Quit</source>
        <translation>退出</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1070"/>
        <source>Time (s)</source>
        <translation>时间 (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1132"/>
        <source>Freq: %1</source>
        <translation>频率:%1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1136"/>
        <source>Time: −%1</source>
        <translation>时间:−%1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="327"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>未设置 OpenAI API 密钥。打开管理密钥以添加。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="199"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>未设置 Anthropic API 密钥。打开管理密钥以添加。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="282"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>未设置 Gemini API 密钥。打开管理密钥以添加。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="321"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>未配置本地模型服务器 URL。请打开"管理密钥"进行设置。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="141"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>未设置 DeepSeek API 密钥。请打开"管理密钥"进行添加。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="164"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>未设置 Mistral API 密钥。打开管理密钥以添加。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="177"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>未设置 OpenRouter API 密钥。打开管理密钥以添加。</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="148"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>未设置 Groq API 密钥。打开管理密钥以添加。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="657"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>帧解析器占用超过 %1% 的 CPU 时间。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="659"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio 正在丢弃帧以保持应用程序响应。请简化或优化帧解析器脚本以减少其工作负载。</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="388"/>
        <source>Expected %1, got '%2'</source>
        <translation>预期 %1，实际为 '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="437"/>
        <source>Expected enum name after 'enum'</source>
        <translation>'enum' 后应为枚举名称</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="451"/>
        <source>Expected oneof name</source>
        <translation>应为 oneof 名称</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="479"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>字段标签 '%1' 超出范围 (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="497"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>map&lt;&gt; 中应为键类型</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="505"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>map&lt;&gt; 中应为值类型</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="513"/>
        <source>Expected map field name</source>
        <translation>应为 map 字段名称</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="525"/>
        <source>Expected map field tag</source>
        <translation>应为 map 字段标签</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="557"/>
        <source>Expected field type, got '%1'</source>
        <translation>预期字段类型，实际为 '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="577"/>
        <source>Expected field name after type</source>
        <translation>类型后应为字段名称</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="587"/>
        <source>Expected field tag number</source>
        <translation>应为字段标签编号</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>消息嵌套过深（限制 %1）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="640"/>
        <source>Expected message name</source>
        <translation>应为消息名称</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="722"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>文件作用域中出现意外标记 '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="768"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>不支持的顶层关键字 '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>自动（平台默认）</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>软件（备用）</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="93"/>
        <source>Row %1</source>
        <translation>行 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="98"/>
        <source>[%1]</source>
        <translation>[%1]</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="112"/>
        <source>Frame %1</source>
        <translation>帧 %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="119"/>
        <source>Decoder</source>
        <translation>解码器</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="123"/>
        <source>Rows</source>
        <translation>行数</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/FrameParserTestDialog.cpp" line="124"/>
        <source>%1 row(s)</source>
        <translation>%1 行</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>GZIP 不支持 QIODevice::Append</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>不支持同时以读写方式打开 gzip</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>只能以读取或写入方式打开 gzip，请选择其一。</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>无法 gzopen() 文件</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QuaZIODevice 不支持 QIODevice::Append</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QuaZIODevice 不支持 QIODevice::ReadWrite</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>ZIP/UNZIP API 错误 %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>生成 PDF 报告</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>生成报告</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="61"/>
        <source>Solid</source>
        <translation>实线</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="62"/>
        <source>Dashed</source>
        <translation>虚线</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="63"/>
        <source>Dotted</source>
        <translation>点线</translation>
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
        <translation>Letter (8.5 × 11 英寸)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="90"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8.5 × 14 英寸)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="91"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7.25 × 10.5 英寸)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="92"/>
        <source>Tabloid (11 × 17 in)</source>
        <translation>Tabloid (11 × 17 英寸)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="93"/>
        <source>Ledger (17 × 11 in)</source>
        <translation>Ledger (17 × 11 英寸)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>%1 — Session Report</source>
        <translation>%1 — 会话报告</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="105"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="279"/>
        <source>Session Report</source>
        <translation>会话报告</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="187"/>
        <source>Branding</source>
        <translation>品牌标识</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="193"/>
        <source>Page</source>
        <translation>页面</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="199"/>
        <source>Sections</source>
        <translation>部分</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="247"/>
        <source>Identity</source>
        <translation>标识</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="261"/>
        <source>Company</source>
        <translation>公司</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="268"/>
        <source>e.g. Acme Test Systems</source>
        <translation>例如 Acme Test Systems</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="272"/>
        <source>Document title</source>
        <translation>文档标题</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="283"/>
        <source>Author</source>
        <translation>作者</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="290"/>
        <source>Prepared by (optional)</source>
        <translation>制作者（可选）</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="299"/>
        <source>Logo</source>
        <translation>徽标</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="312"/>
        <source>File</source>
        <translation>文件</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="323"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG、JPG 或 SVG（可选）</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="325"/>
        <source>Browse…</source>
        <translation>浏览…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="328"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="369"/>
        <source>Paper</source>
        <translation>纸张</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="381"/>
        <source>Page size</source>
        <translation>页面大小</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="509"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>在图表上标注最小值、最大值和平均值</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export HTML</source>
        <translation>导出 HTML</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="497"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>封面页（徽标、文档标题、测试副标题）</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="500"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>测试信息(项目、时间戳、分类和备注)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="503"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>测量摘要(每个参数的最小值、最大值、平均值、标准差)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="506"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>参数趋势(每个数值参数的时间序列图表)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="396"/>
        <source>Plot appearance</source>
        <translation>绘图外观</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="410"/>
        <source>Line width</source>
        <translation>线宽</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="442"/>
        <source>Line style</source>
        <translation>线型</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="482"/>
        <source>Include</source>
        <translation>包含</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="532"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="557"/>
        <source>Export PDF</source>
        <translation>导出 PDF</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>正在生成报告</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>处理中…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>对于包含大量参数的会话,此操作可能需要几秒钟。报告准备就绪后窗口将自动关闭。</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Connection Lost</source>
        <translation>连接丢失</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="42"/>
        <source>Device Unavailable</source>
        <translation>设备不可用</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>The connection to your device was lost.</source>
        <translation>与设备的连接已丢失。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="97"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio 无法连接到您的设备。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>请检查线缆、电源,并确保没有其他应用程序占用该设备。您可以尝试重新连接、切换到其他设备或退出。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>请确保设备已插入、已开机且未被其他应用程序占用。您可以重试、选择其他设备或退出。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>退出</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>选择其他设备</translation>
    </message>
    <message>
        <source>Reconfigure</source>
        <translation type="vanished">重新配置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>重试</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>重新连接</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>选择正确的设备,然后按连接。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>I/O 接口:%1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="336"/>
        <source>Data Grids</source>
        <translation>数据网格</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="339"/>
        <source>Multiple Data Plots</source>
        <translation>多重数据绘图</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="342"/>
        <source>Accelerometers</source>
        <translation>加速度计</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="345"/>
        <source>Gyroscopes</source>
        <translation>陀螺仪</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="348"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="351"/>
        <source>FFT Plots</source>
        <translation>FFT 绘图</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="354"/>
        <source>LED Panels</source>
        <translation>LED 面板</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="357"/>
        <source>Data Plots</source>
        <translation>数据绘图</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="360"/>
        <source>Bars</source>
        <translation>条形图</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="363"/>
        <source>Gauges</source>
        <translation>仪表盘</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="366"/>
        <source>Terminal</source>
        <translation>终端</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="369"/>
        <source>Clock</source>
        <translation>时钟</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="372"/>
        <source>Stopwatch</source>
        <translation>秒表</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="375"/>
        <source>Compasses</source>
        <translation>罗盘</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="378"/>
        <source>Meters</source>
        <translation>仪表</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">温度计</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="381"/>
        <source>3D Plots</source>
        <translation>3D 绘图</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="385"/>
        <source>Image Views</source>
        <translation>图像视图</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="388"/>
        <source>Output Panels</source>
        <translation>输出面板</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="391"/>
        <source>Notifications</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="394"/>
        <source>Waterfalls</source>
        <translation>瀑布图</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="397"/>
        <source>Painter Widgets</source>
        <translation>绘图组件</translation>
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
        <translation>系统</translation>
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
        <translation>会话详情</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>选择一个会话以查看详细信息。</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>项目：</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>开始时间：</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>结束时间：</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(进行中)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>帧数：</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>备注</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>添加会话备注…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>已完成会话的备注为只读。</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>新建标签…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>解锁会话文件以删除会话</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>标签</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>添加</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>回放</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>导出 CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>生成报告</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>会话</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>搜索</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>日期</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>帧</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>标签</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>未找到会话。</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>未打开会话文件。</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Select logo image</source>
        <translation>选择徽标图像</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1000"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>图像 (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="418"/>
        <source>Open Session File</source>
        <translation>打开会话文件</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="420"/>
        <source>Session files (*.db)</source>
        <translation>会话文件 (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1198"/>
        <source>Cannot open session file</source>
        <translation>无法打开会话文件</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="647"/>
        <source>Delete session from %1?</source>
        <translation>删除来自 %1 的会话？</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="650"/>
        <source>Delete Session</source>
        <translation>删除会话</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1059"/>
        <source>No project data</source>
        <translation>无项目数据</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="648"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>此会话的所有读数和原始数据将被永久删除。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="477"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <source>Lock Session File</source>
        <translation>锁定会话文件</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="478"/>
        <source>Choose a password to lock the session file:</source>
        <translation>选择密码以锁定会话文件：</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="487"/>
        <source>Confirm the password:</source>
        <translation>确认密码：</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>Passwords do not match</source>
        <translation>密码不匹配</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="496"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>两次输入的密码不匹配。会话文件未锁定。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="532"/>
        <source>Unlock Session File</source>
        <translation>解锁会话文件</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="533"/>
        <source>Enter the session file password:</source>
        <translation>输入会话文件密码：</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="543"/>
        <source>Incorrect password</source>
        <translation>密码错误</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="544"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>输入的密码与会话文件中存储的密码不匹配。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="637"/>
        <source>Session file locked</source>
        <translation>会话文件已锁定</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="638"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>删除已记录会话前需先解锁会话文件。</translation>
    </message>
    <message>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation type="vanished">此会话不包含嵌入的项目文件 — 仪表板回退到快速绘图布局。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="772"/>
        <source>Export Session to CSV</source>
        <translation>将会话导出为 CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="772"/>
        <source>CSV files (*.csv)</source>
        <translation>CSV 文件 (*.CSV)</translation>
    </message>
    <message>
        <source>Export Complete</source>
        <translation type="vanished">导出完成</translation>
    </message>
    <message>
        <source>Session exported to:
%1</source>
        <translation type="vanished">会话已导出至：
%1</translation>
    </message>
    <message>
        <source>Preparing export…</source>
        <translation type="vanished">正在准备导出…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <source>Done</source>
        <translation>完成</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="934"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <source>Failed</source>
        <translation>失败</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="940"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="980"/>
        <source>Report Failed</source>
        <translation>报告失败</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="942"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="981"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>无法生成报告。请检查输出路径并重试。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="868"/>
        <source>Save PDF Report</source>
        <translation>保存 PDF 报告</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="848"/>
        <source>Loading session data…</source>
        <translation>正在加载会话数据…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="868"/>
        <source>Save HTML Report</source>
        <translation>保存 HTML 报告</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="869"/>
        <source>PDF files (*.pdf)</source>
        <translation>PDF 文件 (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="869"/>
        <source>HTML files (*.html)</source>
        <translation>HTML 文件 (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1060"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>此会话文件不包含嵌入的项目。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1069"/>
        <source>Invalid project data</source>
        <translation>无效的项目数据</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1070"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>嵌入的项目 JSON 格式错误，无法恢复。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1080"/>
        <source>Restore Project</source>
        <translation>恢复项目</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1080"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Serial Studio 项目 (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1088"/>
        <source>Cannot write file</source>
        <translation>无法写入文件</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1088"/>
        <source>Check file permissions and try again.</source>
        <translation>检查文件权限并重试。</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="77"/>
        <source>Empty file path</source>
        <translation>文件路径为空</translation>
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
        <translation>数据库未打开</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="267"/>
        <source>Database not open or empty label</source>
        <translation>数据库未打开或标签为空</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="335"/>
        <source>Invalid label</source>
        <translation>无效标签</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="603"/>
        <source>Cancelled</source>
        <translation>已取消</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="716"/>
        <source>Could not load session data</source>
        <translation>无法加载会话数据</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="212"/>
        <source>Assembling report…</source>
        <translation>正在组装报告…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="221"/>
        <source>Writing output…</source>
        <translation>正在写入输出…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="293"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="357"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="731"/>
        <source>Session Report</source>
        <translation>会话报告</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="360"/>
        <source>Untitled project</source>
        <translation>未命名项目</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="368"/>
        <source>Prepared by</source>
        <translation>准备者</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="371"/>
        <source>Generated on %1</source>
        <translation>生成于 %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="394"/>
        <source>Test ID</source>
        <translation>测试 ID</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="396"/>
        <source>Duration</source>
        <translation>持续时间</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="398"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="515"/>
        <source>Samples</source>
        <translation>采样</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="400"/>
        <source>Parameters</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="402"/>
        <source>Started</source>
        <translation>开始时间</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="404"/>
        <source>Ended</source>
        <translation>结束时间</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="440"/>
        <source>Project</source>
        <translation>项目</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="442"/>
        <source>Test identifier</source>
        <translation>测试标识符</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="443"/>
        <source>Start time</source>
        <translation>开始时间</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="444"/>
        <source>End time</source>
        <translation>结束时间</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="445"/>
        <source>Total duration</source>
        <translation>总持续时间</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Samples acquired</source>
        <translation>已采集采样</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="447"/>
        <source>Parameters logged</source>
        <translation>已记录参数</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="465"/>
        <source>Classification</source>
        <translation>分类</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="473"/>
        <source>Notes</source>
        <translation>备注</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="481"/>
        <source>Test Information</source>
        <translation>测试信息</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="504"/>
        <source>Parameter</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="507"/>
        <source>Units</source>
        <translation>单位</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="516"/>
        <source>Minimum</source>
        <translation>最小值</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="517"/>
        <source>Maximum</source>
        <translation>最大值</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="518"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="680"/>
        <source>Mean</source>
        <translation>平均值</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="519"/>
        <source>Std. Deviation</source>
        <translation>标准差</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="565"/>
        <source>Measurement Summary</source>
        <translation>测量摘要</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="566"/>
        <source>click a column to sort</source>
        <translation>点击列标题进行排序</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="592"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 个采样，历时 %2 秒</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="611"/>
        <source>Combined Parameter View</source>
        <translation>参数组合视图</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="612"/>
        <source>click legend items to toggle signals</source>
        <translation>点击图例项目以切换信号</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="620"/>
        <source>Parameter Trends</source>
        <translation>参数趋势</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="678"/>
        <source>Min</source>
        <translation>最小值</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="679"/>
        <source>Max</source>
        <translation>最大值</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="758"/>
        <source>Page %1 of %2</source>
        <translation>第 %1 页，共 %2 页</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="828"/>
        <source>Loading rendering engine…</source>
        <translation>正在加载渲染引擎…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="850"/>
        <source>Rendering charts…</source>
        <translation>正在渲染图表…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="895"/>
        <source>Generating PDF…</source>
        <translation>正在生成 PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="259"/>
        <source>Open Session File</source>
        <translation>打开会话文件</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Session files (*.db)</source>
        <translation>会话文件 (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="337"/>
        <source>Device Connection Active</source>
        <translation>设备连接活动</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="338"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>要使用此功能,必须断开与设备的连接。是否继续?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="388"/>
        <location filename="../../src/Sessions/Player.cpp" line="470"/>
        <source>Cannot open session file</source>
        <translation>无法打开会话文件</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="389"/>
        <source>Unknown error</source>
        <translation>未知错误</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="407"/>
        <source>No project data</source>
        <translation>无项目数据</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="408"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>此会话不包含嵌入的项目文件 — 仪表板回退到快速绘图布局。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="471"/>
        <source>Check file permissions and try again.</source>
        <translation>检查文件权限并重试。</translation>
    </message>
    <message>
        <source>No sessions found</source>
        <translation type="vanished">未找到会话</translation>
    </message>
    <message>
        <source>This file does not contain any recording sessions.</source>
        <translation type="vanished">此文件不包含任何录制会话。</translation>
    </message>
    <message>
        <source>Session has no columns</source>
        <translation type="vanished">会话没有列</translation>
    </message>
    <message>
        <source>The selected session is missing its column definitions.</source>
        <translation type="vanished">所选会话缺少列定义。</translation>
    </message>
    <message>
        <source>Session has no readings</source>
        <translation type="vanished">会话没有读数</translation>
    </message>
    <message>
        <source>The selected session does not contain any frames.</source>
        <translation type="vanished">所选会话不包含任何帧。</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>文件路径为空</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>此文件不包含任何录制会话。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>已取消</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>所选会话缺少列定义。</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>所选会话不包含任何帧。</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>偏好设置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>常规</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="159"/>
        <source>Language</source>
        <translation>语言</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="175"/>
        <source>Theme</source>
        <translation>主题</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="210"/>
        <source>Workspace Folder</source>
        <translation>工作区文件夹</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Automatically Check for Updates</source>
        <translation>自动检查更新</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Dashboard</source>
        <translation>仪表板</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="386"/>
        <source>Export…</source>
        <translation>导出…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="435"/>
        <source>Data Plotting</source>
        <translation>数据绘图</translation>
    </message>
    <message>
        <source>Point Count</source>
        <translation type="vanished">点数</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="500"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>界面刷新率 (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="737"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>始终显示任务栏按钮</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="624"/>
        <source>Show Actions Panel</source>
        <translation>显示操作面板</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>启用 API 服务器（端口 7777）</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="849"/>
        <source>Console</source>
        <translation>控制台</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="144"/>
        <source>Appearance</source>
        <translation>外观</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="194"/>
        <source>Files &amp; Updates</source>
        <translation>文件与更新</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="255"/>
        <source>Advanced</source>
        <translation>高级</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="338"/>
        <source>Allow External API Connections</source>
        <translation>允许外部 API 连接</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="303"/>
        <source>Auto-Hide Toolbar</source>
        <translation>自动隐藏工具栏</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Taskbar</source>
        <translation>任务栏</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="270"/>
        <source>Rendering Backend</source>
        <translation>渲染后端</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>API Access Token</source>
        <translation>API 访问令牌</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="450"/>
        <source>Time Range</source>
        <translation>时间范围</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Small</source>
        <translation>小</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Normal</source>
        <translation>正常</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Large</source>
        <translation>大</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Extra Large</source>
        <translation>超大</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="562"/>
        <source>Custom</source>
        <translation>自定义</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="609"/>
        <source>Layout</source>
        <translation>布局</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="647"/>
        <source>Video Export</source>
        <translation>视频导出</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="665"/>
        <source>Save Videos by Default</source>
        <translation>默认保存视频</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Behavior</source>
        <translation>行为</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="752"/>
        <source>Show Search Field</source>
        <translation>显示搜索字段</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="767"/>
        <source>Auto-hide Taskbar</source>
        <translation>自动隐藏任务栏</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="785"/>
        <source>Hide Delay (ms)</source>
        <translation>隐藏延迟 (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="809"/>
        <source>Pinned Buttons</source>
        <translation>固定按钮</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="827"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>在任务栏上拖动固定按钮以重新排序。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="848"/>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="851"/>
        <source>Clock</source>
        <translation>时钟</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="852"/>
        <source>Stopwatch</source>
        <translation>秒表</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="853"/>
        <source>Pause / Resume</source>
        <translation>暂停 / 恢复</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="854"/>
        <source>File Transmission</source>
        <translation>文件传输</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="855"/>
        <source>AI Assistant</source>
        <translation>AI 助手</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="984"/>
        <source>Display</source>
        <translation>显示</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="999"/>
        <source>Display Mode</source>
        <translation>显示模式</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="537"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1012"/>
        <source>Font Family</source>
        <translation>字体系列</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="86"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="850"/>
        <source>Notifications</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="384"/>
        <source>Export Protobuf File</source>
        <translation>导出 Protobuf 文件</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="522"/>
        <source>Dashboard Font</source>
        <translation>仪表板字体</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="552"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1027"/>
        <source>Font Size</source>
        <translation>字体大小</translation>
    </message>
    <message>
        <source>Image Export</source>
        <translation type="vanished">图像导出</translation>
    </message>
    <message>
        <source>Save Images by Default</source>
        <translation type="vanished">默认保存图像</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1044"/>
        <source>Show Timestamps</source>
        <translation>显示时间戳</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1063"/>
        <source>Data Transmission</source>
        <translation>数据传输</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1078"/>
        <source>Line Ending</source>
        <translation>行结束符</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1091"/>
        <source>Input Mode</source>
        <translation>输入模式</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1104"/>
        <source>Text Encoding</source>
        <translation>文本编码</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1117"/>
        <source>Checksum</source>
        <translation>校验和</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1130"/>
        <source>Echo Sent Data</source>
        <translation>回显发送数据</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1149"/>
        <source>Escape Codes</source>
        <translation>转义码</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>VT100 Emulation</source>
        <translation>VT100 仿真</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1183"/>
        <source>ANSI Colors</source>
        <translation>ANSI 颜色</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1241"/>
        <source>Delivery</source>
        <translation>传递</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1256"/>
        <source>System Notifications</source>
        <translation>系统通知</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1277"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>当 Serial Studio 不在前台窗口时,将警告/严重事件显示为操作系统桌面通知。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1287"/>
        <source>Application Logs</source>
        <translation>应用程序日志</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1302"/>
        <source>Route Warnings to Notifications</source>
        <translation>将警告路由至通知</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1323"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>默认关闭 — QT 和 QML 频繁发出警告，启用此选项可能会淹没真正的警报。无论此设置如何，关键消息始终会被路由。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1342"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1385"/>
        <source>Apply</source>
        <translation>应用</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>设备设置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>API 服务器活动 (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>API 服务器就绪</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>API 服务器关闭</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>帧解析</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>仅控制台(不解析)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>快速绘图(逗号分隔值)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>通过项目文件解析</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>更改项目文件 (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>选择项目文件</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>数据导出</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>CSV 电子表格</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>会话记录</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>MDF4 记录</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>控制台日志</translation>
    </message>
    <message>
        <source>CSV File</source>
        <translation type="vanished">CSV 文件</translation>
    </message>
    <message>
        <source>Session Database</source>
        <translation type="vanished">会话数据库</translation>
    </message>
    <message>
        <source>MDF4 File</source>
        <translation type="vanished">MDF4 文件</translation>
    </message>
    <message>
        <source>Console Dump</source>
        <translation type="vanished">控制台转储</translation>
    </message>
    <message>
        <source>Create CSV File</source>
        <translation type="vanished">创建 CSV 文件</translation>
    </message>
    <message>
        <source>Create MDF4 File</source>
        <translation type="vanished">创建 MDF4 文件</translation>
    </message>
    <message>
        <source>Create Session Log</source>
        <translation type="vanished">创建会话日志</translation>
    </message>
    <message>
        <source>Export Console Data</source>
        <translation type="vanished">导出控制台数据</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>I/O 接口:%1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>多设备项目</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>此项目从 %1 个独立设备流式传输数据。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>每个设备都有自己的连接设置。在项目编辑器的"数据源"选项卡中配置它们。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>打开项目编辑器</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <source>New Shortcut</source>
        <translation type="vanished">新建快捷方式</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="95"/>
        <source>Choose an Icon</source>
        <translation>选择图标</translation>
    </message>
    <message>
        <source>Save Shortcut</source>
        <translation type="vanished">保存快捷方式</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="24"/>
        <source>New Deployment</source>
        <translation>新建部署</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="108"/>
        <source>Save Deployment</source>
        <translation>保存部署</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="148"/>
        <source>General</source>
        <translation>常规</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="154"/>
        <source>Taskbar</source>
        <translation>任务栏</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="160"/>
        <source>Logging</source>
        <translation>日志记录</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="217"/>
        <source>Identity</source>
        <translation>标识</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="273"/>
        <source>Click to choose an icon</source>
        <translation>点击以选择图标</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="282"/>
        <source>Name:</source>
        <translation>名称:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="291"/>
        <source>Deployment Name</source>
        <translation>部署名称</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="382"/>
        <source>Theme</source>
        <translation>主题</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="392"/>
        <source>Same as Serial Studio</source>
        <translation>与 Serial Studio 相同</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="414"/>
        <source>Actions Panel</source>
        <translation>操作面板</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="425"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="634"/>
        <source>File Transmission</source>
        <translation>文件传输</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="441"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>双击此部署可直接进入该项目的实时仪表板。没有工具栏或设置面板,只有数据,并且 Serial Studio 会在设备断开连接后立即退出。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="487"/>
        <source>Visibility</source>
        <translation>可见性</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="502"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="511"/>
        <source>Always shown</source>
        <translation>始终显示</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="512"/>
        <source>Auto-hide</source>
        <translation>自动隐藏</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="513"/>
        <source>Hidden</source>
        <translation>隐藏</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="528"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>隐藏任务栏会移除窗口最小化/最大化/关闭按钮并强制自动布局,使仪表板始终填充可用区域。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>当用户将光标移至底部边缘附近时,任务栏会滑入。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="534"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>任务栏在仪表板底部永久可见。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="547"/>
        <source>Pinned Buttons</source>
        <translation>固定按钮</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="564"/>
        <source>Console</source>
        <translation>控制台</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="578"/>
        <source>Notifications</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="592"/>
        <source>Clock</source>
        <translation>时钟</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="606"/>
        <source>Stopwatch</source>
        <translation>秒表</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="620"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="745"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>录制内容保存在 Serial Studio 工作区文件夹中</translation>
    </message>
    <message>
        <source>Shortcut Name</source>
        <translation type="vanished">快捷方式名称</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="300"/>
        <source>Change Icon…</source>
        <translation>更改图标…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="317"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="335"/>
        <source>Project</source>
        <translation>项目</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="345"/>
        <source>Choose a project file to begin</source>
        <translation>选择项目文件以开始</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="367"/>
        <source>Behavior</source>
        <translation>行为</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="404"/>
        <source>Fullscreen</source>
        <translation>全屏</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation type="vanished">双击此快捷方式可直接进入该项目的实时仪表板。没有工具栏或设置面板,只有数据,并且 Serial Studio 会在设备断开连接后立即退出。</translation>
    </message>
    <message>
        <source>Embed Project</source>
        <translation type="vanished">嵌入项目</translation>
    </message>
    <message>
        <source>Double-clicking this shortcut takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.

Turn on Embed Project to bake the project into the shortcut, so it keeps working even if the original file is moved or deleted.</source>
        <translation type="vanished">双击此快捷方式可直接进入该项目的实时仪表板。没有工具栏或设置面板,只有数据,并且 Serial Studio 会在设备断开连接后立即退出。

启用"嵌入项目"可将项目嵌入快捷方式中,即使原始文件被移动或删除,快捷方式仍可正常工作。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="684"/>
        <source>Recorders</source>
        <translation>记录器</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="699"/>
        <source>CSV File</source>
        <translation>CSV 文件</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="709"/>
        <source>MDF4 File</source>
        <translation>MDF4 文件</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="719"/>
        <source>Session Database</source>
        <translation>会话数据库</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="729"/>
        <source>Console Log</source>
        <translation>控制台日志</translation>
    </message>
    <message>
        <source>Recordings are saved to each module’s default location.</source>
        <translation type="vanished">记录保存到各模块的默认位置。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="774"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="783"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="110"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="242"/>
        <source>Undo</source>
        <translation>撤销</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="117"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="253"/>
        <source>Redo</source>
        <translation>重做</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="272"/>
        <source>Cut</source>
        <translation>剪切</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="131"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="282"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="292"/>
        <source>Paste</source>
        <translation>粘贴</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="143"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="153"/>
        <source>Format Document</source>
        <translation>格式化文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="160"/>
        <source>Format Selection</source>
        <translation>格式化选区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="227"/>
        <source>Reset to the default parsing script</source>
        <translation>重置为默认解析脚本</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="232"/>
        <source>Open</source>
        <translation>打开</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="237"/>
        <source>Import a script file for data parsing</source>
        <translation>导入用于数据解析的脚本文件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="315"/>
        <source>Open help documentation for data parsing</source>
        <translation>打开数据解析帮助文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Language:</source>
        <translation>语言：</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="378"/>
        <source>Select Template…</source>
        <translation>选择模板…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Undo the last code edit</source>
        <translation>撤销上次代码编辑</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="259"/>
        <source>Redo the previously undone edit</source>
        <translation>重做上次撤销的编辑</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="277"/>
        <source>Cut selected code to clipboard</source>
        <translation>剪切选定代码到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="287"/>
        <source>Copy selected code to clipboard</source>
        <translation>复制选中的代码到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="296"/>
        <source>Paste code from clipboard</source>
        <translation>从剪贴板粘贴代码</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="310"/>
        <source>Help</source>
        <translation>帮助</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="387"/>
        <source>Test With Sample Data</source>
        <translation>使用示例数据测试</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="394"/>
        <source>Evaluate</source>
        <translation>评估</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>创建此数据源的副本</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>移除此数据源</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>主数据源无法移除</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="25"/>
        <source>Session Player</source>
        <translation>会话播放器</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="92"/>
        <source>Loading session…</source>
        <translation>正在加载会话…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="276"/>
        <source>Workspaces</source>
        <translation>工作区</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="406"/>
        <source>Actions</source>
        <translation>操作</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="429"/>
        <source>No Actions Available</source>
        <translation>无可用操作</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="459"/>
        <source>Plugins</source>
        <translation>插件</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="507"/>
        <source>No Plugins Installed</source>
        <translation>未安装插件</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="99"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="543"/>
        <source>Auto Layout</source>
        <translation>自动布局</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="107"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="555"/>
        <source>Full Screen</source>
        <translation>全屏</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="113"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="568"/>
        <source>Add External Window</source>
        <translation>添加外部窗口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="155"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="853"/>
        <source>Help Center</source>
        <translation>帮助中心</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="684"/>
        <source>Tools</source>
        <translation>工具</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="813"/>
        <source>No Tools Available</source>
        <translation>无可用工具</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="881"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Quit</source>
        <translation>退出</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="939"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="940"/>
        <source>Hide</source>
        <translation>隐藏</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>New Workspace…</source>
        <translation>新建工作区…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="757"/>
        <source>Clock</source>
        <translation>时钟</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="141"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="764"/>
        <source>Stopwatch</source>
        <translation>秒表</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="781"/>
        <source>Sessions</source>
        <translation>会话</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="168"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="790"/>
        <source>File Transmission</source>
        <translation>文件传输</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="175"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>AI Assistant</source>
        <translation>AI 助手</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="343"/>
        <source>Show "%1"</source>
        <translation>显示"%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="348"/>
        <source>Show All Hidden Workspaces</source>
        <translation>显示所有隐藏的工作区</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="372"/>
        <source>No Workspaces Available</source>
        <translation>无可用工作区</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="497"/>
        <source>Manage Plugins…</source>
        <translation>管理插件…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="588"/>
        <source>Export</source>
        <translation>导出</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="619"/>
        <source>CSV File</source>
        <translation>CSV 文件</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="625"/>
        <source>MDF4 File</source>
        <translation>MDF4 文件</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="631"/>
        <source>Console Transcript</source>
        <translation>控制台记录</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="640"/>
        <source>Session Database</source>
        <translation>会话数据库</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="654"/>
        <source>No Export Formats Available</source>
        <translation>无可用导出格式</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="119"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="740"/>
        <source>Console</source>
        <translation>控制台</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="125"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="749"/>
        <source>Notifications</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="149"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Preferences</source>
        <translation>偏好设置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="928"/>
        <source>Edit…</source>
        <translation>编辑…</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="874"/>
        <source>Resume</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="875"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="902"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>启动</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>计圈</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>重置</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>总计</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>无计圈记录</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>在秒表运行时按计圈</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="86"/>
        <source>No Data Available</source>
        <translation>无可用数据</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>数据集值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>搜索</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>数据集</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>单位</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(虚拟)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>复制访问代码 %1 到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>数据集访问代码已复制</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>此项目中未定义数据集。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>没有数据集匹配您的搜索。</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>（自定义图标）</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>自动</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="806"/>
        <source>No</source>
        <translation>否</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="806"/>
        <source>Yes</source>
        <translation>是</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>开始菜单</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>菜单</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>搜索…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>设置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>控制台</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>时钟</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>秒表</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>AI 助手</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>恢复</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="949"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT：已连接到 %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="950"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT：未连接</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="974"/>
        <source>MQTT Publisher</source>
        <translation>MQTT 发布者</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="984"/>
        <source>Status:</source>
        <translation>状态:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="992"/>
        <source>Connected</source>
        <translation>已连接</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="993"/>
        <source>Disconnected</source>
        <translation>已断开</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1000"/>
        <source>Broker:</source>
        <translation>代理:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1013"/>
        <source>Mode:</source>
        <translation>模式:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1026"/>
        <source>Messages sent:</source>
        <translation>已发送消息:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1040"/>
        <source>Open MQTT Settings</source>
        <translation>打开 MQTT 设置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>文件传输</translation>
    </message>
    <message>
        <source>Search widgets…</source>
        <translation type="vanished">搜索控件…</translation>
    </message>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">从工作区移除</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>清除</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>发送数据到设备</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>显示时间戳</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>回显</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>模拟 VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>ANSI 颜色</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>显示：%1</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>等待批准</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>完成</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>已拒绝</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>已阻止</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>运行中</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>批准</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>拒绝</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>参数</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>复制</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>全部复制</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>全选</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>结果</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>项目编辑器</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>打开项目编辑器以创建或修改 JSON 布局</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>播放 CSV 文件,如同实时传感器数据</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>偏好设置</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>打开应用程序设置和偏好设置</translation>
    </message>
    <message>
        <source>MQTT</source>
        <translation type="vanished">MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>打开 CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>打开 MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>会话</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>浏览、回放和导出已记录的会话</translation>
    </message>
    <message>
        <source>Shortcuts</source>
        <translation type="vanished">快捷方式</translation>
    </message>
    <message>
        <source>Create an operator shortcut for the current project</source>
        <translation type="vanished">为当前项目创建操作员快捷方式</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>扩展</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>浏览和安装扩展</translation>
    </message>
    <message>
        <source>Configure MQTT connection (publish or subscribe)</source>
        <translation type="vanished">配置 MQTT 连接(发布或订阅)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>部署</translation>
    </message>
    <message>
        <source>Build an operator deployment for the current project</source>
        <translation type="vanished">为当前项目构建操作员部署</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>选择串口 (UART) 通信</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>音频</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>选择音频输入设备 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>选择原始 USB 通信 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>网络</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>选择 TCP/UDP 网络通信</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>选择 MODBUS 通信 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>选择 HID 设备通信 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>选择 Bluetooth Low Energy 通信</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>CAN 总线</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>选择 CAN 总线通信 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>进程</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>选择进程管道通信 (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>示例</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>浏览示例项目</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>帮助中心</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>浏览文档、常见问题和维基</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>在 DeepWiki 上查看详细文档并提问</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>连接或断开已配置的设备</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>关于</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>打开项目</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>打开现有 JSON 项目</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>播放 MDF4 文件，如同实时传感器数据（Pro）</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>助手</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>与 AI 对话以构建和编辑您的项目</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>为当前项目构建操作员应用</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>显示应用程序信息和许可证详情</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI 维基与聊天</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>管理许可证并激活 Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>断开连接</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>连接</translation>
    </message>
    <message>
        <source>Connect or disconnect from device or MQTT broker</source>
        <translation type="vanished">连接或断开设备或 MQTT 代理</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>激活</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>触发设置</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">通过将每次扫描与触发事件对齐来保持波形静止。</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">锁定重复信号,类似示波器的自动按钮。每次扫描从波形的同一点开始,使其保持静止而不是滚动。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>触发</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">模式：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>模式</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>自动</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>正常</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>单次</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>自动模式在无信号越过电平时自由运行。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>正常模式等待越过电平。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>单次模式捕获一次扫描后停止。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="238"/>
        <source>Slope:</source>
        <translation>斜率：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="270"/>
        <source>Trigger on a downward crossing</source>
        <translation>在下降沿越过时触发</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="316"/>
        <source>Timebase:</source>
        <translation>时基：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="381"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>时基留空则使用绘图的时间范围;降低时基可放大快速信号。保持时间在每次触发后忽略新触发一段时间。</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">信号:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>交叉值</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">边沿：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="251"/>
        <source>Rising</source>
        <translation>上升沿</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="255"/>
        <source>Trigger on an upward crossing</source>
        <translation>在上升沿越过时触发</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="266"/>
        <source>Falling</source>
        <translation>下降沿</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">每当信号按所选方向穿过电平时开始新的扫描。自动模式在未找到交叉点时也会自由运行。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="289"/>
        <source>Timing</source>
        <translation>时序</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">时基 (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="329"/>
        <source>Match time range</source>
        <translation>匹配时间范围</translation>
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
        <translation>保持时间：</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="360"/>
        <source>0</source>
        <translation>0</translation>
    </message>
    <message>
        <source>Timebase sets how much time one sweep shows; leave it empty to use the plot's time range. Lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each one.</source>
        <translation type="vanished">时基设置一次扫描显示的时间长度;留空则使用绘图的时间范围。降低时基可放大快速信号。保持时间在每次触发后忽略新触发一段时间。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="396"/>
        <source>Capture Next</source>
        <translation>捕获下一个</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="398"/>
        <source>Arm for one more single-shot capture</source>
        <translation>触发下一次单次捕获</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>电平：</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">触发电平</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">保持时间 (ms)：</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">保持时间</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>源：</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">重新触发</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="411"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>COM 端口</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="81"/>
        <source>Baud Rate</source>
        <translation>波特率</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="174"/>
        <source>Data Bits</source>
        <translation>数据位</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="195"/>
        <source>Parity</source>
        <translation>校验位</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="216"/>
        <source>Stop Bits</source>
        <translation>停止位</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="237"/>
        <source>Flow Control</source>
        <translation>流控制</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="269"/>
        <source>Auto Reconnect</source>
        <translation>自动重连</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="287"/>
        <source>Send DTR Signal</source>
        <translation>发送 DTR 信号</translation>
    </message>
</context>
<context>
    <name>UI::Dashboard</name>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1201"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1944"/>
        <source>Console</source>
        <translation>控制台</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1288"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1956"/>
        <source>Notifications</source>
        <translation>通知</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1374"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1968"/>
        <source>Clock</source>
        <translation>时钟</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1459"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1979"/>
        <source>Stopwatch</source>
        <translation>秒表</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2036"/>
        <location filename="../../src/UI/Dashboard.cpp" line="2052"/>
        <source>%1 (Fallback)</source>
        <translation>%1(备用)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="2078"/>
        <source>LED Panel (%1)</source>
        <translation>LED 面板 (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="148"/>
        <source>Invalid</source>
        <translation>无效</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1099"/>
        <source>Select Background Image</source>
        <translation>选择背景图像</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1101"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>图像 (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>USB 设备</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>传输模式</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>批量流</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>高级(批量 + 控制)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>同步</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>使用批量、控制或同步传输连接到 USB 设备。适用于数据记录器、自定义固件设备和 USB 仪器。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>USB 规范 (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>IN 端点</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>OUT 端点</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>最大数据包大小</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>已启用控制传输</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>发送不正确的控制请求可能会导致连接的硬件崩溃或损坏。请谨慎使用。</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>了解 USB 控制传输</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>数据包大小应与端点报告的最大传输大小匹配。典型值：192 B（FS 音频）、1024 B（HS）。</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>是否立即下载更新？</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>是否立即下载更新？&lt;br /&gt;这是强制更新，现在退出将关闭应用程序。</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;更新日志：&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>%2 的版本 %1 已发布！</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>当前没有可用更新</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>恭喜！您正在运行最新版本的 %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="166"/>
        <source>Add Register</source>
        <translation>添加寄存器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="169"/>
        <source>Add register</source>
        <translation>添加寄存器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="176"/>
        <source>Insert Constant</source>
        <translation>插入常量</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="179"/>
        <source>Insert constant</source>
        <translation>插入常量</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="186"/>
        <source>Import</source>
        <translation>导入</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="189"/>
        <source>Import registers from CSV</source>
        <translation>从 CSV 导入寄存器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="196"/>
        <source>Export</source>
        <translation>导出</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="199"/>
        <source>Export registers to CSV</source>
        <translation>导出寄存器到 CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="211"/>
        <source>Rename</source>
        <translation>重命名</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="214"/>
        <source>Rename table</source>
        <translation>重命名表</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="221"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="224"/>
        <source>Delete table</source>
        <translation>删除表格</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="238"/>
        <source>Help</source>
        <translation>帮助</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="243"/>
        <source>Open help documentation for shared memory</source>
        <translation>打开共享内存帮助文档</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="283"/>
        <source>Permissions</source>
        <translation>权限</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="284"/>
        <source>Register Name</source>
        <translation>寄存器名称</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="285"/>
        <source>Default Value</source>
        <translation>默认值</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read-Only</source>
        <translation>只读</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="324"/>
        <source>Read/Write</source>
        <translation>读/写</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="462"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>复制访问代码 %1 到剪贴板</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="495"/>
        <source>Delete register</source>
        <translation>删除寄存器</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="512"/>
        <source>No registers.</source>
        <translation>无寄存器。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="562"/>
        <source>Register access code copied</source>
        <translation>寄存器访问代码已复制</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="232"/>
        <source>Show Colorbar</source>
        <translation>显示色条</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="245"/>
        <source>Show Axes &amp; Grid</source>
        <translation>显示坐标轴和网格</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="258"/>
        <source>Show Crosshair</source>
        <translation>显示十字准线</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Pause</source>
        <translation>暂停</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="272"/>
        <source>Resume</source>
        <translation>恢复</translation>
    </message>
    <message>
        <source>Clear History</source>
        <translation type="vanished">清除历史记录</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>欢迎使用 %1！</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio 是一款功能强大的实时可视化工具，专为工程师、学生和创客打造。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>您可以开始 14 天全功能试用、使用许可证密钥激活，或自行下载并编译 GPLv3 源代码。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>购买 Pro 版直接支持作者，并帮助资助未来的开发工作。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>自行构建 GPLv3 版本有助于社区发展，并鼓励技术贡献。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>请稍候…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>试用期剩余 %1 天。</translation>
    </message>
    <message>
        <source>You’re currently using the fully-featured trial of %1 Pro. It’s valid for 14 days of personal, non-commercial use.</source>
        <translation type="vanished">您当前正在使用功能完整的 %1 Pro 试用版。试用期为 14 天，仅限个人非商业用途。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>您当前正在使用功能完整的 %1 Pro 试用版。试用期为 14 天，仅限个人非商业用途。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>升级至付费计划以继续使用 Serial Studio Pro。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>或者，编译 GPLv3 源代码以免费使用。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>要查看可用的订阅计划，请点击下方"立即升级"。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>不再提醒试用期。
我了解试用期结束后，需要购买许可证或构建 GPLv3 版本。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>您的 %1 试用期已过期。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>试用期已结束。要继续使用具有所有 Pro 功能的 %1，请升级至付费计划。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>如果您愿意，也可以在 GPLv3 许可证下编译开源版本。</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>感谢试用 %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>立即升级</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>激活</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>以受限模式打开</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>继续</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>开始试用</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <source>Remove from Workspace</source>
        <translation type="vanished">从工作区移除</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="335"/>
        <source>Device Disconnected</source>
        <translation>设备已断开连接</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="332"/>
        <source>Alarm</source>
        <translation>报警</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="333"/>
        <source>critical</source>
        <translation>严重</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="333"/>
        <source>warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="337"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>数值 %1%2 进入了 %3 区间(%4–%5)。</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">值 %1%2 达到高位报警 %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">值 %1%2 达到低位报警 %3%2</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Bar.cpp" line="343"/>
        <source>Alarms</source>
        <translation>报警</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="158"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="181"/>
        <source>N</source>
        <translation>北</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="161"/>
        <source>NE</source>
        <translation>东北</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="164"/>
        <source>E</source>
        <translation>东</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="167"/>
        <source>SE</source>
        <translation>东南</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="170"/>
        <source>S</source>
        <translation>南</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="173"/>
        <source>SW</source>
        <translation>西南</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="176"/>
        <source>W</source>
        <translation>西</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="179"/>
        <source>NW</source>
        <translation>西北</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="128"/>
        <source>Title</source>
        <translation>标题</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="129"/>
        <source>Value</source>
        <translation>值</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">等待数据…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Satellite Imagery</source>
        <translation>卫星影像</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Satellite Imagery with Labels</source>
        <translation>带标注的卫星影像</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="95"/>
        <source>Street Map</source>
        <translation>街道地图</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Topographic Map</source>
        <translation>地形图</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Terrain</source>
        <translation>地形</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="96"/>
        <source>Light Gray Canvas</source>
        <translation>浅灰画布</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="97"/>
        <source>Dark Gray Canvas</source>
        <translation>深灰画布</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="97"/>
        <source>National Geographic</source>
        <translation>国家地理</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="410"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>附加地图图层仅对 Pro 用户可用。</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="411"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>我们无法提供无限制访问,因为 ArcGIS API 密钥会产生实际成本。</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="102"/>
        <source>Time (s)</source>
        <translation>时间 (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="102"/>
        <source>Samples</source>
        <translation>采样</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="175"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>传输脚本在 %1 毫秒后超时</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="194"/>
        <source>Payload exceeds maximum size</source>
        <translation>有效载荷超出最大大小</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="87"/>
        <source>Time (s)</source>
        <translation>时间 (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="115"/>
        <source>Samples</source>
        <translation>采样</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="1046"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>网格间隔:%1 单位</translation>
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
        <translation>灰度</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="428"/>
        <source>Unknown</source>
        <translation>未知</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>编辑工作区</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>新建工作区</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>名称:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>我的工作区</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>选择要包含的控件:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>筛选控件…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="309"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="41"/>
        <source>Workspace</source>
        <translation>工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="129"/>
        <source>Add Widget</source>
        <translation>添加控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="131"/>
        <source>Add widget to workspace</source>
        <translation>添加控件到工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="142"/>
        <source>Rename</source>
        <translation>重命名</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Rename workspace</source>
        <translation>重命名工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="153"/>
        <source>Delete</source>
        <translation>删除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="155"/>
        <source>Delete workspace</source>
        <translation>删除工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="177"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Group</source>
        <translation>组</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="178"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="184"/>
        <source>Widget</source>
        <translation>控件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="179"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="185"/>
        <source>Type</source>
        <translation>类型</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="229"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="267"/>
        <source>(unknown)</source>
        <translation>(未知)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="247"/>
        <source>(group widget)</source>
        <translation>(组控件)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="297"/>
        <source>Remove widget from workspace</source>
        <translation>从工作区移除组件</translation>
    </message>
    <message>
        <source>Remove from workspace</source>
        <translation type="vanished">从工作区移除</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="317"/>
        <source>No widgets in this workspace.</source>
        <translation>此工作区中无组件。</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="127"/>
        <source>Customize</source>
        <translation>自定义</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="129"/>
        <source>Edit workspaces manually</source>
        <translation>手动编辑工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="145"/>
        <source>Move Up</source>
        <translation>上移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="147"/>
        <source>Move the selected workspace up</source>
        <translation>将选定的工作区上移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="158"/>
        <source>Move Down</source>
        <translation>下移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="160"/>
        <source>Move the selected workspace down</source>
        <translation>将选定的工作区下移</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="171"/>
        <source>Add Workspace</source>
        <translation>添加工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>Add workspace</source>
        <translation>添加工作区</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="182"/>
        <source>Cleanup</source>
        <translation>清理</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="185"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>移除 %1 个目标组或数据集已不存在的控件引用</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>No stale widget references in any workspace</source>
        <translation>所有工作区中无过期的控件引用</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="203"/>
        <source>Title</source>
        <translation>标题</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="204"/>
        <source>Widgets</source>
        <translation>组件</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="276"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>无工作区。使用上方工具栏添加一个,或重置为自动布局。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="278"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>项目没有符合条件的组 —— 添加一个带有小部件的组以填充工作区。</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Reset to Auto Layout</source>
        <translation>重置为自动布局</translation>
    </message>
    <message>
        <source>No workspaces.</source>
        <translation type="vanished">无工作区。</translation>
    </message>
</context>
</TS>