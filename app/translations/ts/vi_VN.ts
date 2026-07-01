<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="vi_VN" sourcelanguage="en_US">
<context>
    <name>AI::AnthropicReply</name>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="164"/>
        <source>Anthropic error</source>
        <translation>Lỗi Anthropic</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="310"/>
        <source>Stream parse error: %1</source>
        <translation>Lỗi phân tích luồng: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="359"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="362"/>
        <source>Invalid API key (%1)</source>
        <translation>Khóa API không hợp lệ (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicReply.cpp" line="364"/>
        <source>Rate limited: %1</source>
        <translation>Bị giới hạn tốc độ: %1</translation>
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
        <translation>Cho Phép AI Điều Khiển Thiết Bị?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="219"/>
        <source>This lets the AI assistant configure devices, open and close connections, and send data to your hardware.

Every device action still requires your explicit per-call approval in the chat, even when auto-approve is enabled. Only enable this if you trust the configured AI provider with hardware access.</source>
        <translation>Điều này cho phép trợ lý AI cấu hình thiết bị, mở và đóng kết nối, và gửi dữ liệu đến phần cứng của bạn.

Mỗi hành động thiết bị vẫn yêu cầu sự chấp thuận rõ ràng của bạn cho từng lần gọi trong cuộc trò chuyện, ngay cả khi chế độ tự động chấp thuận được bật. Chỉ bật tính năng này nếu bạn tin tưởng nhà cung cấp AI đã cấu hình với quyền truy cập phần cứng.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="417"/>
        <source>Switch AI provider?</source>
        <translation>Chuyển nhà cung cấp AI?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="418"/>
        <source>Switching to a different provider clears the current conversation. Do you want to continue?</source>
        <translation>Chuyển sang nhà cung cấp khác sẽ xóa cuộc hội thoại hiện tại. Bạn có muốn tiếp tục?</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="421"/>
        <source>Assistant</source>
        <translation>Trợ Lý</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="458"/>
        <source>AI Assistant is not available in this build</source>
        <translation>Trợ Lý AI không khả dụng trong bản dựng này</translation>
    </message>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">Trợ Lý AI yêu cầu giấy phép Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Assistant.cpp" line="463"/>
        <source>Set an API key first</source>
        <translation>Đặt khóa API trước</translation>
    </message>
</context>
<context>
    <name>AI::Conversation</name>
    <message>
        <source>AI Assistant requires a Pro license</source>
        <translation type="vanished">Trợ lý AI yêu cầu giấy phép Pro</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="169"/>
        <source>AI Assistant is not available in this build</source>
        <translation>Trợ Lý AI không khả dụng trong bản dựng này</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="175"/>
        <source>AI subsystem not initialized</source>
        <translation>Hệ thống con AI chưa được khởi tạo</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="181"/>
        <source>Already busy with a previous request</source>
        <translation>Đang xử lý yêu cầu trước đó</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="495"/>
        <source>Tool-call budget reached for this turn; no further tools will run.</source>
        <translation>Đã đạt giới hạn gọi công cụ cho lượt này; không có công cụ nào khác sẽ chạy.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1093"/>
        <source>Waiting for %1 to respond. Loading the model and processing the prompt can take a while on local hardware...</source>
        <translation>Đang chờ %1 phản hồi. Việc tải mô hình và xử lý lời nhắc có thể mất một lúc trên phần cứng cục bộ...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1952"/>
        <source>You have reached the tool-call budget for this turn. Do not request more tools. Summarize what you found so far, and if the task is incomplete, say which steps remain so the user can tell you to continue.</source>
        <translation>Bạn đã đạt giới hạn gọi công cụ cho lượt này. Không yêu cầu thêm công cụ. Tóm tắt những gì bạn đã tìm thấy cho đến nay và nếu tác vụ chưa hoàn thành, hãy nói các bước còn lại để người dùng có thể yêu cầu bạn tiếp tục.</translation>
    </message>
    <message>
        <source>Tool-call budget exceeded</source>
        <translation type="vanished">Vượt quá giới hạn gọi công cụ</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="940"/>
        <source>(The model returned an empty response. Try rephrasing, switching to a different model, or checking that the request is allowed by the provider's safety filters.)</source>
        <translation>(Mô hình trả về phản hồi trống. Hãy thử diễn đạt lại, chuyển sang mô hình khác hoặc kiểm tra xem yêu cầu có được cho phép bởi bộ lọc an toàn của nhà cung cấp không.)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1097"/>
        <source>Sending request to %1...</source>
        <translation>Đang gửi yêu cầu tới %1...</translation>
    </message>
    <message>
        <location filename="../../src/AI/Conversation.cpp" line="1110"/>
        <source>Provider returned no reply</source>
        <translation>Nhà cung cấp không trả về phản hồi</translation>
    </message>
</context>
<context>
    <name>AI::GeminiReply</name>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="146"/>
        <source>Prompt blocked by Gemini safety filter: %1</source>
        <translation>Lời nhắc bị chặn bởi bộ lọc an toàn Gemini: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="200"/>
        <source>Gemini stopped without producing a response: %1</source>
        <translation>Gemini dừng mà không tạo ra phản hồi: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="262"/>
        <source>HTTP %1</source>
        <translation>HTTP %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="265"/>
        <source>Invalid API key (%1)</source>
        <translation>Khóa API không hợp lệ (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="267"/>
        <source>Rate limited: %1</source>
        <translation>Bị giới hạn tốc độ: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiReply.cpp" line="269"/>
        <source>Invalid API key</source>
        <translation>Khóa API không hợp lệ</translation>
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
        <translation>Khóa API không hợp lệ (%1)</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="431"/>
        <source>Rate limited: %1</source>
        <translation>Bị giới hạn tốc độ: %1</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIReply.cpp" line="433"/>
        <source>%1 %2: %3</source>
        <translation>%1 %2: %3</translation>
    </message>
</context>
<context>
    <name>API::GRPC::GRPCServer</name>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="421"/>
        <source>Export Protobuf File</source>
        <translation>Xuất Tệp Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="423"/>
        <source>Protocol Buffers (*.proto)</source>
        <translation>Protocol Buffers (*.proto)</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="469"/>
        <source>Unable to start gRPC server</source>
        <translation>Không thể khởi động máy chủ GRPC</translation>
    </message>
    <message>
        <location filename="../../src/API/GRPC/GRPCServer.cpp" line="470"/>
        <source>Failed to bind to %1</source>
        <translation>Không thể liên kết với %1</translation>
    </message>
</context>
<context>
    <name>API::ProcessLauncher</name>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="82"/>
        <source>No program specified</source>
        <translation>Không chỉ định chương trình</translation>
    </message>
    <message>
        <location filename="../../src/API/ProcessLauncher.cpp" line="88"/>
        <source>Program "%1" not found in PATH</source>
        <translation>Không tìm thấy chương trình "%1" trong PATH</translation>
    </message>
</context>
<context>
    <name>API::Server</name>
    <message>
        <location filename="../../src/API/Server.cpp" line="434"/>
        <source>Unable to start API TCP server</source>
        <translation>Không thể khởi động máy chủ TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="478"/>
        <source>Allow External API Connections?</source>
        <translation>Cho Phép Kết Nối API Từ Bên Ngoài?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="479"/>
        <source>Exposing the API server to external hosts allows other devices on your network to connect to Serial Studio on port 7777.

Only enable this on trusted networks. Untrusted clients may read live data or send commands to your device.</source>
        <translation>Mở máy chủ API cho các máy chủ bên ngoài cho phép các thiết bị khác trên mạng của bạn kết nối với Serial Studio qua cổng 7777.

Chỉ bật tính năng này trên các mạng đáng tin cậy. Các máy khách không đáng tin cậy có thể đọc dữ liệu trực tiếp hoặc gửi lệnh đến thiết bị của bạn.</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="513"/>
        <source>Unable to restart API TCP server</source>
        <translation>Không thể khởi động lại máy chủ TCP API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="600"/>
        <source>Allow API device control?</source>
        <translation>Cho phép API điều khiển thiết bị?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="601"/>
        <source>A program using Serial Studio's local API is requesting to send data to the connected device. Allow API clients to write to the device?</source>
        <translation>Một chương trình sử dụng API cục bộ của Serial Studio đang yêu cầu gửi dữ liệu đến thiết bị đã kết nối. Cho phép các API client ghi vào thiết bị?</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="604"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>API server</source>
        <translation>Máy chủ API</translation>
    </message>
    <message>
        <location filename="../../src/API/Server.cpp" line="1251"/>
        <source>Invalid pending connection</source>
        <translation>Kết nối đang chờ không hợp lệ</translation>
    </message>
</context>
<context>
    <name>About</name>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="39"/>
        <source>About</source>
        <translation>Giới Thiệu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="96"/>
        <source>Version %1</source>
        <translation>Phiên Bản %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="106"/>
        <source>Copyright © %1 %2</source>
        <translation>Bản Quyền © %1 %2</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="112"/>
        <source>All Rights Reserved</source>
        <translation>Bảo Lưu Mọi Quyền</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="127"/>
        <source>%1 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

%1 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.</source>
        <translation>%1 là phần mềm tự do: bạn có thể phân phối lại và/hoặc sửa đổi theo các điều khoản của Giấy Phép Công Cộng GNU do Tổ Chức Phần Mềm Tự Do công bố; phiên bản 3 của Giấy Phép, hoặc (tùy chọn của bạn) bất kỳ phiên bản nào sau đó.

%1 được phân phối với hy vọng sẽ hữu ích, nhưng KHÔNG CÓ BẢO HÀNH NÀO; kể cả bảo hành ngụ ý về KHẢ NĂNG THƯƠNG MẠI hoặc PHÙ HỢP CHO MỤC ĐÍCH CỤ THỂ. Xem Giấy Phép Công Cộng GNU để biết thêm chi tiết.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="146"/>
        <source>This configuration is licensed for commercial and proprietary use. It may be used in closed-source and commercial applications, subject to the terms of the commercial license.</source>
        <translation>Cấu hình này được cấp phép cho mục đích thương mại và sở hữu độc quyền. Có thể sử dụng trong các ứng dụng mã nguồn đóng và thương mại, tuân theo các điều khoản của giấy phép thương mại.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="160"/>
        <source>This configuration is for personal and evaluation purposes only. Commercial use is prohibited unless a valid commercial license is activated.</source>
        <translation>Cấu hình này chỉ dành cho mục đích cá nhân và đánh giá. Nghiêm cấm sử dụng thương mại trừ khi đã kích hoạt giấy phép thương mại hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="174"/>
        <source>This software is provided 'as is' without warranty of any kind, express or implied, including but not limited to the warranties of merchantability or fitness for a particular purpose. In no event shall the author be liable for any damages arising from the use of this software.</source>
        <translation>Phần mềm này được cung cấp 'nguyên trạng' không có bảo hành dưới bất kỳ hình thức nào, rõ ràng hay ngụ ý, bao gồm nhưng không giới hạn ở các bảo hành về khả năng thương mại hoặc sự phù hợp cho mục đích cụ thể. Trong mọi trường hợp, tác giả sẽ không chịu trách nhiệm về bất kỳ thiệt hại nào phát sinh từ việc sử dụng phần mềm này.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="195"/>
        <source>Manage License</source>
        <translation>Quản Lý Giấy Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="203"/>
        <source>Donate</source>
        <translation>Quyên Góp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="214"/>
        <source>Check for Updates</source>
        <translation>Kiểm Tra Cập Nhật</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="223"/>
        <source>What's New</source>
        <translation>Có Gì Mới</translation>
    </message>
    <message>
        <source>Tips &amp;&amp; Tricks</source>
        <translation type="vanished">Mẹo &amp; Thủ Thuật</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="232"/>
        <source>License Agreement</source>
        <translation>Thỏa Thuận Giấy Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="241"/>
        <source>Report Bug</source>
        <translation>Báo Cáo Lỗi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="250"/>
        <source>Acknowledgements</source>
        <translation>Lời Cảm Ơn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="259"/>
        <source>Benchmark</source>
        <translation>Đánh Giá Hiệu Năng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="267"/>
        <source>Website</source>
        <translation>Trang Web</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/About.qml" line="283"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>Accelerometer</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="186"/>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="187"/>
        <source>Settings</source>
        <translation>Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Accelerometer.qml" line="245"/>
        <source>G-FORCE</source>
        <translation>LỰC G</translation>
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
        <translation>Cấu Hình Gia Tốc Kế</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="95"/>
        <source>Configure the accelerometer display range and input units.</source>
        <translation>Cấu hình phạm vi hiển thị và đơn vị đầu vào của gia tốc kế.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="109"/>
        <source>Display Range</source>
        <translation>Phạm Vi Hiển Thị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="130"/>
        <source>Max G:</source>
        <translation>G Tối Đa:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="144"/>
        <source>Enter max G value</source>
        <translation>Nhập giá trị G tối đa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="164"/>
        <source>Input Configuration</source>
        <translation>Cấu Hình Đầu Vào</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="184"/>
        <source>Input already in G</source>
        <translation>Đầu vào đã ở dạng G</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AccelerometerConfigDialog.qml" line="218"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>Acknowledgements</name>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="34"/>
        <source>Acknowledgements</source>
        <translation>Ghi Nhận</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="75"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Acknowledgements.qml" line="86"/>
        <source>About Qt…</source>
        <translation>Giới Thiệu QT…</translation>
    </message>
</context>
<context>
    <name>ActionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="136"/>
        <source>Change Icon</source>
        <translation>Đổi Biểu Tượng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="138"/>
        <source>Change the icon used for this action</source>
        <translation>Đổi biểu tượng được sử dụng cho hành động này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="156"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="160"/>
        <source>Duplicate this action with all its settings</source>
        <translation>Nhân bản hành động này với tất cả cài đặt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="169"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ActionView.qml" line="171"/>
        <source>Delete this action from the project</source>
        <translation>Xóa hành động này khỏi dự án</translation>
    </message>
</context>
<context>
    <name>AddWidgetDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="44"/>
        <source>Add Widget</source>
        <translation>Thêm Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="211"/>
        <source>Available Widgets</source>
        <translation>Widget Khả Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="220"/>
        <source>Click a row to add it to the workspace.</source>
        <translation>Nhấp vào một hàng để thêm vào không gian làm việc.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="228"/>
        <source>Search</source>
        <translation>Tìm Kiếm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="247"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="248"/>
        <source>Group</source>
        <translation>Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="249"/>
        <source>Name</source>
        <translation>Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="316"/>
        <source>(entire group)</source>
        <translation>(toàn bộ nhóm)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="351"/>
        <source>Already in workspace</source>
        <translation>Đã có trong không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="352"/>
        <source>Add to workspace</source>
        <translation>Thêm vào không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="381"/>
        <source>No widgets available.</source>
        <translation>Không có widget khả dụng.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="382"/>
        <source>No widgets match.</source>
        <translation>Không có widget phù hợp.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="399"/>
        <source>%1 widgets</source>
        <translation>%1 widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="400"/>
        <source>%1 of %2 widgets</source>
        <translation>%1 trong số %2 widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/AddWidgetDialog.qml" line="404"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>AlarmBandsEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="35"/>
        <source>Alarm Bands</source>
        <translation>Dải Cảnh Báo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="71"/>
        <source>Info</source>
        <translation>Thông Tin</translation>
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
        <translation>Cảnh Báo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="74"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="154"/>
        <source>Critical</source>
        <translation>Nghiêm Trọng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="83"/>
        <source>Tachometer</source>
        <translation>Máy Đo Tốc Độ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="85"/>
        <source>Idle</source>
        <translation>Không Hoạt Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="86"/>
        <source>Operating</source>
        <translation>Hoạt Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="87"/>
        <source>Caution</source>
        <translation>Thận Trọng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="88"/>
        <source>Redline</source>
        <translation>Vùng Đỏ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="92"/>
        <source>Speedometer</source>
        <translation>Đồng Hồ Tốc Độ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="94"/>
        <source>Cruise</source>
        <translation>Hành Trình</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="95"/>
        <source>Fast</source>
        <translation>Nhanh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="96"/>
        <source>Top Speed</source>
        <translation>Tốc Độ Tối Đa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="100"/>
        <source>Engine Temperature</source>
        <translation>Nhiệt Độ Engine</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="102"/>
        <source>Cold</source>
        <translation>Lạnh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="103"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="112"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="144"/>
        <source>Normal</source>
        <translation>Bình Thường</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="104"/>
        <source>Warm</source>
        <translation>Ấm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="105"/>
        <source>Overheat</source>
        <translation>Quá Nhiệt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="109"/>
        <source>Pressure</source>
        <translation>Áp Suất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="111"/>
        <source>Vacuum</source>
        <translation>Chân Không</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="113"/>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="122"/>
        <source>High</source>
        <translation>Cao</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="114"/>
        <source>Burst</source>
        <translation>Vượt Ngưỡng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="118"/>
        <source>Battery Voltage</source>
        <translation>Điện Áp Pin</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="120"/>
        <source>Low</source>
        <translation>Thấp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="121"/>
        <source>Nominal</source>
        <translation>Danh Định</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="126"/>
        <source>Fuel Level</source>
        <translation>Mức Nhiên Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="128"/>
        <source>Empty</source>
        <translation>Trống</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="129"/>
        <source>Reserve</source>
        <translation>Dự Trữ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="134"/>
        <source>Signal Strength</source>
        <translation>Cường Độ Tín Hiệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="136"/>
        <source>No Signal</source>
        <translation>Không Có Tín Hiệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="137"/>
        <source>Weak</source>
        <translation>Yếu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="138"/>
        <source>Good</source>
        <translation>Tốt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="142"/>
        <source>CPU / System Load</source>
        <translation>Tải CPU / Hệ Thống</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="145"/>
        <source>Busy</source>
        <translation>Bận</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="146"/>
        <source>Overload</source>
        <translation>Quá Tải</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="150"/>
        <source>OK / Warning / Critical</source>
        <translation>OK / Cảnh Báo / Nghiêm Trọng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="158"/>
        <source>Indicator (On / Off)</source>
        <translation>Chỉ Báo (Bật / Tắt)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="160"/>
        <source>On</source>
        <translation>Bật</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="164"/>
        <source>Fault Indicator</source>
        <translation>Chỉ Báo Lỗi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="166"/>
        <source>Fault</source>
        <translation>Lỗi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="287"/>
        <source>Choose Band Color</source>
        <translation>Chọn Màu Dải</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="314"/>
        <source>Presets</source>
        <translation>Cài Đặt Sẵn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="337"/>
        <source>Preset</source>
        <translation>Cài Đặt Sẵn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="352"/>
        <source>Choose preset…</source>
        <translation>Chọn cài đặt sẵn…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="462"/>
        <source>Blink</source>
        <translation>Nhấp Nháy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="582"/>
        <source>Reset to severity default</source>
        <translation>Đặt lại về mặc định mức độ nghiêm trọng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="596"/>
        <source>Click to choose a color. Right-click to reset to severity default.</source>
        <translation>Nhấp để chọn màu. Nhấp chuột phải để đặt lại về mặc định mức độ nghiêm trọng.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="597"/>
        <source>Click to choose a custom color.</source>
        <translation>Nhấp để chọn màu tùy chỉnh.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="628"/>
        <source>Flash the LED while the value sits in this band.</source>
        <translation>Nhấp nháy LED khi giá trị nằm trong dải này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="702"/>
        <source>No bands defined. Pick a preset above or add a band to get started.</source>
        <translation>Chưa định nghĩa dải nào. Chọn cài đặt sẵn ở trên hoặc thêm dải để bắt đầu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="826"/>
        <source>Apply</source>
        <translation>Áp Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="829"/>
        <source>Apply changes to the dataset.</source>
        <translation>Áp dụng các thay đổi vào tập dữ liệu.</translation>
    </message>
    <message>
        <source>Apply Preset</source>
        <translation type="vanished">Áp Dụng Cài Đặt Sẵn</translation>
    </message>
    <message>
        <source>Replace the current bands with the selected preset, scaled to this dataset's range.</source>
        <translation type="vanished">Thay thế các dải hiện tại bằng cài đặt sẵn đã chọn, được điều chỉnh theo phạm vi của tập dữ liệu này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="363"/>
        <source>Range</source>
        <translation>Phạm Vi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="391"/>
        <source>Bands</source>
        <translation>Các Dải</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="402"/>
        <source>Add Band</source>
        <translation>Thêm Dải</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="406"/>
        <source>Add a new band continuing from the last one.</source>
        <translation>Thêm một dải mới tiếp nối từ dải cuối cùng.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="437"/>
        <source>Min</source>
        <translation>Tối Thiểu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="443"/>
        <source>Max</source>
        <translation>Tối Đa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="449"/>
        <source>Severity</source>
        <translation>Mức Độ Nghiêm Trọng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="455"/>
        <source>Color</source>
        <translation>Màu Sắc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="469"/>
        <source>Label</source>
        <translation>Nhãn</translation>
    </message>
    <message>
        <source>Choose a custom color.</source>
        <translation type="vanished">Chọn màu tùy chỉnh.</translation>
    </message>
    <message>
        <source>auto</source>
        <translation type="vanished">tự động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="639"/>
        <source>(optional)</source>
        <translation>(tùy chọn)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="656"/>
        <source>Move up.</source>
        <translation>Di chuyển lên.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="675"/>
        <source>Move down.</source>
        <translation>Di chuyển xuống.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="688"/>
        <source>Remove this band.</source>
        <translation>Xóa dải này.</translation>
    </message>
    <message>
        <source>No bands defined. Apply a preset or add a band to get started.</source>
        <translation type="vanished">Chưa định nghĩa dải nào. Áp dụng cài đặt sẵn hoặc thêm dải để bắt đầu.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="719"/>
        <source>Preview</source>
        <translation>Xem Trước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="815"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml" line="817"/>
        <source>Discard changes.</source>
        <translation>Hủy các thay đổi.</translation>
    </message>
    <message>
        <source>Save</source>
        <translation type="vanished">Lưu</translation>
    </message>
    <message>
        <source>Save changes to the dataset.</source>
        <translation type="vanished">Lưu các thay đổi vào tập dữ liệu.</translation>
    </message>
</context>
<context>
    <name>AssistantPanel</name>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="31"/>
        <source>Assistant</source>
        <translation>Trợ Lý</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="79"/>
        <source>New chat</source>
        <translation>Trò Chuyện Mới</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="125"/>
        <source>Toggle chat list</source>
        <translation>Bật/Tắt Danh Sách Trò Chuyện</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="238"/>
        <source>Settings</source>
        <translation>Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="274"/>
        <source>Manage API keys…</source>
        <translation>Quản Lý Khóa API…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="334"/>
        <source>Help me discover Serial Studio's features</source>
        <translation>Giúp tôi khám phá các tính năng của Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="335"/>
        <source>What can this app do for my telemetry?</source>
        <translation>Ứng dụng này có thể làm gì cho dữ liệu đo từ xa của tôi?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="336"/>
        <source>Walk me through what this project already contains</source>
        <translation>Hướng dẫn tôi qua những gì dự án này đã chứa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="337"/>
        <source>List the sources in this project</source>
        <translation>Liệt kê các nguồn trong dự án này</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="340"/>
        <source>What is a session database, and why would I use one?</source>
        <translation>Cơ sở dữ liệu phiên là gì và tại sao tôi nên sử dụng nó?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="341"/>
        <source>CSV vs MDF4 export - what is the difference?</source>
        <translation>Xuất CSV so với MDF4 - sự khác biệt là gì?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="342"/>
        <source>What is a frame parser, and when do I need one?</source>
        <translation>Bộ phân tích khung là gì và khi nào tôi cần nó?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="343"/>
        <source>When should I use Lua vs JavaScript for the parser?</source>
        <translation>Khi nào nên dùng Lua so với JavaScript cho bộ phân tích?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="344"/>
        <source>Plot, Bar, and Gauge - when to use each?</source>
        <translation>Plot, Bar và Gauge - khi nào dùng từng loại?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="345"/>
        <source>What is the difference between a transform and a frame parser?</source>
        <translation>Sự khác biệt giữa phép biến đổi và bộ phân tích khung là gì?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="348"/>
        <source>Add a UART source for an Arduino</source>
        <translation>Thêm nguồn UART cho Arduino</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="349"/>
        <source>Set up an IMU project from scratch</source>
        <translation>Thiết lập dự án IMU từ đầu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="350"/>
        <source>Configure an MQTT subscriber</source>
        <translation>Cấu hình subscriber MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="351"/>
        <source>Add a CAN bus source</source>
        <translation>Thêm nguồn CAN bus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="352"/>
        <source>Set up a Modbus poller</source>
        <translation>Thiết lập bộ thăm dò Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="353"/>
        <source>Add a network (TCP/UDP) source</source>
        <translation>Thêm nguồn mạng (TCP/UDP)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="354"/>
        <source>Write a CSV frame parser for me</source>
        <translation>Viết bộ phân tích frame CSV cho tôi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="355"/>
        <source>Help me parse a JSON frame</source>
        <translation>Giúp tôi phân tích frame JSON</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="356"/>
        <source>Add an EMA smoothing transform to a dataset</source>
        <translation>Thêm phép biến đổi làm mượt EMA vào dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="357"/>
        <source>Decode hexadecimal frames</source>
        <translation>Giải mã frame thập lục phân</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="358"/>
        <source>Calibrate a sensor with a linear transform</source>
        <translation>Hiệu chỉnh cảm biến bằng phép biến đổi tuyến tính</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="361"/>
        <source>Suggest dashboard widgets for my data</source>
        <translation>Đề xuất widget bảng điều khiển cho dữ liệu của tôi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="362"/>
        <source>Build an executive overview workspace</source>
        <translation>Xây dựng không gian làm việc tổng quan điều hành</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="363"/>
        <source>Add a painter widget for a custom visualization</source>
        <translation>Thêm widget painter cho trực quan hóa tùy chỉnh</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="364"/>
        <source>Show Plot, FFT, and Waterfall for one dataset</source>
        <translation>Hiển thị Plot, FFT và Waterfall cho một dataset</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="365"/>
        <source>Group my datasets into useful workspaces</source>
        <translation>Nhóm các dataset của tôi thành các không gian làm việc hữu ích</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="416"/>
        <source>How can I help with your project?</source>
        <translation>Tôi có thể hỗ trợ gì cho dự án của bạn?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="417"/>
        <source>Set up your API key to get started</source>
        <translation>Thiết lập khóa API để bắt đầu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="429"/>
        <source>Describe what you would like to build, and I will configure the sources, groups, datasets, frame parsers, and transforms for you.</source>
        <translation>Mô tả những gì bạn muốn xây dựng, và tôi sẽ cấu hình các nguồn, nhóm, dataset, bộ phân tích frame và phép biến đổi cho bạn.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="432"/>
        <source>To start chatting, paste an API key for the selected provider. Keys are encrypted on this machine and never leave your computer except to talk to the provider you choose.</source>
        <translation>Để bắt đầu trò chuyện, dán khóa API cho nhà cung cấp đã chọn. Các khóa được mã hóa trên máy này và không bao giờ rời khỏi máy tính của bạn ngoại trừ để giao tiếp với nhà cung cấp bạn chọn.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="453"/>
        <source>Open API Key Setup</source>
        <translation>Mở Cài Đặt Khóa API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="463"/>
        <source>Get a key from %1</source>
        <translation>Lấy khóa từ %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="644"/>
        <source>Drop files or folders to let the assistant read them</source>
        <translation>Thả tệp hoặc thư mục để trợ lý đọc chúng</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="689"/>
        <source>Added folder "%1" - readable this session</source>
        <translation>Đã thêm thư mục "%1" - có thể đọc trong phiên này</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="690"/>
        <source>Added "%1" - readable this session</source>
        <translation>Đã thêm "%1" - có thể đọc trong phiên này</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="774"/>
        <source>Ask Serial Studio anything…</source>
        <translation>Hỏi Serial Studio bất cứ điều gì…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="794"/>
        <source>Clear conversation</source>
        <translation>Xóa cuộc hội thoại</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="838"/>
        <source>Stop generating</source>
        <translation>Dừng tạo</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="839"/>
        <source>Send message (Enter)</source>
        <translation>Gửi tin nhắn (Enter)</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="159"/>
        <source>Provider</source>
        <translation>Nhà Cung Cấp</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="192"/>
        <source>Model selection</source>
        <translation>Lựa chọn mô hình</translation>
    </message>
    <message>
        <source>Run editing actions without asking each time. Blocked actions stay blocked.</source>
        <translation type="vanished">Thực hiện các thao tác chỉnh sửa mà không hỏi mỗi lần. Các thao tác bị chặn vẫn bị chặn.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="250"/>
        <source>Auto-approve edits</source>
        <translation>Tự động phê duyệt chỉnh sửa</translation>
    </message>
    <message>
        <source>Let the AI configure devices, connect/disconnect and send data. Each action still asks for your approval.</source>
        <translation type="vanished">Cho phép AI cấu hình thiết bị, kết nối/ngắt kết nối và gửi dữ liệu. Mỗi hành động vẫn yêu cầu sự chấp thuận của bạn.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/AssistantPanel.qml" line="259"/>
        <source>Allow device control</source>
        <translation>Cho phép điều khiển thiết bị</translation>
    </message>
    <message>
        <source>Manage API keys</source>
        <translation type="vanished">Quản Lý Khóa API</translation>
    </message>
    <message>
        <source>Working</source>
        <translation type="vanished">Đang Xử Lý</translation>
    </message>
    <message>
        <source>Ready</source>
        <translation type="vanished">Sẵn Sàng</translation>
    </message>
    <message>
        <source>  •  cache %1k tok</source>
        <translation type="vanished">•  bộ nhớ đệm %1k tok</translation>
    </message>
    <message>
        <source>  •  cache write %1k tok</source>
        <translation type="vanished">•  ghi bộ nhớ đệm %1k tok</translation>
    </message>
</context>
<context>
    <name>Audio</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="89"/>
        <source>No Microphone Detected</source>
        <translation>Không Phát Hiện Micro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="98"/>
        <source>Connect a mic or check your settings</source>
        <translation>Kết nối micro hoặc kiểm tra cài đặt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="123"/>
        <source>Input Device</source>
        <translation>Thiết Bị Đầu Vào</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="142"/>
        <source>Sample Rate</source>
        <translation>Tốc Độ Lấy Mẫu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="161"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="230"/>
        <source>Sample Format</source>
        <translation>Định Dạng Mẫu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="180"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="249"/>
        <source>Channels</source>
        <translation>Kênh</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Audio.qml" line="211"/>
        <source>Output Device</source>
        <translation>Thiết Bị Đầu Ra</translation>
    </message>
</context>
<context>
    <name>AuthenticateDialog</name>
    <message>
        <source>Dialog</source>
        <translation type="vanished">Hộp Thoại</translation>
    </message>
    <message>
        <source>Please provide the user name and password for the download location.</source>
        <translation type="vanished">Vui lòng cung cấp tên người dùng và mật khẩu cho vị trí tải xuống.</translation>
    </message>
    <message>
        <source>&amp;User name:</source>
        <translation type="vanished">Tên người &amp;dùng:</translation>
    </message>
    <message>
        <source>&amp;Password:</source>
        <translation type="vanished">&amp;Mật Khẩu:</translation>
    </message>
</context>
<context>
    <name>AxisRangeDialog</name>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="65"/>
        <source>Axis Range Configuration</source>
        <translation>Cấu Hình Phạm Vi Trục</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="179"/>
        <source>Configure the visible range for the plot axes. Values update in real-time as you type.</source>
        <translation>Cấu hình phạm vi hiển thị cho các trục biểu đồ. Giá trị cập nhật theo thời gian thực khi nhập.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="187"/>
        <source>X Axis</source>
        <translation>Trục X</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="212"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="283"/>
        <source>Minimum:</source>
        <translation>Tối Thiểu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="224"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="295"/>
        <source>Enter min value</source>
        <translation>Nhập giá trị tối thiểu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="233"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="304"/>
        <source>Maximum:</source>
        <translation>Tối Đa:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="245"/>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="316"/>
        <source>Enter max value</source>
        <translation>Nhập giá trị tối đa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="260"/>
        <source>Y Axis</source>
        <translation>Trục Y</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="335"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/AxisRangeDialog.qml" line="345"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>BackupRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="31"/>
        <source>Recover Backup</source>
        <translation>Khôi Phục Bản Sao Lưu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="94"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="165"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="166"/>
        <source>Untitled</source>
        <translation>Chưa Đặt Tên</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="97"/>
        <source>Project Loaded</source>
        <translation>Dự Án Đã Tải</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="98"/>
        <source>Auto-save</source>
        <translation>Tự Động Lưu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="99"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="119"/>
        <source>Before Restore</source>
        <translation>Trước Khi Khôi Phục</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="100"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="106"/>
        <source>Before Delete Dataset</source>
        <translation>Trước Khi Xóa Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="101"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="107"/>
        <source>Before Delete Group</source>
        <translation>Trước Khi Xóa Group</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="102"/>
        <source>Before New Project</source>
        <translation>Trước Khi Tạo Dự Án Mới</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="103"/>
        <source>Before Open Project</source>
        <translation>Trước Khi Mở Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="104"/>
        <source>Before Load JSON</source>
        <translation>Trước Khi Tải JSON</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="105"/>
        <source>Before Apply Template</source>
        <translation>Trước Khi Áp Dụng Template</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="108"/>
        <source>Before Delete Action</source>
        <translation>Trước Khi Xóa Action</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="109"/>
        <source>Before Delete Output Widget</source>
        <translation>Trước Khi Xóa Widget Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="110"/>
        <source>Before Move Dataset</source>
        <translation>Trước Khi Di Chuyển Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="111"/>
        <source>Before Move Group</source>
        <translation>Trước Khi Di Chuyển Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="112"/>
        <source>Before Delete Workspace</source>
        <translation>Trước Khi Xóa Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="113"/>
        <source>Before Clear All Workspaces</source>
        <translation>Trước Khi Xóa Tất Cả Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="114"/>
        <source>Before Remove Widget</source>
        <translation>Trước Khi Xóa Widget</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="115"/>
        <source>Before Reorder Workspaces</source>
        <translation>Trước Khi Sắp Xếp Lại Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="116"/>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="117"/>
        <source>Before Batch Operation</source>
        <translation>Trước Khi Thực Hiện Hàng Loạt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="118"/>
        <source>Before Add Tile</source>
        <translation>Trước Khi Thêm Ô</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="142"/>
        <source>%1 (and %2 more)</source>
        <translation>%1 (và %2 nữa)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="159"/>
        <source> The frame parser code also differs and will be replaced.</source>
        <translation>Mã bộ phân tích frame cũng khác và sẽ được thay thế.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="164"/>
        <source>Title changes from “%1” to “%2”. Group structure unchanged.</source>
        <translation>Tiêu đề thay đổi từ "%1" thành "%2". Cấu trúc nhóm không thay đổi.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="169"/>
        <source>Same groups and datasets, but the frame parser code differs — restoring will replace it.</source>
        <translation>Cùng nhóm và dataset, nhưng mã bộ phân tích frame khác — khôi phục sẽ thay thế nó.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="171"/>
        <source>Same groups and datasets as your current project. Restoring may still revert field-level edits.</source>
        <translation>Cùng nhóm và bộ dữ liệu với dự án hiện tại. Khôi phục vẫn có thể hoàn tác các chỉnh sửa cấp trường.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="178"/>
        <source>Restoring removes %1 and brings back %2.</source>
        <translation>Khôi phục sẽ xóa %1 và khôi phục lại %2.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="181"/>
        <source>Restoring removes %1.</source>
        <translation>Khôi phục sẽ xóa %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="183"/>
        <source>Restoring brings back %1.</source>
        <translation>Khôi phục sẽ khôi phục lại %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="209"/>
        <source>Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.</source>
        <translation>Chọn bản sao lưu để khôi phục. Dự án hiện tại được lưu tự động trước, nên có thể hoàn tác khôi phục.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="292"/>
        <source>No backups for this project yet. Edit or save the project to start the rolling backup.</source>
        <translation>Chưa có bản sao lưu cho dự án này. Chỉnh sửa hoặc lưu dự án để bắt đầu sao lưu luân phiên.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="320"/>
        <source>Open Folder</source>
        <translation>Mở Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="328"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/BackupRecovery.qml" line="334"/>
        <source>Restore</source>
        <translation>Khôi Phục</translation>
    </message>
</context>
<context>
    <name>Benchmark</name>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="32"/>
        <source>Benchmark</source>
        <translation>Đánh Giá Hiệu Năng</translation>
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
        <translation>n/a</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Pass</source>
        <translation>Đạt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="90"/>
        <source>Fail</source>
        <translation>Không Đạt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="111"/>
        <source>Hotpath Benchmark</source>
        <translation>Đánh Giá Hiệu Năng Hotpath</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="122"/>
        <source>Measures how fast this computer can extract, parse, and visualize frames through Serial Studio's data pipeline.</source>
        <translation>Đo tốc độ máy tính này có thể trích xuất, phân tích và hiển thị frame qua pipeline dữ liệu của Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="168"/>
        <source>The interface will freeze while running</source>
        <translation>Giao diện sẽ đóng băng trong khi chạy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="179"/>
        <source>Each phase runs flat-out on the main thread, so the window stops responding until it finishes. Your current project is reloaded automatically when the benchmark ends.</source>
        <translation>Mỗi giai đoạn chạy liên tục trên luồng chính, do đó cửa sổ ngừng phản hồi cho đến khi hoàn tất. Dự án hiện tại của bạn sẽ được tải lại tự động khi benchmark kết thúc.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="236"/>
        <source>Frames per phase:</source>
        <translation>Số frame mỗi giai đoạn:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="250"/>
        <source>Minimum duration:</source>
        <translation>Thời lượng tối thiểu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="279"/>
        <source>Stages</source>
        <translation>Giai Đoạn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="287"/>
        <source>Parsers</source>
        <translation>Bộ Phân Tích</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="296"/>
        <source>Data export</source>
        <translation>Xuất dữ liệu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="304"/>
        <source>Dashboard</source>
        <translation>Bảng Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="316"/>
        <source>Data</source>
        <translation>Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="326"/>
        <source>Numeric only</source>
        <translation>Chỉ giá trị số</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="335"/>
        <source>Mixed (numeric + text)</source>
        <translation>Hỗn hợp (số + văn bản)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="351"/>
        <source>Select at least one stage and one data type to run a benchmark.</source>
        <translation>Chọn ít nhất một giai đoạn và một loại dữ liệu để chạy benchmark.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="210"/>
        <source>Running %1...</source>
        <translation>Đang Chạy %1...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="211"/>
        <source>Preparing...</source>
        <translation>Đang Chuẩn Bị...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="383"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="395"/>
        <source>Throughput</source>
        <translation>Thông Lượng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="407"/>
        <source>Time</source>
        <translation>Thời Gian</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="419"/>
        <source>Result</source>
        <translation>Kết Quả</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="520"/>
        <source>Run a test to see results</source>
        <translation>Chạy thử nghiệm để xem kết quả</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="539"/>
        <source>Pass/Fail applies to the data-pipeline and parser stages (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard stages are informational.</source>
        <translation>Đạt/Không Đạt áp dụng cho các giai đoạn data-pipeline và parser (data pipeline và Built-in số 1024 K frames/s; Built-in hỗn hợp 512 K; Lua số 256 K; JavaScript số và Lua hỗn hợp 128 K; JavaScript hỗn hợp 64 K). Các giai đoạn export và dashboard chỉ mang tính thông tin.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Built-in numeric 1024 K frames/s; Built-in mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Đạt/Không Đạt áp dụng cho các giai đoạn data-pipeline và parser (data pipeline và Built-in số 1024 K frames/s; Built-in hỗn hợp 512 K; Lua số 256 K; JavaScript số và Lua hỗn hợp 128 K; JavaScript hỗn hợp 64 K). Các giai đoạn export và dashboard chỉ mang tính thông tin.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline and Native numeric 1024 K frames/s; Native mixed 512 K; Lua numeric 256 K; JavaScript numeric and Lua mixed 128 K; JavaScript mixed 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Đạt/Không Đạt áp dụng cho các giai đoạn pipeline dữ liệu và phân tích cú pháp (pipeline dữ liệu và Native số 1024 K khung/s; Native hỗn hợp 512 K; Lua số 256 K; JavaScript số và Lua hỗn hợp 128 K; JavaScript hỗn hợp 64 K). Các giai đoạn xuất và bảng điều khiển chỉ mang tính thông tin.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="554"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the data-pipeline and parser phases (data pipeline 1024 K frames/s; numeric: Lua 256 K, JavaScript 128 K; mixed: Lua 128 K, JavaScript 64 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Đạt/Không Đạt chỉ áp dụng cho các giai đoạn data-pipeline và parser (data pipeline 1024 K frames/s; số: Lua 256 K, JavaScript 128 K; hỗn hợp: Lua 128 K, JavaScript 64 K). Các giai đoạn export và dashboard chỉ mang tính thông tin.</translation>
    </message>
    <message>
        <source>Pass/Fail applies to the parser phases only (Lua target 256 K frames/s, JavaScript 128 K). The export and dashboard phases are informational.</source>
        <translation type="vanished">Đạt/Không Đạt chỉ áp dụng cho các giai đoạn parser (Lua mục tiêu 256 K frames/s, JavaScript 128 K). Các giai đoạn export và dashboard chỉ mang tính thông tin.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="561"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="570"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Running...</source>
        <translation>Đang Chạy...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Benchmark.qml" line="580"/>
        <source>Run Benchmark</source>
        <translation>Chạy Benchmark</translation>
    </message>
</context>
<context>
    <name>BenchmarkRunner</name>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="255"/>
        <source>Data pipeline</source>
        <translation>Đường ống dữ liệu</translation>
    </message>
    <message>
        <source>Lua parser</source>
        <translation type="vanished">Trình phân tích Lua</translation>
    </message>
    <message>
        <source>JavaScript parser</source>
        <translation type="vanished">Trình phân tích JavaScript</translation>
    </message>
    <message>
        <source>Lua + data export</source>
        <translation type="vanished">Lua + xuất dữ liệu</translation>
    </message>
    <message>
        <source>Lua + dashboard</source>
        <translation type="vanished">Lua + dashboard</translation>
    </message>
    <message>
        <source>Native parser (numeric)</source>
        <translation type="vanished">Trình phân tích cú pháp Native (số)</translation>
    </message>
    <message>
        <source>Native parser (mixed)</source>
        <translation type="vanished">Trình phân tích cú pháp Native (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="279"/>
        <source>Built-in parser (numeric)</source>
        <translation>Trình phân tích cú pháp Built-in (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="297"/>
        <source>Built-in parser (mixed)</source>
        <translation>Trình phân tích cú pháp Built-in (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="281"/>
        <source>Lua parser (numeric)</source>
        <translation>Trình phân tích Lua (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="288"/>
        <source>JavaScript parser (numeric)</source>
        <translation>Trình phân tích JavaScript (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="299"/>
        <source>Lua parser (mixed)</source>
        <translation>Trình phân tích Lua (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="306"/>
        <source>JavaScript parser (mixed)</source>
        <translation>Trình phân tích JavaScript (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="322"/>
        <source>Built-in + data export (numeric)</source>
        <translation>Built-in + xuất dữ liệu (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="329"/>
        <source>Lua + data export (numeric)</source>
        <translation>Lua + xuất dữ liệu (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="336"/>
        <source>JavaScript + data export (numeric)</source>
        <translation>JavaScript + xuất dữ liệu (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="345"/>
        <source>Built-in + data export (mixed)</source>
        <translation>Built-in + xuất dữ liệu (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="347"/>
        <source>Lua + data export (mixed)</source>
        <translation>Lua + xuất dữ liệu (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="354"/>
        <source>JavaScript + data export (mixed)</source>
        <translation>JavaScript + xuất dữ liệu (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="370"/>
        <source>Built-in + dashboard (numeric)</source>
        <translation>Built-in + dashboard (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="372"/>
        <source>Lua + dashboard (numeric)</source>
        <translation>Lua + dashboard (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="182"/>
        <source>100 K frames</source>
        <translation>100 K khung</translation>
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
        <translation>1 giây</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>2 seconds</source>
        <translation>2 giây</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>5 seconds</source>
        <translation>5 giây</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="183"/>
        <source>10 seconds</source>
        <translation>10 giây</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="200"/>
        <source>Serial Studio %1 - Hotpath Benchmark</source>
        <translation>Serial Studio %1 - Đánh Giá Hiệu Năng Hotpath</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="202"/>
        <source>%1 (%2), workload: %3 frames minimum, %4 s minimum</source>
        <translation>%1 (%2), khối lượng công việc: tối thiểu %3 khung, tối thiểu %4 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Pipeline</source>
        <translation>Pipeline</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Throughput</source>
        <translation>Thông Lượng</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Target</source>
        <translation>Mục Tiêu</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Time</source>
        <translation>Thời Gian</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="209"/>
        <source>Result</source>
        <translation>Kết Quả</translation>
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
        <translation>n/a</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Pass</source>
        <translation>Đạt</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="221"/>
        <source>Fail</source>
        <translation>Không Đạt</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="227"/>
        <source>%1 s</source>
        <translation>%1 s</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="379"/>
        <source>JavaScript + dashboard (numeric)</source>
        <translation>JavaScript + bảng điều khiển (số)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="388"/>
        <source>Built-in + dashboard (mixed)</source>
        <translation>Tích hợp sẵn + bảng điều khiển (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="390"/>
        <source>Lua + dashboard (mixed)</source>
        <translation>Lua + bảng điều khiển (hỗn hợp)</translation>
    </message>
    <message>
        <location filename="../../src/Benchmark/BenchmarkRunner.cpp" line="397"/>
        <source>JavaScript + dashboard (mixed)</source>
        <translation>JavaScript + bảng điều khiển (hỗn hợp)</translation>
    </message>
    <message>
        <source>Showing dashboard...</source>
        <translation type="vanished">Đang hiển thị bảng điều khiển...</translation>
    </message>
</context>
<context>
    <name>BluetoothLE</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="54"/>
        <source>Device</source>
        <translation>Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="106"/>
        <source>Service</source>
        <translation>Dịch Vụ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="142"/>
        <source>Characteristic</source>
        <translation>Đặc Tính</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="200"/>
        <source>Scanning…</source>
        <translation>Đang Quét…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="236"/>
        <source>No Bluetooth Adapter Detected</source>
        <translation>Không Phát Hiện Bộ Chuyển Đổi Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="247"/>
        <source>Connect a Bluetooth adapter or check your system settings</source>
        <translation>Kết nối bộ chuyển đổi Bluetooth hoặc kiểm tra cài đặt hệ thống</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="274"/>
        <source>This OS is not Supported Yet.</source>
        <translation>Hệ Điều Hành Này Chưa Được Hỗ Trợ.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/BluetoothLE.qml" line="285"/>
        <source>We'll update Serial Studio to work with this operating system as soon as Qt officially supports it</source>
        <translation>Serial Studio sẽ được cập nhật để hoạt động với hệ điều hành này ngay khi QT chính thức hỗ trợ</translation>
    </message>
</context>
<context>
    <name>CANBus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="57"/>
        <source>No CAN Drivers Found</source>
        <translation>Không Tìm Thấy Trình Điều Khiển CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="70"/>
        <source>Install CAN hardware drivers for your system</source>
        <translation>Cài đặt trình điều khiển phần cứng CAN cho hệ thống của bạn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="97"/>
        <source>CAN Driver</source>
        <translation>Trình Điều Khiển CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="140"/>
        <source>Interface</source>
        <translation>Giao Diện</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="169"/>
        <source>Bitrate</source>
        <translation>Tốc Độ Bit</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="223"/>
        <source>Flexible Data-Rate</source>
        <translation>Tốc Độ Dữ Liệu Linh Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="243"/>
        <source>Loopback</source>
        <translation>Loopback</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="263"/>
        <source>Listen-Only</source>
        <translation>Chỉ Lắng Nghe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="283"/>
        <source>DBC Database</source>
        <translation>Cơ Sở Dữ Liệu DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="287"/>
        <source>Import DBC File…</source>
        <translation>Nhập Tệp DBC…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml" line="320"/>
        <source>No CAN Interfaces Found</source>
        <translation>Không Tìm Thấy Giao Diện CAN</translation>
    </message>
</context>
<context>
    <name>CSV::Player</name>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="206"/>
        <source>Select CSV file</source>
        <translation>Chọn tệp CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="208"/>
        <source>CSV files (*.csv)</source>
        <translation>Tệp CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="366"/>
        <source>Device Connection Active</source>
        <translation>Kết Nối Thiết Bị Đang Hoạt Động</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="367"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Để sử dụng tính năng này, bạn phải ngắt kết nối khỏi thiết bị. Bạn có muốn tiếp tục không?</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Cannot read CSV file</source>
        <translation>Không thể đọc tệp CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="381"/>
        <source>Check file permissions and location</source>
        <translation>Kiểm tra quyền truy cập và vị trí tệp</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="411"/>
        <source>Insufficient Data in CSV File</source>
        <translation>Dữ Liệu Không Đủ trong Tệp CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="412"/>
        <source>The CSV file must contain at least one data row to proceed. Check the file and try again.</source>
        <translation>Tệp CSV phải chứa ít nhất một hàng dữ liệu để tiếp tục. Kiểm tra tệp và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="642"/>
        <source>Invalid CSV</source>
        <translation>CSV Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="643"/>
        <source>The CSV file does not contain any data or headers.</source>
        <translation>Tệp CSV không chứa dữ liệu hoặc tiêu đề nào.</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <source>Select a date/time column</source>
        <translation>Chọn cột ngày/giờ</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="652"/>
        <location filename="../../src/CSV/Player.cpp" line="664"/>
        <source>Set interval manually</source>
        <translation>Đặt khoảng thời gian thủ công</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="654"/>
        <source>CSV Date/Time Selection</source>
        <translation>Chọn Ngày/giờ CSV</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="655"/>
        <source>Choose how to handle the date/time data:</source>
        <translation>Chọn cách xử lý dữ liệu ngày/giờ:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="667"/>
        <source>Set Interval</source>
        <translation>Đặt Khoảng Thời Gian</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="668"/>
        <source>Please enter the interval between rows in milliseconds:</source>
        <translation>Nhập khoảng thời gian giữa các hàng tính bằng mili giây:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="684"/>
        <source>Select Date/Time Column</source>
        <translation>Chọn Cột Ngày/giờ</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="685"/>
        <source>Please select the column that contains the date/time data:</source>
        <translation>Vui lòng chọn cột chứa dữ liệu ngày/giờ:</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>Invalid Selection</source>
        <translation>Lựa Chọn Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/CSV/Player.cpp" line="695"/>
        <source>The selected column is not valid.</source>
        <translation>Cột đã chọn không hợp lệ.</translation>
    </message>
</context>
<context>
    <name>ChatSidebar</name>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="44"/>
        <source>Chats</source>
        <translation>Trò Chuyện</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="57"/>
        <location filename="../../qml/AI/ChatSidebar.qml" line="115"/>
        <source>New chat</source>
        <translation>Trò Chuyện Mới</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="125"/>
        <source>%1 messages</source>
        <translation>%1 Thông Điệp</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="147"/>
        <source>Rename...</source>
        <translation>Đổi Tên…</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="158"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="197"/>
        <source>Rename chat</source>
        <translation>Đổi Tên Trò Chuyện</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="217"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ChatSidebar.qml" line="222"/>
        <source>Rename</source>
        <translation>Đổi Tên</translation>
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
        <translation>Xuất Console là tính năng Pro.</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="332"/>
        <source>This feature requires a license. Please purchase one to enable console export.</source>
        <translation>Tính năng này yêu cầu giấy phép. Vui lòng mua giấy phép để kích hoạt xuất console.</translation>
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
        <translation>Không Kết Thúc Dòng</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="255"/>
        <source>New Line</source>
        <translation>Dòng Mới</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="256"/>
        <source>Carriage Return</source>
        <translation>Xuống Dòng</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="257"/>
        <source>CR + NL</source>
        <translation>CR + NL</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="267"/>
        <source>Plain Text</source>
        <translation>Văn Bản Thuần</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="268"/>
        <source>Hexadecimal</source>
        <translation>Thập Lục Phân</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="290"/>
        <source>No Checksum</source>
        <translation>Không Có Checksum</translation>
    </message>
    <message>
        <location filename="../../src/Console/Handler.cpp" line="926"/>
        <source>Device %1</source>
        <translation>Thiết Bị %1</translation>
    </message>
</context>
<context>
    <name>ConstantsLibraryDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="44"/>
        <source>Insert Constant</source>
        <translation>Chèn Hằng Số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Fundamental</source>
        <translation>Cơ Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="126"/>
        <source>Speed of light in vacuum</source>
        <translation>Tốc độ ánh sáng trong chân không</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="127"/>
        <source>Planck constant</source>
        <translation>Hằng số Planck</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="128"/>
        <source>Elementary charge</source>
        <translation>Điện tích nguyên tố</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="129"/>
        <source>Avogadro constant</source>
        <translation>Hằng số Avogadro</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="130"/>
        <source>Boltzmann constant</source>
        <translation>Hằng số Boltzmann</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="131"/>
        <source>Stefan-Boltzmann constant</source>
        <translation>Hằng số Stefan-Boltzmann</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Mechanics</source>
        <translation>Cơ Học</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="133"/>
        <source>Standard gravity</source>
        <translation>Gia tốc trọng trường chuẩn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="134"/>
        <source>Gravitational constant</source>
        <translation>Hằng số hấp dẫn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Pressure</source>
        <translation>Áp Suất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="136"/>
        <source>Standard atmosphere</source>
        <translation>Áp suất khí quyển chuẩn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="137"/>
        <source>Sea-level barometric pressure</source>
        <translation>Áp suất khí quyển mực nước biển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Temperature</source>
        <translation>Nhiệt Độ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="139"/>
        <source>Absolute zero (Celsius)</source>
        <translation>Độ không tuyệt đối (Celsius)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="140"/>
        <source>Water freezing point</source>
        <translation>Điểm đóng băng của nước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="141"/>
        <source>Water boiling point (1 atm)</source>
        <translation>Điểm sôi của nước (1 atm)</translation>
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
        <translation>Khí &amp; Chất Lỏng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="143"/>
        <source>Universal gas constant</source>
        <translation>Hằng số khí lý tưởng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="144"/>
        <source>Specific gas constant (dry air)</source>
        <translation>Hằng số khí riêng (không khí khô)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="145"/>
        <source>Specific gas constant (water vapor)</source>
        <translation>Hằng số khí riêng (hơi nước)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="146"/>
        <source>Air density (sea level, 15°C)</source>
        <translation>Khối lượng riêng không khí (mực nước biển, 15°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="147"/>
        <source>Water density (4°C)</source>
        <translation>Khối lượng riêng nước (4°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="148"/>
        <source>Speed of sound in air (20°C)</source>
        <translation>Tốc độ âm thanh trong không khí (20°C)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="149"/>
        <source>Heat capacity ratio (dry air)</source>
        <translation>Tỷ số nhiệt dung (không khí khô)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Electromagnetism</source>
        <translation>Điện Từ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="151"/>
        <source>Vacuum permittivity</source>
        <translation>Hằng số điện môi chân không</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="152"/>
        <source>Vacuum permeability</source>
        <translation>Độ từ th투chân không</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Math</source>
        <translation>Toán Học</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="154"/>
        <source>Pi</source>
        <translation>Pi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="155"/>
        <source>Euler's number</source>
        <translation>Số Euler</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="156"/>
        <source>Golden ratio</source>
        <translation>Tỷ lệ vàng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="212"/>
        <source>Physics Constants</source>
        <translation>Hằng Số Vật Lý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="221"/>
        <source>SI-unit preset values. Click a row to insert it into %1.</source>
        <translation>Giá trị đơn vị SI định sẵn. Nhấp vào một hàng để chèn vào %1.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="231"/>
        <source>Search</source>
        <translation>Tìm Kiếm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="250"/>
        <source>Symbol</source>
        <translation>Ký Hiệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="251"/>
        <source>Name</source>
        <translation>Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="252"/>
        <source>Value</source>
        <translation>Giá Trị</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="253"/>
        <source>Category</source>
        <translation>Danh Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="357"/>
        <source>No constants match.</source>
        <translation>Không có hằng số nào khớp.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="378"/>
        <source>%1 constants</source>
        <translation>%1 hằng số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="379"/>
        <source>%1 of %2 constants</source>
        <translation>%1 trong số %2 hằng số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ConstantsLibraryDialog.qml" line="383"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>ControlScriptView</name>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="33"/>
        <source>Control Loop</source>
        <translation>Vòng Lặp Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="45"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="168"/>
        <source>Undo</source>
        <translation>Hoàn Tác</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="52"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="179"/>
        <source>Redo</source>
        <translation>Làm Lại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="60"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="198"/>
        <source>Cut</source>
        <translation>Cắt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="61"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="208"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="62"/>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="218"/>
        <source>Paste</source>
        <translation>Dán</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="67"/>
        <source>Select All</source>
        <translation>Chọn Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="77"/>
        <source>Format Document</source>
        <translation>Định Dạng Tài Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="84"/>
        <source>Format Selection</source>
        <translation>Định Dạng Vùng Chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="148"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="153"/>
        <source>Reset to the default control loop</source>
        <translation>Đặt lại về vòng lặp điều khiển mặc định</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="163"/>
        <source>Import a control loop file</source>
        <translation>Nhập file vòng lặp điều khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="241"/>
        <source>Open the control loop documentation</source>
        <translation>Mở tài liệu hướng dẫn vòng lặp điều khiển</translation>
    </message>
    <message>
        <source>Reset to the default control script</source>
        <translation type="vanished">Đặt lại về script điều khiển mặc định</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="158"/>
        <source>Open</source>
        <translation>Mở</translation>
    </message>
    <message>
        <source>Import a control script file</source>
        <translation type="vanished">Nhập file script điều khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="173"/>
        <source>Undo the last code edit</source>
        <translation>Hoàn tác chỉnh sửa mã gần nhất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="185"/>
        <source>Redo the previously undone edit</source>
        <translation>Làm lại thao tác vừa hoàn tác</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="203"/>
        <source>Cut selected code to clipboard</source>
        <translation>Cắt mã đã chọn vào clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="213"/>
        <source>Copy selected code to clipboard</source>
        <translation>Sao chép mã đã chọn vào clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="222"/>
        <source>Paste code from clipboard</source>
        <translation>Dán mã từ clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="236"/>
        <source>Help</source>
        <translation>Trợ Giúp</translation>
    </message>
    <message>
        <source>Open the control script documentation</source>
        <translation type="vanished">Mở tài liệu hướng dẫn script điều khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="251"/>
        <source>Validate</source>
        <translation>Xác Thực</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ControlScriptView.qml" line="255"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Xác minh rằng script biên dịch chính xác</translation>
    </message>
</context>
<context>
    <name>CrashRecovery</name>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="31"/>
        <source>Recovery Options</source>
        <translation>Tùy Chọn Khôi Phục</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="57"/>
        <source>Serial Studio has closed unexpectedly several times in a row.</source>
        <translation>Serial Studio đã đóng bất ngờ nhiều lần liên tiếp.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="82"/>
        <source>Consecutive crashes: %1</source>
        <translation>Số lần sự cố liên tiếp: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="90"/>
        <source>Last reported stage: %1</source>
        <translation>Giai đoạn báo cáo cuối cùng: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="98"/>
        <source>Detected at: %1</source>
        <translation>Phát hiện tại: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="113"/>
        <source>Pick a recovery action. Serial Studio will quit after applying it so the next launch starts clean.</source>
        <translation>Chọn một hành động khôi phục. Serial Studio sẽ thoát sau khi áp dụng để lần khởi động tiếp theo bắt đầu sạch.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="127"/>
        <source>Reset Rendering Backend to Default</source>
        <translation>Đặt Lại Backend Kết Xuất Về Mặc Định</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="135"/>
        <source>Skip Restoring the Last Opened Project</source>
        <translation>Bỏ Qua Khôi Phục Dự Án Đã Mở Gần Nhất</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="142"/>
        <source>Reset all Preferences</source>
        <translation>Đặt Lại Tất Cả Tùy Chọn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/CrashRecovery.qml" line="160"/>
        <source>Continue Anyway</source>
        <translation>Tiếp Tục Dù Sao</translation>
    </message>
</context>
<context>
    <name>CsvPlayer</name>
    <message>
        <location filename="../../qml/Dialogs/CsvPlayer.qml" line="35"/>
        <source>CSV Player</source>
        <translation>Trình Phát CSV</translation>
    </message>
</context>
<context>
    <name>DBCPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="44"/>
        <source>DBC File Preview</source>
        <translation>Xem Trước Tệp DBC</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="169"/>
        <source>DBC File: %1</source>
        <translation>Tệp DBC: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="177"/>
        <source>Review the CAN messages and signals to import into a new Serial Studio project.</source>
        <translation>Xem lại các thông điệp CAN và tín hiệu để nhập vào dự án Serial Studio mới.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="185"/>
        <source>Messages</source>
        <translation>Thông Điệp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="219"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="229"/>
        <source>Message Name</source>
        <translation>Tên Thông Điệp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="235"/>
        <source>CAN ID</source>
        <translation>ID CAN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="242"/>
        <source>Signals</source>
        <translation>Tín Hiệu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="323"/>
        <source>No messages found in DBC file.</source>
        <translation>Không tìm thấy thông điệp nào trong tệp DBC.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="341"/>
        <source>Total: %1 messages, %2 signals</source>
        <translation>Tổng: %1 tin nhắn, %2 tín hiệu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="348"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml" line="359"/>
        <source>Create Project</source>
        <translation>Tạo Dự Án</translation>
    </message>
</context>
<context>
    <name>Dashboard</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard.qml" line="262"/>
        <source>Dashboard %1</source>
        <translation>Bảng Điều Khiển %1</translation>
    </message>
</context>
<context>
    <name>DashboardButton</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="40"/>
        <source>Send</source>
        <translation>Gửi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardButton.qml" line="64"/>
        <source>No transmit function defined</source>
        <translation>Chưa định nghĩa hàm truyền</translation>
    </message>
</context>
<context>
    <name>DashboardCanvas</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="326"/>
        <source>Set Wallpaper…</source>
        <translation>Đặt Hình Nền…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="332"/>
        <source>Clear Wallpaper</source>
        <translation>Xóa Hình Nền</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="342"/>
        <source>Tile Windows</source>
        <translation>Xếp Cửa Sổ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="361"/>
        <source>Pro features detected in this project.</source>
        <translation>Phát hiện tính năng Pro trong dự án này.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="363"/>
        <source>Fallback widgets are active. Purchase a license for full functionality.</source>
        <translation>Widget dự phòng đang hoạt động. Mua giấy phép để sử dụng đầy đủ tính năng.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="504"/>
        <source>Empty Workspace</source>
        <translation>Không Gian Làm Việc Trống</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="518"/>
        <source>Use the search bar to find and add widgets, or right-click a widget in another workspace to add it here.</source>
        <translation>Sử dụng thanh tìm kiếm để tìm và thêm widget, hoặc nhấp chuột phải vào widget trong không gian làm việc khác để thêm vào đây.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml" line="533"/>
        <source>Search Widgets</source>
        <translation>Tìm Kiếm Widget</translation>
    </message>
</context>
<context>
    <name>DashboardLayout</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="37"/>
        <source>Dashboard</source>
        <translation>Bảng Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="215"/>
        <source>API Server Active (%1)</source>
        <translation>Máy Chủ API Đang Hoạt Động (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="216"/>
        <source>API Server Ready</source>
        <translation>Máy Chủ API Sẵn Sàng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/DashboardLayout.qml" line="217"/>
        <source>API Server Off</source>
        <translation>Máy Chủ API Tắt</translation>
    </message>
</context>
<context>
    <name>DashboardOutputPanel</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="155"/>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="307"/>
        <source>Send</source>
        <translation>Gửi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml" line="295"/>
        <source>Enter command…</source>
        <translation>Nhập lệnh…</translation>
    </message>
</context>
<context>
    <name>DashboardSlider</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardSlider.qml" line="90"/>
        <source>No transmit function defined</source>
        <translation>Không có hàm truyền được định nghĩa</translation>
    </message>
</context>
<context>
    <name>DashboardTextField</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="47"/>
        <source>Enter command…</source>
        <translation>Nhập lệnh…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="57"/>
        <source>Send</source>
        <translation>Gửi</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardTextField.qml" line="76"/>
        <source>No transmit function defined</source>
        <translation>Không có hàm truyền được định nghĩa</translation>
    </message>
</context>
<context>
    <name>DashboardToggle</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="57"/>
        <source>ON</source>
        <translation>BẬT</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="59"/>
        <source>OFF</source>
        <translation>TẮT</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Output/DashboardToggle.qml" line="70"/>
        <source>No transmit function defined</source>
        <translation>Không có hàm truyền được định nghĩa</translation>
    </message>
</context>
<context>
    <name>DataGrid</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="98"/>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="99"/>
        <source>Resume</source>
        <translation>Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="313"/>
        <source>Awaiting data…</source>
        <translation>Đang chờ dữ liệu…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/DataGrid.qml" line="368"/>
        <source>Open %1 in a separate window</source>
        <translation>Mở %1 trong cửa sổ riêng</translation>
    </message>
</context>
<context>
    <name>DataModel::ControlScriptEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="289"/>
        <source>Select Javascript file to import</source>
        <translation>Chọn tệp Javascript để nhập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="357"/>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="368"/>
        <source>Code Validation Failed</source>
        <translation>Xác Thực Mã Thất Bại</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="358"/>
        <source>Line %1: %2</source>
        <translation>Dòng %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="369"/>
        <source>The script must define a setup() and/or loop() function.</source>
        <translation>Script phải định nghĩa hàm setup() và/hoặc loop().</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="374"/>
        <source>Code Validation Successful</source>
        <translation>Xác Thực Mã Thành Công</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/ControlScriptEditor.cpp" line="375"/>
        <source>No syntax errors detected in the control loop.</source>
        <translation>Không phát hiện lỗi cú pháp trong vòng lặp điều khiển.</translation>
    </message>
    <message>
        <source>No syntax errors detected in the control script.</source>
        <translation type="vanished">Không phát hiện lỗi cú pháp trong script điều khiển.</translation>
    </message>
</context>
<context>
    <name>DataModel::DBCImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>Import DBC File</source>
        <translation>Nhập Tệp DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="125"/>
        <source>DBC Files (*.dbc);;All Files (*)</source>
        <translation>Tệp DBC (*.DBC);;Tất Cả Tệp (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="160"/>
        <source>Failed to parse DBC file: %1</source>
        <translation>Không thể phân tích tệp DBC: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="161"/>
        <source>Verify the file format and try again.</source>
        <translation>Xác minh định dạng tệp và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="163"/>
        <source>DBC Import Error</source>
        <translation>Lỗi Nhập DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="171"/>
        <source>DBC file contains no messages</source>
        <translation>Tệp DBC không chứa thông điệp nào</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="172"/>
        <source>The selected file does not contain any CAN message definitions.</source>
        <translation>Tệp đã chọn không chứa bất kỳ định nghĩa thông điệp CAN nào.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="174"/>
        <source>DBC Import Warning</source>
        <translation>Cảnh Báo Nhập DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="269"/>
        <source>Overview</source>
        <translation>Tổng Quan</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="326"/>
        <source>Active</source>
        <translation>Hoạt Động</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Không thể tải dự án đã nhập</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Không thể tải JSON dự án đã tạo.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="218"/>
        <source>The project editor is now open for customization.</source>
        <translation>Trình chỉnh sửa dự án hiện đã mở để tùy chỉnh.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="220"/>
        <source> Skipped %1 signal(s) using extended multiplexing (SG_MUL_VAL_); only simple multiplexing is supported.</source>
        <translation>Đã bỏ qua %1 tín hiệu sử dụng ghép kênh mở rộng (SG_MUL_VAL_); chỉ hỗ trợ ghép kênh đơn giản.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="225"/>
        <source>Successfully imported DBC file with %1 messages and %2 signals.</source>
        <translation>Đã nhập thành công tệp DBC với %1 thông báo và %2 tín hiệu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="230"/>
        <source>DBC Import Complete</source>
        <translation>Hoàn Tất Nhập DBC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/DBCImporter.cpp" line="254"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
</context>
<context>
    <name>DataModel::DatasetTransformEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="63"/>
        <source>Dataset Value Transform</source>
        <translation>Biến Đổi Giá Trị Dataset</translation>
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
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="106"/>
        <source>Enter raw value (e.g., 1024)</source>
        <translation>Nhập giá trị thô (ví dụ: 1024)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="111"/>
        <source>Test</source>
        <translation>Kiểm Tra</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="112"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="114"/>
        <source>Apply</source>
        <translation>Áp Dụng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="115"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="129"/>
        <source>Language:</source>
        <translation>Ngôn Ngữ:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="132"/>
        <source>Template:</source>
        <translation>Mẫu:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="143"/>
        <source>Input:</source>
        <translation>Đầu Vào:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="146"/>
        <source>Output:</source>
        <translation>Đầu Ra:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="224"/>
        <source>Transform — %1</source>
        <translation>Biến Đổi — %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="304"/>
        <source>The value transform has a syntax error and was not applied.</source>
        <translation>Biến đổi giá trị có lỗi cú pháp và không được áp dụng.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="313"/>
        <source>The value transform must define a transform(value) function.</source>
        <translation>Biến đổi giá trị phải định nghĩa hàm transform(value).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="314"/>
        <source>Define a transform(value) function that returns a number, or use Clear to remove the transform.</source>
        <translation>Định nghĩa hàm transform(value) trả về một số, hoặc dùng Xóa để loại bỏ biến đổi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="332"/>
        <source>Enter a value</source>
        <translation>Nhập giá trị</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="339"/>
        <source>Invalid number</source>
        <translation>Số không hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="408"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Định Dạng Tài Liệu	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="409"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Định Dạng Lựa Chọn	ctrl+i</translation>
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
-- Định nghĩa hàm transform(value) nhận giá trị đọc được
-- từ dataset trực tiếp và trả về một số đã biến đổi. Nếu
-- không định nghĩa hàm transform(), giá trị thô sẽ được giữ
-- nguyên và không có gì được lưu cùng dự án.
--
-- Ví dụ:
--    function transform(value)
--      return value * 0.01 + 273.15
--    end
--
-- Các biến toàn cục được khai báo ở cấp cao nhất sẽ tồn tại
-- giữa các frame, hữu ích cho bộ lọc, bộ tích lũy và các
-- phép biến đổi có trạng thái khác, giống như script phân
-- tích frame:
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
-- Sử dụng menu Template để xem các ví dụ có sẵn, hoặc
-- nhấn Test để thử hàm của bạn.
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
 * Định nghĩa hàm transform(value) nhận giá trị đọc được
 * từ dataset trực tiếp và trả về một số đã biến đổi. Nếu
 * không định nghĩa hàm transform(), giá trị thô sẽ được giữ
 * nguyên và không có gì được lưu cùng dự án.
 *
 * Ví dụ:
 *   function transform(value) {
 *     return value * 0.01 + 273.15;
 *   }
 *
 * Các biến toàn cục được khai báo ở cấp cao nhất sẽ tồn tại
 * giữa các frame, hữu ích cho bộ lọc, bộ tích lũy và các
 * phép biến đổi có trạng thái khác -- giống như script phân
 * tích frame:
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
 * Sử dụng menu Template để xem các ví dụ có sẵn, hoặc
 * nhấn Test để thử hàm của bạn.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="608"/>
        <source>Failed to create the Lua engine.</source>
        <translation>Không thể tạo công cụ Lua.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="646"/>
        <source>Line %1: %2</source>
        <translation>Dòng %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="691"/>
        <source>Engine error</source>
        <translation>Lỗi Engine</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="714"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="727"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="744"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="755"/>
        <source>Error: %1</source>
        <translation>Lỗi: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="720"/>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="748"/>
        <source>Error: transform() not defined</source>
        <translation>Lỗi: transform() chưa được định nghĩa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="732"/>
        <source>Error: transform() must return a number</source>
        <translation>Lỗi: transform() phải trả về một số</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/DatasetTransformEditor.cpp" line="800"/>
        <source>Select Template…</source>
        <translation>Chọn Template…</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameBuilder</name>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1429"/>
        <source>JavaScript transform exceeded budget</source>
        <translation>Chuyển đổi JavaScript vượt quá giới hạn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1430"/>
        <source>A dataset transform took longer than %1 ms; remaining datasets in the frame fell back to raw values until the next frame. Profile or simplify the transform code.</source>
        <translation>Một phép biến đổi tập dữ liệu mất hơn %1 ms; các tập dữ liệu còn lại trong frame sử dụng giá trị thô cho đến frame tiếp theo. Hãy phân tích hoặc đơn giản hóa mã chuyển đổi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="230"/>
        <source>Frame pool exhausted</source>
        <translation>Bộ nhớ frame đã cạn kiệt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="232"/>
        <source>A downstream consumer (dashboard, CSV/MDF4 export, session DB, or API subscriber) is not draining frames fast enough. Serial Studio is falling back to per-frame allocations until the backlog clears. Disable a heavy consumer or reduce the data rate.</source>
        <translation>Một tiến trình phía sau (dashboard, xuất CSV/MDF4, cơ sở dữ liệu phiên, hoặc API subscriber) không xử lý frame đủ nhanh. Serial Studio sẽ chuyển sang cấp phát từng frame cho đến khi xóa hết tồn đọng. Hãy tắt bớt tiến trình nặng hoặc giảm tốc độ dữ liệu.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1666"/>
        <source>Device A</source>
        <translation>Thiết Bị A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1705"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1819"/>
        <source>Channel %1</source>
        <translation>Kênh %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1714"/>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1835"/>
        <source>Quick Plot</source>
        <translation>Biểu Đồ Nhanh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1721"/>
        <source>Quick Plot Data</source>
        <translation>Dữ Liệu Biểu Đồ Nhanh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1733"/>
        <source>Multiple Plots</source>
        <translation>Nhiều Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1830"/>
        <source>Audio Input</source>
        <translation>Đầu Vào Âm Thanh</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserModel</name>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Plain text (UTF-8)</source>
        <translation>Văn bản thuần (UTF-8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Hexadecimal</source>
        <translation>Thập Lục Phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="257"/>
        <source>Binary (raw bytes)</source>
        <translation>Nhị phân (byte thô)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="265"/>
        <source>End delimiter only</source>
        <translation>Chỉ dấu phân cách cuối</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="266"/>
        <source>Start + end delimiters</source>
        <translation>Dấu phân cách đầu + cuối</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="267"/>
        <source>Start delimiter only</source>
        <translation>Chỉ dấu phân cách đầu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="268"/>
        <source>No delimiters (whole chunk)</source>
        <translation>Không có dấu phân cách (toàn bộ khối)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="279"/>
        <source>No Checksum</source>
        <translation>Không Có Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="310"/>
        <source>Select Frame Parser Template</source>
        <translation>Chọn Mẫu Trình Phân Tích Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="311"/>
        <source>Choose a template to load:</source>
        <translation>Chọn mẫu để tải:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="494"/>
        <source>Invalid hexadecimal input.</source>
        <translation>Đầu vào thập lục phân không hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="521"/>
        <source>No template selected.</source>
        <translation>Không có mẫu nào được chọn.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="561"/>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation>%1 frame đã trích xuất | %2 byte đã tiêu thụ | %3 byte đã đệm | %4 đã loại bỏ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="632"/>
        <source>Invalid JSON: %1</source>
        <translation>JSON Không Hợp Lệ: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/FrameParserModel.cpp" line="728"/>
        <source>Parameters</source>
        <translation>Tham Số</translation>
    </message>
</context>
<context>
    <name>DataModel::FrameParserTestDialog</name>
    <message>
        <source>None</source>
        <translation type="vanished">Không Có</translation>
    </message>
    <message>
        <source>Invalid Hex Input</source>
        <translation type="vanished">Đầu Vào Hex Không Hợp Lệ</translation>
    </message>
    <message>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation type="vanished">Vui lòng nhập các byte thập lục phân hợp lệ.

Định dạng hợp lệ: 01 A2 FF 3C</translation>
    </message>
    <message>
        <source>(no frames)</source>
        <translation type="vanished">(không có frame)</translation>
    </message>
    <message>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation type="vanished">Trích xuất không tạo ra frame hoàn chỉnh. Kiểm tra dấu phân cách bắt đầu / kết thúc và chế độ phát hiện.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 frame đã trích xuất | %2 byte đã tiêu thụ | %3 byte đã đệm | %4 đã loại bỏ</translation>
    </message>
    <message>
        <source>Pipeline Configuration</source>
        <translation type="vanished">Cấu Hình Pipeline</translation>
    </message>
    <message>
        <source>Pipeline Results</source>
        <translation type="vanished">Kết Quả Pipeline</translation>
    </message>
    <message>
        <source>Detection</source>
        <translation type="vanished">Phát Hiện</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Bộ Giải Mã</translation>
    </message>
    <message>
        <source>Checksum</source>
        <translation type="vanished">Checksum</translation>
    </message>
    <message>
        <source>Start Delimiter</source>
        <translation type="vanished">Dấu Phân Cách Đầu</translation>
    </message>
    <message>
        <source>End Delimiter</source>
        <translation type="vanished">Dấu Phân Cách Cuối</translation>
    </message>
    <message>
        <source>Hex Delimiters</source>
        <translation type="vanished">Dấu Phân Cách Hex</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Chỉ dấu phân cách cuối</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Dấu phân cách đầu + cuối</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Chỉ dấu phân cách đầu</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Không có dấu phân cách (toàn bộ khối)</translation>
    </message>
    <message>
        <source>Plain text (UTF-8)</source>
        <translation type="vanished">Văn bản thuần (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Thập Lục Phân</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Nhị phân (byte thô)</translation>
    </message>
    <message>
        <source>HEX</source>
        <translation type="vanished">HEX</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation type="vanished">Xóa</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Đánh Giá</translation>
    </message>
    <message>
        <source>Enter raw stream bytes here...</source>
        <translation type="vanished">Nhập byte luồng thô tại đây...</translation>
    </message>
    <message>
        <source>Stage</source>
        <translation type="vanished">Chuẩn Bị</translation>
    </message>
    <message>
        <source>Frame Data Input</source>
        <translation type="vanished">Đầu Vào Dữ Liệu Frame</translation>
    </message>
    <message>
        <source>Frame Parser Results</source>
        <translation type="vanished">Kết Quả Trình Phân Tích Frame</translation>
    </message>
    <message>
        <source>Enter frame data here…</source>
        <translation type="vanished">Nhập dữ liệu frame tại đây…</translation>
    </message>
    <message>
        <source>Dataset Index</source>
        <translation type="vanished">Chỉ Số Dataset</translation>
    </message>
    <message>
        <source>Value</source>
        <translation type="vanished">Giá Trị</translation>
    </message>
    <message>
        <source>Enter frame data above, enable HEX mode if needed, then click "Evaluate" to run the frame parser.

Example (Text): a,b,c,d,e,f
Example (HEX):  48 65 6C 6C 6F</source>
        <translation type="vanished">Nhập dữ liệu frame ở trên, bật chế độ HEX nếu cần, sau đó nhấn "Đánh Giá" để chạy trình phân tích frame.

Ví dụ (Text): a,b,c,d,e,f
Ví dụ (HEX):  48 65 6C 6C 6F</translation>
    </message>
    <message>
        <source>Test Frame Parser</source>
        <translation type="vanished">Kiểm Tra Trình Phân Tích Frame</translation>
    </message>
    <message>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation type="vanished">Nhập byte hex (ví dụ: 01 A2 FF)</translation>
    </message>
    <message>
        <source>(empty)</source>
        <translation type="vanished">(trống)</translation>
    </message>
    <message>
        <source>No data returned</source>
        <translation type="vanished">Không có dữ liệu trả về</translation>
    </message>
</context>
<context>
    <name>DataModel::JsCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="219"/>
        <source>Change Scripting Language?</source>
        <translation>Thay Đổi Ngôn Ngữ Kịch Bản?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="220"/>
        <source>Switching the scripting language replaces the current parser code with the equivalent template in the new language.

Any unsaved changes are lost. Continue?</source>
        <translation>Chuyển đổi ngôn ngữ kịch bản sẽ thay thế mã trình phân tích hiện tại bằng mẫu tương đương trong ngôn ngữ mới.

Mọi thay đổi chưa lưu sẽ bị mất. Tiếp tục?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Lua file to import</source>
        <translation>Chọn tệp Lua để nhập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="382"/>
        <source>Select Javascript file to import</source>
        <translation>Chọn tệp Javascript để nhập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="414"/>
        <source>Code Validation Successful</source>
        <translation>Xác Thực Mã Thành Công</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="415"/>
        <source>No syntax errors detected in the parser code.</source>
        <translation>Không phát hiện lỗi cú pháp trong mã trình phân tích.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="525"/>
        <source>Select Frame Parser Template</source>
        <translation>Chọn Mẫu Trình Phân Tích Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/JsCodeEditor.cpp" line="526"/>
        <source>Choose a template to load:</source>
        <translation>Chọn mẫu để tải:</translation>
    </message>
</context>
<context>
    <name>DataModel::ModbusMapImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="299"/>
        <source>Import Modbus Register Map</source>
        <translation>Nhập Sơ Đồ Thanh Ghi Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="303"/>
        <source>Modbus Register Maps (*.csv *.xml *.json);;CSV Files (*.csv);;XML Files (*.xml);;JSON Files (*.json);;All Files (*)</source>
        <translation>Sơ Đồ Thanh Ghi Modbus (*.CSV *.XML *.JSON);;Tệp CSV (*.CSV);;Tệp XML (*.XML);;Tệp JSON (*.JSON);;Tất Cả Tệp (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="340"/>
        <source>No registers found</source>
        <translation>Không tìm thấy thanh ghi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="341"/>
        <source>The file could not be parsed or contains no register definitions.</source>
        <translation>Tệp không thể phân tích hoặc không chứa định nghĩa thanh ghi nào.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="343"/>
        <source>Modbus Import</source>
        <translation>Nhập Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="787"/>
        <source>Overview</source>
        <translation>Tổng Quan</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="813"/>
        <source>On</source>
        <translation>Bật</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Không thể tải dự án đã nhập</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Không thể tải JSON dự án đã tạo.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="388"/>
        <source>Successfully imported %1 registers in %2 groups.</source>
        <translation>Đã nhập thành công %1 thanh ghi trong %2 nhóm.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="390"/>
        <source>The project editor is now open for customization.</source>
        <translation>Trình chỉnh sửa dự án hiện đã mở để tùy chỉnh.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ModbusMapImporter.cpp" line="392"/>
        <source>Modbus Import Complete</source>
        <translation>Hoàn Tất Nhập Modbus</translation>
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
        <translation type="vanished">Văn bản thuần (UTF-8)</translation>
    </message>
    <message>
        <source>Hexadecimal</source>
        <translation type="vanished">Thập Lục Phân</translation>
    </message>
    <message>
        <source>Base64</source>
        <translation type="vanished">Base64</translation>
    </message>
    <message>
        <source>Binary (raw bytes)</source>
        <translation type="vanished">Binary (byte thô)</translation>
    </message>
    <message>
        <source>End delimiter only</source>
        <translation type="vanished">Chỉ dấu phân cách cuối</translation>
    </message>
    <message>
        <source>Start + end delimiters</source>
        <translation type="vanished">Dấu phân cách đầu + cuối</translation>
    </message>
    <message>
        <source>Start delimiter only</source>
        <translation type="vanished">Chỉ dấu phân cách đầu</translation>
    </message>
    <message>
        <source>No delimiters (whole chunk)</source>
        <translation type="vanished">Không có dấu phân cách (toàn bộ khối)</translation>
    </message>
    <message>
        <source>No Checksum</source>
        <translation type="vanished">Không Có Checksum</translation>
    </message>
    <message>
        <source>Invalid hexadecimal input.</source>
        <translation type="vanished">Đầu vào thập lục phân không hợp lệ.</translation>
    </message>
    <message>
        <source>No template selected.</source>
        <translation type="vanished">Không có mẫu nào được chọn.</translation>
    </message>
    <message>
        <source>%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | %4 dropped</source>
        <translation type="vanished">%1 frame đã trích xuất | %2 byte đã tiêu thụ | %3 byte đã đệm | %4 đã loại bỏ</translation>
    </message>
    <message>
        <source>Parameters</source>
        <translation type="vanished">Tham Số</translation>
    </message>
</context>
<context>
    <name>DataModel::OutputCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="288"/>
        <source>Select Javascript file to import</source>
        <translation>Chọn tệp Javascript để nhập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="346"/>
        <source>Select Output Widget Template</source>
        <translation>Chọn Mẫu Widget Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/OutputCodeEditor.cpp" line="347"/>
        <source>Choose a template to load:</source>
        <translation>Chọn mẫu để tải:</translation>
    </message>
</context>
<context>
    <name>DataModel::PainterCodeEditor</name>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="300"/>
        <source>Select Javascript file to import</source>
        <translation>Chọn tệp Javascript để nhập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="387"/>
        <source>Select Painter Widget Template</source>
        <translation>Chọn Mẫu Widget Painter</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="388"/>
        <source>Choose a template to load:</source>
        <translation>Chọn mẫu để tải:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="432"/>
        <source>Add datasets for this template?</source>
        <translation>Thêm dataset cho mẫu này?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Editors/PainterCodeEditor.cpp" line="433"/>
        <source>"%1" expects %2 dataset(s); the current group has %3.

Add %4 dataset(s) using the template's defaults?</source>
        <translation>"%1" yêu cầu %2 dataset; nhóm hiện tại có %3.

Thêm %4 dataset sử dụng giá trị mặc định của mẫu?</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectEditor</name>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1471"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1474"/>
        <source>Frame Parser</source>
        <translation>Bộ Phân Tích Frame</translation>
    </message>
    <message>
        <source>Groups</source>
        <translation type="vanished">Nhóm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1768"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1769"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1786"/>
        <source>Shared Memory</source>
        <translation>Bộ Nhớ Dùng Chung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1755"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1774"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1775"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5835"/>
        <source>Dataset Values</source>
        <translation>Giá Trị Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1935"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1948"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1949"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1964"/>
        <source>Workspaces</source>
        <translation>Không Gian Làm Việc</translation>
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
        <translation type="vanished">Script Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2058"/>
        <source>Publishing</source>
        <translation>Xuất Bản</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2068"/>
        <source>Enable Publishing</source>
        <translation>Bật Xuất Bản</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2069"/>
        <source>Broadcast frames, raw bytes and notifications to the broker</source>
        <translation>Phát các khung, byte thô và thông báo đến broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2080"/>
        <source>Payload</source>
        <translation>Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2081"/>
        <source>Selects what gets published: parsed dashboard data or raw RX bytes</source>
        <translation>Chọn nội dung xuất bản: dữ liệu bảng điều khiển đã phân tích hoặc byte RX thô</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2091"/>
        <source>Publish Rate (Hz)</source>
        <translation>Tốc Độ Xuất Bản (Hz)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2092"/>
        <source>How many times per second to publish (1-30 Hz). Higher rates increase broker load; dashboard data is rate-limited so a slow broker never blocks frame parsing.</source>
        <translation>Số lần xuất bản mỗi giây (1-30 Hz). Tốc độ cao tăng tải broker; dữ liệu bảng điều khiển bị giới hạn tốc độ nên broker chậm không bao giờ chặn phân tích khung.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2104"/>
        <source>Topic Base</source>
        <translation>Topic Gốc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2105"/>
        <source>serial-studio/device</source>
        <translation>serial-studio/thiết-bị</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2106"/>
        <source>Base topic used for frame and raw-byte publishing</source>
        <translation>Topic cơ sở dùng để xuất bản frame và byte thô</translation>
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
        <translation>Mặc định là Topic Cơ Sở khi để trống</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2118"/>
        <source>Topic the user script publishes to</source>
        <translation>Topic mà script người dùng xuất bản đến</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2128"/>
        <source>Publish Notifications</source>
        <translation>Xuất Bản Thông Báo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2129"/>
        <source>Mirror dashboard notifications to a dedicated topic</source>
        <translation>Phản chiếu thông báo dashboard đến một topic chuyên dụng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2140"/>
        <source>Notification Topic</source>
        <translation>Topic Thông Báo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2142"/>
        <source>Topic where dashboard notifications are mirrored</source>
        <translation>Topic nơi thông báo dashboard được phản chiếu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2155"/>
        <source>Broker</source>
        <translation>Broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2165"/>
        <source>Hostname</source>
        <translation>Tên Máy Chủ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2166"/>
        <source>broker.hivemq.com</source>
        <translation>broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2167"/>
        <source>Hostname or IP address of the MQTT broker</source>
        <translation>Tên máy chủ hoặc địa chỉ IP của broker MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2176"/>
        <source>Port</source>
        <translation>Cổng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2177"/>
        <source>TCP port exposed by the broker (1883 plain, 8883 TLS)</source>
        <translation>Cổng TCP do broker cung cấp (1883 thường, 8883 TLS)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2187"/>
        <source>Custom Client ID</source>
        <translation>ID Khách Tùy Chỉnh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2189"/>
        <source>Off: a fresh random id is generated on every project load. On: use the id below.</source>
        <translation>Tắt: tạo ID ngẫu nhiên mới mỗi khi tải dự án. Bật: dùng ID bên dưới.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2200"/>
        <source>Client ID</source>
        <translation>ID Khách</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2201"/>
        <source>Identifier sent to the broker on CONNECT</source>
        <translation>Mã định danh gửi đến broker khi CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2214"/>
        <source>Protocol Version</source>
        <translation>Phiên Bản Giao Thức</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2215"/>
        <source>MQTT protocol revision used on CONNECT</source>
        <translation>Phiên bản giao thức MQTT sử dụng khi CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2224"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2225"/>
        <source>Seconds between PINGREQ packets when idle</source>
        <translation>Số giây giữa các gói PINGREQ khi nhàn rỗi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2234"/>
        <source>Clean Session</source>
        <translation>Clean Session</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2235"/>
        <source>Discard any persistent session state on CONNECT</source>
        <translation>Loại bỏ mọi trạng thái phiên lưu trữ khi CONNECT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2250"/>
        <source>Username</source>
        <translation>Tên Người Dùng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2251"/>
        <source>Username for broker authentication (leave empty for anonymous)</source>
        <translation>Tên người dùng để xác thực broker (để trống cho chế độ ẩn danh)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2261"/>
        <source>Password</source>
        <translation>Mật Khẩu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2262"/>
        <source>Password for broker authentication</source>
        <translation>Mật khẩu để xác thực broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2273"/>
        <source>SSL / TLS</source>
        <translation>SSL / TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2283"/>
        <source>Use SSL/TLS</source>
        <translation>Sử Dụng SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2284"/>
        <source>Tunnel the broker connection over TLS</source>
        <translation>Đường hầm kết nối broker qua TLS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2297"/>
        <source>Protocol</source>
        <translation>Giao Thức</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2298"/>
        <source>Negotiated TLS protocol family</source>
        <translation>Họ giao thức TLS được thương lượng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2308"/>
        <source>Peer Verify</source>
        <translation>Xác Minh Peer</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2309"/>
        <source>How strictly the broker's certificate chain is validated</source>
        <translation>Mức độ nghiêm ngặt xác thực chuỗi chứng chỉ của broker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2319"/>
        <source>Verify Depth</source>
        <translation>Độ Sâu Xác Minh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2320"/>
        <source>Maximum certificate chain length accepted (0 = unlimited)</source>
        <translation>Độ dài chuỗi chứng chỉ tối đa được chấp nhận (0 = không giới hạn)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2550"/>
        <source>Project Information</source>
        <translation>Thông Tin Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2560"/>
        <source>Project Title</source>
        <translation>Tiêu Đề Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2561"/>
        <source>Untitled Project</source>
        <translation>Dự Án Chưa Đặt Tên</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2562"/>
        <source>Name or description of the project</source>
        <translation>Tên hoặc mô tả của dự án</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2580"/>
        <source>Group Information</source>
        <translation>Thông Tin Nhóm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2590"/>
        <source>Group Title</source>
        <translation>Tiêu Đề Nhóm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2591"/>
        <source>Untitled Group</source>
        <translation>Nhóm Chưa Đặt Tên</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2592"/>
        <source>Title or description of this dataset group</source>
        <translation>Tiêu đề hoặc mô tả của nhóm dataset này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2607"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3241"/>
        <source>Device %1</source>
        <translation>Thiết Bị %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2625"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2790"/>
        <source>Input Device</source>
        <translation>Thiết Bị Đầu Vào</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2626"/>
        <source>Select which connected device provides data for this group</source>
        <translation>Chọn thiết bị đã kết nối cung cấp dữ liệu cho nhóm này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2642"/>
        <source>Image Configuration</source>
        <translation>Cấu Hình Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2655"/>
        <source>Detection Mode</source>
        <translation>Chế Độ Phát Hiện</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2657"/>
        <source>Auto-detect reads JPEG/PNG magic bytes; Manual uses explicit start/end sequences</source>
        <translation>Tự động phát hiện đọc byte magic JPEG/PNG; Thủ công sử dụng chuỗi bắt đầu/kết thúc rõ ràng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2667"/>
        <source>Start Sequence (Hex)</source>
        <translation>Chuỗi Bắt Đầu (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2668"/>
        <source>e.g. FF D8 FF</source>
        <translation>ví dụ: FF D8 FF</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2669"/>
        <source>Hex bytes marking the start of an image frame</source>
        <translation>Byte hex đánh dấu điểm bắt đầu của khung hình ảnh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2678"/>
        <source>End Sequence (Hex)</source>
        <translation>Chuỗi Kết Thúc (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2679"/>
        <source>e.g. FF D9</source>
        <translation>ví dụ: FF D9</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2680"/>
        <source>Hex bytes marking the end of an image frame</source>
        <translation>Byte hex đánh dấu điểm kết thúc của khung hình ảnh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2693"/>
        <source>Time</source>
        <translation>Thời Gian</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2693"/>
        <source>Samples</source>
        <translation>Mẫu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2705"/>
        <source>Plot every curve against time or against the sample number</source>
        <translation>Vẽ đồ thị mỗi đường cong theo thời gian hoặc theo số mẫu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2721"/>
        <source>URL</source>
        <translation>URL</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2723"/>
        <source>Web address to load in this widget</source>
        <translation>Địa chỉ web để tải trong widget này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2763"/>
        <source>Composite Widget</source>
        <translation>Widget Tổng Hợp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2764"/>
        <source>Select how this group of datasets should be visualized (optional)</source>
        <translation>Chọn cách nhóm tập dữ liệu này sẽ được hiển thị trực quan (tùy chọn)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2800"/>
        <source>Device Name</source>
        <translation>Tên Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2801"/>
        <source>Device 1</source>
        <translation>Thiết Bị 1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2802"/>
        <source>Human-readable name for this input device</source>
        <translation>Tên dễ đọc cho thiết bị đầu vào này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2811"/>
        <source>Bus Type</source>
        <translation>Loại Bus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2812"/>
        <source>Select the hardware interface for this input device</source>
        <translation>Chọn giao diện phần cứng cho thiết bị đầu vào này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Serial Port</source>
        <translation>Cổng Nối Tiếp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Network Socket</source>
        <translation>Socket Mạng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2814"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Audio Input</source>
        <translation>Đầu Vào Âm Thanh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>CAN Bus</source>
        <translation>Magistrala CAN</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2816"/>
        <source>Raw USB</source>
        <translation>USB Thô</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>HID Device</source>
        <translation>Thiết Bị HID</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>Process</source>
        <translation>Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2817"/>
        <source>MQTT Subscriber</source>
        <translation>Người Đăng Ký MQTT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2831"/>
        <source>Frame Detection</source>
        <translation>Phát Hiện Khung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2845"/>
        <source>Frame Detection Method</source>
        <translation>Phương Thức Phát Hiện Khung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2846"/>
        <source>Select how incoming data frames are identified</source>
        <translation>Chọn cách xác định các khung dữ liệu đến</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2856"/>
        <source>Hexadecimal Delimiters</source>
        <translation>Dấu Phân Cách Thập Lục Phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2857"/>
        <source>Enter frame start/end sequences as hexadecimal values</source>
        <translation>Nhập chuỗi bắt đầu/kết thúc khung dưới dạng giá trị thập lục phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2873"/>
        <source>Frame Start Delimiter</source>
        <translation>Dấu Phân Cách Bắt Đầu Khung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2874"/>
        <source>e.g. /*</source>
        <translation>ví dụ: /*</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2875"/>
        <source>Sequence that marks the beginning of a data frame</source>
        <translation>Chuỗi đánh dấu điểm bắt đầu của một khung dữ liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2887"/>
        <source>Frame End Delimiter</source>
        <translation>Dấu Phân Cách Kết Thúc Khung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2888"/>
        <source>e.g. */</source>
        <translation>ví dụ: */</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2889"/>
        <source>Sequence that marks the end of a data frame</source>
        <translation>Chuỗi đánh dấu điểm kết thúc của một khung dữ liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2895"/>
        <source>Payload Processing &amp; Validation</source>
        <translation>Xử Lý &amp; Xác Thực Payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2906"/>
        <source>Data Conversion Method</source>
        <translation>Phương Thức Chuyển Đổi Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2907"/>
        <source>Select how incoming binary data is decoded before parsing</source>
        <translation>Chọn cách giải mã dữ liệu nhị phân đến trước khi phân tích</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2923"/>
        <source>Checksum Algorithm</source>
        <translation>Thuật Toán Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2924"/>
        <source>Select the checksum algorithm used to validate frames</source>
        <translation>Chọn thuật toán checksum dùng để xác thực các frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2972"/>
        <source>Connection Settings</source>
        <translation>Cài Đặt Kết Nối</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3209"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3479"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5502"/>
        <source>General Information</source>
        <translation>Thông Tin Chung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3218"/>
        <source>Action Title</source>
        <translation>Tiêu Đề Hành Động</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3220"/>
        <source>Untitled Action</source>
        <translation>Hành Động Chưa Đặt Tên</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3221"/>
        <source>Name or description of this action</source>
        <translation>Tên hoặc mô tả của hành động này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3230"/>
        <source>Action Icon</source>
        <translation>Biểu Tượng Hành Động</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3231"/>
        <source>Default Icon</source>
        <translation>Biểu Tượng Mặc Định</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3232"/>
        <source>Icon displayed for this action in the dashboard</source>
        <translation>Biểu tượng hiển thị cho hành động này trong bảng điều khiển</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3259"/>
        <source>Target Device</source>
        <translation>Thiết Bị Đích</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3260"/>
        <source>Select which connected device this action sends data to</source>
        <translation>Chọn thiết bị đã kết nối mà hành động này gửi dữ liệu đến</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3272"/>
        <source>Data Payload</source>
        <translation>Tải Trọng Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3283"/>
        <source>Send as Binary</source>
        <translation>Gửi dưới Dạng Nhị Phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3284"/>
        <source>Send raw binary data when this action is triggered</source>
        <translation>Gửi dữ liệu nhị phân thô khi hành động này được kích hoạt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3295"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3307"/>
        <source>Command</source>
        <translation>Lệnh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3296"/>
        <source>Transmit Data (Hex)</source>
        <translation>Truyền Dữ Liệu (Hex)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3297"/>
        <source>Hexadecimal payload to send when the action is triggered</source>
        <translation>Tải trọng thập lục phân để gửi khi hành động được kích hoạt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3308"/>
        <source>Transmit Data</source>
        <translation>Truyền Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3309"/>
        <source>Text payload to send when the action is triggered</source>
        <translation>Tải trọng văn bản để gửi khi hành động được kích hoạt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3320"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5561"/>
        <source>Text Encoding</source>
        <translation>Mã Hóa Văn Bản</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3321"/>
        <source>Character encoding used to serialize the text payload</source>
        <translation>Mã hóa ký tự được sử dụng để tuần tự hóa dữ liệu văn bản</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3345"/>
        <source>End-of-Line Sequence</source>
        <translation>Chuỗi Kết Thúc Dòng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3346"/>
        <source>EOL characters to append to the message (e.g. \n, \r\n)</source>
        <translation>Ký tự EOL được thêm vào thông điệp (ví dụ: </translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3358"/>
        <source>Execution Behavior</source>
        <translation>Hành Vi Thực Thi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3369"/>
        <source>Auto-Execute on Connect</source>
        <translation>Tự Động Thực Thi Khi Kết Nối</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3370"/>
        <source>Automatically trigger this action when the device connects</source>
        <translation>Tự động kích hoạt hành động này khi thiết bị kết nối</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3376"/>
        <source>Timer Behavior</source>
        <translation>Hành Vi Bộ Đếm Thời Gian</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3385"/>
        <source>Timer Mode</source>
        <translation>Chế Độ Bộ Đếm Thời Gian</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3388"/>
        <source>Choose when and how this action should repeat automatically</source>
        <translation>Chọn thời điểm và cách thức hành động này tự động lặp lại</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3395"/>
        <source>Interval (ms)</source>
        <translation>Khoảng Thời Gian (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3399"/>
        <source>Timer Interval (ms)</source>
        <translation>Khoảng Thời Gian Bộ Đếm (ms)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3400"/>
        <source>Milliseconds between each repeated trigger of this action</source>
        <translation>Số mili giây giữa mỗi lần kích hoạt lặp lại của hành động này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3407"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3411"/>
        <source>Repeat Count</source>
        <translation>Số Lần Lặp Lại</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3412"/>
        <source>Number of times to send the command on each trigger</source>
        <translation>Số lần gửi lệnh trong mỗi lần kích hoạt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3489"/>
        <source>Untitled Dataset</source>
        <translation>Dataset Chưa Đặt Tên</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3490"/>
        <source>Dataset Title</source>
        <translation>Tiêu Đề Dataset</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3491"/>
        <source>Name of the dataset, used for labeling and identification</source>
        <translation>Tên của dataset, dùng để gắn nhãn và nhận dạng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3503"/>
        <source>Virtual Dataset</source>
        <translation>Dataset Ảo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3504"/>
        <source>Virtual datasets compute their value from transforms and data tables, they do not require a frame index</source>
        <translation>Dataset ảo tính toán giá trị từ các phép biến đổi và bảng dữ liệu, không yêu cầu chỉ số khung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3520"/>
        <source>Hide on Dashboard</source>
        <translation>Ẩn trên Dashboard</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3521"/>
        <source>Suppress this dataset's standalone dashboard tile; the painter widget can still read its values</source>
        <translation>Ẩn ô dashboard riêng của dataset này; widget vẽ vẫn có thể đọc các giá trị của nó</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3534"/>
        <source>Frame Index</source>
        <translation>Chỉ Số Frame</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3535"/>
        <source>Frame position used for aligning datasets in time</source>
        <translation>Vị trí frame dùng để căn chỉnh các dataset theo thời gian</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3544"/>
        <source>Measurement Unit</source>
        <translation>Đơn Vị Đo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3545"/>
        <source>Volts, Amps, etc.</source>
        <translation>Volt, Ampe, v.v.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3546"/>
        <source>Unit of measurement, such as volts or amps (optional)</source>
        <translation>Đơn vị đo lường, chẳng hạn như volt hoặc ampe (tùy chọn)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3567"/>
        <source>Lower bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Giới hạn dưới của phạm vi giá trị dataset; các widget và FFT sẽ dùng giá trị này khi phạm vi riêng của chúng không được thiết lập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3580"/>
        <source>Upper bound of the dataset value range; widgets and FFT fall back to it when their own range is left unset</source>
        <translation>Giới hạn trên của phạm vi giá trị dataset; các widget và FFT sẽ dùng giá trị này khi phạm vi riêng của chúng không được thiết lập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3593"/>
        <source>Plot Settings</source>
        <translation>Cài Đặt Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3616"/>
        <source>Enable Plot Widget</source>
        <translation>Bật Widget Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3618"/>
        <source>Plot data in real-time</source>
        <translation>Vẽ dữ liệu theo thời gian thực</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3907"/>
        <source>Fixed decimal places for the value display; overrides the format (-1 = auto)</source>
        <translation>Số chữ số thập phân cố định cho hiển thị giá trị; ghi đè định dạng (-1 = tự động)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2704"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3637"/>
        <source>X-Axis Source</source>
        <translation>Nguồn Trục X</translation>
    </message>
    <message>
        <source>Choose which dataset to use for the X-Axis in plots</source>
        <translation type="vanished">Chọn tập dữ liệu để sử dụng cho trục X trong biểu đồ</translation>
    </message>
    <message>
        <source>Minimum Plot Value (optional)</source>
        <translation type="vanished">Giá Trị Biểu Đồ Tối Thiểu (tùy chọn)</translation>
    </message>
    <message>
        <source>Lower bound for plot display range</source>
        <translation type="vanished">Giới hạn dưới cho phạm vi hiển thị biểu đồ</translation>
    </message>
    <message>
        <source>Maximum Plot Value (optional)</source>
        <translation type="vanished">Giá Trị Biểu Đồ Tối Đa (tùy chọn)</translation>
    </message>
    <message>
        <source>Upper bound for plot display range</source>
        <translation type="vanished">Giới hạn trên cho phạm vi hiển thị biểu đồ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3651"/>
        <source>Frequency Analysis</source>
        <translation>Phân Tích Tần Số</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3662"/>
        <source>Enable FFT Analysis</source>
        <translation>Bật Phân Tích FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3663"/>
        <source>Perform frequency-domain analysis of the dataset</source>
        <translation>Thực hiện phân tích miền tần số của tập dữ liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3673"/>
        <source>Enable Waterfall Plot</source>
        <translation>Bật Biểu Đồ Waterfall</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3674"/>
        <source>Show a scrolling spectrogram of frequency content over time (Pro)</source>
        <translation>Hiển thị phổ tần số cuộn theo thời gian (Pro)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3697"/>
        <source>Waterfall Y Axis</source>
        <translation>Trục Y Thác Nước</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3698"/>
        <source>Choose Time (default) or any dataset whose value drives the Y axis -- produces a Campbell diagram when bound to e.g. RPM</source>
        <translation>Chọn Thời Gian (mặc định) hoặc bất kỳ tập dữ liệu nào có giá trị điều khiển trục Y -- tạo biểu đồ Campbell khi liên kết với RPM chẳng hạn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3725"/>
        <source>FFT Window Size</source>
        <translation>Kích Thước Cửa Sổ FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3726"/>
        <source>Number of samples used for each FFT calculation window</source>
        <translation>Số lượng mẫu sử dụng cho mỗi cửa sổ tính toán FFT</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3737"/>
        <source>FFT Sampling Rate (Hz, required)</source>
        <translation>Tần Số Lấy Mẫu FFT (Hz, bắt buộc)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3738"/>
        <source>Sampling frequency used for FFT (in Hz)</source>
        <translation>Tần số lấy mẫu sử dụng cho FFT (tính bằng Hz)</translation>
    </message>
    <message>
        <source>Minimum Value (recommended)</source>
        <translation type="vanished">Giá Trị Tối Thiểu (khuyến nghị)</translation>
    </message>
    <message>
        <source>Lower bound for data normalization</source>
        <translation type="vanished">Giới hạn dưới cho chuẩn hóa dữ liệu</translation>
    </message>
    <message>
        <source>Maximum Value (recommended)</source>
        <translation type="vanished">Giá Trị Tối Đa (khuyến nghị)</translation>
    </message>
    <message>
        <source>Upper bound for data normalization</source>
        <translation type="vanished">Giới hạn trên cho chuẩn hóa dữ liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3787"/>
        <source>Widget Settings</source>
        <translation>Cài Đặt Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3810"/>
        <source>Widget</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3811"/>
        <source>Select the visual widget used to display this dataset</source>
        <translation>Chọn widget trực quan dùng để hiển thị tập dữ liệu này</translation>
    </message>
    <message>
        <source>Minimum Display Value (required)</source>
        <translation type="vanished">Giá Trị Hiển Thị Tối Thiểu (bắt buộc)</translation>
    </message>
    <message>
        <source>Lower bound of the gauge or bar display range</source>
        <translation type="vanished">Giới hạn dưới của phạm vi hiển thị đồng hồ đo hoặc thanh</translation>
    </message>
    <message>
        <source>Maximum Display Value (required)</source>
        <translation type="vanished">Giá Trị Hiển Thị Tối Đa (bắt buộc)</translation>
    </message>
    <message>
        <source>Upper bound of the gauge or bar display range</source>
        <translation type="vanished">Giới hạn trên của phạm vi hiển thị đồng hồ đo hoặc thanh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3867"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3902"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4063"/>
        <source>Auto</source>
        <translation>Tự Động</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3868"/>
        <source>Tick Count</source>
        <translation>Số Lượng Vạch Chia</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3872"/>
        <source>Major-tick count on the dial scale (0 = auto-fit to widget size)</source>
        <translation>Số lượng vạch chia chính trên thang đo (0 = tự động điều chỉnh theo kích thước widget)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3891"/>
        <source>Label Format</source>
        <translation>Định Dạng Nhãn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3892"/>
        <source>Decimal places or notation used on tick labels and the value display</source>
        <translation>Số chữ số thập phân hoặc ký hiệu được sử dụng trên nhãn vạch chia và hiển thị giá trị</translation>
    </message>
    <message>
        <source>Show Value Indicator</source>
        <translation type="vanished">Hiển Thị Chỉ Báo Giá Trị</translation>
    </message>
    <message>
        <source>Toggle the boxed numeric readout that sits below or beside the widget</source>
        <translation type="vanished">Bật/tắt khung hiển thị số nằm bên dưới hoặc bên cạnh widget</translation>
    </message>
    <message>
        <source>Alarm Settings</source>
        <translation type="vanished">Cài Đặt Cảnh Báo</translation>
    </message>
    <message>
        <source>Enable Alarms</source>
        <translation type="vanished">Bật Cảnh Báo</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds alarm thresholds</source>
        <translation type="vanished">Kích hoạt cảnh báo trực quan khi giá trị vượt ngưỡng cảnh báo</translation>
    </message>
    <message>
        <source>Low Threshold</source>
        <translation type="vanished">Ngưỡng Thấp</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value drops below this threshold</source>
        <translation type="vanished">Kích hoạt cảnh báo trực quan khi giá trị giảm xuống dưới ngưỡng này</translation>
    </message>
    <message>
        <source>High Threshold</source>
        <translation type="vanished">Ngưỡng Cao</translation>
    </message>
    <message>
        <source>Triggers a visual alarm when the value exceeds this threshold</source>
        <translation type="vanished">Kích hoạt cảnh báo trực quan khi giá trị vượt quá ngưỡng này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3967"/>
        <source>LED Display Settings</source>
        <translation>Cài Đặt Hiển Thị LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3978"/>
        <source>Show in LED Panel</source>
        <translation>Hiển Thị trong Bảng LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3979"/>
        <source>Enable visual status monitoring using an LED display</source>
        <translation>Bật giám sát trạng thái trực quan bằng màn hình LED</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3993"/>
        <source>LED On Threshold (required)</source>
        <translation>Ngưỡng Bật LED (bắt buộc)</translation>
    </message>
    <message>
        <source>LED lights up when value meets or exceeds this threshold</source>
        <translation type="vanished">LED sáng lên khi giá trị đạt hoặc vượt ngưỡng này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Off</source>
        <translation>Tắt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Auto Start</source>
        <translation>Tự Động Bắt Đầu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Start on Trigger</source>
        <translation>Bắt Đầu Khi Có Tín Hiệu Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4014"/>
        <source>Toggle on Trigger</source>
        <translation>Chuyển Đổi Khi Có Tín Hiệu Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4015"/>
        <source>Repeat N Times</source>
        <translation>Lặp Lại N Lần</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Plain Text (UTF8)</source>
        <translation>Văn Bản Thuần (UTF8)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Hexadecimal</source>
        <translation>Thập Lục Phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4018"/>
        <source>Base64</source>
        <translation>Base64</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4019"/>
        <source>Binary (Direct)</source>
        <translation>Nhị Phân (Trực Tiếp)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4024"/>
        <source>No Checksum</source>
        <translation>Không Có Checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4028"/>
        <source>End Delimiter Only</source>
        <translation>Chỉ Ký Tự Kết Thúc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4028"/>
        <source>Start Delimiter Only</source>
        <translation>Chỉ Dấu Phân Cách Đầu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4029"/>
        <source>Start + End Delimiter</source>
        <translation>Dấu Phân Cách Đầu + Cuối</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4029"/>
        <source>No Delimiters</source>
        <translation>Không Có Dấu Phân Cách</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4036"/>
        <source>Auto-detect</source>
        <translation>Tự Động Phát Hiện</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4036"/>
        <source>Manual Delimiters</source>
        <translation>Dấu Phân Cách Thủ Công</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Button</source>
        <translation>Nút</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Slider</source>
        <translation>Thanh Trượt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Toggle</source>
        <translation>Công Tắc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4039"/>
        <source>Text Field</source>
        <translation>Trường Văn Bản</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4040"/>
        <source>Knob</source>
        <translation>Núm Xoay</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4044"/>
        <source>Data Grid</source>
        <translation>Lưới Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4045"/>
        <source>GPS Map</source>
        <translation>Bản Đồ GPS</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4046"/>
        <source>Gyroscope</source>
        <translation>Con Quay Hồi Chuyển</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4047"/>
        <source>Multiple Plot</source>
        <translation>Đồ Thị Đa Trục</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4048"/>
        <source>Accelerometer</source>
        <translation>Gia Tốc Kế</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4049"/>
        <source>3D Plot</source>
        <translation>Đồ Thị 3D</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4050"/>
        <source>Image View</source>
        <translation>Khung Xem Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4051"/>
        <source>Painter Widget</source>
        <translation>Widget Vẽ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4052"/>
        <source>Web View</source>
        <translation>Khung Xem Web</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4053"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4056"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4071"/>
        <source>None</source>
        <translation>Không</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4057"/>
        <source>Bar</source>
        <translation>Thanh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4058"/>
        <source>Gauge</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4059"/>
        <source>Compass</source>
        <translation>La Bàn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4060"/>
        <source>Meter</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Nhiệt Kế</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4064"/>
        <source>Integer (0 decimals)</source>
        <translation>Số Nguyên (0 chữ số thập phân)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4065"/>
        <source>1 decimal</source>
        <translation>1 số thập phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4066"/>
        <source>2 decimals</source>
        <translation>2 số thập phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4067"/>
        <source>3 decimals</source>
        <translation>3 số thập phân</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4068"/>
        <source>Scientific</source>
        <translation>Khoa Học</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4072"/>
        <source>New Line (\n)</source>
        <translation>Dòng Mới (</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4073"/>
        <source>Carriage Return (\r)</source>
        <translation>Về Đầu Dòng (\r)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4074"/>
        <source>CRLF (\r\n)</source>
        <translation>CRLF (\r</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4077"/>
        <source>No</source>
        <translation>Không</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4078"/>
        <source>Yes</source>
        <translation>Có</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4908"/>
        <source>(multiple)</source>
        <translation>(nhiều)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="4922"/>
        <source>Mixed</source>
        <translation>Hỗn Hợp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5512"/>
        <source>Label</source>
        <translation>Nhãn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5513"/>
        <source>Display label</source>
        <translation>Hiển thị nhãn</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5523"/>
        <source>Button Icon</source>
        <translation>Biểu Tượng Nút</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5532"/>
        <source>Colorize Icon</source>
        <translation>Tô Màu Biểu Tượng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5533"/>
        <source>Tint the icon with the button color</source>
        <translation>Nhuộm biểu tượng với màu của nút</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5550"/>
        <source>Initial Value</source>
        <translation>Giá Trị Khởi Tạo</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5562"/>
        <source>Character encoding used when transmit() returns a string value</source>
        <translation>Bảng mã ký tự được sử dụng khi transmit() trả về giá trị chuỗi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5580"/>
        <source>Value Range</source>
        <translation>Phạm Vi Giá Trị</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3566"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5590"/>
        <source>Minimum Value</source>
        <translation>Giá Trị Tối Thiểu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1638"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="1639"/>
        <source>Dashboard Widgets</source>
        <translation>Widget Bảng Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2005"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2008"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="2009"/>
        <source>Control Loop</source>
        <translation>Vòng Lặp Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3579"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5599"/>
        <source>Maximum Value</source>
        <translation>Giá Trị Tối Đa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3638"/>
        <source>Choose Time or a dataset to drive the X-Axis in plots</source>
        <translation>Chọn Thời Gian hoặc một dataset để điều khiển trục X trong các biểu đồ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3748"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3837"/>
        <source>Minimum Value (optional)</source>
        <translation>Giá Trị Tối Thiểu (tùy chọn)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3749"/>
        <source>Lower bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Giới hạn dưới cho chuẩn hóa dữ liệu; sử dụng phạm vi giá trị của tập dữ liệu khi không được thiết lập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3761"/>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3850"/>
        <source>Maximum Value (optional)</source>
        <translation>Giá Trị Tối Đa (tùy chọn)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3762"/>
        <source>Upper bound for data normalization; falls back to the dataset value range when left unset</source>
        <translation>Giới hạn trên cho chuẩn hóa dữ liệu; sử dụng phạm vi giá trị của tập dữ liệu khi không được thiết lập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3838"/>
        <source>Lower bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Giới hạn dưới của phạm vi đồng hồ đo hoặc thanh; sử dụng phạm vi giá trị của tập dữ liệu khi không được thiết lập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3851"/>
        <source>Upper bound of the gauge or bar range; falls back to the dataset value range when left unset</source>
        <translation>Giới hạn trên của phạm vi đồng hồ đo hoặc thanh; sử dụng phạm vi giá trị của tập dữ liệu khi không được thiết lập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3903"/>
        <source>Decimal Points</source>
        <translation>Số Chữ Số Thập Phân</translation>
    </message>
    <message>
        <source>Decimal places shown in the data grid value column (-1 = auto)</source>
        <translation type="vanished">Số chữ số thập phân hiển thị trong cột giá trị của lưới dữ liệu (-1 = tự động)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3950"/>
        <source>On</source>
        <translation>Bật</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="3994"/>
        <source>LED lights up when value meets or exceeds this threshold; define alarm bands for multi-state colors</source>
        <translation>LED sáng lên khi giá trị đạt hoặc vượt ngưỡng này; định nghĩa các dải cảnh báo cho màu sắc đa trạng thái</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5608"/>
        <source>Step Size</source>
        <translation>Kích Thước Bước</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5836"/>
        <source>Raw and transformed values for every dataset (read-only)</source>
        <translation>Giá trị thô và đã biến đổi cho mọi dataset (chỉ đọc)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="5845"/>
        <source>Shared table defined in this project</source>
        <translation>Bảng dùng chung được định nghĩa trong dự án này</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6468"/>
        <source>Remove 1 widget reference whose target group or dataset no longer exists?</source>
        <translation>Xóa 1 tham chiếu widget có nhóm hoặc tập dữ liệu đích không còn tồn tại?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6469"/>
        <source>Remove %1 widget references whose target groups or datasets no longer exist?</source>
        <translation>Xóa %1 tham chiếu widget có nhóm hoặc tập dữ liệu đích không còn tồn tại?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6474"/>
        <source>This will only affect workspace tile placement; no groups, datasets, or data are deleted.</source>
        <translation>Thao tác này chỉ ảnh hưởng đến vị trí ô trong không gian làm việc; không có nhóm, tập dữ liệu hoặc dữ liệu nào bị xóa.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectEditor.cpp" line="6477"/>
        <source>Clean Up Workspaces</source>
        <translation>Dọn Dẹp Không Gian Làm Việc</translation>
    </message>
</context>
<context>
    <name>DataModel::ProjectModel</name>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="516"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="538"/>
        <source>Project error</source>
        <translation>Lỗi dự án</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="516"/>
        <source>Project title cannot be empty!</source>
        <translation>Tiêu đề dự án không được để trống!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="525"/>
        <source>You need to add at least one group!</source>
        <translation>Bạn cần thêm ít nhất một nhóm!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="538"/>
        <source>You need to add at least one dataset!</source>
        <translation>Bạn cần thêm ít nhất một tập dữ liệu!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="585"/>
        <source>Your project needs a title</source>
        <translation>Dự án của bạn cần có tiêu đề</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="587"/>
        <source>Add a group to get started</source>
        <translation>Thêm một nhóm để bắt đầu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="589"/>
        <source>Add a dataset to a group</source>
        <translation>Thêm tập dữ liệu vào nhóm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="603"/>
        <source>Open the Project view at the top of the tree and enter a name. You can rename the project at any time.</source>
        <translation>Mở chế độ xem Dự án ở đầu cây và nhập tên. Bạn có thể đổi tên dự án bất cứ lúc nào.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="606"/>
        <source>Groups organize datasets into dashboard widgets. Use the Group button in the toolbar above to create one, then add datasets to it.</source>
        <translation>Nhóm tổ chức các tập dữ liệu thành widget bảng điều khiển. Sử dụng nút Nhóm trên thanh công cụ để tạo nhóm, sau đó thêm tập dữ liệu vào đó.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="610"/>
        <source>Datasets are the values that appear on the dashboard. Select a group in the tree and use the Dataset button in the toolbar to add one.</source>
        <translation>Tập dữ liệu là các giá trị xuất hiện trên bảng điều khiển. Chọn một nhóm trong cây và sử dụng nút Tập dữ liệu trên thanh công cụ để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="644"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="653"/>
        <source>Lock Project</source>
        <translation>Khóa Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="645"/>
        <source>Choose a password to lock the project:</source>
        <translation>Chọn mật khẩu để khóa dự án:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="653"/>
        <source>Confirm the password:</source>
        <translation>Xác nhận mật khẩu:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="658"/>
        <source>Passwords do not match</source>
        <translation>Mật khẩu không khớp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="659"/>
        <source>The two passwords you entered do not match. The project was not locked.</source>
        <translation>Hai mật khẩu bạn nhập không khớp. Dự án chưa được khóa.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="693"/>
        <source>Unlock Project</source>
        <translation>Mở Khóa Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="694"/>
        <source>Enter the project password:</source>
        <translation>Nhập mật khẩu dự án:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="704"/>
        <source>Incorrect password</source>
        <translation>Mật khẩu không đúng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="705"/>
        <source>The password you entered does not match the one stored in the project file.</source>
        <translation>Mật khẩu bạn nhập không khớp với mật khẩu được lưu trong tệp dự án.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="736"/>
        <source>New Project</source>
        <translation>Dự Án Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="936"/>
        <source>Samples</source>
        <translation>Mẫu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="935"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="984"/>
        <source>Time</source>
        <translation>Thời Gian</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1523"/>
        <source>Multiple data sources require a Pro license</source>
        <translation>Nhiều nguồn dữ liệu yêu cầu giấy phép Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1524"/>
        <source>Serial Studio Pro allows connecting to multiple devices simultaneously. Please upgrade to unlock this feature.</source>
        <translation>Serial Studio Pro cho phép kết nối đồng thời với nhiều thiết bị. Vui lòng nâng cấp để mở khóa tính năng này.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1536"/>
        <source>Device %1</source>
        <translation>Thiết Bị %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1568"/>
        <source>Do you want to delete data source "%1"?</source>
        <translation>Bạn có muốn xóa nguồn dữ liệu "%1" không?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1569"/>
        <source>Groups using this source will move to the default source. This action cannot be undone.</source>
        <translation>Các nhóm sử dụng nguồn này sẽ chuyển về nguồn mặc định. Hành động này không thể hoàn tác.</translation>
    </message>
    <message>
        <source> (Copy)</source>
        <translation type="vanished">(Bản Sao)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1806"/>
        <source>Do you want to save your changes?</source>
        <translation>Lưu các thay đổi?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1807"/>
        <source>You have unsaved modifications in this project!</source>
        <translation>Dự án có các thay đổi chưa được lưu!</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1846"/>
        <source>Save Serial Studio Project</source>
        <translation>Lưu Dự Án Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1848"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2512"/>
        <source>Serial Studio Project Files (*.ssproj)</source>
        <translation>Tệp Dự Án Serial Studio (*.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="1869"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2110"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2503"/>
        <source>Untitled Project</source>
        <translation>Dự Án Chưa Đặt Tên</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2125"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2658"/>
        <source>Device A</source>
        <translation>Thiết Bị A</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2337"/>
        <source>Select Project File</source>
        <translation>Chọn Tệp Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2339"/>
        <source>Project Files (*.json *.ssproj)</source>
        <translation>Tệp Dự Án (*.json *.ssproj)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2383"/>
        <source>JSON validation error</source>
        <translation>Lỗi xác thực JSON</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2477"/>
        <source>Project upgraded from an earlier file format</source>
        <translation>Dự Án Đã Được Nâng Cấp Từ Định Dạng Tập Tin Cũ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2478"/>
        <source>This project was saved with schema version %1; the current version is %2. Defaults have been applied to any new fields. Save the project to lock in the upgrade.</source>
        <translation>Dự án này được lưu với phiên bản schema %1; phiên bản hiện tại là %2. Các trường mới đã được áp dụng giá trị mặc định. Lưu dự án để hoàn tất nâng cấp.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2510"/>
        <source>Save Imported Project</source>
        <translation>Lưu Dự Án Đã Nhập</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2702"/>
        <source>Multi-source projects require a Pro license</source>
        <translation>Dự án đa nguồn yêu cầu giấy phép Pro</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2703"/>
        <source>This project contains multiple data sources. Only the first source has been loaded. A Serial Studio Pro license is required to use multi-source projects.</source>
        <translation>Dự án này chứa nhiều nguồn dữ liệu. Chỉ nguồn đầu tiên được tải. Cần có giấy phép Serial Studio Pro để sử dụng dự án đa nguồn.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2941"/>
        <source>Workspace IDs remapped on load</source>
        <translation>ID Không Gian Làm Việc Được Ánh Xạ Lại Khi Tải</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2942"/>
        <source>%1 custom workspace ID(s) overlapped the new reserved auto range and were moved into the user range. Save the project to make the remap permanent.</source>
        <translation>%1 ID không gian làm việc tùy chỉnh bị trùng với dải tự động mới đã được chuyển sang dải người dùng. Lưu dự án để áp dụng ánh xạ vĩnh viễn.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3125"/>
        <source>Legacy frame parser function updated</source>
        <translation>Hàm phân tích frame cũ đã được cập nhật</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3126"/>
        <source>Your project used a legacy frame parser function with a 'separator' argument. It has been automatically migrated to the new format.</source>
        <translation>Dự án của bạn đã sử dụng hàm phân tích frame cũ với tham số 'separator'. Nó đã được tự động chuyển đổi sang định dạng mới.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3370"/>
        <source>Do you want to delete group "%1"?</source>
        <translation>Bạn có muốn xóa nhóm "%1" không?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3371"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3416"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3448"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4186"/>
        <source>This action cannot be undone. Do you wish to proceed?</source>
        <translation>Hành động này không thể hoàn tác. Bạn có muốn tiếp tục không?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3415"/>
        <source>Do you want to delete action "%1"?</source>
        <translation>Bạn có muốn xóa hành động "%1" không?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="3447"/>
        <source>Do you want to delete dataset "%1"?</source>
        <translation>Bạn có muốn xóa dataset "%1" không?</translation>
    </message>
    <message>
        <source>%1 (Copy)</source>
        <translation type="vanished">%1 (Bản Sao)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4098"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4134"/>
        <source>Output Controls</source>
        <translation>Điều Khiển Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4146"/>
        <source>New Button</source>
        <translation>Nút Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4149"/>
        <source>New Slider</source>
        <translation>Thanh Trượt Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4152"/>
        <source>New Toggle</source>
        <translation>Công Tắc Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4155"/>
        <source>New Text Field</source>
        <translation>Trường Văn Bản Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4158"/>
        <source>New Knob</source>
        <translation>Núm Xoay Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4185"/>
        <source>Do you want to delete output widget "%1"?</source>
        <translation>Bạn có muốn xóa widget đầu ra "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4350"/>
        <source>Group</source>
        <translation>Nhóm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4368"/>
        <source>New Dataset</source>
        <translation>Dataset Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4371"/>
        <source>New Plot</source>
        <translation>Biểu Đồ Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4375"/>
        <source>New FFT Plot</source>
        <translation>Biểu Đồ FFT Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4379"/>
        <source>New Level Indicator</source>
        <translation>Chỉ Báo Mức Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4383"/>
        <source>New Gauge</source>
        <translation>Đồng Hồ Đo Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4387"/>
        <source>New Compass</source>
        <translation>La Bàn Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4393"/>
        <source>New Meter</source>
        <translation>Đồng Hồ Đo Mới</translation>
    </message>
    <message>
        <source>New Thermometer</source>
        <translation type="vanished">Nhiệt Kế Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4397"/>
        <source>New LED Indicator</source>
        <translation>Chỉ Báo LED Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4401"/>
        <source>New Waterfall</source>
        <translation>Thác Nước Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4469"/>
        <source>Channel %1</source>
        <translation>Kênh %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4540"/>
        <source>New Action</source>
        <translation>Hành Động Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4681"/>
        <source>Are you sure you want to change the group-level widget?</source>
        <translation>Xác nhận thay đổi widget cấp nhóm?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4683"/>
        <source>Existing datasets for this group are deleted</source>
        <translation>Các tập dữ liệu hiện có của nhóm này sẽ bị xóa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4751"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4752"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4753"/>
        <source>Accelerometer %1</source>
        <translation>Gia Tốc Kế %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4768"/>
        <source>Gyro %1</source>
        <translation>Con Quay Hồi Chuyển %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Latitude</source>
        <translation>Vĩ Độ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Longitude</source>
        <translation>Kinh Độ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="4783"/>
        <source>Altitude</source>
        <translation>Độ Cao</translation>
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
        <translation>Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5259"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5465"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6727"/>
        <source>Shared Table</source>
        <translation>Bảng Dùng Chung</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5336"/>
        <source>register</source>
        <translation>thanh ghi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5465"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6727"/>
        <source>New Shared Table</source>
        <translation>Bảng Chia Sẻ Mới</translation>
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
        <translation>Tên:</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5488"/>
        <source>Rename Table</source>
        <translation>Đổi Tên Bảng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5507"/>
        <source>Rename Group</source>
        <translation>Đổi Tên Nhóm</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5531"/>
        <source>Rename Dataset</source>
        <translation>Đổi Tên Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5558"/>
        <source>Rename Data Source</source>
        <translation>Đổi Tên Nguồn Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5577"/>
        <source>Rename Action</source>
        <translation>Đổi Tên Hành Động</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5598"/>
        <source>New Register</source>
        <translation>Thanh Ghi Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5621"/>
        <source>Rename Register</source>
        <translation>Đổi Tên Thanh Ghi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5655"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5680"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7389"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8173"/>
        <source>This action cannot be undone.</source>
        <translation>Hành động này không thể hoàn tác.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5656"/>
        <source>This removes %1 register(s) along with the table. This action cannot be undone.</source>
        <translation>Thao tác này sẽ xóa %1 thanh ghi cùng với bảng. Hành động này không thể hoàn tác.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5659"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5679"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7388"/>
        <source>Delete "%1"?</source>
        <translation>Xóa "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5662"/>
        <source>Delete Table</source>
        <translation>Xóa Bảng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5682"/>
        <source>Delete Register</source>
        <translation>Xóa Thanh Ghi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5704"/>
        <source>Export Table</source>
        <translation>Xuất Bảng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5706"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5747"/>
        <source>CSV files (*.csv)</source>
        <translation>Tệp CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5745"/>
        <source>Import Table</source>
        <translation>Nhập Bảng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="5996"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6268"/>
        <source>New Workspace</source>
        <translation>Không Gian Làm Việc Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6017"/>
        <source>Rename Workspace</source>
        <translation>Đổi Tên Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6063"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6251"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6351"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6485"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6569"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6710"/>
        <source>Folder</source>
        <translation>Thư Mục</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6251"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6485"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6710"/>
        <source>New Folder</source>
        <translation>Thư Mục Mới</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6290"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6506"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6748"/>
        <source>Rename Folder</source>
        <translation>Đổi Tên Thư Mục</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6308"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6524"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6766"/>
        <source>Delete folder "%1"?</source>
        <translation>Xóa thư mục "%1"?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6309"/>
        <source>The folder is removed; its workspaces and sub-folders move up to the parent.</source>
        <translation>Thư mục bị xóa; các không gian làm việc và thư mục con sẽ chuyển lên thư mục cha.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6311"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6527"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6770"/>
        <source>Delete Folder</source>
        <translation>Xóa Thư Mục</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6525"/>
        <source>The folder is removed; its groups and sub-folders move up to the parent.</source>
        <translation>Thư mục bị xóa; các nhóm và thư mục con sẽ chuyển lên thư mục cha.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6767"/>
        <source>The folder is removed; its tables and sub-folders move up to the parent. The accessor path of those tables changes accordingly.</source>
        <translation>Thư mục bị xóa; các bảng và thư mục con sẽ chuyển lên thư mục cha. Đường dẫn truy cập của các bảng đó sẽ thay đổi tương ứng.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6855"/>
        <source>Overview</source>
        <translation>Tổng Quan</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="6864"/>
        <source>All Data</source>
        <translation>Tất Cả Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7157"/>
        <source>Discard workspace customisations?</source>
        <translation>Hủy bỏ tùy chỉnh không gian làm việc?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7158"/>
        <source>Switching off Customize discards your edits and rebuilds the workspace list from the project's groups.</source>
        <translation>Tắt Tùy chỉnh sẽ hủy các chỉnh sửa của bạn và xây dựng lại danh sách không gian làm việc từ các nhóm của dự án.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7161"/>
        <source>Customize Workspaces</source>
        <translation>Tùy Chỉnh Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7391"/>
        <source>Delete Workspace</source>
        <translation>Xóa Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7787"/>
        <source>Project file removed from disk</source>
        <translation>Tệp dự án đã bị xóa khỏi đĩa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7788"/>
        <source>%1 was deleted or renamed by another program. Save the project to recreate it.</source>
        <translation>%1 đã bị xóa hoặc đổi tên bởi chương trình khác. Lưu dự án để tạo lại tệp.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7809"/>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7830"/>
        <source>Project file changed on disk</source>
        <translation>Tệp dự án đã thay đổi trên đĩa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7810"/>
        <source>%1 was modified by another program. The in-memory project was kept; reopen the file to load the external changes.</source>
        <translation>%1 đã bị sửa đổi bởi chương trình khác. Dự án trong bộ nhớ được giữ nguyên; mở lại tệp để tải các thay đổi bên ngoài.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7826"/>
        <source>The project file was modified by another program.

Reload it and discard your unsaved changes?</source>
        <translation>Tệp dự án đã bị sửa đổi bởi chương trình khác.

Tải lại và hủy các thay đổi chưa lưu?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7828"/>
        <source>The project file was modified by another program.

Reload it?</source>
        <translation>Tệp dự án đã bị sửa đổi bởi chương trình khác.

Tải lại tệp?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="7859"/>
        <source>File save error</source>
        <translation>Lỗi lưu tệp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8172"/>
        <source>Delete %1 selected items?</source>
        <translation>Xóa %1 mục đã chọn?</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="8175"/>
        <source>Delete Items</source>
        <translation>Xóa Mục</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/ProjectModel.cpp" line="2546"/>
        <source>File open error</source>
        <translation>Lỗi mở tệp</translation>
    </message>
</context>
<context>
    <name>DataModel::ProtoImporter</name>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="902"/>
        <source>Import Protocol Buffers File</source>
        <translation>Nhập Tệp Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="904"/>
        <source>Proto Files (*.proto);;All Files (*)</source>
        <translation>Tệp Proto (*.proto);;Tất Cả Tệp (*)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="938"/>
        <source>Failed to open proto file: %1</source>
        <translation>Không thể mở tệp proto: %1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="939"/>
        <source>Verify the file path and read permissions, then try again.</source>
        <translation>Xác minh đường dẫn tệp và quyền đọc, sau đó thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="941"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="950"/>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="968"/>
        <source>Protobuf Import Error</source>
        <translation>Lỗi Nhập Protobuf</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="947"/>
        <source>Proto file is too large (the limit is 10 MB).</source>
        <translation>Tệp proto quá lớn (giới hạn là 10 MB).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="948"/>
        <source>Verify you selected the correct .proto definition file.</source>
        <translation>Xác minh bạn đã chọn đúng tệp định nghĩa .proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="965"/>
        <source>Failed to parse proto file at line %1: %2</source>
        <translation>Không thể phân tích tệp proto tại dòng %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="966"/>
        <source>Only proto3 syntax is supported. Verify the file format and try again.</source>
        <translation>Chỉ hỗ trợ cú pháp proto3. Xác minh định dạng tệp và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="973"/>
        <source>Proto file contains no message definitions</source>
        <translation>Tệp proto không chứa định nghĩa message nào</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="974"/>
        <source>The selected file has no `message` blocks to import.</source>
        <translation>Tệp đã chọn không có khối `message` để nhập.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="976"/>
        <source>Protobuf Import Warning</source>
        <translation>Cảnh Báo Nhập Protobuf</translation>
    </message>
    <message>
        <source>Failed to load imported project</source>
        <translation type="vanished">Không thể tải dự án đã nhập</translation>
    </message>
    <message>
        <source>The generated project JSON could not be loaded.</source>
        <translation type="vanished">Không thể tải JSON dự án đã tạo.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1014"/>
        <source>Successfully imported %1 message(s) and %2 field(s) from the proto file.</source>
        <translation>Đã nhập thành công %1 message và %2 trường từ tệp proto.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1017"/>
        <source>The project editor is now open for customization.</source>
        <translation>Trình chỉnh sửa dự án hiện đã mở để tùy chỉnh.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="1019"/>
        <source>Protobuf Import Complete</source>
        <translation>Nhập Protobuf Hoàn Tất</translation>
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
        <translation>Đầu Vào Hex Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="155"/>
        <source>Please enter valid hexadecimal bytes.

Valid format: 01 A2 FF 3C</source>
        <translation>Vui lòng nhập các byte thập lục phân hợp lệ.

Định dạng hợp lệ: 01 A2 FF 3C</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="160"/>
        <source>No transmit function code to evaluate.</source>
        <translation>Không có mã hàm truyền để đánh giá.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="177"/>
        <source>transmit function is not callable</source>
        <translation>hàm transmit không thể gọi được</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="238"/>
        <source>HEX</source>
        <translation>HEX</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="239"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="240"/>
        <source>Evaluate</source>
        <translation>Đánh Giá</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="241"/>
        <source>Input Value</source>
        <translation>Giá Trị Đầu Vào</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="242"/>
        <source>Transmit Function Output</source>
        <translation>Đầu Ra Hàm Truyền</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="243"/>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="267"/>
        <source>Enter value to transmit…</source>
        <translation>Nhập giá trị để truyền…</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="244"/>
        <source>Raw string output appears here</source>
        <translation>Đầu ra chuỗi thô xuất hiện ở đây</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="245"/>
        <source>Hex byte output appears here</source>
        <translation>Đầu ra byte hex xuất hiện ở đây</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="248"/>
        <source>Test Transmit Function</source>
        <translation>Kiểm Tra Hàm Truyền</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="261"/>
        <source>Enter hex bytes (e.g., 01 A2 FF)</source>
        <translation>Nhập byte hex (ví dụ: 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Dialogs/TransmitTestDialog.cpp" line="367"/>
        <source>(empty) No data returned</source>
        <translation>(trống) Không có dữ liệu trả về</translation>
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
        <translation>Bộ Nhớ Chia Sẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="167"/>
        <source>Add Folder</source>
        <translation>Thêm Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="169"/>
        <source>Add a top-level folder</source>
        <translation>Thêm thư mục cấp cao nhất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="177"/>
        <source>Add Shared Table</source>
        <translation>Thêm Bảng Chia Sẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="179"/>
        <source>Add shared table</source>
        <translation>Thêm bảng chia sẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="188"/>
        <source>Help</source>
        <translation>Trợ Giúp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="193"/>
        <source>Open help documentation for shared memory</source>
        <translation>Mở tài liệu trợ giúp về bộ nhớ chia sẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="202"/>
        <source>Title</source>
        <translation>Tiêu Đề</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="203"/>
        <source>Registers</source>
        <translation>Thanh Ghi</translation>
    </message>
    <message>
        <source>Name</source>
        <translation type="vanished">Tên</translation>
    </message>
    <message>
        <source>Description</source>
        <translation type="vanished">Mô Tả</translation>
    </message>
    <message>
        <source>Entries</source>
        <translation type="vanished">Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DataTablesView.qml" line="297"/>
        <source>No shared tables.</source>
        <translation>Không có bảng chia sẻ.</translation>
    </message>
</context>
<context>
    <name>DatabaseExplorer</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="35"/>
        <source>Sessions</source>
        <translation>Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="218"/>
        <source>Open</source>
        <translation>Mở</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="220"/>
        <source>Open a session file</source>
        <translation>Mở tệp phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="226"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="229"/>
        <source>Close session file</source>
        <translation>Đóng tệp phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="242"/>
        <source>Replay</source>
        <translation>Phát Lại</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="246"/>
        <source>Replay selected session on the dashboard</source>
        <translation>Phát lại phiên đã chọn trên bảng điều khiển</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="252"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="258"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Mở khóa tệp phiên để xóa các phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="259"/>
        <source>Delete the selected session</source>
        <translation>Xóa phiên đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="276"/>
        <source>Unlock</source>
        <translation>Mở Khóa</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="277"/>
        <source>Lock</source>
        <translation>Khóa</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="282"/>
        <source>Unlock the session file to allow deletions</source>
        <translation>Mở khóa tệp phiên để cho phép xóa</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="283"/>
        <source>Set a password to prevent session deletions</source>
        <translation>Đặt mật khẩu để ngăn xóa phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="298"/>
        <source>Export CSV</source>
        <translation>Xuất CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="303"/>
        <source>Export selected session to CSV</source>
        <translation>Xuất phiên đã chọn sang CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="310"/>
        <source>Export PDF</source>
        <translation>Xuất PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="315"/>
        <source>Generate a PDF report for the selected session</source>
        <translation>Tạo báo cáo PDF cho phiên đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="329"/>
        <source>Restore Project</source>
        <translation>Khôi Phục Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="333"/>
        <source>Restore the project file from this session file</source>
        <translation>Khôi phục tệp dự án từ tệp phiên này</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="402"/>
        <source>Loading session…</source>
        <translation>Đang tải phiên…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/DatabaseExplorer.qml" line="403"/>
        <source>Working…</source>
        <translation>Đang Xử Lý…</translation>
    </message>
</context>
<context>
    <name>DatasetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="112"/>
        <source>Pro features detected in this project.</source>
        <translation>Phát hiện tính năng Pro trong dự án này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="114"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Đang sử dụng widget dự phòng. Mua giấy phép để mở khóa đầy đủ chức năng.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="142"/>
        <source>Plots</source>
        <translation>Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="147"/>
        <source>Plot</source>
        <translation>Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="151"/>
        <source>Toggle 2D plot visualization for this dataset</source>
        <translation>Bật/tắt biểu đồ 2D cho tập dữ liệu này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="163"/>
        <source>FFT Plot</source>
        <translation>Biểu Đồ FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="166"/>
        <source>Toggle FFT plot to visualize frequency content</source>
        <translation>Bật/tắt biểu đồ FFT để hiển thị nội dung tần số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="178"/>
        <source>Waterfall</source>
        <translation>Thác Nước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="182"/>
        <source>Toggle waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Bật/tắt biểu đồ thác nước (spectrogram) — sử dụng cài đặt FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="199"/>
        <source>Widgets</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="205"/>
        <source>Bar/Level</source>
        <translation>Thanh/mức</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="209"/>
        <source>Toggle bar/level indicator for this dataset</source>
        <translation>Bật/tắt thanh chỉ báo/mức cho tập dữ liệu này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="220"/>
        <source>Gauge</source>
        <translation>Đồng Hồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="225"/>
        <source>Toggle gauge widget for analog-style display</source>
        <translation>Bật/tắt widget đồng hồ để hiển thị kiểu analog</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="237"/>
        <source>Compass</source>
        <translation>La Bàn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="241"/>
        <source>Toggle compass widget for directional data</source>
        <translation>Bật/tắt widget la bàn cho dữ liệu hướng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="252"/>
        <source>Meter</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="257"/>
        <source>Toggle analog meter (half-arc) widget</source>
        <translation>Bật/tắt widget đồng hồ đo analog (nửa vòng cung)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="308"/>
        <source>Define colored value ranges with severity tiers for this dataset's gauge or LED.</source>
        <translation>Định nghĩa các dải giá trị có màu với mức độ nghiêm trọng cho đồng hồ đo hoặc LED của tập dữ liệu này.</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Nhiệt Kế</translation>
    </message>
    <message>
        <source>Toggle thermometer widget for temperature data</source>
        <translation type="vanished">Bật/tắt widget nhiệt kế cho dữ liệu nhiệt độ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="268"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="273"/>
        <source>Toggle LED indicator for binary or thresholded values</source>
        <translation>Bật/tắt chỉ báo LED cho giá trị nhị phân hoặc ngưỡng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="293"/>
        <source>Behavior</source>
        <translation>Hành Vi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="298"/>
        <source>Alarm Bands</source>
        <translation>Dải Báo Động</translation>
    </message>
    <message>
        <source>Define colored value ranges with severity tiers for this dataset's gauge.</source>
        <translation type="vanished">Định nghĩa các dải giá trị có màu với mức độ nghiêm trọng cho đồng hồ đo của tập dữ liệu này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="314"/>
        <source>Transform</source>
        <translation>Biến Đổi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="318"/>
        <source>Edit a value transform expression for calibration, filtering, or unit conversion</source>
        <translation>Chỉnh sửa biểu thức biến đổi giá trị để hiệu chuẩn, lọc hoặc chuyển đổi đơn vị</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="331"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="336"/>
        <source>Duplicate this dataset with the same configuration</source>
        <translation>Nhân bản tập dữ liệu này với cùng cấu hình</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="341"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/DatasetView.qml" line="344"/>
        <source>Delete this dataset from the group</source>
        <translation>Xóa tập dữ liệu này khỏi nhóm</translation>
    </message>
</context>
<context>
    <name>Donate</name>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="36"/>
        <source>Support Serial Studio</source>
        <translation>Hỗ Trợ Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="85"/>
        <source>Support the development of %1!</source>
        <translation>Hỗ trợ phát triển %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="96"/>
        <source>Serial Studio is free &amp; open-source software supported by volunteers. Consider donating or obtaining a Pro license to support development efforts :)</source>
        <translation>Serial Studio là phần mềm mã nguồn mở &amp; miễn phí được hỗ trợ bởi tình nguyện viên. Hãy cân nhắc quyên góp hoặc mua giấy phép Pro để hỗ trợ nỗ lực phát triển :)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="109"/>
        <source>You can also support this project by sharing it, reporting bugs and proposing new features!</source>
        <translation>Bạn cũng có thể hỗ trợ dự án này bằng cách chia sẻ, báo cáo lỗi và đề xuất tính năng mới!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="123"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="134"/>
        <source>Donate</source>
        <translation>Quyên Góp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Donate.qml" line="149"/>
        <source>Get Serial Studio Pro</source>
        <translation>Mua Serial Studio Pro</translation>
    </message>
</context>
<context>
    <name>Downloader</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="142"/>
        <source>Stop</source>
        <translation>Dừng</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="143"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="377"/>
        <source>Downloading updates</source>
        <translation>Đang Tải Xuống Bản Cập Nhật</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="470"/>
        <source>Time remaining</source>
        <translation>Thời gian còn lại</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="144"/>
        <source>unknown</source>
        <translation>không xác định</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Error</source>
        <translation>Lỗi</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="243"/>
        <source>Cannot find downloaded update!</source>
        <translation>Không tìm thấy bản cập nhật đã tải xuống!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="259"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="260"/>
        <source>Download complete!</source>
        <translation>Tải xuống hoàn tất!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="261"/>
        <source>The installer opens separately</source>
        <translation>Trình cài đặt mở riêng biệt</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="268"/>
        <source>Click "OK" to begin installing the update</source>
        <translation>Nhấn "OK" để bắt đầu cài đặt bản cập nhật</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="270"/>
        <source>In order to install the update, you may need to quit the application.</source>
        <translation>Để cài đặt bản cập nhật, bạn có thể cần thoát ứng dụng.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="274"/>
        <source>In order to install the update, you may need to quit the application. This is a mandatory update, exiting now will close the application.</source>
        <translation>Để cài đặt bản cập nhật, bạn có thể cần thoát ứng dụng. Đây là bản cập nhật bắt buộc, thoát ngay bây giờ sẽ đóng ứng dụng.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="290"/>
        <source>Click the "Open" button to apply the update</source>
        <translation>Nhấp vào nút "Mở" để áp dụng bản cập nhật</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="303"/>
        <source>Updater</source>
        <translation>Trình Cập Nhật</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="307"/>
        <source>Are you sure you want to cancel the download?</source>
        <translation>Bạn có chắc chắn muốn hủy tải xuống không?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="309"/>
        <source>Are you sure you want to cancel the download? This is a mandatory update, exiting now will close the application</source>
        <translation>Bạn có chắc chắn muốn hủy tải xuống không? Đây là bản cập nhật bắt buộc, thoát ngay bây giờ sẽ đóng ứng dụng</translation>
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
        <translation>trên</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="421"/>
        <source>Downloading Updates</source>
        <translation>Đang Tải Bản Cập Nhật</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Time Remaining</source>
        <translation>Thời Gian Còn Lại</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="422"/>
        <source>Unknown</source>
        <translation>Không Xác Định</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="446"/>
        <source>about %1 hours</source>
        <translation>khoảng %1 giờ</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="448"/>
        <source>about one hour</source>
        <translation>khoảng một giờ</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="456"/>
        <source>%1 minutes</source>
        <translation>%1 phút</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="458"/>
        <source>1 minute</source>
        <translation>1 phút</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="465"/>
        <source>%1 seconds</source>
        <translation>%1 giây</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Downloader.cpp" line="467"/>
        <source>1 second</source>
        <translation>1 giây</translation>
    </message>
    <message>
        <source>Time remaining: 0 minutes</source>
        <translation type="vanished">Thời gian còn lại: 0 phút</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Mở</translation>
    </message>
</context>
<context>
    <name>ExamplesBrowser</name>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="33"/>
        <source>Examples Browser</source>
        <translation>Trình Duyệt Ví Dụ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="90"/>
        <source>Search in Examples…</source>
        <translation>Tìm Kiếm trong Ví Dụ…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="119"/>
        <source>Back</source>
        <translation>Quay Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="151"/>
        <source>Pro</source>
        <translation>Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="172"/>
        <source>Download &amp;&amp; Open</source>
        <translation>Tải Xuống &amp;&amp; Mở</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="187"/>
        <source>View on GitHub</source>
        <translation>Xem trên GitHub</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="244"/>
        <source>Fetching examples…</source>
        <translation>Đang tải ví dụ…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="567"/>
        <source>Loading...</source>
        <translation>Đang Tải...</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="568"/>
        <source>No README available.</source>
        <translation>Không có README.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="608"/>
        <source>Copied to Clipboard</source>
        <translation>Đã Sao Chép vào Clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="671"/>
        <source>No screenshot available</source>
        <translation>Không có ảnh chụp màn hình</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="703"/>
        <source>Details</source>
        <translation>Chi Tiết</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="732"/>
        <source>Info</source>
        <translation>Thông Tin</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="755"/>
        <source>Category:</source>
        <translation>Danh Mục:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="768"/>
        <source>Difficulty:</source>
        <translation>Độ Khó:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="786"/>
        <source>Project:</source>
        <translation>Dự Án:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="828"/>
        <source>No Results Found</source>
        <translation>Không Tìm Thấy Kết Quả</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="839"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Kiểm tra chính tả hoặc thử từ khóa tìm kiếm khác.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="854"/>
        <source>%1 examples</source>
        <translation>%1 ví dụ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExamplesBrowser.qml" line="863"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>ExtensionManager</name>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="31"/>
        <source>Extension Manager</source>
        <translation>Trình Quản Lý Tiện Ích Mở Rộng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="77"/>
        <source>Search extensions…</source>
        <translation>Tìm kiếm tiện ích mở rộng…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="118"/>
        <source>Refresh</source>
        <translation>Làm Mới</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="132"/>
        <source>Repos</source>
        <translation>Kho</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="162"/>
        <source>Repository Settings</source>
        <translation>Cài Đặt Kho Lưu Trữ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="174"/>
        <source>Back</source>
        <translation>Quay Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="215"/>
        <source>Install</source>
        <translation>Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="232"/>
        <source>Uninstall</source>
        <translation>Gỡ Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="259"/>
        <source>Run</source>
        <translation>Chạy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="283"/>
        <source>Stop</source>
        <translation>Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="317"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="367"/>
        <source>Fetching extensions…</source>
        <translation>Đang tải tiện ích…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="604"/>
        <source>Running</source>
        <translation>Đang Chạy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="626"/>
        <source>Update</source>
        <translation>Cập Nhật</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="626"/>
        <source>Installed</source>
        <translation>Đã Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="643"/>
        <source>Unavailable</source>
        <translation>Không Khả Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="822"/>
        <source>No description available.</source>
        <translation>Không có mô tả.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="863"/>
        <source>Details</source>
        <translation>Chi Tiết</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="884"/>
        <source>Type:</source>
        <translation>Loại:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="897"/>
        <source>Author:</source>
        <translation>Tác Giả:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="909"/>
        <source>Version:</source>
        <translation>Phiên Bản:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="921"/>
        <source>License:</source>
        <translation>Giấy Phép:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="982"/>
        <source>No preview</source>
        <translation>Không có xem trước</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1011"/>
        <source>  PLUGIN OUTPUT</source>
        <translation>ĐẦU RA PLUGIN</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1041"/>
        <source>No output yet. Run the plugin to see its log here.</source>
        <translation>Chưa có đầu ra. Chạy plugin để xem nhật ký tại đây.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1076"/>
        <source>No preview available</source>
        <translation>Không có xem trước</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1120"/>
        <source>Repositories</source>
        <translation>Kho Lưu Trữ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1133"/>
        <source>Add URLs to remote repositories or local folder paths.</source>
        <translation>Thêm URL đến kho lưu trữ từ xa hoặc đường dẫn thư mục cục bộ.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1170"/>
        <source>LOCAL</source>
        <translation>CỤC BỘ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1227"/>
        <source>URL or local path…</source>
        <translation>URL hoặc đường dẫn cục bộ…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1235"/>
        <source>Add</source>
        <translation>Thêm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1258"/>
        <source>Browse…</source>
        <translation>Duyệt…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1295"/>
        <source>No Results Found</source>
        <translation>Không Tìm Thấy Kết Quả</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1306"/>
        <source>Check the spelling or try a different search term.</source>
        <translation>Kiểm tra chính tả hoặc thử từ khóa tìm kiếm khác.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1330"/>
        <source>No Extensions Available</source>
        <translation>Không Có Tiện Ích Mở Rộng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1341"/>
        <source>Add a repository URL or local path in the Repos settings, then refresh.</source>
        <translation>Thêm URL kho lưu trữ hoặc đường dẫn cục bộ trong cài đặt Repos, sau đó làm mới.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1356"/>
        <source>%1 extensions</source>
        <translation>%1 tiện ích mở rộng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ExtensionManager.qml" line="1365"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>FFTPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="167"/>
        <source>Interpolation: %1</source>
        <translation>Nội Suy: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="195"/>
        <source>Show Area Under Plot</source>
        <translation>Hiển Thị Vùng dưới Đồ Thị</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="213"/>
        <source>Show X Axis Label</source>
        <translation>Hiển Thị Nhãn Trục X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="225"/>
        <source>Show Y Axis Label</source>
        <translation>Hiển Thị Nhãn Trục Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="243"/>
        <source>Show Crosshair</source>
        <translation>Hiển Thị Tâm Ngắm</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="250"/>
        <source>Resume</source>
        <translation>Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="269"/>
        <source>Reset View</source>
        <translation>Đặt Lại Chế Độ Xem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="275"/>
        <source>Axis Range Settings</source>
        <translation>Cài Đặt Phạm Vi Trục</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="304"/>
        <source>Magnitude (dB)</source>
        <translation>Độ Lớn (dB)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/FFTPlot.qml" line="305"/>
        <source>Frequency (Hz)</source>
        <translation>Tần Số (Hz)</translation>
    </message>
</context>
<context>
    <name>FileDropArea</name>
    <message>
        <location filename="../../qml/Widgets/FileDropArea.qml" line="130"/>
        <source>Drop Projects and CSV files here</source>
        <translation>Thả Tệp Dự Án và CSV vào Đây</translation>
    </message>
</context>
<context>
    <name>FileTransmission</name>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="33"/>
        <source>File Transmission</source>
        <translation>Truyền Tệp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="101"/>
        <source>Transfer Protocol:</source>
        <translation>Giao Thức Truyền:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="134"/>
        <source>File Selection:</source>
        <translation>Chọn Tệp:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="151"/>
        <source>Select File…</source>
        <translation>Chọn Tệp…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="169"/>
        <source>Transmission Interval:</source>
        <translation>Khoảng Cách Truyền:</translation>
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
        <translation>Kích Thước Khối:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="233"/>
        <source>bytes</source>
        <translation>byte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="243"/>
        <source>Timeout:</source>
        <translation>Thời Gian Chờ:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="281"/>
        <source>Max Retries:</source>
        <translation>Số Lần Thử Lại Tối Đa:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="339"/>
        <source>Progress: %1%</source>
        <translation>Tiến Độ: %1%</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="372"/>
        <source>%1 / %2 bytes</source>
        <translation>%1 / %2 byte</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="380"/>
        <source>Errors: %1</source>
        <translation>Lỗi: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="418"/>
        <source>Pause Transmission</source>
        <translation>Tạm Dừng Truyền</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="419"/>
        <source>Resume Transmission</source>
        <translation>Tiếp Tục Truyền</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="422"/>
        <source>Stop Transmission</source>
        <translation>Dừng Truyền</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="423"/>
        <source>Begin Transmission</source>
        <translation>Bắt Đầu Truyền</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="460"/>
        <source>Activity Log</source>
        <translation>Nhật Ký Hoạt Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FileTransmission.qml" line="464"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
</context>
<context>
    <name>FlowDiagram</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="448"/>
        <source>Frame Parser</source>
        <translation>Bộ Phân Tích Frame</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="455"/>
        <source>Device %1</source>
        <translation>Thiết Bị %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="245"/>
        <source>Group</source>
        <translation>Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="372"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="756"/>
        <source>Folder</source>
        <translation>Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="517"/>
        <source>Action</source>
        <translation>Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="575"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1862"/>
        <source>Output Panel</source>
        <translation>Bảng Điều Khiển Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="619"/>
        <source>Control</source>
        <translation>Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="694"/>
        <source>Table</source>
        <translation>Bảng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="708"/>
        <source>%1 regs</source>
        <translation>%1 thanh ghi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="406"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="708"/>
        <source>empty</source>
        <translation>trống</translation>
    </message>
    <message>
        <source>Control Script</source>
        <translation type="vanished">Script Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="843"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1251"/>
        <source>Open the transform code editor for this dataset.</source>
        <translation>Mở trình chỉnh sửa mã biến đổi cho tập dữ liệu này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1632"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1635"/>
        <source>Dataset Container</source>
        <translation>Vùng Chứa Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1644"/>
        <source>Multi-Plot</source>
        <translation>Đồ Thị Đa Biến</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1647"/>
        <source>Multiple Plot</source>
        <translation>Đồ Thị Nhiều Biến</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1656"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1659"/>
        <source>Accelerometer</source>
        <translation>Gia Tốc Kế</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1668"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1671"/>
        <source>Gyroscope</source>
        <translation>Con Quay Hồi Chuyển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1680"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1683"/>
        <source>GPS Map</source>
        <translation>Bản Đồ GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1691"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1694"/>
        <source>3D Plot</source>
        <translation>Đồ Thị 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1702"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1705"/>
        <source>Image View</source>
        <translation>Chế Độ Xem Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1714"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1717"/>
        <source>Painter Widget</source>
        <translation>Widget Vẽ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1726"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1729"/>
        <source>Web View</source>
        <translation>Khung Xem Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1738"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1741"/>
        <source>Data Grid</source>
        <translation>Lưới Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1754"/>
        <source>Generic</source>
        <translation>Chung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1767"/>
        <source>Plot</source>
        <translation>Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1780"/>
        <source>FFT Plot</source>
        <translation>Biểu Đồ FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1793"/>
        <source>Gauge</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1806"/>
        <source>Level Indicator</source>
        <translation>Chỉ Báo Mức</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1819"/>
        <source>Compass</source>
        <translation>La Bàn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1832"/>
        <source>Meter</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <source>Edit Control Script…</source>
        <translation type="vanished">Chỉnh Sửa Script Điều Khiển…</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Nhiệt Kế</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="404"/>
        <source>Control Loop</source>
        <translation>Vòng Lặp Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="799"/>
        <source>Shared Memory</source>
        <translation>Bộ Nhớ Chia Sẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1845"/>
        <source>LED Indicator</source>
        <translation>Đèn Báo LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1874"/>
        <source>Slider</source>
        <translation>Thanh Trượt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1887"/>
        <source>Toggle</source>
        <translation>Công Tắc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1900"/>
        <source>Knob</source>
        <translation>Núm Xoay</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1913"/>
        <source>Text Field</source>
        <translation>Trường Văn Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1926"/>
        <source>Button</source>
        <translation>Nút</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1950"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2026"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2114"/>
        <source>Add Group</source>
        <translation>Thêm Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1966"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2042"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2130"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2175"/>
        <source>Add Dataset</source>
        <translation>Thêm Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1980"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2056"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2144"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2189"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2396"/>
        <source>Add Output</source>
        <translation>Thêm Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="1996"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2069"/>
        <source>Add Action</source>
        <translation>Thêm Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2005"/>
        <source>Add Data Source</source>
        <translation>Thêm Nguồn Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2012"/>
        <source>Add Data Table</source>
        <translation>Thêm Bảng Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2080"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2216"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2283"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2411"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2445"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2501"/>
        <source>Rename…</source>
        <translation>Đổi Tên…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2088"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2246"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2316"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2368"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2419"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2475"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
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
        <translation>Xóa…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2160"/>
        <source>Edit Frame Parser…</source>
        <translation>Chỉnh Sửa Bộ Phân Tích Frame…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2202"/>
        <source>Edit Painter Code…</source>
        <translation>Chỉnh Sửa Mã Painter…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2224"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2292"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2344"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2453"/>
        <source>Move Up</source>
        <translation>Di Chuyển Lên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2235"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2304"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2356"/>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2464"/>
        <source>Move Down</source>
        <translation>Di Chuyển Xuống</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2272"/>
        <source>Edit Transform Code…</source>
        <translation>Chỉnh Sửa Mã Transform…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2527"/>
        <source>Edit Code…</source>
        <translation>Chỉnh Sửa Mã…</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/FlowDiagram.qml" line="2542"/>
        <source>Edit Control Loop…</source>
        <translation>Chỉnh Sửa Vòng Lặp Điều Khiển…</translation>
    </message>
</context>
<context>
    <name>FrameParserTest</name>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="39"/>
        <source>Test Frame Parser</source>
        <translation>Kiểm Tra Trình Phân Tích Frame</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="96"/>
        <source>Frame %1</source>
        <translation>Khung %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="98"/>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="209"/>
        <source>Decoder</source>
        <translation>Bộ Giải Mã</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="100"/>
        <source>Rows</source>
        <translation>Hàng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="101"/>
        <source>%1 row(s)</source>
        <translation>%1 hàng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="105"/>
        <source>Row %1</source>
        <translation>Hàng %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="164"/>
        <source>Pipeline Configuration</source>
        <translation>Cấu Hình Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="186"/>
        <source>Detection</source>
        <translation>Phát Hiện</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="232"/>
        <source>Start</source>
        <translation>Bắt Đầu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="253"/>
        <source>End</source>
        <translation>Kết Thúc</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="274"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="299"/>
        <source>Hex Delimiters</source>
        <translation>Dấu Phân Cách Hex</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="315"/>
        <source>Frame Data Input</source>
        <translation>Đầu Vào Dữ Liệu Frame</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="342"/>
        <source>Enter hex bytes (e.g. 01 A2 FF)</source>
        <translation>Nhập byte hex (ví dụ: 01 A2 FF)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="343"/>
        <source>Enter raw stream bytes here...</source>
        <translation>Nhập byte luồng thô tại đây...</translation>
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
        <translation>Mẫu không chứa dấu phân cách frame đã cấu hình nên không thể trích xuất frame. Nhập chúng vào mẫu (ví dụ: 
 cho xuống dòng) hoặc điều chỉnh chế độ phát hiện.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="407"/>
        <source>Pipeline Results</source>
        <translation>Kết Quả Pipeline</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="480"/>
        <source>Stage</source>
        <translation>Chuẩn Bị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="487"/>
        <source>Value</source>
        <translation>Giá Trị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="530"/>
        <source>Extraction did not produce a complete frame. Check the start / end delimiters and the detection mode.</source>
        <translation>Quá trình trích xuất không tạo ra frame hoàn chỉnh. Kiểm tra dấu phân cách bắt đầu / kết thúc và chế độ phát hiện.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="532"/>
        <source>Enter sample data above and press Evaluate to preview the parsed output</source>
        <translation>Nhập dữ liệu mẫu ở trên và nhấn Đánh Giá để xem trước kết quả phân tích</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="614"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="625"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/FrameParserTest.qml" line="632"/>
        <source>Evaluate</source>
        <translation>Đánh Giá</translation>
    </message>
</context>
<context>
    <name>FrameParserView</name>
    <message>
        <source>Undo</source>
        <translation type="vanished">Hoàn Tác</translation>
    </message>
    <message>
        <source>Redo</source>
        <translation type="vanished">Làm Lại</translation>
    </message>
    <message>
        <source>Cut</source>
        <translation type="vanished">Cắt</translation>
    </message>
    <message>
        <source>Copy</source>
        <translation type="vanished">Sao Chép</translation>
    </message>
    <message>
        <source>Paste</source>
        <translation type="vanished">Dán</translation>
    </message>
    <message>
        <source>Select All</source>
        <translation type="vanished">Chọn Tất Cả</translation>
    </message>
    <message>
        <source>Format Document</source>
        <translation type="vanished">Định Dạng Tài Liệu</translation>
    </message>
    <message>
        <source>Format Selection</source>
        <translation type="vanished">Định Dạng Vùng Chọn</translation>
    </message>
    <message>
        <source>Reset</source>
        <translation type="vanished">Đặt Lại</translation>
    </message>
    <message>
        <source>Reset to the default parsing script</source>
        <translation type="vanished">Đặt lại về script phân tích mặc định</translation>
    </message>
    <message>
        <source>Open</source>
        <translation type="vanished">Mở</translation>
    </message>
    <message>
        <source>Import a script file for data parsing</source>
        <translation type="vanished">Nhập file script để phân tích dữ liệu</translation>
    </message>
    <message>
        <source>Undo the last code edit</source>
        <translation type="vanished">Hoàn tác chỉnh sửa mã gần nhất</translation>
    </message>
    <message>
        <source>Redo the previously undone edit</source>
        <translation type="vanished">Làm lại thao tác vừa hoàn tác</translation>
    </message>
    <message>
        <source>Cut selected code to clipboard</source>
        <translation type="vanished">Cắt mã đã chọn vào clipboard</translation>
    </message>
    <message>
        <source>Copy selected code to clipboard</source>
        <translation type="vanished">Sao chép mã đã chọn vào clipboard</translation>
    </message>
    <message>
        <source>Paste code from clipboard</source>
        <translation type="vanished">Dán mã từ clipboard</translation>
    </message>
    <message>
        <source>Help</source>
        <translation type="vanished">Trợ Giúp</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Mở tài liệu trợ giúp về phân tích dữ liệu</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Ngôn Ngữ:</translation>
    </message>
    <message>
        <source>Select Template…</source>
        <translation type="vanished">Chọn Mẫu…</translation>
    </message>
    <message>
        <source>Test With Sample Data</source>
        <translation type="vanished">Kiểm Tra với Dữ Liệu Mẫu</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Đánh Giá</translation>
    </message>
</context>
<context>
    <name>GPS</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="105"/>
        <source>Auto Center</source>
        <translation>Tự Động Căn Giữa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="121"/>
        <source>Plot Trajectory</source>
        <translation>Vẽ Quỹ Đạo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="138"/>
        <source>Zoom In</source>
        <translation>Phóng To</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="149"/>
        <source>Zoom Out</source>
        <translation>Thu Nhỏ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="173"/>
        <source>Show Weather</source>
        <translation>Hiển Thị Thời Tiết</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="191"/>
        <source>NASA Weather Overlay</source>
        <translation>Lớp Phủ Thời Tiết NASA</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/GPS.qml" line="223"/>
        <source>Base Map: %1</source>
        <translation>Bản Đồ Nền: %1</translation>
    </message>
</context>
<context>
    <name>GroupFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="132"/>
        <source>Add Sub-folder</source>
        <translation>Thêm Thư Mục Con</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="134"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Thêm thư mục con bên trong thư mục này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="142"/>
        <source>Add Group</source>
        <translation>Thêm Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="144"/>
        <source>Add a group inside this folder</source>
        <translation>Thêm nhóm vào trong thư mục này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="154"/>
        <source>Rename</source>
        <translation>Đổi Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="156"/>
        <source>Rename folder</source>
        <translation>Đổi tên thư mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="164"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="166"/>
        <source>Delete folder</source>
        <translation>Xóa thư mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="179"/>
        <source>Title</source>
        <translation>Tiêu Đề</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="180"/>
        <source>Contents</source>
        <translation>Nội Dung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupFolderView.qml" line="270"/>
        <source>This folder is empty. Use the toolbar to add a group or sub-folder.</source>
        <translation>Thư mục này trống. Sử dụng thanh công cụ để thêm nhóm hoặc thư mục con.</translation>
    </message>
</context>
<context>
    <name>GroupTemplateMenu</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="43"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="45"/>
        <source>Dataset Container</source>
        <translation>Vùng Chứa Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="51"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="53"/>
        <source>Data Grid</source>
        <translation>Lưới Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="59"/>
        <source>Multi-Plot</source>
        <translation>Đồ Thị Đa Trục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="61"/>
        <source>Multiple Plot</source>
        <translation>Đồ Thị Đa Trục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="67"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="69"/>
        <source>3D Plot</source>
        <translation>Đồ Thị 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="75"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="77"/>
        <source>Accelerometer</source>
        <translation>Gia Tốc Kế</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="83"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="85"/>
        <source>Gyroscope</source>
        <translation>Con Quay Hồi Chuyển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="91"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="93"/>
        <source>GPS Map</source>
        <translation>Bản Đồ GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="99"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="101"/>
        <source>Image View</source>
        <translation>Khung Xem Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="107"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="109"/>
        <source>Web View</source>
        <translation>Khung Xem Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="115"/>
        <location filename="../../qml/ProjectEditor/Views/GroupTemplateMenu.qml" line="117"/>
        <source>Painter Widget</source>
        <translation>Widget Vẽ</translation>
    </message>
</context>
<context>
    <name>GroupView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="100"/>
        <source>Pro features detected in this project.</source>
        <translation>Phát hiện tính năng Pro trong dự án này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="102"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Đang sử dụng widget dự phòng. Mua giấy phép để mở khóa đầy đủ chức năng.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="132"/>
        <source>Add Dataset</source>
        <translation>Thêm Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="139"/>
        <source>Dataset</source>
        <translation>Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="142"/>
        <source>Add a generic dataset to the current group</source>
        <translation>Thêm tập dữ liệu chung vào nhóm hiện tại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="149"/>
        <source>Plot</source>
        <translation>Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="153"/>
        <source>Add a 2D plot to visualize numeric data</source>
        <translation>Thêm biểu đồ 2D để trực quan hóa dữ liệu số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="161"/>
        <source>FFT Plot</source>
        <translation>Biểu Đồ FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="166"/>
        <source>Add an FFT plot for frequency domain visualization</source>
        <translation>Thêm biểu đồ FFT để trực quan hóa miền tần số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="172"/>
        <source>Waterfall</source>
        <translation>Thác Nước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="177"/>
        <source>Add a waterfall (spectrogram) plot — uses the FFT settings</source>
        <translation>Thêm biểu đồ thác nước (spectrogram) — sử dụng cài đặt FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="183"/>
        <source>Bar/Level</source>
        <translation>Thanh/mức</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="187"/>
        <source>Add a bar or level indicator for scaled values</source>
        <translation>Thêm chỉ báo thanh hoặc mức cho giá trị tỷ lệ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="198"/>
        <source>Add a gauge widget for analog-style visualization</source>
        <translation>Thêm widget đồng hồ đo để trực quan hóa kiểu tương tự</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="205"/>
        <source>Compass</source>
        <translation>La Bàn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="209"/>
        <source>Add a compass to display directional or angular data</source>
        <translation>Thêm la bàn để hiển thị dữ liệu hướng hoặc góc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="215"/>
        <source>Meter</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="219"/>
        <source>Add an analog meter (half-arc) widget</source>
        <translation>Thêm widget đồng hồ đo analog (nửa vòng cung)</translation>
    </message>
    <message>
        <source>Thermometer</source>
        <translation type="vanished">Nhiệt Kế</translation>
    </message>
    <message>
        <source>Add a thermometer widget for temperature data</source>
        <translation type="vanished">Thêm widget nhiệt kế cho dữ liệu nhiệt độ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="226"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="231"/>
        <source>Add an LED indicator for binary status signals</source>
        <translation>Thêm đèn báo LED cho tín hiệu trạng thái nhị phân</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="244"/>
        <source>Add Control</source>
        <translation>Thêm Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="250"/>
        <source>Button</source>
        <translation>Nút</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="254"/>
        <source>Add a button that sends a command on click</source>
        <translation>Thêm nút gửi lệnh khi nhấp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="260"/>
        <source>Slider</source>
        <translation>Thanh Trượt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="264"/>
        <source>Add a slider for sending scaled numeric values</source>
        <translation>Thêm thanh trượt để gửi giá trị số theo tỷ lệ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="270"/>
        <source>Toggle</source>
        <translation>Công Tắc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="274"/>
        <source>Add a toggle switch for on/off commands</source>
        <translation>Thêm công tắc bật/tắt cho lệnh on/off</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="281"/>
        <source>Text Field</source>
        <translation>Trường Văn Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="284"/>
        <source>Add a text field for typing and sending commands</source>
        <translation>Thêm trường văn bản để nhập và gửi lệnh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="290"/>
        <source>Knob</source>
        <translation>Núm Xoay</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="294"/>
        <source>Add a rotary knob for setpoint control</source>
        <translation>Thêm núm xoay để điều khiển điểm đặt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="313"/>
        <source>Edit Code</source>
        <translation>Chỉnh Sửa Mã</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="317"/>
        <source>Edit the JavaScript that draws this painter widget</source>
        <translation>Chỉnh sửa mã JavaScript vẽ widget painter này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="330"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="334"/>
        <source>Duplicate the current group and its contents</source>
        <translation>Nhân bản nhóm hiện tại và nội dung của nó</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="339"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupView.qml" line="344"/>
        <source>Delete the current group and all contained datasets</source>
        <translation>Xóa nhóm hiện tại và tất cả dataset chứa trong đó</translation>
    </message>
</context>
<context>
    <name>GroupsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="33"/>
        <source>Dashboard Widgets</source>
        <translation>Widget Bảng Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="127"/>
        <source>Add Folder</source>
        <translation>Thêm Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="129"/>
        <source>Add a top-level folder</source>
        <translation>Thêm thư mục cấp cao nhất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="137"/>
        <source>Add Group</source>
        <translation>Thêm Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="139"/>
        <source>Add a group from a template</source>
        <translation>Thêm nhóm từ mẫu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="154"/>
        <source>Title</source>
        <translation>Tiêu Đề</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="155"/>
        <source>Contents</source>
        <translation>Nội Dung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/GroupsView.qml" line="245"/>
        <source>No groups yet. Use the toolbar to add a group or a folder.</source>
        <translation>Chưa có nhóm nào. Sử dụng thanh công cụ để thêm nhóm hoặc thư mục.</translation>
    </message>
</context>
<context>
    <name>Gyroscope</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="410"/>
        <source>ROLL ↔</source>
        <translation>NGHIÊNG ↔</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="440"/>
        <source>YAW ↻</source>
        <translation>XOAY ↻</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Gyroscope.qml" line="470"/>
        <source>PITCH ↕</source>
        <translation>CHÚC ↕</translation>
    </message>
</context>
<context>
    <name>HID</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="50"/>
        <source>HID Device</source>
        <translation>Thiết Bị HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="80"/>
        <source>Usage Page</source>
        <translation>Trang Sử Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="96"/>
        <source>Usage</source>
        <translation>Sử Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="137"/>
        <source>Connect gamepads, joysticks, steering wheels, flight controllers, and other HID-class USB devices.</source>
        <translation>Kết nối gamepad, joystick, vô lăng, bộ điều khiển bay và các thiết bị USB lớp HID khác.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/HID.qml" line="145"/>
        <source>HID Usage Tables (USB.org)</source>
        <translation>Bảng Sử Dụng HID (USB.org)</translation>
    </message>
</context>
<context>
    <name>HelpCenter</name>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="32"/>
        <source>Help Center</source>
        <translation>Trung Tâm Trợ Giúp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="94"/>
        <source>Fetching help pages…</source>
        <translation>Đang tải trang trợ giúp…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="128"/>
        <source>Search…</source>
        <translation>Tìm Kiếm…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="246"/>
        <source>Loading…</source>
        <translation>Đang Tải…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="290"/>
        <source>Select a page from the sidebar</source>
        <translation>Chọn một trang từ thanh bên</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="320"/>
        <source>Copied to Clipboard</source>
        <translation>Đã sao chép vào bộ nhớ tạm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="352"/>
        <source>View Online</source>
        <translation>Xem trực Tuyến</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="371"/>
        <source>%1 pages</source>
        <translation>%1 trang</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/HelpCenter.qml" line="380"/>
        <source>Close</source>
        <translation>Đóng</translation>
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
        <translation>Network Socket</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="271"/>
        <source>Bluetooth LE</source>
        <translation>Bluetooth LE</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="273"/>
        <source>Audio</source>
        <translation>Âm Thanh</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="274"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="275"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="276"/>
        <source>USB Device</source>
        <translation>Thiết Bị USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="277"/>
        <source>HID Device</source>
        <translation>Thiết Bị HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="278"/>
        <source>Process</source>
        <translation>Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="279"/>
        <source>MQTT Subscriber</source>
        <translation>Subscriber MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="685"/>
        <source>Your trial period has ended.</source>
        <translation>Thời gian dùng thử đã kết thúc.</translation>
    </message>
    <message>
        <location filename="../../src/IO/ConnectionManager.cpp" line="686"/>
        <source>To continue using Serial Studio, please activate your license.</source>
        <translation>Để tiếp tục sử dụng Serial Studio, vui lòng kích hoạt giấy phép.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Audio</name>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="752"/>
        <source>channels</source>
        <translation>kênh</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="805"/>
        <source> channels</source>
        <translation>kênh</translation>
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
        <translation>Thiết Bị Đầu Vào</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1350"/>
        <source>Sample Rate</source>
        <translation>Tốc Độ Lấy Mẫu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1358"/>
        <source>Sample Format</source>
        <translation>Định Dạng Mẫu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Audio.cpp" line="1366"/>
        <source>Channels</source>
        <translation>Kênh</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::BluetoothLE</name>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="456"/>
        <source>BLE I/O Module Error</source>
        <translation>Lỗi Mô-đun BLE I/O</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="751"/>
        <source>Select Device</source>
        <translation>Chọn Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="762"/>
        <source>Select Service</source>
        <translation>Chọn Dịch Vụ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="773"/>
        <source>Select Characteristic</source>
        <translation>Chọn Đặc Tính</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="972"/>
        <source>Error while configuring BLE service</source>
        <translation>Lỗi khi cấu hình dịch vụ BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1190"/>
        <source>Operation error</source>
        <translation>Lỗi thao tác</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1193"/>
        <source>Characteristic write error</source>
        <translation>Lỗi ghi đặc tính</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1196"/>
        <source>Descriptor write error</source>
        <translation>Lỗi ghi bộ mô tả</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1199"/>
        <source>Unknown error</source>
        <translation>Lỗi không xác định</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1202"/>
        <source>Characteristic read error</source>
        <translation>Lỗi đọc đặc tính</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1205"/>
        <source>Descriptor read error</source>
        <translation>Lỗi đọc bộ mô tả</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1440"/>
        <source>BLE Device</source>
        <translation>Thiết Bị BLE</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1448"/>
        <source>Service</source>
        <translation>Dịch Vụ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1455"/>
        <source>Notify Characteristic</source>
        <translation>Đặc Tính Thông Báo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1462"/>
        <source>Characteristic</source>
        <translation>Đặc Tính</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::CANBus</name>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="274"/>
        <source>CAN Device Creation Failed</source>
        <translation>Tạo Thiết Bị CAN Thất Bại</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="275"/>
        <source>Unable to create CAN bus device. Check your hardware and drivers.</source>
        <translation>Không thể tạo thiết bị CAN bus. Kiểm tra phần cứng và trình điều khiển.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="301"/>
        <source>CAN Connection Failed</source>
        <translation>Kết Nối CAN Thất Bại</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="299"/>
        <source>Unable to connect to CAN bus device. Check your hardware connection and settings.</source>
        <translation>Không thể kết nối với thiết bị CAN bus. Kiểm tra kết nối phần cứng và cài đặt.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="318"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="324"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="330"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="335"/>
        <source>CAN Bus Not Available</source>
        <translation>Magistrala CAN Không Khả Dụng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="319"/>
        <source>No CAN bus plugins found on this system.

On Linux, ensure SocketCAN kernel modules are loaded.</source>
        <translation>Không tìm thấy plugin magistrala CAN nào trên hệ thống này.

Trên Linux, đảm bảo các module kernel SOCKETCAN đã được tải.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="325"/>
        <source>No CAN bus plugins found on this system.

On Windows, install CAN hardware drivers (PEAK, Vector, etc.).</source>
        <translation>Không tìm thấy plugin magistrala CAN nào trên hệ thống này.

Trên Windows, cài đặt driver phần cứng CAN (PEAK, VECTOR, v.v.).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="331"/>
        <source>No CAN bus plugins found on this system.

CAN bus support on macOS is limited and may require third-party hardware drivers.</source>
        <translation>Không tìm thấy plugin magistrala CAN nào trên hệ thống này.

Hỗ trợ magistrala CAN trên macOS bị hạn chế và có thể yêu cầu driver phần cứng của bên thứ ba.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="336"/>
        <source>No CAN bus plugins are available on this platform.</source>
        <translation>Không có plugin magistrala CAN nào khả dụng trên nền tảng này.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="348"/>
        <source>Invalid CAN Configuration</source>
        <translation>Cấu Hình CAN Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="349"/>
        <source>The CAN bus configuration is incomplete. Select a valid plugin and interface.</source>
        <translation>Cấu hình magistrala CAN chưa đầy đủ. Chọn một plugin và giao diện hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="356"/>
        <source>Invalid Selection</source>
        <translation>Lựa Chọn Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="357"/>
        <source>The selected plugin or interface is no longer available. Refresh the lists and try again.</source>
        <translation>Plugin hoặc giao diện đã chọn không còn khả dụng. Làm mới danh sách và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="365"/>
        <source>No Devices Available</source>
        <translation>Không Có Thiết Bị Khả Dụng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="366"/>
        <source>The plugin or interface list is empty. Refresh the lists and ensure your CAN hardware is connected.</source>
        <translation>Danh sách plugin hoặc giao diện trống. Làm mới danh sách và đảm bảo phần cứng CAN đã được kết nối.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="695"/>
        <source>CAN Bus Error</source>
        <translation>Lỗi CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="696"/>
        <source>An error occurred but the CAN device is no longer available.</source>
        <translation>Đã xảy ra lỗi nhưng thiết bị CAN không còn khả dụng.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="703"/>
        <source>Error code: %1</source>
        <translation>Mã lỗi: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="706"/>
        <source>CAN Bus Communication Error</source>
        <translation>Lỗi Truyền Thông CAN Bus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="721"/>
        <source>Connect a %1 adapter, then refresh</source>
        <translation>Kết nối bộ chuyển đổi %1, sau đó làm mới</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="725"/>
        <source>Load SocketCAN kernel modules first</source>
        <translation>Tải các module kernel SOCKETCAN trước</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="728"/>
        <source>Set up a virtual CAN interface first</source>
        <translation>Thiết lập giao diện CAN ảo trước</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="730"/>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="750"/>
        <source>No interfaces found for %1</source>
        <translation>Không tìm thấy giao diện nào cho %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="734"/>
        <source>Install &lt;a href='https://www.peak-system.com/Drivers.523.0.html?&amp;L=1'&gt;PEAK CAN drivers&lt;/a&gt;</source>
        <translation>Cài đặt &lt;a href='https://www.PEAK-system.com/Drivers.523.0.html?&amp;L=1'&gt;trình điều khiển PEAK CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="738"/>
        <source>Install &lt;a href='https://www.vector.com/us/en/products/products-a-z/libraries-drivers/'&gt;Vector CAN drivers&lt;/a&gt;</source>
        <translation>Cài đặt &lt;a href='https://www.VECTOR.com/us/en/products/products-a-z/libraries-drivers/'&gt;trình điều khiển VECTOR CAN&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="742"/>
        <source>Install &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;SysTec CAN drivers&lt;/a&gt;</source>
        <translation>Cài đặt &lt;a href='https://www.systec-electronic.com/en/company/support/driver'&gt;trình điều khiển CAN SysTec&lt;/a&gt;</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="745"/>
        <source>Install %1 drivers</source>
        <translation>Cài đặt trình điều khiển %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="748"/>
        <source>Install %1 drivers for macOS</source>
        <translation>Cài đặt trình điều khiển %1 cho macOS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="762"/>
        <source>

If the interface is down, bring it up first:
sudo ip link set %1 up type can bitrate %2</source>
        <translation>Nếu giao diện đang tắt, hãy kích hoạt nó trước:
sudo ip link set %1 up type can bitrate %2

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="785"/>
        <source>No CAN driver selected</source>
        <translation>Chưa chọn trình điều khiển CAN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="869"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="877"/>
        <source>Interface</source>
        <translation>Giao Diện</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/CanBus.cpp" line="885"/>
        <source>Bitrate</source>
        <translation>Tốc Độ Bit</translation>
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
        <translation>Chỉ Lắng Nghe</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::GsUsbCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="382"/>
        <source>Failed to initialize libusb for the CANable adapter.</source>
        <translation>Không thể khởi tạo libusb cho bộ chuyển đổi CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="410"/>
        <source>Unable to enumerate USB devices.</source>
        <translation>Không thể liệt kê các thiết bị USB.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="430"/>
        <source>The selected CANable adapter is no longer connected, or another application has it open. On Windows the device must use the WinUSB driver (candleLight installs it automatically).</source>
        <translation>Bộ chuyển đổi CANable đã chọn không còn được kết nối, hoặc một ứng dụng khác đang mở nó. Trên Windows, thiết bị phải sử dụng trình điều khiển WinUSB (candleLight tự động cài đặt nó).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="443"/>
        <source>Could not claim the CANable USB interface.</source>
        <translation>Không thể chiếm giao diện USB CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="510"/>
        <source>CANable adapter is not open for writing.</source>
        <translation>Bộ chuyển đổi CANable không được mở để ghi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="545"/>
        <source>Failed to transmit CAN frame to the adapter.</source>
        <translation>Không thể truyền khung CAN đến bộ chuyển đổi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="561"/>
        <source>CAN bus error reported by the CANable adapter.</source>
        <translation>Lỗi CAN bus được báo cáo bởi bộ chuyển đổi CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="616"/>
        <source>A CAN frame was not acknowledged on the bus.</source>
        <translation>Một khung CAN không được xác nhận trên bus.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="715"/>
        <source>CANable adapter rejected the host-format handshake.</source>
        <translation>Bộ chuyển đổi CANable từ chối bắt tay định dạng máy chủ.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="722"/>
        <source>Could not read CANable timing constants.</source>
        <translation>Không thể đọc hằng số thời gian CANable.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="728"/>
        <source>The bitrate %1 bps is not supported by this CANable adapter.</source>
        <translation>Tốc độ bit %1 bps không được hỗ trợ bởi bộ chuyển đổi CANable này.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="734"/>
        <source>CANable adapter rejected the requested bitrate.</source>
        <translation>Bộ chuyển đổi CANable từ chối tốc độ bit được yêu cầu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/GsUsbCanBackend.cpp" line="748"/>
        <source>Could not start the CANable channel.</source>
        <translation>Không thể khởi động kênh CANable.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::HID</name>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="173"/>
        <source>Unknown error</source>
        <translation>Lỗi không xác định</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="176"/>
        <source>

Check that your user is in the 'plugdev' group or that a udev rule grants access to this device.</source>
        <translation>Kiểm tra người dùng của bạn có trong nhóm 'plugdev' hoặc quy tắc udev đã cấp quyền truy cập thiết bị này.

</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="180"/>
        <source>Failed to open "%1"</source>
        <translation>Không thể mở "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="285"/>
        <source>HID Device Error</source>
        <translation>Lỗi Thiết Bị HID</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="286"/>
        <source>The HID device was disconnected or encountered a fatal read error.</source>
        <translation>Thiết bị HID đã bị ngắt kết nối hoặc gặp lỗi đọc nghiêm trọng.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="395"/>
        <source>Select Device</source>
        <translation>Chọn Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/HID.cpp" line="537"/>
        <source>HID Device</source>
        <translation>Thiết Bị HID</translation>
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
        <translation>TLS 1.3 hoặc Mới Hơn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="62"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 hoặc Mới Hơn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="63"/>
        <source>Any Protocol</source>
        <translation>Bất Kỳ Giao Thức Nào</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="64"/>
        <source>Secure Protocols Only</source>
        <translation>Chỉ Giao Thức Bảo Mật</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="66"/>
        <source>None</source>
        <translation>Không Có</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="67"/>
        <source>Query Peer</source>
        <translation>Truy Vấn Đối Tác</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="68"/>
        <source>Verify Peer</source>
        <translation>Xác Minh Đối Tác</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="69"/>
        <source>Auto Verify Peer</source>
        <translation>Tự Động Xác Minh Đối Tác</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="167"/>
        <source>MQTT Feature Requires a Commercial License</source>
        <translation>Tính Năng MQTT Yêu Cầu Giấy Phép Thương Mại</translation>
    </message>
    <message>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio commercial license (Hobbyist tier or above).</source>
        <translation type="vanished">Đăng ký vào broker MQTT chỉ khả dụng với giấy phép thương mại Serial Studio hợp lệ (cấp Hobbyist trở lên).</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="168"/>
        <source>Subscribing to an MQTT broker is only available with a valid Serial Studio license or an active trial.</source>
        <translation>Đăng ký vào MQTT broker chỉ khả dụng với giấy phép Serial Studio hợp lệ hoặc bản dùng thử đang hoạt động.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="393"/>
        <source>Use System Database</source>
        <translation>Sử Dụng Cơ Sở Dữ Liệu Hệ Thống</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="394"/>
        <source>Load From Folder…</source>
        <translation>Tải từ Thư Mục…</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="427"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Chọn Thư Mục Chứng Chỉ PEM</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="682"/>
        <source>Hostname</source>
        <translation>Tên Máy Chủ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="689"/>
        <source>Port</source>
        <translation>Cổng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="698"/>
        <source>Topic Filter</source>
        <translation>Bộ Lọc Topic</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="705"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="712"/>
        <source>Username</source>
        <translation>Tên Người Dùng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="719"/>
        <source>Password</source>
        <translation>Mật Khẩu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="726"/>
        <source>MQTT Version</source>
        <translation>Phiên Bản MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="734"/>
        <source>Clean Session</source>
        <translation>Phiên Sạch</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="741"/>
        <source>Keep Alive (s)</source>
        <translation>Keep Alive (s)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="750"/>
        <source>Auto Keep Alive</source>
        <translation>Tự Động Keep Alive</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="767"/>
        <source>SSL/TLS Enabled</source>
        <translation>Đã Bật SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="777"/>
        <source>SSL Protocol</source>
        <translation>Giao Thức SSL</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="785"/>
        <source>Peer Verify Mode</source>
        <translation>Chế Độ Xác Minh Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="793"/>
        <source>Peer Verify Depth</source>
        <translation>Độ Sâu Xác Minh Peer</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="894"/>
        <source>MQTT Subscription Error</source>
        <translation>Lỗi Đăng Ký MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="895"/>
        <source>Failed to subscribe to topic "%1".</source>
        <translation>Không thể đăng ký topic "%1".</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="922"/>
        <source>Invalid MQTT Protocol Version</source>
        <translation>Phiên Bản Giao Thức MQTT Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="923"/>
        <source>The broker rejected the configured MQTT protocol version.</source>
        <translation>Broker từ chối phiên bản giao thức MQTT đã cấu hình.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="926"/>
        <source>Client ID Rejected</source>
        <translation>ID Máy Khách Bị từ Chối</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="927"/>
        <source>The broker rejected the client ID. Try a different identifier.</source>
        <translation>Broker từ chối ID máy khách. Thử mã định danh khác.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="930"/>
        <source>MQTT Server Unavailable</source>
        <translation>Máy Chủ MQTT Không Khả Dụng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="931"/>
        <source>The broker is currently unavailable. Retry later.</source>
        <translation>Broker hiện không khả dụng. Thử lại sau.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="934"/>
        <source>Authentication Error</source>
        <translation>Lỗi Xác Thực</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="935"/>
        <source>The credentials provided were rejected by the broker.</source>
        <translation>Thông tin xác thực được cung cấp đã bị broker từ chối.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="938"/>
        <source>Authorization Error</source>
        <translation>Lỗi Ủy Quyền</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="939"/>
        <source>Account lacks permission for this operation.</source>
        <translation>Tài khoản thiếu quyền cho thao tác này.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="942"/>
        <source>Network or Transport Error</source>
        <translation>Lỗi Mạng hoặc Truyền Tải</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="943"/>
        <source>Network/transport layer issue while connecting to the broker.</source>
        <translation>Sự cố lớp mạng/truyền tải khi kết nối tới broker.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="946"/>
        <source>MQTT Protocol Violation</source>
        <translation>Vi Phạm Giao Thức MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="947"/>
        <source>The broker reported a protocol violation and closed the connection.</source>
        <translation>Broker báo cáo vi phạm giao thức và đã đóng kết nối.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="950"/>
        <source>MQTT 5 Error</source>
        <translation>Lỗi MQTT 5</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="951"/>
        <source>An MQTT 5 protocol-level error occurred.</source>
        <translation>Đã xảy ra lỗi cấp giao thức MQTT 5.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="954"/>
        <source>MQTT Error</source>
        <translation>Lỗi MQTT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/MQTT.cpp" line="955"/>
        <source>An unexpected MQTT error occurred.</source>
        <translation>Đã xảy ra lỗi MQTT không mong đợi.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Modbus</name>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="371"/>
        <source>Invalid Serial Port</source>
        <translation>Cổng Nối Tiếp Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="372"/>
        <source>The selected serial port "%1" is no longer available. Refresh the port list and try again.</source>
        <translation>Cổng nối tiếp "%1" đã chọn không còn khả dụng. Làm mới danh sách cổng và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="416"/>
        <source>Modbus Initialization Failed</source>
        <translation>Khởi Tạo Modbus Thất Bại</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="417"/>
        <source>Unable to create Modbus device. Check your system configuration and try again.</source>
        <translation>Không thể tạo thiết bị Modbus. Kiểm tra cấu hình hệ thống và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="442"/>
        <source>Modbus Connection Failed</source>
        <translation>Kết Nối Modbus Thất Bại</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="444"/>
        <source>Unable to connect to "%1". Check your connection settings.</source>
        <translation>Không thể kết nối tới "%1". Kiểm tra cài đặt kết nối.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="445"/>
        <source>"%1": %2</source>
        <translation>"%1": %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="609"/>
        <source>None</source>
        <translation>Không</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="610"/>
        <source>Even</source>
        <translation>Chẵn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="611"/>
        <source>Odd</source>
        <translation>Lẻ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="612"/>
        <source>Space</source>
        <translation>Space</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="613"/>
        <source>Mark</source>
        <translation>Mark</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="665"/>
        <source>Holding Registers (0x03)</source>
        <translation>Holding Registers (0x03)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="666"/>
        <source>Input Registers (0x04)</source>
        <translation>Thanh Ghi Đầu Vào (0x04)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="667"/>
        <source>Coils (0x01)</source>
        <translation>Coil (0x01)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="668"/>
        <source>Discrete Inputs (0x02)</source>
        <translation>Đầu Vào Rời Rạc (0x02)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="850"/>
        <source>No register groups configured</source>
        <translation>Chưa cấu hình nhóm thanh ghi</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="851"/>
        <source>Add at least one register group before generating a project.</source>
        <translation>Thêm ít nhất một nhóm thanh ghi trước khi tạo dự án.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="853"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="865"/>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="890"/>
        <source>Modbus Project Generator</source>
        <translation>Trình Tạo Dự Án Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="862"/>
        <source>Failed to load generated project</source>
        <translation>Không thể tải dự án đã tạo</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="863"/>
        <source>The generated project JSON could not be loaded.</source>
        <translation>Không thể tải JSON dự án đã tạo.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="885"/>
        <source>Successfully generated project with %1 groups and %2 datasets.</source>
        <translation>Đã tạo thành công dự án với %1 nhóm và %2 tập dữ liệu.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="888"/>
        <source>The project editor is now open for customization.</source>
        <translation>Trình chỉnh sửa dự án hiện đã mở để tùy chỉnh.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="903"/>
        <source>Modbus Project</source>
        <translation>Dự Án Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="908"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="928"/>
        <source>Holding Registers</source>
        <translation>Thanh Ghi Giữ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="929"/>
        <source>Input Registers</source>
        <translation>Thanh Ghi Đầu Vào</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="930"/>
        <source>Coils</source>
        <translation>Cuộn Dây</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="931"/>
        <source>Discrete Inputs</source>
        <translation>Đầu Vào Rời Rạc</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="945"/>
        <source>Unknown</source>
        <translation>Không Xác Định</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="958"/>
        <source>Register %1</source>
        <translation>Thanh Ghi %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Coil %1</source>
        <translation>Cuộn Dây %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="966"/>
        <source>Discrete %1</source>
        <translation>Rời Rạc %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1372"/>
        <source>Error code: %1</source>
        <translation>Mã lỗi: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1375"/>
        <source>Modbus Communication Error</source>
        <translation>Lỗi Truyền Thông Modbus</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1387"/>
        <source>Select Port</source>
        <translation>Chọn Cổng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1540"/>
        <source>Protocol</source>
        <translation>Giao Thức</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1548"/>
        <source>Slave Address</source>
        <translation>Địa Chỉ Slave</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1557"/>
        <source>Poll Interval (ms)</source>
        <translation>Khoảng Thời Gian Truy Vấn (ms)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1597"/>
        <source>Host / IP</source>
        <translation>Host / IP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1604"/>
        <source>Port</source>
        <translation>Cổng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1619"/>
        <source>Serial Port</source>
        <translation>Cổng Nối Tiếp</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1627"/>
        <source>Baud Rate</source>
        <translation>Tốc Độ Baud</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1635"/>
        <source>Parity</source>
        <translation>Chẵn Lẻ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1643"/>
        <source>Data Bits</source>
        <translation>Bit Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Modbus.cpp" line="1651"/>
        <source>Stop Bits</source>
        <translation>Bit Dừng</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::Network</name>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="547"/>
        <source>Network socket error</source>
        <translation>Lỗi socket mạng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="563"/>
        <source>Socket Type</source>
        <translation>Loại Socket</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="571"/>
        <source>Remote Address</source>
        <translation>Địa Chỉ từ Xa</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="579"/>
        <source>TCP Port</source>
        <translation>Cổng TCP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="588"/>
        <source>UDP Local Port</source>
        <translation>Cổng Cục Bộ UDP</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Network.cpp" line="597"/>
        <source>UDP Remote Port</source>
        <translation>Cổng từ Xa UDP</translation>
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
        <translation>Không thể khởi động tiến trình</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="184"/>
        <source>Executable "%1" not found in PATH.</source>
        <translation>Không tìm thấy tệp thực thi "%1" trong PATH.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="368"/>
        <source>Select Executable</source>
        <translation>Chọn Tệp Thực Thi</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="392"/>
        <source>Select Working Directory</source>
        <translation>Chọn Thư Mục Làm Việc</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="417"/>
        <source>Select Named Pipe / FIFO</source>
        <translation>Chọn Named Pipe / FIFO</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="514"/>
        <source>The process crashed.</source>
        <translation>Tiến trình đã gặp sự cố.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="515"/>
        <source>Exit code: %1</source>
        <translation>Mã thoát: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="518"/>
        <source>Process "%1" stopped</source>
        <translation>Tiến trình "%1" đã dừng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="536"/>
        <source>Unknown error</source>
        <translation>Lỗi không xác định</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="537"/>
        <source>Process Error</source>
        <translation>Lỗi Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Pipe Error</source>
        <translation>Lỗi Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="551"/>
        <source>Could not open named pipe: %1</source>
        <translation>Không thể mở named pipe: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="763"/>
        <source>Mode</source>
        <translation>Chế Độ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Launch Process</source>
        <translation>Khởi Chạy Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="766"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="771"/>
        <source>Executable</source>
        <translation>Tệp Thực Thi</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="778"/>
        <source>Arguments</source>
        <translation>Tham Số</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="785"/>
        <source>Working Directory</source>
        <translation>Thư Mục Làm Việc</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/Process.cpp" line="792"/>
        <source>Pipe Path</source>
        <translation>Đường Dẫn Pipe</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SeeedCanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="206"/>
        <source>The bitrate %1 bps is not supported by the USB-CAN Analyzer.</source>
        <translation>Tốc độ bit %1 bps không được hỗ trợ bởi USB-CAN Analyzer.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="216"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Không thể mở cổng nối tiếp %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="227"/>
        <source>Failed to initialize the USB-CAN Analyzer.</source>
        <translation>Không thể khởi tạo USB-CAN Analyzer.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="264"/>
        <source>USB-CAN Analyzer is not open for writing.</source>
        <translation>USB-CAN Analyzer chưa mở để ghi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SeeedCanBackend.cpp" line="310"/>
        <source>CAN bus error reported by the USB-CAN Analyzer.</source>
        <translation>Lỗi CAN bus được báo cáo bởi USB-CAN Analyzer.</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::SlcanBackend</name>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="167"/>
        <source>The bitrate %1 bps is not a standard slcan rate.</source>
        <translation>Tốc độ bit %1 bps không phải là tốc độ slcan chuẩn.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="177"/>
        <source>Could not open serial port %1: %2</source>
        <translation>Không thể mở cổng nối tiếp %1: %2</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="190"/>
        <source>Failed to open the slcan channel.</source>
        <translation>Không thể mở kênh slcan.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="229"/>
        <source>slcan adapter is not open for writing.</source>
        <translation>Bộ chuyển đổi slcan chưa mở để ghi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/SlcanBackend.cpp" line="267"/>
        <source>CAN bus error reported by the slcan adapter.</source>
        <translation>Lỗi CAN bus được báo cáo bởi bộ chuyển đổi slcan.</translation>
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
        <translation>Không Có</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="261"/>
        <source>Failed to connect to serial port "%1"</source>
        <translation>Không thể kết nối với cổng nối tiếp "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="349"/>
        <location filename="../../src/IO/Drivers/UART.cpp" line="733"/>
        <source>Select Port</source>
        <translation>Chọn Cổng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="392"/>
        <source>Even</source>
        <translation>Chẵn</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="393"/>
        <source>Odd</source>
        <translation>Lẻ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="394"/>
        <source>Space</source>
        <translation>Khoảng Trắng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="395"/>
        <source>Mark</source>
        <translation>Đánh Dấu</translation>
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
        <translation>"%1" không phải là đường dẫn hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="563"/>
        <source>Please type another path to register a custom serial device</source>
        <translation>Vui lòng nhập đường dẫn khác để đăng ký thiết bị nối tiếp tùy chỉnh</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="798"/>
        <source>Unknown</source>
        <translation>Không Rõ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="799"/>
        <source>Critical error on serial port "%1"</source>
        <translation>Lỗi nghiêm trọng trên cổng nối tiếp "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="800"/>
        <source>Unknown error</source>
        <translation>Lỗi không xác định</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="822"/>
        <source>No error occurred.</source>
        <translation>Không có lỗi xảy ra.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="823"/>
        <source>The specified device could not be found. Check the connection and try again.</source>
        <translation>Không tìm thấy thiết bị được chỉ định. Kiểm tra kết nối và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="824"/>
        <source>Permission denied. Ensure the application has the necessary access rights to the device.</source>
        <translation>Quyền truy cập bị từ chối. Đảm bảo ứng dụng có quyền truy cập cần thiết vào thiết bị.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="825"/>
        <source>Failed to open the device. It may already be in use or unavailable.</source>
        <translation>Không thể mở thiết bị. Thiết bị có thể đang được sử dụng hoặc không khả dụng.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="826"/>
        <source>An error occurred while writing data to the device.</source>
        <translation>Lỗi xảy ra khi ghi dữ liệu vào thiết bị.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="827"/>
        <source>An error occurred while reading data from the device.</source>
        <translation>Lỗi xảy ra khi đọc dữ liệu từ thiết bị.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="828"/>
        <source>A critical resource error occurred. The device may have been disconnected or is no longer accessible.</source>
        <translation>Lỗi tài nguyên nghiêm trọng xảy ra. Thiết bị có thể đã bị ngắt kết nối hoặc không còn truy cập được.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="829"/>
        <source>The requested operation is not supported on this device.</source>
        <translation>Thao tác được yêu cầu không được hỗ trợ trên thiết bị này.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="830"/>
        <source>An unknown error occurred. Check the device and try again.</source>
        <translation>Đã xảy ra lỗi không xác định. Kiểm tra thiết bị và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="831"/>
        <source>The operation timed out. The device may not be responding.</source>
        <translation>Thao tác đã hết thời gian chờ. Thiết bị có thể không phản hồi.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="832"/>
        <source>The device is not open. Open the device before attempting this operation.</source>
        <translation>Thiết bị chưa được mở. Mở thiết bị trước khi thực hiện thao tác này.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="994"/>
        <source>Serial Port</source>
        <translation>Cổng Nối Tiếp</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1002"/>
        <source>Baud Rate</source>
        <translation>Tốc Độ Baud</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1010"/>
        <source>Parity</source>
        <translation>Chẵn Lẻ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1018"/>
        <source>Data Bits</source>
        <translation>Bit Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1026"/>
        <source>Stop Bits</source>
        <translation>Bit Dừng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1034"/>
        <source>Flow Control</source>
        <translation>Điều Khiển Luồng</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1042"/>
        <source>DTR</source>
        <translation>DTR</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/UART.cpp" line="1049"/>
        <source>Auto-Reconnect</source>
        <translation>Kết Nối Lại Tự Động</translation>
    </message>
</context>
<context>
    <name>IO::Drivers::USB</name>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="156"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="164"/>
        <source>USB Error</source>
        <translation>Lỗi USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="157"/>
        <source>Failed to initialize the USB subsystem. Check that libusb is available on your system.</source>
        <translation>Không thể khởi tạo hệ thống con USB. Kiểm tra xem libusb có sẵn trên hệ thống của bạn.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="165"/>
        <source>No USB device selected. Select a device and try again.</source>
        <translation>Chưa chọn thiết bị USB. Chọn một thiết bị và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="171"/>
        <source>Unknown Device</source>
        <translation>Thiết Bị Không Xác Định</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="176"/>
        <source>Failed to open "%1"</source>
        <translation>Không thể mở "%1"</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="177"/>
        <source>Could not open the USB device: %1.

On Linux, ensure you have read/write permission on the device node (add a udev rule or run as root). On macOS, the kernel driver may need to be detached first.</source>
        <translation>Không thể mở thiết bị USB: %1.

Trên Linux, đảm bảo bạn có quyền đọc/ghi trên nút thiết bị (thêm quy tắc udev hoặc chạy với quyền root). Trên macOS, có thể cần tách trình điều khiển kernel trước.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="199"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="214"/>
        <location filename="../../src/IO/Drivers/USB.cpp" line="572"/>
        <source>USB Device Error</source>
        <translation>Lỗi Thiết Bị USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="215"/>
        <source>Could not claim interface %1 on the USB device.

Another driver or application may already have it open. On Linux, try unloading the kernel driver (e.g. cdc_acm) or adding a udev rule.</source>
        <translation>Không thể chiếm giao diện %1 trên thiết bị USB.

Trình điều khiển hoặc ứng dụng khác có thể đã mở nó. Trên Linux, thử gỡ trình điều khiển kernel (ví dụ: cdc_acm) hoặc thêm quy tắc udev.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="360"/>
        <source>Select Device</source>
        <translation>Chọn Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="379"/>
        <source>Select IN Endpoint</source>
        <translation>Chọn Điểm Cuối IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="390"/>
        <source>None (Read-only)</source>
        <translation>Không Có (Chỉ Đọc)</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="464"/>
        <source>Enable Advanced USB Control Transfers?</source>
        <translation>Bật Truyền Điều Khiển USB Nâng Cao?</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="465"/>
        <source>This enables control transfers in addition to bulk transfers. Sending incorrect control requests can crash or damage connected hardware. Only enable this if you know what you are doing.</source>
        <translation>Tính năng này bật truyền điều khiển bên cạnh truyền khối. Gửi yêu cầu điều khiển không đúng có thể làm hỏng hoặc gây hại cho phần cứng đã kết nối. Chỉ bật nếu bạn biết mình đang làm gì.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="469"/>
        <source>Advanced USB Mode</source>
        <translation>Chế Độ USB Nâng Cao</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="573"/>
        <source>The USB device was disconnected or encountered a fatal read error.</source>
        <translation>Thiết bị USB đã bị ngắt kết nối hoặc gặp lỗi đọc nghiêm trọng.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="710"/>
        <source>No isochronous IN endpoint was found on this device, but bulk endpoints are available.

Switch the Transfer Mode to "Bulk Stream" and try again.</source>
        <translation>Không tìm thấy điểm cuối IN đẳng thời trên thiết bị này, nhưng có sẵn các điểm cuối khối.

Chuyển Chế Độ Truyền sang "Luồng Khối" và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="715"/>
        <source>No bulk IN endpoint was found on this device, but isochronous endpoints are available.

Switch the Transfer Mode to "Isochronous" and try again.</source>
        <translation>Không tìm thấy điểm cuối IN khối trên thiết bị này, nhưng có sẵn các điểm cuối đẳng thời.

Chuyển Chế Độ Truyền sang "Đẳng Thời" và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="719"/>
        <source>No usable IN endpoint was found on this device.

The device may not expose data endpoints in its active configuration, or it may require a specific driver.</source>
        <translation>Không tìm thấy điểm cuối IN khả dụng trên thiết bị này.

Thiết bị có thể không hiển thị các điểm cuối dữ liệu trong cấu hình đang hoạt động hoặc có thể yêu cầu trình điều khiển cụ thể.</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1211"/>
        <source>USB Device</source>
        <translation>Thiết Bị USB</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1219"/>
        <source>Transfer Mode</source>
        <translation>Chế Độ Truyền</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Bulk Stream</source>
        <translation>Luồng Bulk</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Advanced Control</source>
        <translation>Điều Khiển Nâng Cao</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1222"/>
        <source>Isochronous</source>
        <translation>Isochronous</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1227"/>
        <source>IN Endpoint</source>
        <translation>Điểm Cuối IN</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1235"/>
        <source>OUT Endpoint</source>
        <translation>Điểm Cuối OUT</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/USB.cpp" line="1243"/>
        <source>ISO Packet Size</source>
        <translation>Kích Thước Gói ISO</translation>
    </message>
</context>
<context>
    <name>IO::FileTransmission</name>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="211"/>
        <source>No file selected…</source>
        <translation>Chưa chọn tệp…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="246"/>
        <source>Plain Text</source>
        <translation>Văn Bản Thuần</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="247"/>
        <source>Raw Binary</source>
        <translation>Nhị Phân Thô</translation>
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
        <translation>Chọn Tệp để Truyền</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="291"/>
        <source>File selected: %1 (%2 bytes)</source>
        <translation>Đã chọn tệp: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="294"/>
        <source>Error opening file: %1</source>
        <translation>Lỗi mở tệp: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="380"/>
        <source>Starting %1 transfer…</source>
        <translation>Đang bắt đầu truyền %1…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="608"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="628"/>
        <source>Transmission complete</source>
        <translation>Truyền hoàn tất</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="610"/>
        <source>Plain text transmission complete</source>
        <translation>Truyền văn bản thuần túy hoàn tất</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="630"/>
        <source>Raw binary transmission complete (%1 bytes)</source>
        <translation>Truyền nhị phân thô hoàn tất (%1 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="654"/>
        <source>Transfer complete</source>
        <translation>Truyền hoàn tất</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="655"/>
        <source>Transfer completed successfully (%1 bytes)</source>
        <translation>Truyền hoàn tất thành công (%1 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission.cpp" line="657"/>
        <location filename="../../src/IO/FileTransmission.cpp" line="658"/>
        <source>Transfer failed: %1</source>
        <translation>Truyền thất bại: %1</translation>
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
        <translation>Khung dữ liệu bị mất</translation>
    </message>
    <message>
        <location filename="../../src/IO/FrameReader.cpp" line="352"/>
        <source>Incoming data is arriving faster than Serial Studio can process it; %1 frame(s) have been dropped. Reduce the data rate or disable a heavy consumer.</source>
        <translation>Dữ liệu đến nhanh hơn khả năng xử lý của Serial Studio; %1 khung dữ liệu đã bị mất. Giảm tốc độ dữ liệu hoặc tắt các thành phần tiêu tốn nhiều tài nguyên.</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::XMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="83"/>
        <source>Cannot open file: %1</source>
        <translation>Không thể mở tệp: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="93"/>
        <source>Waiting for receiver…</source>
        <translation>Đang chờ bên nhận…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="112"/>
        <source>Transfer cancelled</source>
        <translation>Truyền đã bị hủy</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="113"/>
        <source>Transfer cancelled by user</source>
        <translation>Truyền đã bị hủy bởi người dùng</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="142"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Quá nhiều lần thử lại, truyền đã bị hủy bỏ</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="143"/>
        <source>Maximum retries exceeded</source>
        <translation>Đã vượt quá số lần thử lại tối đa</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="147"/>
        <source>NAK received, retrying block %1 (%2/%3)</source>
        <translation>Đã nhận NAK, thử lại khối %1 (%2/%3)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="155"/>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="384"/>
        <source>Failed to seek in file</source>
        <translation>Không thể tìm kiếm trong tệp</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="165"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Truyền đã bị hủy bởi bên nhận</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="166"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Bên nhận đã hủy truyền</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="180"/>
        <source>Transfer complete</source>
        <translation>Truyền hoàn tất</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="207"/>
        <source>Receiver ready (CRC mode), sending data…</source>
        <translation>Bên nhận đã sẵn sàng (chế độ CRC), đang gửi dữ liệu…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="302"/>
        <source>File read returned more data than requested</source>
        <translation>Đọc tệp trả về nhiều dữ liệu hơn yêu cầu</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="316"/>
        <source>Sending block %1 (%2 bytes)</source>
        <translation>Đang gửi khối %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="329"/>
        <source>Sending EOT…</source>
        <translation>Đang Gửi EOT…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="370"/>
        <source>Transfer timed out</source>
        <translation>Truyền tải hết thời gian chờ</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="371"/>
        <source>Timeout: no response from receiver</source>
        <translation>Hết thời gian chờ: không có phản hồi từ bên nhận</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/XMODEM.cpp" line="375"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Hết thời gian chờ, đang thử lại (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::YMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="62"/>
        <source>Cannot open file: %1</source>
        <translation>Không thể mở tệp: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="74"/>
        <source>Waiting for receiver…</source>
        <translation>Đang chờ bên nhận…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="96"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="173"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Truyền tải bị hủy bởi bên nhận</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="97"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="174"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Bên nhận đã hủy truyền tải</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="133"/>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="316"/>
        <source>Sending first EOT…</source>
        <translation>Đang gửi EOT đầu tiên…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="151"/>
        <source>Too many retries, transfer aborted</source>
        <translation>Quá nhiều lần thử lại, truyền tải đã bị hủy</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="152"/>
        <source>Maximum retries exceeded</source>
        <translation>Vượt quá số lần thử lại tối đa</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="156"/>
        <source>NAK received, retrying block %1</source>
        <translation>Đã nhận NAK, đang thử lại khối %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="161"/>
        <source>Failed to seek in file</source>
        <translation>Không thể tìm kiếm trong tệp</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="220"/>
        <source>Sending second EOT…</source>
        <translation>Đang gửi EOT thứ hai…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="242"/>
        <source>Transfer complete</source>
        <translation>Truyền tải hoàn tất</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="284"/>
        <source>Sending file header: %1 (%2 bytes)</source>
        <translation>Đang gửi tiêu đề tệp: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="299"/>
        <source>Sending end-of-batch marker…</source>
        <translation>Đang gửi dấu hiệu kết thúc lô…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/YMODEM.cpp" line="330"/>
        <source>Sending block %1 (%2/%3 bytes)</source>
        <translation>Đang gửi khối %1 (%2/%3 byte)</translation>
    </message>
</context>
<context>
    <name>IO::Protocols::ZMODEM</name>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="86"/>
        <source>Cannot open file: %1</source>
        <translation>Không thể mở tệp: %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="103"/>
        <source>File is too large for ZMODEM (%1 bytes, limit 4 GiB).</source>
        <translation>Tệp quá lớn cho ZMODEM (%1 byte, giới hạn 4 GiB).</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="128"/>
        <source>Transfer cancelled</source>
        <translation>Đã hủy truyền</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="129"/>
        <source>Transfer cancelled by user</source>
        <translation>Người dùng đã hủy truyền</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="269"/>
        <source>Hex header CRC mismatch, dropping frame</source>
        <translation>CRC tiêu đề hex không khớp, bỏ frame</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="420"/>
        <source>Sending ZRQINIT, waiting for receiver…</source>
        <translation>Đang gửi ZRQINIT, chờ bên nhận…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="444"/>
        <source>Sending file info: %1 (%2 bytes)</source>
        <translation>Đang gửi thông tin tệp: %1 (%2 byte)</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="459"/>
        <source>Failed to seek to offset %1</source>
        <translation>Không thể tìm đến vị trí %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="488"/>
        <source>File read returned more data than requested</source>
        <translation>Đọc tệp trả về nhiều dữ liệu hơn yêu cầu</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="514"/>
        <source>File data sent, waiting for confirmation…</source>
        <translation>Đã gửi dữ liệu tệp, chờ xác nhận…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="525"/>
        <source>Sending ZFIN…</source>
        <translation>Đang Gửi ZFIN…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="561"/>
        <source>Receiver ready, sending file info…</source>
        <translation>Bên nhận đã sẵn sàng, đang gửi thông tin tệp…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="571"/>
        <source>Receiver requests data from offset %1</source>
        <translation>Bên nhận yêu cầu dữ liệu từ vị trí %1</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="579"/>
        <source>Receiver skipped the file</source>
        <translation>Bên nhận đã bỏ qua tệp</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="591"/>
        <source>Too many errors, transfer aborted</source>
        <translation>Quá nhiều lỗi, truyền tải đã hủy bỏ</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="592"/>
        <source>Maximum retries exceeded</source>
        <translation>Vượt quá số lần thử lại tối đa</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="596"/>
        <source>NAK received, retrying (%1/%2)…</source>
        <translation>Nhận NAK, đang thử lại (%1/%2)…</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="617"/>
        <source>Transfer complete</source>
        <translation>Truyền tải hoàn tất</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="627"/>
        <source>Transfer cancelled by receiver</source>
        <translation>Truyền tải bị hủy bởi bên nhận</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="628"/>
        <source>Receiver cancelled the transfer</source>
        <translation>Bên nhận đã hủy truyền tải</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="636"/>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="637"/>
        <source>Receiver reported a file error</source>
        <translation>Bên nhận báo lỗi tệp</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="835"/>
        <source>Transfer timed out</source>
        <translation>Hết thời gian truyền</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="836"/>
        <source>Timeout: no response from receiver</source>
        <translation>Hết thời gian: không có phản hồi từ bên nhận</translation>
    </message>
    <message>
        <location filename="../../src/IO/FileTransmission/ZMODEM.cpp" line="840"/>
        <source>Timeout, retrying (%1/%2)…</source>
        <translation>Hết thời gian, đang thử lại (%1/%2)…</translation>
    </message>
</context>
<context>
    <name>IconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="41"/>
        <source>Select Icon</source>
        <translation>Chọn Biểu Tượng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="118"/>
        <source>Search Online…</source>
        <translation>Tìm Kiếm Trực Tuyến…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="131"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/IconPicker.qml" line="143"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
</context>
<context>
    <name>ImageView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="68"/>
        <source>Normal</source>
        <translation>Bình Thường</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="69"/>
        <source>Grayscale</source>
        <translation>Thang Độ Xám</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="70"/>
        <source>High Contrast</source>
        <translation>Độ Tương Phản Cao</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="71"/>
        <source>Vivid</source>
        <translation>Sống Động</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="72"/>
        <source>Night Vision</source>
        <translation>Tầm Nhìn Ban Đêm</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="73"/>
        <source>Infrared</source>
        <translation>Hồng Ngoại</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="74"/>
        <source>Deep Blue</source>
        <translation>Xanh Đậm</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="75"/>
        <source>Amber</source>
        <translation>Hổ Phách</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="172"/>
        <source>Export Images</source>
        <translation>Xuất Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="182"/>
        <source>Open Export Folder</source>
        <translation>Mở Thư Mục Xuất</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="198"/>
        <source>Zoom In</source>
        <translation>Phóng To</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="211"/>
        <source>Zoom Out</source>
        <translation>Thu Nhỏ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="231"/>
        <source>Show Crosshair</source>
        <translation>Hiển Thị Tâm Ngắm</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/ImageView.qml" line="556"/>
        <source>Waiting for Image…</source>
        <translation>Đang Chờ Hình Ảnh…</translation>
    </message>
</context>
<context>
    <name>KeyManagerDialog</name>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="23"/>
        <source>API Keys</source>
        <translation>Khóa API</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="47"/>
        <source>Anthropic Claude. The default is Claude Haiku 4.5 ($1 input / $5 output per million tokens). Sonnet 4.6 and Opus 4.7 are also available. Supports streaming, tool use, extended thinking, and prompt caching.</source>
        <translation>Anthropic Claude. Mặc định là Claude Haiku 4.5 ($1 đầu vào / $5 đầu ra trên triệu token). Sonnet 4.6 và OPUS 4.7 cũng khả dụng. Hỗ trợ streaming, sử dụng công cụ, suy nghĩ mở rộng và bộ nhớ đệm prompt.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="52"/>
        <source>OpenAI Chat Completions. The default is GPT-5 mini for fast, cost-conscious agentic work. GPT-5.2 is the stronger general-purpose option, and GPT-5.2 Chat tracks the model currently used in ChatGPT.</source>
        <translation>OpenAI Chat Completions. Mặc định là GPT-5 mini cho công việc tác nhân nhanh, tiết kiệm chi phí. GPT-5.2 là tùy chọn đa năng mạnh hơn, và GPT-5.2 Chat theo dõi mô hình hiện đang được sử dụng trong ChatGPT.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="57"/>
        <source>Google Gemini. The default is Gemini 2.0 Flash, which has a generous free tier (subject to rate limits). Gemini 1.5 Pro and Gemini 1.5 Flash are also available.</source>
        <translation>Google Gemini. Mặc định là Gemini 2.0 Flash, có gói miễn phí hào phóng (tuân theo giới hạn tốc độ). Gemini 1.5 Pro và Gemini 1.5 Flash cũng khả dụng.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="61"/>
        <source>DeepSeek. OpenAI-compatible API. The default is deepseek-chat (V3). deepseek-reasoner (R1) is also available. Often the cheapest cloud option for tool use.</source>
        <translation>DeepSeek. API tương thích OpenAI. Mặc định là deepseek-chat (V3). deepseek-reasoner (R1) cũng khả dụng. Thường là tùy chọn đám mây rẻ nhất cho sử dụng công cụ.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="65"/>
        <source>OpenRouter. One key, ~200 models from Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen, and others. Free-tier models (suffixed :free) are rate-limited but require no additional billing. Pay-as-you-go for the rest.</source>
        <translation>OpenRouter. Một khóa, ~200 mô hình từ Anthropic, OpenAI, Google, Meta, Mistral, DeepSeek, Qwen và các nhà cung cấp khác. Mô hình gói miễn phí (hậu tố :free) bị giới hạn tốc độ nhưng không yêu cầu thanh toán thêm. Trả theo mức sử dụng cho phần còn lại.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="70"/>
        <source>Groq. Hardware-accelerated inference (LPUs) for very fast Llama, Mixtral, Gemma, DeepSeek-R1 distill, and Qwen models. Generous free tier with daily token limits.</source>
        <translation>Groq. Suy luận tăng tốc phần cứng (LPU) cho các mô hình Llama, Mixtral, Gemma, DeepSeek-R1 distill và Qwen rất nhanh. Gói miễn phí hào phóng với giới hạn token hàng ngày.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="74"/>
        <source>Mistral. The default is Mistral Large. Codestral targets code completion, Pixtral handles vision, and the Ministral models are tuned for edge / low-latency use.</source>
        <translation>Mistral. Mặc định là Mistral Large. Codestral nhắm đến hoàn thiện mã, Pixtral xử lý hình ảnh, và các mô hình Ministral được tinh chỉnh cho sử dụng edge / độ trễ thấp.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="78"/>
        <source>Local model server. Works with any OpenAI-compatible endpoint -- Ollama, llama.cpp's llama-server, LM Studio, or vLLM. Nothing leaves your machine. The model list is queried live from the server.</source>
        <translation>Máy chủ mô hình cục bộ. Hoạt động với bất kỳ điểm cuối tương thích OpenAI nào -- Ollama, llama-server của llama.cpp, LM Studio, hoặc vLLM. Không có dữ liệu nào rời khỏi máy tính của bạn. Danh sách mô hình được truy vấn trực tiếp từ máy chủ.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="100"/>
        <source>Bring your own API keys. They are encrypted at rest with a per-machine key and never leave your computer except to communicate with the provider you select.</source>
        <translation>Sử dụng khóa API của riêng bạn. Chúng được mã hóa khi lưu trữ bằng khóa riêng cho từng máy và không bao giờ rời khỏi máy tính của bạn trừ khi giao tiếp với nhà cung cấp bạn chọn.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="166"/>
        <source>Key set</source>
        <translation>Đã đặt khóa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="167"/>
        <source>No key</source>
        <translation>Chưa có khóa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="204"/>
        <source>A key is on file -- paste a new one to replace it</source>
        <translation>Đã có khóa trong hồ sơ -- dán khóa mới để thay thế</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="205"/>
        <source>Paste your API key here</source>
        <translation>Dán khóa API của bạn vào đây</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="212"/>
        <source>Hide key</source>
        <translation>Ẩn khóa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="213"/>
        <source>Show key while typing</source>
        <translation>Hiển thị khóa khi nhập</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="224"/>
        <source>Get key</source>
        <translation>Lấy khóa</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="225"/>
        <source>Open the provider's console to create a new key</source>
        <translation>Mở bảng điều khiển của nhà cung cấp để tạo khóa mới</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="236"/>
        <source>Save</source>
        <translation>Lưu</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="254"/>
        <source>Remove the stored key for %1</source>
        <translation>Xóa khóa đã lưu cho %1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="278"/>
        <source>http://localhost:11434/v1</source>
        <translation>http://localhost:11434/v1</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="282"/>
        <source>Install Ollama</source>
        <translation>Cài Đặt Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="283"/>
        <source>Open the Ollama download page</source>
        <translation>Mở trang tải xuống Ollama</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="294"/>
        <source>Apply</source>
        <translation>Áp Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="309"/>
        <source>Re-query the model list</source>
        <translation>Truy vấn lại danh sách mô hình</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="357"/>
        <source>No API keys configured yet. Add a key to get started.</source>
        <translation>Chưa cấu hình khóa API nào. Thêm khóa để bắt đầu.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="360"/>
        <source>One provider is ready.</source>
        <translation>Một nhà cung cấp đã sẵn sàng.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/KeyManagerDialog.qml" line="362"/>
        <source>%1 providers are ready.</source>
        <translation>%1 nhà cung cấp đã sẵn sàng.</translation>
    </message>
</context>
<context>
    <name>LicenseManagement</name>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="37"/>
        <source>Licensing</source>
        <translation>Cấp Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="84"/>
        <source>Please wait…</source>
        <translation>Vui lòng đợi…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="124"/>
        <source>Activate Serial Studio Pro</source>
        <translation>Kích Hoạt Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="131"/>
        <source>Paste your license key below to unlock Pro features like MQTT, 3D plotting, and more.</source>
        <translation>Dán khóa cấp phép của bạn bên dưới để mở khóa các tính năng Pro như MQTT, biểu đồ 3D và nhiều hơn nữa.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="138"/>
        <source>Your license includes 5 device activations.
Plans include Monthly, Yearly, and Lifetime options.</source>
        <translation>Giấy phép của bạn bao gồm 5 lần kích hoạt thiết bị.
Các gói bao gồm tùy chọn Hàng tháng, Hàng năm và Trọn đời.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="151"/>
        <source>Paste your license key here…</source>
        <translation>Dán khóa cấp phép của bạn vào đây…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="170"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="333"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="382"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="176"/>
        <source>Paste</source>
        <translation>Dán</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="182"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="339"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="388"/>
        <source>Select All</source>
        <translation>Chọn Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="235"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="426"/>
        <source>Product</source>
        <translation>Sản Phẩm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="242"/>
        <source>Serial Studio %1</source>
        <translation>Serial Studio %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="253"/>
        <source>Licensee</source>
        <translation>Người Được Cấp Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="272"/>
        <source>Licensee E-Mail</source>
        <translation>E-mail Người Được Cấp Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="289"/>
        <source>Device Usage</source>
        <translation>Sử Dụng Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="297"/>
        <source>%1 devices in use (Unlimited plan)</source>
        <translation>%1 thiết bị đang sử dụng (Gói không giới hạn)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="298"/>
        <source>%1 of %2 devices used</source>
        <translation>Đã sử dụng %1 trong số %2 thiết bị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="308"/>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="443"/>
        <source>Device ID</source>
        <translation>ID Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="355"/>
        <source>License Key</source>
        <translation>Khóa Cấp Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="433"/>
        <source>Serial Studio %1 (Offline)</source>
        <translation>Serial Studio %1 (Ngoại Tuyến)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="460"/>
        <source>Expires</source>
        <translation>Hết Hạn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="467"/>
        <source>%1 (%2 days left)</source>
        <translation>%1 (còn %2 ngày)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="480"/>
        <source>Your offline license expires soon. Request a new certificate to stay activated.</source>
        <translation>Giấy phép ngoại tuyến của bạn sắp hết hạn. Yêu cầu chứng chỉ mới để duy trì kích hoạt.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="497"/>
        <source>Customer Portal</source>
        <translation>Cổng Khách Hàng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="508"/>
        <source>Buy License</source>
        <translation>Mua Giấy Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="515"/>
        <source>Activate</source>
        <translation>Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="526"/>
        <source>Activate Offline…</source>
        <translation>Kích Hoạt Ngoại Tuyến…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/LicenseManagement.qml" line="535"/>
        <source>Deactivate</source>
        <translation>Hủy Kích Hoạt</translation>
    </message>
</context>
<context>
    <name>Licensing::LemonSqueezy</name>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="512"/>
        <source>There was an issue validating your license.</source>
        <translation>Đã xảy ra sự cố khi xác thực giấy phép của bạn.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="530"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="711"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="830"/>
        <source>The license key you provided does not belong to Serial Studio.</source>
        <translation>Khóa giấy phép bạn cung cấp không thuộc về Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="531"/>
        <source>Please double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Vui lòng kiểm tra lại rằng bạn đã mua giấy phép từ cửa hàng chính thức của Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="542"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="720"/>
        <source>This license key was activated on a different device.</source>
        <translation>Khóa giấy phép này đã được kích hoạt trên thiết bị khác.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="543"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="721"/>
        <source>Deactivate it there first or contact support for help.</source>
        <translation>Hủy kích hoạt ở đó trước hoặc liên hệ bộ phận hỗ trợ để được trợ giúp.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="554"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="731"/>
        <source>This license is not currently active.</source>
        <translation>Giấy phép này hiện không hoạt động.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="555"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="732"/>
        <source>It may have expired or been deactivated (status: %1).</source>
        <translation>Có thể đã hết hạn hoặc bị hủy kích hoạt (trạng thái: %1).</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="565"/>
        <source>Something went wrong on the server.</source>
        <translation>Đã xảy ra sự cố trên máy chủ.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="566"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="742"/>
        <source>No activation ID was returned.</source>
        <translation>Không nhận được ID kích hoạt.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="576"/>
        <source>Could not validate your license at this time.</source>
        <translation>Không thể xác thực giấy phép của bạn vào lúc này.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="577"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="751"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="840"/>
        <source>Try again later.</source>
        <translation>Thử lại sau.</translation>
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
        <translation>Giấy phép của bạn đã được kích hoạt thành công.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="649"/>
        <source>Thank you for supporting Serial Studio!
You now have access to all premium features.</source>
        <translation>Cảm ơn bạn đã ủng hộ Serial Studio!
Bạn hiện có quyền truy cập vào tất cả các tính năng cao cấp.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="703"/>
        <source>There was an issue activating your license.</source>
        <translation>Đã xảy ra sự cố khi kích hoạt giấy phép của bạn.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="712"/>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="831"/>
        <source>Double-check that you purchased your license from the official Serial Studio store.</source>
        <translation>Kiểm tra kỹ rằng bạn đã mua giấy phép từ cửa hàng Serial Studio chính thức.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="741"/>
        <source>Something went wrong on the server…</source>
        <translation>Đã xảy ra lỗi trên máy chủ…</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="750"/>
        <source>Could not activate your license at this time.</source>
        <translation>Không thể kích hoạt giấy phép của bạn vào lúc này.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="822"/>
        <source>There was an issue deactivating your license.</source>
        <translation>Đã xảy ra sự cố khi hủy kích hoạt giấy phép của bạn.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="839"/>
        <source>Could not deactivate your license at this time.</source>
        <translation>Không thể hủy kích hoạt giấy phép của bạn vào lúc này.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="849"/>
        <source>Your license has been deactivated.</source>
        <translation>Giấy phép của bạn đã được hủy kích hoạt.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/LemonSqueezy.cpp" line="850"/>
        <source>Access to Pro features has been removed.
Thank you again for supporting Serial Studio!</source>
        <translation>Quyền truy cập vào các tính năng Pro đã bị xóa.
Cảm ơn bạn một lần nữa vì đã ủng hộ Serial Studio!</translation>
    </message>
</context>
<context>
    <name>Licensing::OfflineLicense</name>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="183"/>
        <source>Could not open the certificate file.</source>
        <translation>Không thể mở tệp chứng chỉ.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="190"/>
        <source>The certificate file is too large to be valid.</source>
        <translation>Tệp chứng chỉ quá lớn để hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="217"/>
        <source>Could not write the device file.</source>
        <translation>Không thể ghi tệp thiết bị.</translation>
    </message>
</context>
<context>
    <name>MDF4::Export</name>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="629"/>
        <source>MDF4 Export is a Pro feature.</source>
        <translation>Xuất MDF4 là tính năng Pro.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Export.cpp" line="630"/>
        <source>This feature requires a license. Please purchase one to enable MDF4 export.</source>
        <translation>Tính năng này yêu cầu giấy phép. Vui lòng mua giấy phép để bật xuất MDF4.</translation>
    </message>
</context>
<context>
    <name>MDF4::Player</name>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="399"/>
        <source>Select MDF4 file</source>
        <translation>Chọn tệp MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="401"/>
        <source>MDF4 files (*.mf4 *.dat)</source>
        <translation>Tệp MDF4 (*.mf4 *.dat)</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="423"/>
        <source>Disconnect from device?</source>
        <translation>Ngắt kết nối khỏi thiết bị?</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="424"/>
        <source>You must disconnect from the current device before opening a MDF4 file.</source>
        <translation>Bạn phải ngắt kết nối khỏi thiết bị hiện tại trước khi mở tệp MDF4.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="440"/>
        <source>Cannot open MDF4 file</source>
        <translation>Không thể mở tệp MDF4</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="441"/>
        <source>The file may be corrupted or in an unsupported format.</source>
        <translation>Tệp có thể bị hỏng hoặc ở định dạng không được hỗ trợ.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="448"/>
        <source>Invalid MDF4 file</source>
        <translation>Tệp MDF4 không hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="449"/>
        <source>Failed to read file structure. The file may be corrupted.</source>
        <translation>Không đọc được cấu trúc tệp. Tệp có thể bị hỏng.</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="464"/>
        <source>No data in file</source>
        <translation>Không có dữ liệu trong tệp</translation>
    </message>
    <message>
        <location filename="../../src/MDF4/Player.cpp" line="465"/>
        <source>The MDF4 file contains no measurement data.</source>
        <translation>Tệp MDF4 không chứa dữ liệu đo lường.</translation>
    </message>
</context>
<context>
    <name>MQTT</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="51"/>
        <source>Hostname</source>
        <translation>Tên Máy Chủ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="59"/>
        <source>e.g. broker.hivemq.com</source>
        <translation>vd. broker.hivemq.com</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="67"/>
        <source>Port</source>
        <translation>Cổng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="87"/>
        <source>Topic Filter</source>
        <translation>Bộ Lọc Topic</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="95"/>
        <source>e.g. sensors/#</source>
        <translation>ví dụ: sensors/#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="103"/>
        <source>Client ID</source>
        <translation>ID Client</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="120"/>
        <source>Regenerate</source>
        <translation>Tạo Lại</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="130"/>
        <source>Username</source>
        <translation>Tên Đăng Nhập</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="145"/>
        <source>Password</source>
        <translation>Mật Khẩu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="161"/>
        <source>Version</source>
        <translation>Phiên Bản</translation>
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
        <translation>Sử Dụng SSL/TLS</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="258"/>
        <source>SSL Protocol</source>
        <translation>Giao Thức SSL</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="289"/>
        <source>Peer Verify</source>
        <translation>Xác Minh Ngang Hàng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="320"/>
        <source>Verify Depth</source>
        <translation>Độ Sâu Xác Minh</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="341"/>
        <source>CA Certificates</source>
        <translation>Chứng Chỉ CA</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml" line="349"/>
        <source>Load From Folder…</source>
        <translation>Tải từ Thư Mục…</translation>
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
        <translation>TLS 1.3 hoặc Mới Hơn</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="799"/>
        <source>DTLS 1.2 or Later</source>
        <translation>DTLS 1.2 hoặc Mới Hơn</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="800"/>
        <source>Any Protocol</source>
        <translation>Bất Kỳ Giao Thức Nào</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="801"/>
        <source>Secure Protocols Only</source>
        <translation>Chỉ Giao Thức Bảo Mật</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="803"/>
        <source>None</source>
        <translation>Không Có</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="804"/>
        <source>Query Peer</source>
        <translation>Truy Vấn Đối Tác</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="805"/>
        <source>Verify Peer</source>
        <translation>Xác Minh Đối Tác</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="806"/>
        <source>Auto Verify Peer</source>
        <translation>Tự Động Xác Minh Đối Tác</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1123"/>
        <source>Raw RX Data</source>
        <translation>Dữ Liệu RX Thô</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1124"/>
        <source>Custom Script</source>
        <translation>Tập Lệnh Tùy Chỉnh</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1125"/>
        <source>Dashboard Data (CSV)</source>
        <translation>Dữ Liệu Bảng Điều Khiển (CSV)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1126"/>
        <source>Dashboard Data (JSON)</source>
        <translation>Dữ Liệu Bảng Điều Khiển (JSON)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1281"/>
        <source>MQTT publisher unavailable</source>
        <translation>Trình xuất bản MQTT không khả dụng</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1282"/>
        <source>A valid commercial license is required to use MQTT publishing.</source>
        <translation>Cần có giấy phép thương mại hợp lệ để sử dụng tính năng xuất bản MQTT.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1284"/>
        <location filename="../../src/MQTT/Publisher.cpp" line="1853"/>
        <source>MQTT Test Connection</source>
        <translation>Kiểm Tra Kết Nối MQTT</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1303"/>
        <source>Select PEM Certificates Directory</source>
        <translation>Chọn Thư Mục Chứng Chỉ PEM</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker reachable</source>
        <translation>Broker MQTT có thể truy cập</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1850"/>
        <source>MQTT broker unreachable</source>
        <translation>Broker MQTT không thể truy cập</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT broker connection failed</source>
        <translation>Kết nối broker MQTT thất bại</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="1864"/>
        <source>MQTT Publisher</source>
        <translation>Trình Xuất Bản MQTT</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherScriptEditor</name>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="50"/>
        <source>MQTT Publisher Script</source>
        <translation>Script Xuất Bản MQTT</translation>
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
        <translation>Byte khung mẫu (văn bản hoặc hex)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="98"/>
        <source>Hex</source>
        <translation>Hex</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="99"/>
        <source>Test</source>
        <translation>Kiểm Tra</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="100"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="102"/>
        <source>Apply</source>
        <translation>Áp Dụng</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="103"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="112"/>
        <source>Language:</source>
        <translation>Ngôn Ngữ:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="115"/>
        <source>Template:</source>
        <translation>Mẫu:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="126"/>
        <source>Frame:</source>
        <translation>Khung:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="130"/>
        <source>Output:</source>
        <translation>Đầu Ra:</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="270"/>
        <source>Enter a frame</source>
        <translation>Nhập một khung</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="277"/>
        <source>Invalid hex</source>
        <translation>Hex không hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="360"/>
        <source>Format Document	Ctrl+Shift+I</source>
        <translation>Định Dạng Tài Liệu	ctrl+shift+i</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="361"/>
        <source>Format Selection	Ctrl+I</source>
        <translation>Định Dạng Vùng Chọn	ctrl+i</translation>
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
-- Định nghĩa hàm mqtt(frame) nhận các byte thô
-- của một khung đã phân tích và trả về payload để xuất bản lên
-- broker, hoặc nil để bỏ qua khung này.
--
-- Tham số frame khớp với những gì script Frame Parser
-- nhìn thấy: một chuỗi Lua chứa các byte giữa các
-- dấu phân cách FrameReader.
--
-- Ví dụ:
--   function mqtt(frame)
--     return frame    -- chuyển tiếp nguyên trạng
--   end
--
-- Sử dụng menu Mẫu để xem các ví dụ có sẵn.
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
 * Định nghĩa hàm mqtt(frame) nhận các byte thô
 * của một khung đã phân tích và trả về payload để xuất bản lên
 * broker, hoặc null để bỏ qua khung này.
 *
 * Tham số frame khớp với những gì script Frame Parser
 * nhìn thấy: một chuỗi chứa các byte giữa các
 * dấu phân cách FrameReader.
 *
 * Ví dụ:
 *   function mqtt(frame) {
 *     return frame;   // chuyển tiếp nguyên trạng
 *   }
 *
 * Sử dụng menu Mẫu để xem các ví dụ có sẵn.
 */</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="615"/>
        <source>Script is empty</source>
        <translation>Script trống</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="622"/>
        <source>Lua engine error</source>
        <translation>Lỗi bộ máy Lua</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="644"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="658"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="682"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="696"/>
        <source>Error: %1</source>
        <translation>Lỗi: %1</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="652"/>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="688"/>
        <source>mqtt() is not defined</source>
        <translation>mqtt() chưa được định nghĩa</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="669"/>
        <source>(nil -- frame skipped)</source>
        <translation>(nil -- bỏ qua khung)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="671"/>
        <source>(non-string return)</source>
        <translation>(trả về không phải chuỗi)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="701"/>
        <source>(null -- frame skipped)</source>
        <translation>(null -- bỏ qua khung)</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/PublisherScriptEditor.cpp" line="779"/>
        <source>Select Template…</source>
        <translation>Chọn Mẫu…</translation>
    </message>
</context>
<context>
    <name>MQTT::PublisherWorker</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="674"/>
        <source>Configure broker hostname and port before testing the connection.</source>
        <translation>Cấu hình tên máy chủ và cổng broker trước khi kiểm tra kết nối.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="710"/>
        <source>Successfully connected to %1:%2.</source>
        <translation>Đã kết nối thành công tới %1:%2.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="721"/>
        <source>Timed out after 5 seconds without reaching the broker.</source>
        <translation>Hết thời gian chờ sau 5 giây mà không kết nối được broker.</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="205"/>
        <source>Console Only Mode</source>
        <translation>Chế Độ Chỉ Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="208"/>
        <source>Quick Plot Mode</source>
        <translation>Chế Độ Vẽ Đồ Thị Nhanh</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="215"/>
        <source>Empty Project</source>
        <translation>Dự Án Trống</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="696"/>
        <source>Serial Studio</source>
        <translation>Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="704"/>
        <source>Waiting for data…</source>
        <translation>Đang chờ dữ liệu…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/MainWindow.qml" line="705"/>
        <source>Connecting to device…</source>
        <translation>Đang kết nối với thiết bị…</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="146"/>
        <source>Code sample</source>
        <translation>Mẫu mã</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="148"/>
        <source>Completer</source>
        <translation>Trình Hoàn Thành</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="150"/>
        <source>Highlighter</source>
        <translation>Trình Tô Sáng Cú Pháp</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="152"/>
        <source>Style</source>
        <translation>Kiểu</translation>
    </message>
    <message>
        <location filename="../../../lib/QCodeEditor/example/src/MainWindow.cpp" line="214"/>
        <source> spaces</source>
        <translation>khoảng trắng</translation>
    </message>
</context>
<context>
    <name>MarkdownWebView</name>
    <message>
        <location filename="../../qml/Widgets/MarkdownWebView.qml" line="36"/>
        <source>Copied to Clipboard</source>
        <translation>Đã Sao Chép vào Clipboard</translation>
    </message>
</context>
<context>
    <name>Mdf4Player</name>
    <message>
        <location filename="../../qml/Dialogs/Mdf4Player.qml" line="23"/>
        <source>MDF4 Player</source>
        <translation>Trình Phát MDF4</translation>
    </message>
</context>
<context>
    <name>MessageBubble</name>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="97"/>
        <source>Error</source>
        <translation>Lỗi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>You</source>
        <translation>Bạn</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="98"/>
        <source>Assistant</source>
        <translation>Trợ Lý</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="208"/>
        <source>Discovery</source>
        <translation>Khám Phá</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="209"/>
        <source>Execution</source>
        <translation>Thực Thi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="239"/>
        <source>Approve %1 actions in %2?</source>
        <translation>Phê duyệt %1 hành động trong %2?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="249"/>
        <source>These calls will run together. Expand each card below to inspect arguments.</source>
        <translation>Các lệnh gọi này sẽ chạy cùng nhau. Mở rộng từng thẻ bên dưới để kiểm tra tham số.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="260"/>
        <source>Approve all</source>
        <translation>Chấp Nhận Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="266"/>
        <source>Deny all</source>
        <translation>Từ Chối Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="332"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="384"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="436"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="337"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="389"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="441"/>
        <source>Copy All</source>
        <translation>Sao Chép Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageBubble.qml" line="345"/>
        <location filename="../../qml/AI/MessageBubble.qml" line="397"/>
        <source>Select All</source>
        <translation>Chọn Tất Cả</translation>
    </message>
</context>
<context>
    <name>MessageWebView</name>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="63"/>
        <source>You</source>
        <translation>Bạn</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="64"/>
        <source>Assistant</source>
        <translation>Trợ Lý</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="65"/>
        <location filename="../../qml/AI/MessageWebView.qml" line="71"/>
        <source>Error</source>
        <translation>Lỗi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="66"/>
        <source>Discovery</source>
        <translation>Khám Phá</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="67"/>
        <source>Execution</source>
        <translation>Thực Thi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="68"/>
        <source>Running</source>
        <translation>Đang Chạy</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="69"/>
        <source>Awaiting approval</source>
        <translation>Chờ Phê Duyệt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="70"/>
        <source>Done</source>
        <translation>Hoàn Thành</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="72"/>
        <source>Denied</source>
        <translation>Đã từ Chối</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="73"/>
        <source>Blocked</source>
        <translation>Đã Chặn</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="74"/>
        <source>Approve</source>
        <translation>Phê Duyệt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="75"/>
        <source>Deny</source>
        <translation>Từ Chối</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="76"/>
        <source>Approve all</source>
        <translation>Phê Duyệt Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="77"/>
        <source>Deny all</source>
        <translation>Từ Chối Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="78"/>
        <source>Arguments</source>
        <translation>Tham Số</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="79"/>
        <source>Result</source>
        <translation>Kết Quả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="80"/>
        <source>Approve {n} actions in {family}?</source>
        <translation>Phê Duyệt {n} Hành Động trong {family}?</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="81"/>
        <source>These calls will run together. Expand each card to inspect arguments.</source>
        <translation>Các lệnh gọi này sẽ chạy cùng nhau. Mở rộng từng thẻ để kiểm tra tham số.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/MessageWebView.qml" line="82"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
</context>
<context>
    <name>Misc::Examples</name>
    <message>
        <location filename="../../src/Misc/Examples.cpp" line="282"/>
        <source>Failed to load README: %1</source>
        <translation>Không thể tải README: %1</translation>
    </message>
</context>
<context>
    <name>Misc::ExtensionManager</name>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="244"/>
        <source>Theme</source>
        <translation>Giao Diện</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="247"/>
        <source>Frame Parser</source>
        <translation>Bộ Phân Tích Frame</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="250"/>
        <source>Project Template</source>
        <translation>Mẫu Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="253"/>
        <source>Plugin</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="256"/>
        <source>All Types</source>
        <translation>Tất Cả Loại</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="473"/>
        <source>Reset Extensions</source>
        <translation>Đặt Lại Tiện Ích Mở Rộng</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="474"/>
        <source>This uninstalls all extensions, removes all custom repositories, and restores the default settings. Continue?</source>
        <translation>Thao tác này sẽ gỡ cài đặt tất cả tiện ích mở rộng, xóa tất cả kho tùy chỉnh và khôi phục cài đặt mặc định. Tiếp tục?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="513"/>
        <source>Select Extension Repository Folder</source>
        <translation>Chọn Thư Mục Kho Tiện Ích Mở Rộng</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1017"/>
        <source>Installed (repository no longer available)</source>
        <translation>Đã cài đặt (kho không còn khả dụng)</translation>
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
        <translation>Lỗi Plugin</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1324"/>
        <source>Plugin "%1" is not installed.</source>
        <translation>Plugin "%1" chưa được cài đặt.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1335"/>
        <source>Extension "%1" is not a plugin (type: %2).</source>
        <translation>Tiện ích mở rộng "%1" không phải là plugin (loại: %2).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1356"/>
        <source>Cannot read plugin metadata file:
%1/info.json</source>
        <translation>Không thể đọc tệp metadata của plugin:
%1/info.json</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1378"/>
        <source>Plugin "%1" requires gRPC but this build does not include gRPC support.</source>
        <translation>Plugin "%1" yêu cầu GRPC nhưng bản build này không bao gồm hỗ trợ GRPC.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1387"/>
        <source>This plugin uses gRPC for high-performance data streaming. The API server needs to be enabled.

Would you like to enable it now?</source>
        <translation>Plugin này sử dụng GRPC để truyền dữ liệu hiệu suất cao. Máy chủ API cần được bật.

Bạn có muốn bật ngay bây giờ không?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1390"/>
        <source>Plugins need the API server to communicate with Serial Studio. Would you like to enable it now?</source>
        <translation>Các plugin cần máy chủ API để giao tiếp với Serial Studio. Bạn có muốn bật nó ngay bây giờ không?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1393"/>
        <source>API Server Required</source>
        <translation>Yêu Cầu Máy Chủ API</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1422"/>
        <source>Plugin "%1" has no 'entry' field in info.json.</source>
        <translation>Plugin "%1" không có trường 'entry' trong info.json.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1432"/>
        <source>Entry point not found:
%1</source>
        <translation>Không tìm thấy điểm vào:
%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1441"/>
        <source>Plugin "%1" has an invalid entry point path.</source>
        <translation>Plugin "%1" có đường dẫn điểm vào không hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1484"/>
        <source>Missing Dependency</source>
        <translation>Thiếu Phụ Thuộc</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ExtensionManager.cpp" line="1485"/>
        <source>This plugin requires "%1" but it was not found on your system.

Would you like to open the download page?</source>
        <translation>Plugin này yêu cầu "%1" nhưng không tìm thấy trên hệ thống của bạn.

Bạn có muốn mở trang tải xuống không?</translation>
    </message>
</context>
<context>
    <name>Misc::GraphicsBackend</name>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="246"/>
        <source>Restart Required</source>
        <translation>Yêu Cầu Khởi Động Lại</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="247"/>
        <source>The new rendering backend will take effect after restarting Serial Studio. Restart now to apply the change?</source>
        <translation>Backend kết xuất mới sẽ có hiệu lực sau khi khởi động lại Serial Studio. Khởi động lại ngay để áp dụng thay đổi?</translation>
    </message>
</context>
<context>
    <name>Misc::HelpCenter</name>
    <message>
        <location filename="../../src/Misc/HelpCenter.cpp" line="303"/>
        <source>Failed to load page: %1</source>
        <translation>Không thể tải trang: %1</translation>
    </message>
</context>
<context>
    <name>Misc::IconEngine</name>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="152"/>
        <source>Invalid icon identifier</source>
        <translation>Mã định danh biểu tượng không hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/Misc/IconEngine.cpp" line="222"/>
        <source>Empty SVG data received</source>
        <translation>Nhận được dữ liệu SVG trống</translation>
    </message>
</context>
<context>
    <name>Misc::ShortcutGenerator</name>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="73"/>
        <source>Windows Shortcut (*.lnk)</source>
        <translation>Lối Tắt Windows (*.lnk)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="75"/>
        <source>macOS Application (*.app)</source>
        <translation>Ứng Dụng macOS (*.app)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="77"/>
        <source>Desktop Entry (*.desktop)</source>
        <translation>Desktop Entry (*.desktop)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="101"/>
        <source>Use a .icns icon for the sharpest result in Finder and the Dock.</source>
        <translation>Sử dụng biểu tượng .icns để có kết quả sắc nét nhất trong Finder và Dock.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="103"/>
        <source>Leave the icon empty to inherit the Serial Studio executable icon.</source>
        <translation>Để trống biểu tượng để kế thừa biểu tượng của tệp thực thi Serial Studio.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="105"/>
        <source>Place the file under ~/.local/share/applications/ to expose it in your application launcher.</source>
        <translation>Đặt tệp trong ~/.local/share/applications/ để hiển thị trong trình khởi chạy ứng dụng.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="116"/>
        <source>Apple Icon Image (*.icns)</source>
        <translation>Hình Ảnh Biểu Tượng Apple (*.icns)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="118"/>
        <source>Windows Icon (*.ico)</source>
        <translation>Biểu Tượng Windows (*.ico)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="120"/>
        <source>Vector or Raster Image (*.svg *.png)</source>
        <translation>Hình Ảnh VECTOR hoặc Raster (*.svg *.png)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="212"/>
        <source>A Pro license is required to generate shortcuts.</source>
        <translation>Cần có giấy phép Pro để tạo lối tắt.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="217"/>
        <source>No output path was provided.</source>
        <translation>Không có đường dẫn đầu ra được cung cấp.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator.cpp" line="258"/>
        <source>Failed to write shortcut file.</source>
        <translation>Không thể ghi tệp lối tắt.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="125"/>
        <source>Could not write the bundle launcher: %1</source>
        <translation>Không thể ghi trình khởi chạy bundle: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="144"/>
        <source>Could not mark the bundle launcher as executable.</source>
        <translation>Không thể đánh dấu trình khởi chạy bundle là tệp thực thi.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="164"/>
        <source>Could not write Info.plist: %1</source>
        <translation>Không thể ghi Info.plist: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="222"/>
        <source>Could not replace the existing shortcut at %1.</source>
        <translation>Không thể thay thế lối tắt hiện có tại %1.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="232"/>
        <source>Could not create the .app bundle directory layout.</source>
        <translation>Không thể tạo cấu trúc thư mục bundle .app.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="140"/>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="271"/>
        <source>Windows shortcut writer is not available on this platform.</source>
        <translation>Trình ghi lối tắt Windows không khả dụng trên nền tảng này.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_macOS.cpp" line="285"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="199"/>
        <source>Linux shortcut writer is not available on this platform.</source>
        <translation>Trình ghi lối tắt Linux không khả dụng trên nền tảng này.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="107"/>
        <source>Could not initialise COM (required to write .lnk shortcuts).</source>
        <translation>Không thể khởi tạo COM (cần thiết để ghi lối tắt .lnk).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="118"/>
        <source>CoCreateInstance(IShellLink) failed (HRESULT 0x%1).</source>
        <translation>CoCreateInstance(IShellLink) thất bại (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="153"/>
        <source>QueryInterface(IPersistFile) failed (HRESULT 0x%1).</source>
        <translation>QueryInterface(IPersistFile) thất bại (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="163"/>
        <source>Saving the .lnk file failed (HRESULT 0x%1).</source>
        <translation>Lưu tệp .lnk thất bại (HRESULT 0x%1).</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="154"/>
        <location filename="../../src/Misc/ShortcutGenerator_Windows.cpp" line="185"/>
        <source>macOS shortcut writer is not available on this platform.</source>
        <translation>Trình tạo lối tắt macOS không khả dụng trên nền tảng này.</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="86"/>
        <source>Could not open the shortcut path for writing: %1</source>
        <translation>Không thể mở đường dẫn lối tắt để ghi: %1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="91"/>
        <source>Serial Studio shortcut</source>
        <translation>Lối tắt Serial Studio</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ShortcutGenerator_Linux.cpp" line="112"/>
        <source>Could not mark the shortcut as executable.</source>
        <translation>Không thể đánh dấu lối tắt là thực thi được.</translation>
    </message>
</context>
<context>
    <name>Misc::ThemeManager</name>
    <message>
        <location filename="../../src/Misc/ThemeManager.cpp" line="398"/>
        <source>System</source>
        <translation>Hệ Thống</translation>
    </message>
</context>
<context>
    <name>Misc::Utilities</name>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="161"/>
        <source>Ok</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="163"/>
        <source>Save</source>
        <translation>Lưu</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="165"/>
        <source>Save all</source>
        <translation>Lưu Tất Cả</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="167"/>
        <source>Open</source>
        <translation>Mở</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="169"/>
        <source>Yes</source>
        <translation>Có</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="171"/>
        <source>Yes to all</source>
        <translation>Có Tất Cả</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="173"/>
        <source>No</source>
        <translation>Không</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="175"/>
        <source>No to all</source>
        <translation>Không Tất Cả</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="177"/>
        <source>Abort</source>
        <translation>Hủy Bỏ</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="179"/>
        <source>Retry</source>
        <translation>Thử Lại</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="181"/>
        <source>Ignore</source>
        <translation>Bỏ Qua</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="183"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="185"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="187"/>
        <source>Discard</source>
        <translation>Loại Bỏ</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="189"/>
        <source>Help</source>
        <translation>Trợ Giúp</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="191"/>
        <source>Apply</source>
        <translation>Áp Dụng</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="193"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../src/Misc/Utilities.cpp" line="195"/>
        <source>Restore defaults</source>
        <translation>Khôi phục mặc định</translation>
    </message>
</context>
<context>
    <name>Misc::WorkspaceManager</name>
    <message>
        <location filename="../../src/Misc/WorkspaceManager.cpp" line="261"/>
        <source>Select Workspace Location</source>
        <translation>Chọn Vị Trí Không Gian Làm Việc</translation>
    </message>
</context>
<context>
    <name>Modbus</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="49"/>
        <source>Protocol</source>
        <translation>Giao Thức</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="70"/>
        <source>Serial Port</source>
        <translation>Cổng Nối Tiếp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="93"/>
        <source>Baud Rate</source>
        <translation>Tốc Độ Baud</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="184"/>
        <source>Parity</source>
        <translation>Chẵn Lẻ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="205"/>
        <source>Data Bits</source>
        <translation>Bit Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="226"/>
        <source>Stop Bits</source>
        <translation>Bit Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="247"/>
        <source>Host</source>
        <translation>Máy Chủ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="257"/>
        <source>IP Address</source>
        <translation>Địa Chỉ IP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="271"/>
        <source>Port</source>
        <translation>Cổng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="280"/>
        <source>TCP Port</source>
        <translation>Cổng TCP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="308"/>
        <source>Slave Address</source>
        <translation>Địa Chỉ Slave</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="313"/>
        <source>1-247</source>
        <translation>1-247</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="328"/>
        <source>Poll Interval (ms)</source>
        <translation>Chu Kỳ Truy Vấn (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="333"/>
        <source>Polling interval</source>
        <translation>Khoảng thời gian truy vấn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="361"/>
        <source>Configure Register Groups…</source>
        <translation>Cấu Hình Nhóm Thanh Ghi…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="371"/>
        <source>Import Register Map…</source>
        <translation>Nhập Bản Đồ Thanh Ghi…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="386"/>
        <source>%1 group(s) configured</source>
        <translation>Đã cấu hình %1 nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Modbus.qml" line="387"/>
        <source>No groups configured</source>
        <translation>Chưa cấu hình nhóm nào</translation>
    </message>
</context>
<context>
    <name>ModbusGroupsDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="45"/>
        <source>Modbus Register Groups</source>
        <translation>Nhóm Thanh Ghi Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="166"/>
        <source>Configure multiple register groups to poll different register types in sequence.</source>
        <translation>Cấu hình nhiều nhóm thanh ghi để truy vấn các loại thanh ghi khác nhau theo trình tự.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="174"/>
        <source>Add New Group</source>
        <translation>Thêm Nhóm Mới</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="198"/>
        <source>Register Type:</source>
        <translation>Loại Thanh Ghi:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="210"/>
        <source>Start Address:</source>
        <translation>Địa Chỉ Bắt Đầu:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="217"/>
        <source>0-65535</source>
        <translation>0-65535</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="223"/>
        <source>Register Count:</source>
        <translation>Số Lượng Thanh Ghi:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="234"/>
        <source>1-125</source>
        <translation>1-125</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="239"/>
        <source>Add Group</source>
        <translation>Thêm Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="262"/>
        <source>Configured Groups</source>
        <translation>Các Nhóm Đã Cấu Hình</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="296"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="303"/>
        <source>Type</source>
        <translation>Loại</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="311"/>
        <source>Start</source>
        <translation>Bắt Đầu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="318"/>
        <source>Count</source>
        <translation>Số Lượng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="325"/>
        <source>Action</source>
        <translation>Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="400"/>
        <source>Remove</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="412"/>
        <source>No groups configured.
Add groups above to poll multiple register types.</source>
        <translation>Chưa cấu hình nhóm.
Thêm nhóm ở trên để truy vấn nhiều loại thanh ghi.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="430"/>
        <source>Total groups: %1</source>
        <translation>Tổng số nhóm: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="434"/>
        <source>Generate Project</source>
        <translation>Tạo Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="440"/>
        <source>Clear All</source>
        <translation>Xóa Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusGroupsDialog.qml" line="446"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>ModbusPreviewDialog</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="33"/>
        <source>Modbus Register Map Preview</source>
        <translation>Xem Trước Sơ Đồ Thanh Ghi Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="155"/>
        <source>File: %1</source>
        <translation>Tệp: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="163"/>
        <source>Review the registers to import into a new Serial Studio project.</source>
        <translation>Xem lại các thanh ghi để nhập vào dự án Serial Studio mới.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="171"/>
        <source>Registers</source>
        <translation>Thanh Ghi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="205"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="212"/>
        <source>Name</source>
        <translation>Tên</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="221"/>
        <source>Address</source>
        <translation>Địa Chỉ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="227"/>
        <source>Type</source>
        <translation>Loại</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="235"/>
        <source>Data Type</source>
        <translation>Kiểu Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="242"/>
        <source>Units</source>
        <translation>Đơn Vị</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="343"/>
        <source>No registers found in file.</source>
        <translation>Không tìm thấy thanh ghi nào trong tệp.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="361"/>
        <source>Total: %1 registers in %2 groups</source>
        <translation>Tổng: %1 thanh ghi trong %2 nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="367"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml" line="379"/>
        <source>Create Project</source>
        <translation>Tạo Dự Án</translation>
    </message>
</context>
<context>
    <name>MqttPublisherView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="33"/>
        <source>MQTT Publisher</source>
        <translation>Nhà Xuất Bản MQTT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="110"/>
        <source>Connected to broker</source>
        <translation>Đã kết nối với broker</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="111"/>
        <source>Not connected</source>
        <translation>Chưa kết nối</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="124"/>
        <source>Test Connection</source>
        <translation>Kiểm Tra Kết Nối</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="129"/>
        <source>Probe the broker with the current settings</source>
        <translation>Thăm dò broker với cài đặt hiện tại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="130"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="147"/>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="162"/>
        <source>Enable publishing first</source>
        <translation>Bật xuất bản trước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="140"/>
        <source>Edit Script</source>
        <translation>Chỉnh Sửa Script</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="146"/>
        <source>Edit the publisher script (Lua or JavaScript)</source>
        <translation>Chỉnh sửa script nhà xuất bản (Lua hoặc JavaScript)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="158"/>
        <source>Load CA Certs</source>
        <translation>Tải Chứng Chỉ CA</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="164"/>
        <source>Add PEM certificates from a folder</source>
        <translation>Thêm chứng chỉ PEM từ một thư mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MqttPublisherView.qml" line="165"/>
        <source>Enable SSL/TLS first</source>
        <translation>Bật SSL/TLS trước</translation>
    </message>
</context>
<context>
    <name>MultiPlot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="305"/>
        <source>Interpolation: %1</source>
        <translation>Nội Suy: %1</translation>
    </message>
    <message>
        <source>Show Legends</source>
        <translation type="vanished">Hiển Thị Chú Giải</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="327"/>
        <source>Show X Axis Label</source>
        <translation>Hiển Thị Nhãn trục X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="338"/>
        <source>Show Y Axis Label</source>
        <translation>Hiển Thị Nhãn trục Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="350"/>
        <source>Show Crosshair</source>
        <translation>Hiển Thị Tâm Ngắm</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="357"/>
        <source>Resume</source>
        <translation>Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="374"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Chế Độ Quét / Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="386"/>
        <source>Trigger Settings</source>
        <translation>Cài Đặt Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="410"/>
        <source>Reset View</source>
        <translation>Đặt Lại Chế Độ Xem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/MultiPlot.qml" line="416"/>
        <source>Axis Range Settings</source>
        <translation>Cài Đặt Phạm Vi trục</translation>
    </message>
    <message>
        <source>Samples</source>
        <translation type="vanished">Mẫu</translation>
    </message>
</context>
<context>
    <name>MultiSelectionView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="35"/>
        <source>%1 items selected</source>
        <translation>Đã chọn %1 mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="118"/>
        <source>Plots</source>
        <translation>Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="123"/>
        <source>Plot</source>
        <translation>Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="127"/>
        <source>Toggle 2D plot for every selected dataset</source>
        <translation>Bật/tắt biểu đồ 2D cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="139"/>
        <source>FFT Plot</source>
        <translation>Biểu Đồ FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="142"/>
        <source>Toggle FFT plot for every selected dataset</source>
        <translation>Bật/tắt biểu đồ FFT cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="154"/>
        <source>Waterfall</source>
        <translation>Thác Nước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="157"/>
        <source>Toggle waterfall for every selected dataset</source>
        <translation>Bật/tắt thác nước cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="173"/>
        <source>Widgets</source>
        <translation>Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="179"/>
        <source>Bar/Level</source>
        <translation>Thanh/mức</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="182"/>
        <source>Set bar/level for every selected dataset</source>
        <translation>Đặt thanh/mức cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="193"/>
        <source>Gauge</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="197"/>
        <source>Set gauge for every selected dataset</source>
        <translation>Đặt đồng hồ đo cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="209"/>
        <source>Compass</source>
        <translation>La Bàn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="212"/>
        <source>Set compass for every selected dataset</source>
        <translation>Đặt la bàn cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="223"/>
        <source>Meter</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="227"/>
        <source>Set meter for every selected dataset</source>
        <translation>Đặt đồng hồ đo cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="238"/>
        <source>LED</source>
        <translation>LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="242"/>
        <source>Toggle LED for every selected dataset</source>
        <translation>Bật/tắt LED cho mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="266"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="268"/>
        <source>Duplicate every selected dataset</source>
        <translation>Nhân bản mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="276"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="279"/>
        <source>Delete every selected dataset</source>
        <translation>Xóa mọi tập dữ liệu đã chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/MultiSelectionView.qml" line="319"/>
        <source>Editing %1 items. Shared fields apply to all; per-item fields are locked.</source>
        <translation>Đang chỉnh sửa %1 mục. Các trường dùng chung áp dụng cho tất cả; các trường riêng lẻ bị khóa.</translation>
    </message>
</context>
<context>
    <name>NativeTemplates</name>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="292"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="430"/>
        <source>Bytes per value</source>
        <translation>Số byte trên mỗi giá trị</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="293"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="431"/>
        <source>Number of bytes combined into each channel value.</source>
        <translation>Số byte được kết hợp thành mỗi giá trị kênh.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="301"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="439"/>
        <source>Endianness</source>
        <translation>Thứ Tự Byte</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="302"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="440"/>
        <source>Byte order used when combining multi-byte values.</source>
        <translation>Thứ tự byte được sử dụng khi kết hợp các giá trị nhiều byte.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="310"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="448"/>
        <source>Signed values</source>
        <translation>Giá trị có dấu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="311"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="449"/>
        <source>Interprets each value as two's-complement signed.</source>
        <translation>Diễn giải mỗi giá trị dưới dạng số có dấu bù hai.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="651"/>
        <source>Tag routing table</source>
        <translation>Bảng định tuyến thẻ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="652"/>
        <source>Comma-separated tag:index entries, e.g. 1:0,2:1,3:2. Tags may be decimal or 0x-prefixed hex.</source>
        <translation>Các mục tag:index phân tách bằng dấu phẩy, ví dụ 1:0,2:1,3:2. Thẻ có thể là số thập phân hoặc số hex có tiền tố 0x.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1096"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1300"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1245"/>
        <source>Validate checksum</source>
        <translation>Xác thực checksum</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1097"/>
        <source>Rejects messages with an invalid Fletcher checksum.</source>
        <translation>Từ chối các thông điệp có checksum Fletcher không hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1301"/>
        <source>Rejects messages with an invalid additive checksum.</source>
        <translation>Từ chối các thông điệp có checksum cộng dồn không hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1454"/>
        <source>Protocol version</source>
        <translation>Phiên bản giao thức</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1455"/>
        <source>Selects the expected start marker (0xFE for v1, 0xFD for v2).</source>
        <translation>Chọn dấu hiệu bắt đầu mong đợi (0xFE cho v1, 0xFD cho v2).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1883"/>
        <source>Validate CRC</source>
        <translation>Xác Thực CRC</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="1884"/>
        <source>Rejects frames with an invalid CRC-24Q checksum.</source>
        <translation>Từ chối các frame có checksum CRC-24Q không hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2059"/>
        <source>Channel count</source>
        <translation>Số lượng kênh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2060"/>
        <source>Number of output channels (registers or coils).</source>
        <translation>Số lượng kênh đầu ra (thanh ghi hoặc coil).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2068"/>
        <source>Register offset</source>
        <translation>Offset thanh ghi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2069"/>
        <source>Address offset subtracted from single-write echoes (40001 for legacy Modicon maps).</source>
        <translation>Offset địa chỉ được trừ khỏi echo ghi đơn (40001 cho bản đồ Modicon cũ).</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2079"/>
        <source>Signed registers</source>
        <translation>Thanh ghi có dấu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2080"/>
        <source>Interprets 16-bit registers as two's-complement signed values.</source>
        <translation>Diễn giải thanh ghi 16-bit dưới dạng giá trị có dấu bù hai.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2386"/>
        <source>Payload layout</source>
        <translation>Bố cục payload</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2387"/>
        <source>Array emits every element in order; map routes keys through the key list.</source>
        <translation>Mảng phát ra từng phần tử theo thứ tự; map định tuyến các khóa qua danh sách khóa.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2397"/>
        <source>Keys (map mode)</source>
        <translation>Khóa (chế độ map)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp" line="2398"/>
        <source>Comma-separated map keys in channel order. Only used for the map layout.</source>
        <translation>Các khóa map phân tách bằng dấu phẩy theo thứ tự kênh. Chỉ dùng cho bố cục map.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="184"/>
        <source>Scalar fields</source>
        <translation>Trường vô hướng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="185"/>
        <source>Comma-separated JSON fields repeated in every generated frame.</source>
        <translation>Các trường JSON phân tách bằng dấu phẩy được lặp lại trong mỗi khung được tạo.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="192"/>
        <source>Sample array field</source>
        <translation>Trường mảng mẫu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="193"/>
        <source>JSON field holding the batched sample array.</source>
        <translation>Trường JSON chứa mảng mẫu theo lô.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="334"/>
        <source>Records array field</source>
        <translation>Trường mảng bản ghi</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="335"/>
        <source>JSON field holding the array of record objects.</source>
        <translation>Trường JSON chứa mảng các đối tượng bản ghi.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="341"/>
        <source>Record fields (in channel order)</source>
        <translation>Trường bản ghi (theo thứ tự kênh)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/MultiFrameTemplates.cpp" line="342"/>
        <source>Comma-separated record fields. The position of each field sets its channel index.</source>
        <translation>Các trường bản ghi phân tách bằng dấu phẩy. Vị trí của mỗi trường xác định chỉ số kênh của nó.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="605"/>
        <source>Column widths</source>
        <translation>Độ Rộng Cột</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="606"/>
        <source>Comma-separated character counts per field. Leave empty to split on whitespace.</source>
        <translation>Số ký tự phân tách bằng dấu phẩy cho mỗi trường. Để trống để tách theo khoảng trắng.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="614"/>
        <source>Trim whitespace</source>
        <translation>Loại Bỏ Khoảng Trắng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="615"/>
        <source>Removes padding around every sliced field.</source>
        <translation>Xóa khoảng đệm xung quanh mỗi trường đã tách.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="744"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="893"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1360"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1787"/>
        <source>Keys (in channel order)</source>
        <translation>Khóa (theo thứ tự kênh)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="745"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="894"/>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1788"/>
        <source>Comma-separated key names. The position of each key sets its channel index.</source>
        <translation>Tên khóa phân tách bằng dấu phẩy. Vị trí của mỗi khóa xác định chỉ số kênh của nó.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="753"/>
        <source>Pair separator</source>
        <translation>Ký Tự Phân Tách Cặp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="754"/>
        <source>Character between key=value pairs.</source>
        <translation>Ký tự giữa các cặp khóa=giá_trị.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="760"/>
        <source>Key-value separator</source>
        <translation>Ký Tự Phân Tách Khóa-Giá Trị</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="761"/>
        <source>Character between a key and its value.</source>
        <translation>Ký tự giữa khóa và giá trị của nó.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="767"/>
        <source>Numeric values only</source>
        <translation>Chỉ giá trị số</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="768"/>
        <source>Ignores pairs whose value is not a number.</source>
        <translation>Bỏ qua các cặp có giá trị không phải là số.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1010"/>
        <source>Command routing table</source>
        <translation>Bảng định tuyến lệnh</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1011"/>
        <source>Semicolon-separated entries of NAME:index list, e.g. CSQ:0,1;CREG:2,3;CGATT:4.</source>
        <translation>Danh sách NAME:index phân tách bằng dấu chấm phẩy, ví dụ: CSQ:0,1;CREG:2,3;CGATT:4.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1236"/>
        <source>Talker prefix</source>
        <translation>Tiền tố talker</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1237"/>
        <source>Two-letter talker id, e.g. GP for GPS or GN for multi-constellation receivers.</source>
        <translation>ID talker hai chữ cái, ví dụ: GP cho GPS hoặc GN cho bộ thu đa chòm sao.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1246"/>
        <source>Rejects sentences whose *hh checksum does not match.</source>
        <translation>Từ chối các câu có checksum *hh không khớp.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1361"/>
        <source>Comma-separated parameter names. The position of each key sets its channel index.</source>
        <translation>Tên tham số phân tách bằng dấu phẩy. Vị trí của mỗi khóa xác định chỉ số kênh của nó.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1500"/>
        <source>Fields (in channel order)</source>
        <translation>Trường (theo thứ tự kênh)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1501"/>
        <source>Comma-separated field names. The position of each field sets its channel index.</source>
        <translation>Tên trường phân tách bằng dấu phẩy. Vị trí của mỗi trường xác định chỉ số kênh của nó.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1620"/>
        <source>Tags (in channel order)</source>
        <translation>Thẻ (theo thứ tự kênh)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/NativeTemplates/TextTemplates.cpp" line="1621"/>
        <source>Comma-separated tag names. The position of each tag sets its channel index.</source>
        <translation>Tên thẻ phân tách bằng dấu phẩy. Vị trí của mỗi thẻ xác định chỉ số kênh của nó.</translation>
    </message>
    <message>
        <source>Register blocks</source>
        <translation type="vanished">Khối Thanh Ghi</translation>
    </message>
    <message>
        <source>Polled register blocks with typed, scaled entries. Written by the Modbus register map importer.</source>
        <translation type="vanished">Khối thanh ghi được truy vấn với các mục có kiểu và tỷ lệ. Được tạo bởi trình nhập bản đồ thanh ghi Modbus.</translation>
    </message>
    <message>
        <source>Signal map</source>
        <translation type="vanished">Bản Đồ Tín Hiệu</translation>
    </message>
    <message>
        <source>CAN messages with their signal bit layouts and scaling. Written by the DBC importer.</source>
        <translation type="vanished">Thông điệp CAN với bố cục bit tín hiệu và tỷ lệ của chúng. Được tạo bởi trình nhập DBC.</translation>
    </message>
</context>
<context>
    <name>Network</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="78"/>
        <source>Socket Type</source>
        <translation>Loại Socket</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="99"/>
        <source>Local Port</source>
        <translation>Cổng Cục Bộ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="106"/>
        <source>Type 0 for automatic port</source>
        <translation>Nhập 0 để tự động chọn cổng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="132"/>
        <source>Remote Address</source>
        <translation>Địa Chỉ từ Xa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="156"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml" line="189"/>
        <source>Remote Port</source>
        <translation>Cổng từ Xa</translation>
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
        <translation>Lọc theo kênh…</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="187"/>
        <source>Clear all notifications</source>
        <translation>Xóa tất cả thông báo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="271"/>
        <source>(no title)</source>
        <translation>(không có tiêu đề)</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/NotificationLog.qml" line="329"/>
        <source>No notifications yet</source>
        <translation>Chưa có thông báo nào</translation>
    </message>
</context>
<context>
    <name>OfflineActivation</name>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="37"/>
        <source>Activate Offline</source>
        <translation>Kích Hoạt Ngoại Tuyến</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="55"/>
        <source>Activate Serial Studio Pro on a machine with no internet access. No account or connection is needed on this computer.</source>
        <translation>Kích hoạt Serial Studio Pro trên máy không có kết nối internet. Không cần tài khoản hoặc kết nối trên máy tính này.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="86"/>
        <source>Save your device file</source>
        <translation>Lưu tệp thiết bị của bạn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="95"/>
        <source>Save this computer's device file. It identifies this machine and contains no personal information.</source>
        <translation>Lưu tệp thiết bị của máy tính này. Tệp này xác định máy này và không chứa thông tin cá nhân.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="102"/>
        <source>Save Device File…</source>
        <translation>Lưu Tệp Thiết Bị…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="136"/>
        <source>Get your license file</source>
        <translation>Lấy tệp giấy phép của bạn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="145"/>
        <source>On another computer with internet access, go to the address below, upload the device file, and enter your email and license key.</source>
        <translation>Trên máy tính khác có kết nối internet, truy cập địa chỉ bên dưới, tải lên tệp thiết bị, và nhập email cùng khóa giấy phép của bạn.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="157"/>
        <source>Open in Browser</source>
        <translation>Mở trong Trình Duyệt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="193"/>
        <source>Import your license</source>
        <translation>Nhập giấy phép của bạn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="202"/>
        <source>Copy the license file to this computer and import it. Pro features unlock immediately.</source>
        <translation>Sao chép tệp giấy phép vào máy tính này và nhập vào. Các tính năng Pro sẽ được mở khóa ngay lập tức.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="209"/>
        <source>Import License File…</source>
        <translation>Nhập Tệp Giấy Phép…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="224"/>
        <source>Save Device File</source>
        <translation>Lưu Tệp Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="226"/>
        <source>Serial Studio device file (*.ssmachine)</source>
        <translation>Tệp thiết bị Serial Studio (*.ssmachine)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="235"/>
        <source>Import License File</source>
        <translation>Nhập Tệp Giấy Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="236"/>
        <source>Serial Studio license (*.sslic)</source>
        <translation>Giấy phép Serial Studio (*.sslic)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OfflineActivation.qml" line="236"/>
        <source>All files (*)</source>
        <translation>Tất cả tệp (*)</translation>
    </message>
</context>
<context>
    <name>OfflineLicense</name>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="192"/>
        <source>License certificate is valid.</source>
        <translation>Chứng chỉ giấy phép hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="194"/>
        <source>The certificate signature is invalid or corrupted.</source>
        <translation>Chữ ký chứng chỉ không hợp lệ hoặc bị hỏng.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="197"/>
        <source>This certificate was issued for a different device.</source>
        <translation>Chứng chỉ này được cấp cho thiết bị khác.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="200"/>
        <source>This certificate has expired.</source>
        <translation>Chứng chỉ này đã hết hạn.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="202"/>
        <source>This certificate does not grant a valid license tier.</source>
        <translation>Chứng chỉ này không cấp cấp độ giấy phép hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineCertificate.cpp" line="206"/>
        <source>The certificate file is malformed or unreadable.</source>
        <translation>Tệp chứng chỉ bị lỗi hoặc không thể đọc được.</translation>
    </message>
</context>
<context>
    <name>OnlineIconPicker</name>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="41"/>
        <source>Search Online Icons</source>
        <translation>Tìm Biểu Tượng Trực Tuyến</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="71"/>
        <source>Download failed: %1</source>
        <translation>Tải xuống thất bại: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="96"/>
        <source>Search icons (e.g. temperature, arrow, play)…</source>
        <translation>Tìm biểu tượng (ví dụ: nhiệt độ, mũi tên, phát)…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="108"/>
        <source>Search</source>
        <translation>Tìm Kiếm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="147"/>
        <source>Search for icons above to get started</source>
        <translation>Tìm kiếm biểu tượng ở trên để bắt đầu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="248"/>
        <source>OK</source>
        <translation>OK</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/OnlineIconPicker.qml" line="258"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
</context>
<context>
    <name>OutputWidgetView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="91"/>
        <source>Output widgets require a Pro license.</source>
        <translation>Widget đầu ra yêu cầu giấy phép Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="93"/>
        <source>You can configure output widgets, but they only appear on the dashboard with a Pro license.</source>
        <translation>Bạn có thể cấu hình widget đầu ra, nhưng chúng chỉ xuất hiện trên bảng điều khiển với giấy phép Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="125"/>
        <source>Button</source>
        <translation>Nút</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="129"/>
        <source>Send a command on click</source>
        <translation>Gửi lệnh khi nhấp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="134"/>
        <source>Slider</source>
        <translation>Thanh Trượt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="138"/>
        <source>Send scaled numeric values</source>
        <translation>Gửi giá trị số đã chia tỷ lệ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="143"/>
        <source>Toggle</source>
        <translation>Công Tắc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="147"/>
        <source>Send on/off commands</source>
        <translation>Gửi lệnh bật/tắt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="152"/>
        <source>Text Field</source>
        <translation>Trường Văn Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="156"/>
        <source>Type and send arbitrary commands</source>
        <translation>Nhập và gửi lệnh tùy ý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="160"/>
        <source>Knob</source>
        <translation>Núm Xoay</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="165"/>
        <source>Rotary input for setpoints</source>
        <translation>Đầu vào xoay cho điểm đặt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="182"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="185"/>
        <source>Duplicate this output widget</source>
        <translation>Nhân bản widget đầu ra này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="195"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="197"/>
        <source>Delete this output widget</source>
        <translation>Xóa widget đầu ra này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="274"/>
        <source>Transmit Function</source>
        <translation>Hàm Truyền</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="284"/>
        <source>Import</source>
        <translation>Nhập</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="290"/>
        <source>Import transmit function from a .js file</source>
        <translation>Nhập hàm truyền từ tệp .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="297"/>
        <source>Template</source>
        <translation>Mẫu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="301"/>
        <source>Select a pre-built transmit function template</source>
        <translation>Chọn mẫu hàm truyền có sẵn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="306"/>
        <source>Test</source>
        <translation>Kiểm Tra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="312"/>
        <source>Test the transmit function with sample input</source>
        <translation>Kiểm tra chức năng truyền với dữ liệu mẫu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="353"/>
        <source>Undo</source>
        <translation>Hoàn Tác</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="359"/>
        <source>Redo</source>
        <translation>Làm Lại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="367"/>
        <source>Cut</source>
        <translation>Cắt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="372"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="377"/>
        <source>Paste</source>
        <translation>Dán</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="384"/>
        <source>Select All</source>
        <translation>Chọn Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="391"/>
        <source>Format Document</source>
        <translation>Định Dạng Tài Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/OutputWidgetView.qml" line="396"/>
        <source>Format Selection</source>
        <translation>Định Dạng Vùng Chọn</translation>
    </message>
</context>
<context>
    <name>Painter</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Painter.qml" line="56"/>
        <source>Painter Widget Error</source>
        <translation>Lỗi Widget Painter</translation>
    </message>
</context>
<context>
    <name>PainterCodeDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="30"/>
        <source>Painter Widget Code Editor</source>
        <translation>Trình Soạn Mã Widget Vẽ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="76"/>
        <source>paint(ctx, w, h)</source>
        <translation>paint(ctx, w, h)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="86"/>
        <source>Import</source>
        <translation>Nhập</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="92"/>
        <source>Import painter code from a .js file</source>
        <translation>Nhập mã vẽ từ tệp .js</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="99"/>
        <source>Template</source>
        <translation>Mẫu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="103"/>
        <source>Select a built-in painter template</source>
        <translation>Chọn mẫu vẽ tích hợp sẵn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="108"/>
        <source>Format</source>
        <translation>Định Dạng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="113"/>
        <source>Reformat the painter code</source>
        <translation>Định dạng lại mã vẽ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="119"/>
        <source>Test</source>
        <translation>Kiểm Tra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="124"/>
        <source>Open a live preview with simulated dataset values</source>
        <translation>Mở bản xem trước trực tiếp với giá trị tập dữ liệu mô phỏng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="127"/>
        <source>Preview</source>
        <translation>Xem Trước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="182"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="191"/>
        <source>Cut</source>
        <translation>Cắt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="192"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="193"/>
        <source>Paste</source>
        <translation>Dán</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="194"/>
        <source>Select All</source>
        <translation>Chọn Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="196"/>
        <source>Undo</source>
        <translation>Hoàn Tác</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="197"/>
        <source>Redo</source>
        <translation>Làm Lại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="199"/>
        <source>Format Document</source>
        <translation>Định Dạng Tài Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterCodeDialog.qml" line="200"/>
        <source>Format Selection</source>
        <translation>Định Dạng Vùng Chọn</translation>
    </message>
</context>
<context>
    <name>PainterTestDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="28"/>
        <source>Painter Live Preview</source>
        <translation>Xem Trước Trực Tiếp Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="32"/>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="37"/>
        <source>Preview</source>
        <translation>Xem Trước</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="113"/>
        <source>Simulated datasets</source>
        <translation>Tập dữ liệu mô phỏng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="180"/>
        <source>Runtime OK</source>
        <translation>Runtime OK</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="181"/>
        <source>Awaiting first frame...</source>
        <translation>Đang chờ frame đầu tiên...</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="194"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="236"/>
        <source>Clear console</source>
        <translation>Xóa console</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/PainterTestDialog.qml" line="245"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>Plot</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="294"/>
        <source>Interpolation: %1</source>
        <translation>Nội Suy: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="307"/>
        <source>Show Area Under Plot</source>
        <translation>Hiển Thị Vùng dưới Đồ Thị</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="326"/>
        <source>Show X Axis Label</source>
        <translation>Hiển Thị Nhãn Trục X</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="337"/>
        <source>Show Y Axis Label</source>
        <translation>Hiển Thị Nhãn Trục Y</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="349"/>
        <source>Show Crosshair</source>
        <translation>Hiển Thị Tâm Ngắm</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="356"/>
        <source>Resume</source>
        <translation>Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="373"/>
        <source>Sweep / Trigger Mode</source>
        <translation>Chế Độ Quét / Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="385"/>
        <source>Trigger Settings</source>
        <translation>Cài Đặt Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="409"/>
        <source>Reset View</source>
        <translation>Đặt Lại Chế Độ Xem</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot.qml" line="415"/>
        <source>Axis Range Settings</source>
        <translation>Cài Đặt Phạm Vi Trục</translation>
    </message>
</context>
<context>
    <name>Plot3D</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="202"/>
        <source>Interpolate</source>
        <translation>Nội Suy</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="220"/>
        <source>Orbit Navigation</source>
        <translation>Điều Hướng Quỹ Đạo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="230"/>
        <source>Pan Navigation</source>
        <translation>Điều Hướng Toàn Cảnh</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="241"/>
        <source>Orthogonal View</source>
        <translation>Chế Độ Xem Trực Giao</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="247"/>
        <source>Top View</source>
        <translation>Chế Độ Xem Trên</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="253"/>
        <source>Left View</source>
        <translation>Chế Độ Xem Trái</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="259"/>
        <source>Front View</source>
        <translation>Chế Độ Xem Trước</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="276"/>
        <source>Auto Center</source>
        <translation>Tự Động Căn Giữa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="292"/>
        <source>Anaglyph 3D</source>
        <translation>3D Anaglyph</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Plot3D.qml" line="306"/>
        <source>Invert Eye Positions</source>
        <translation>Đảo Vị Trí Mắt</translation>
    </message>
</context>
<context>
    <name>PlotCommon</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/PlotCommon.qml" line="71"/>
        <source>None</source>
        <translation>Không</translation>
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
        <translation>Tuyến Tính</translation>
    </message>
</context>
<context>
    <name>PlotWidget</name>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1392"/>
        <source>Time</source>
        <translation>Thời Gian</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1415"/>
        <source>ΔX: %1  ΔY: %2 — Drag to move, right-click to clear</source>
        <translation>ΔX: %1  ΔY: %2 — Kéo để di chuyển, nhấp chuột phải để xóa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1418"/>
        <source>Click to place cursor</source>
        <translation>Nhấp để đặt con trỏ</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/PlotWidget.qml" line="1420"/>
        <source>Click to place second cursor — Drag to move</source>
        <translation>Nhấp để đặt con trỏ thứ hai — Kéo để di chuyển</translation>
    </message>
</context>
<context>
    <name>ProNotice</name>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="119"/>
        <source>Visit Website</source>
        <translation>Truy Cập Trang Web</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="127"/>
        <source>Buy License</source>
        <translation>Mua Giấy Phép</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/ProNotice.qml" line="140"/>
        <source>Activate</source>
        <translation>Kích Hoạt</translation>
    </message>
</context>
<context>
    <name>ProUpgradeNotice</name>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="26"/>
        <source>Assistant — Pro feature</source>
        <translation>Trợ Lý — Tính năng Pro</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="44"/>
        <source>The Assistant is a Serial Studio Pro feature. Activate your license to unlock it.</source>
        <translation>Trợ Lý là tính năng của Serial Studio Pro. Kích hoạt giấy phép để mở khóa.</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="52"/>
        <source>Activate</source>
        <translation>Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ProUpgradeNotice.qml" line="66"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>Process</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="69"/>
        <source>Mode</source>
        <translation>Chế Độ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Launch Process</source>
        <translation>Khởi Chạy Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="77"/>
        <source>Named Pipe</source>
        <translation>Named Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="101"/>
        <source>Executable</source>
        <translation>Tệp Thực Thi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="116"/>
        <source>/path/to/executable</source>
        <translation>/đường/dẫn/đến/tệp/thực/thi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="133"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="209"/>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="257"/>
        <source>Browse</source>
        <translation>Duyệt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="145"/>
        <source>Arguments</source>
        <translation>Tham Số</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="156"/>
        <source>--arg1 value1 --arg2 value2</source>
        <translation>--arg1 giá_trị1 --arg2 giá_trị2</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="177"/>
        <source>Working Dir</source>
        <translation>Thư Mục Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="192"/>
        <source>(optional) /working/directory</source>
        <translation>(tùy chọn) /thư/mục/làm/việc</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="223"/>
        <source>Pipe Path</source>
        <translation>Đường Dẫn Pipe</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="273"/>
        <source>Pick Running Process…</source>
        <translation>Chọn Tiến Trình Đang Chạy…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="311"/>
        <source>Launch a child process and capture its stdout, or connect to a named pipe written by an existing process.</source>
        <translation>Khởi chạy tiến trình con và bắt stdout của nó, hoặc kết nối với named pipe được ghi bởi tiến trình đang tồn tại.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/Process.qml" line="319"/>
        <source>Learn about named pipes</source>
        <translation>Tìm hiểu về named pipe</translation>
    </message>
</context>
<context>
    <name>ProcessPicker</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="60"/>
        <source>Select Running Process</source>
        <translation>Chọn Tiến Trình Đang Chạy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="211"/>
        <source>Select a running process to derive a named-pipe path suggestion.</source>
        <translation>Chọn một tiến trình đang chạy để tạo gợi ý đường dẫn named-pipe.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="217"/>
        <source>Filter Processes</source>
        <translation>Lọc Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="231"/>
        <source>Type to filter by name…</source>
        <translation>Nhập để lọc theo tên…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="235"/>
        <source>Refresh</source>
        <translation>Làm Mới</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="243"/>
        <source>Running Processes</source>
        <translation>Các Tiến Trình Đang Chạy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="281"/>
        <source>Process</source>
        <translation>Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="288"/>
        <source>PID</source>
        <translation>PID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="383"/>
        <source>No processes match the filter.</source>
        <translation>Không có tiến trình nào khớp với bộ lọc.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="384"/>
        <source>No running processes found.
Click Refresh to update the list.</source>
        <translation>Không tìm thấy tiến trình đang chạy.
Nhấn Làm Mới để cập nhật danh sách.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="400"/>
        <source>%1 process(es)</source>
        <translation>%1 tiến trình</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="404"/>
        <source>Select</source>
        <translation>Chọn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/ProcessPicker.qml" line="410"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>ProjectEditor</name>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="43"/>
        <source>modified</source>
        <translation>đã chỉnh sửa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="367"/>
        <source>This project is password protected</source>
        <translation>Dự án này được bảo vệ bằng mật khẩu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="368"/>
        <source>Editing is available in Project mode</source>
        <translation>Chỉnh sửa khả dụng trong chế độ Dự án</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="379"/>
        <source>Enter the password to make changes, or open a different project.</source>
        <translation>Nhập mật khẩu để thực hiện thay đổi hoặc mở dự án khác.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="380"/>
        <source>Switch to Project mode to load and edit a project.</source>
        <translation>Chuyển sang chế độ Dự án để tải và chỉnh sửa dự án.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="402"/>
        <source>Unlock</source>
        <translation>Mở Khóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="403"/>
        <source>Switch to Project Mode</source>
        <translation>Chuyển sang Chế Độ Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="422"/>
        <source>Open Other Project</source>
        <translation>Mở Dự Án Khác</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="423"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/ProjectEditor.qml" line="439"/>
        <source>Create New Project</source>
        <translation>Tạo Dự Án Mới</translation>
    </message>
</context>
<context>
    <name>ProjectStructure</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="32"/>
        <source>Project Structure</source>
        <translation>Cấu Trúc Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="71"/>
        <source>Search</source>
        <translation>Tìm Kiếm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="357"/>
        <source>Move Up</source>
        <translation>Di Chuyển Lên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="365"/>
        <source>Move Down</source>
        <translation>Di Chuyển Xuống</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="386"/>
        <source>Rename</source>
        <translation>Đổi Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="404"/>
        <source>Hide Selected (%1)</source>
        <translation>Ẩn Mục Đã Chọn (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="405"/>
        <source>Show Selected (%1)</source>
        <translation>Hiển Thị Mục Đã Chọn (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="406"/>
        <source>Hide</source>
        <translation>Ẩn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="406"/>
        <source>Show</source>
        <translation>Hiển Thị</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="429"/>
        <source>Duplicate Selected (%1)</source>
        <translation>Nhân Bản Đã Chọn (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="430"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="452"/>
        <source>Delete Selected (%1)</source>
        <translation>Xóa Đã Chọn (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="453"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="504"/>
        <source>New Folder</source>
        <translation>Thư Mục Mới</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="517"/>
        <source>New Sub-Folder</source>
        <translation>Thư Mục Con Mới</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="533"/>
        <source>Move to Folder</source>
        <translation>Di Chuyển đến Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="540"/>
        <source>Top Level</source>
        <translation>Cấp Cao Nhất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectStructure.qml" line="573"/>
        <source>Move Here</source>
        <translation>Di Chuyển đến Đây</translation>
    </message>
</context>
<context>
    <name>ProjectToolbar</name>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="142"/>
        <source>New</source>
        <translation>Mới</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="145"/>
        <source>Create a new JSON project</source>
        <translation>Tạo dự án JSON mới</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="158"/>
        <source>Open</source>
        <translation>Mở</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="162"/>
        <source>Open an existing JSON project</source>
        <translation>Mở dự án JSON hiện có</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="168"/>
        <source>Save</source>
        <translation>Lưu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="175"/>
        <source>Save the current project</source>
        <translation>Lưu dự án hiện tại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="181"/>
        <source>Save As</source>
        <translation>Lưu dưới Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="188"/>
        <source>Save the current project under a new name</source>
        <translation>Lưu dự án hiện tại với tên mới</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="200"/>
        <source>Import</source>
        <translation>Nhập</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="204"/>
        <source>Protobuf</source>
        <translation>Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="208"/>
        <source>Generate a project from a Protocol Buffers (.proto) schema</source>
        <translation>Tạo dự án từ lược đồ Protocol Buffers (.proto)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="230"/>
        <source>Lock</source>
        <translation>Khóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="234"/>
        <source>Set a password and lock the Project Editor</source>
        <translation>Đặt mật khẩu và khóa Trình Chỉnh Sửa Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="245"/>
        <source>Add Device</source>
        <translation>Thêm Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="249"/>
        <source>Add a new data source (device) to the project</source>
        <translation>Thêm nguồn dữ liệu (thiết bị) mới vào dự án</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="272"/>
        <source>Action</source>
        <translation>Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="275"/>
        <source>Add a new action to the project</source>
        <translation>Thêm một hành động mới vào dự án</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="260"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="264"/>
        <source>Output</source>
        <translation>Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="218"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="222"/>
        <source>Restore</source>
        <translation>Khôi Phục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="226"/>
        <source>Restore a recent automatic snapshot of the current project</source>
        <translation>Khôi phục bản chụp tự động gần đây của dự án hiện tại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="267"/>
        <source>Add a new output control panel with a button</source>
        <translation>Thêm một bảng điều khiển đầu ra mới với một nút</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="288"/>
        <source>Slider</source>
        <translation>Thanh Trượt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="291"/>
        <source>Add an output slider control</source>
        <translation>Thêm một điều khiển thanh trượt đầu ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="298"/>
        <source>Toggle</source>
        <translation>Công Tắc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="301"/>
        <source>Add an output toggle control</source>
        <translation>Thêm một điều khiển công tắc đầu ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="308"/>
        <source>Knob</source>
        <translation>Núm Xoay</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="311"/>
        <source>Add an output knob control</source>
        <translation>Thêm một điều khiển núm xoay đầu ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="319"/>
        <source>Text Field</source>
        <translation>Trường Văn Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="321"/>
        <source>Add an output text field control</source>
        <translation>Thêm điều khiển trường văn bản đầu ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="328"/>
        <source>Button</source>
        <translation>Nút</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="331"/>
        <source>Add an output button control</source>
        <translation>Thêm điều khiển nút đầu ra</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="344"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="348"/>
        <source>Dataset</source>
        <translation>Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="350"/>
        <source>Add a generic dataset</source>
        <translation>Thêm tập dữ liệu chung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="364"/>
        <source>Plot</source>
        <translation>Biểu Đồ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="367"/>
        <source>Add a 2D plot dataset</source>
        <translation>Thêm tập dữ liệu biểu đồ 2D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="374"/>
        <source>FFT Plot</source>
        <translation>Biểu Đồ FFT</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="377"/>
        <source>Add a Fast Fourier Transform plot</source>
        <translation>Thêm biểu đồ biến đổi Fourier nhanh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="384"/>
        <source>Gauge</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="387"/>
        <source>Add a gauge widget for numeric data</source>
        <translation>Thêm widget đồng hồ đo cho dữ liệu số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="395"/>
        <source>Level Indicator</source>
        <translation>Chỉ Báo Mức</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="397"/>
        <source>Add a vertical bar level indicator</source>
        <translation>Thêm chỉ báo mức dạng thanh dọc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="404"/>
        <source>Compass</source>
        <translation>La Bàn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="407"/>
        <source>Add a compass widget for directional data</source>
        <translation>Thêm widget la bàn cho dữ liệu hướng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="415"/>
        <source>LED Indicator</source>
        <translation>Chỉ Báo LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="417"/>
        <source>Add an LED-style status indicator</source>
        <translation>Thêm chỉ báo trạng thái kiểu LED</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="430"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="434"/>
        <source>Group</source>
        <translation>Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="436"/>
        <source>Add a dataset container group</source>
        <translation>Thêm nhóm vùng chứa tập dữ liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="438"/>
        <source>Dataset Container</source>
        <translation>Vùng Chứa Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="442"/>
        <source>Image</source>
        <translation>Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="444"/>
        <source>Add an image/video stream viewer</source>
        <translation>Thêm trình xem luồng hình ảnh/video</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="446"/>
        <source>Image View</source>
        <translation>Chế Độ Xem Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="450"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="454"/>
        <source>Web View</source>
        <translation>Khung Xem Web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="452"/>
        <source>Add an web viewer</source>
        <translation>Thêm trình xem web</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="462"/>
        <source>Painter</source>
        <translation>Painter</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="466"/>
        <source>Add a custom JavaScript-rendered painter widget</source>
        <translation>Thêm widget vẽ tùy chỉnh được render bằng JavaScript</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="467"/>
        <source>Painter widgets require a Pro license — adding one will fall back to a data grid</source>
        <translation>Widget Vẽ yêu cầu giấy phép Pro — việc thêm widget này sẽ chuyển về lưới dữ liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="468"/>
        <source>Painter Widget</source>
        <translation>Widget Vẽ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="480"/>
        <source>Table</source>
        <translation>Bảng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="483"/>
        <source>Add a data table view</source>
        <translation>Thêm chế độ xem bảng dữ liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="485"/>
        <source>Data Grid</source>
        <translation>Lưới Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="491"/>
        <source>Multi-Plot</source>
        <translation>Đồ Thị Đa Biến</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="493"/>
        <source>Add a 2D plot with multiple signals</source>
        <translation>Thêm đồ thị 2D với nhiều tín hiệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="495"/>
        <source>Multiple Plot</source>
        <translation>Đồ Thị Đa Trục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="500"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="505"/>
        <source>3D Plot</source>
        <translation>Đồ Thị 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="503"/>
        <source>Add a 3D plot visualization</source>
        <translation>Thêm trực quan hóa đồ thị 3D</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="511"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="515"/>
        <source>Accelerometer</source>
        <translation>Gia Tốc Kế</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="513"/>
        <source>Add a group for 3-axis accelerometer data</source>
        <translation>Thêm nhóm cho dữ liệu gia tốc kế 3 trục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="521"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="524"/>
        <source>Gyroscope</source>
        <translation>Con Quay Hồi Chuyển</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="525"/>
        <source>Add a group for 3-axis gyroscope data</source>
        <translation>Thêm nhóm cho dữ liệu con quay hồi chuyển 3 trục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="530"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="535"/>
        <source>GPS Map</source>
        <translation>Bản Đồ GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="533"/>
        <source>Add a map widget for GPS data</source>
        <translation>Thêm widget bản đồ cho dữ liệu GPS</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="547"/>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="551"/>
        <source>Assistant</source>
        <translation>Trợ Lý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="554"/>
        <source>Open the Assistant</source>
        <translation>Mở Trợ Lý</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="560"/>
        <source>Help Center</source>
        <translation>Trung Tâm Trợ Giúp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Sections/ProjectToolbar.qml" line="564"/>
        <source>Open the Project Editor documentation</source>
        <translation>Mở tài liệu Trình Chỉnh Sửa Dự Án</translation>
    </message>
</context>
<context>
    <name>ProjectView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="34"/>
        <source>Project Summary</source>
        <translation>Tóm Tắt Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="81"/>
        <source>Pro features detected in this project.</source>
        <translation>Phát hiện tính năng Pro trong dự án này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="83"/>
        <source>Using fallback widgets. Buy a license to unlock full functionality.</source>
        <translation>Đang sử dụng widget dự phòng. Mua giấy phép để mở khóa đầy đủ chức năng.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="118"/>
        <source>Project Title:</source>
        <translation>Tiêu Đề Dự Án:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="129"/>
        <source>Untitled Project</source>
        <translation>Dự Án Chưa Đặt Tên</translation>
    </message>
    <message>
        <source>Points:</source>
        <translation type="vanished">Điểm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="147"/>
        <source>Settings</source>
        <translation>Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="173"/>
        <source>Time Range:</source>
        <translation>Phạm Vi Thời Gian:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="221"/>
        <source>Point Count:</source>
        <translation>Số Lượng Điểm:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="240"/>
        <source>Change-Driven Transforms:</source>
        <translation>Biến Đổi theo Thay Đổi:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="250"/>
        <source>Run a dataset's transform only when one of its inputs changes. Speeds up large table-driven projects; off by default.</source>
        <translation>Chỉ chạy biến đổi của tập dữ liệu khi một trong các đầu vào của nó thay đổi. Tăng tốc các dự án lớn dựa trên bảng; tắt theo mặc định.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="297"/>
        <source>Source</source>
        <translation>Nguồn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="298"/>
        <source>Sources</source>
        <translation>Nguồn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="303"/>
        <source>Group</source>
        <translation>Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="304"/>
        <source>Groups</source>
        <translation>Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="309"/>
        <source>Dataset</source>
        <translation>Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="310"/>
        <source>Datasets</source>
        <translation>Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="315"/>
        <source>Action</source>
        <translation>Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="316"/>
        <source>Actions</source>
        <translation>Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/ProjectView.qml" line="404"/>
        <source>Double-click a block to edit it. Right-click anywhere to add a group, dataset, action, data table, or device.</source>
        <translation>Nhấp đúp vào một khối để chỉnh sửa. Nhấp chuột phải bất kỳ đâu để thêm nhóm, tập dữ liệu, hành động, bảng dữ liệu hoặc thiết bị.</translation>
    </message>
</context>
<context>
    <name>ProtoPreviewDialog</name>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="41"/>
        <source>Protocol Buffers File Preview</source>
        <translation>Xem Trước Tệp Protocol Buffers</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="165"/>
        <source>Proto File: %1</source>
        <translation>Tệp Proto: %1</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="173"/>
        <source>Browse the messages below, then create the project. Every message becomes a dashboard group; matching-type channel blocks get a MultiPlot and mixed-type messages get a DataGrid.</source>
        <translation>Duyệt qua các tin nhắn bên dưới, sau đó tạo dự án. Mỗi tin nhắn trở thành một nhóm bảng điều khiển; các khối kênh cùng loại sẽ có MultiPlot và tin nhắn hỗn hợp loại sẽ có DataGrid.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="183"/>
        <source>Show fields for</source>
        <translation>Hiển thị trường cho</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="209"/>
        <source>Fields</source>
        <translation>Trường</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="243"/>
        <source>Tag</source>
        <translation>Thẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="253"/>
        <source>Field Name</source>
        <translation>Tên Trường</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="258"/>
        <source>Type</source>
        <translation>Loại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="345"/>
        <source>No fields in the selected message.</source>
        <translation>Không có trường nào trong tin nhắn đã chọn.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="363"/>
        <source>Total: %1 messages, %2 fields</source>
        <translation>Tổng: %1 tin nhắn, %2 trường</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="370"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml" line="381"/>
        <source>Create Project</source>
        <translation>Tạo Dự Án</translation>
    </message>
</context>
<context>
    <name>Publisher</name>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="71"/>
        <source>No error</source>
        <translation>Không có lỗi</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="73"/>
        <source>The broker rejected the connection due to an unsupported protocol version. Match the broker's MQTT version and try again.</source>
        <translation>Broker từ chối kết nối do phiên bản giao thức không được hỗ trợ. Khớp phiên bản MQTT của broker và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="76"/>
        <source>The broker rejected the client ID. It may be malformed, too long, or already in use. Regenerate it and try again.</source>
        <translation>Broker từ chối ID client. ID có thể bị sai định dạng, quá dài hoặc đã được sử dụng. Tạo lại ID và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="79"/>
        <source>The network reached the broker, but the broker is currently unavailable. Verify its status and try again later.</source>
        <translation>Mạng đã kết nối tới broker, nhưng broker hiện không khả dụng. Xác minh trạng thái của broker và thử lại sau.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="82"/>
        <source>The username or password is incorrect or malformed. Double-check the credentials and try again.</source>
        <translation>Tên người dùng hoặc mật khẩu không chính xác hoặc bị sai định dạng. Kiểm tra lại thông tin xác thực và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="85"/>
        <source>The broker denied the connection due to insufficient permissions. Verify that the account has the required ACLs.</source>
        <translation>Broker từ chối kết nối do không đủ quyền. Xác minh rằng tài khoản có các ACL cần thiết.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="88"/>
        <source>A network or transport-layer issue prevented the connection. Check connectivity, ports, and TLS configuration.</source>
        <translation>Sự cố ở tầng mạng hoặc tầng vận chuyển đã ngăn kết nối. Kiểm tra kết nối mạng, cổng và cấu hình TLS.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="91"/>
        <source>The client detected an MQTT protocol violation and closed the connection. Verify broker and client compatibility.</source>
        <translation>Client phát hiện vi phạm giao thức MQTT và đã đóng kết nối. Xác minh tính tương thích giữa broker và client.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="94"/>
        <source>An unexpected error occurred. Check the broker logs and the application console for details.</source>
        <translation>Đã xảy ra lỗi không mong đợi. Kiểm tra nhật ký broker và bảng điều khiển ứng dụng để biết chi tiết.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="97"/>
        <source>An MQTT 5 protocol-level error occurred. Inspect the broker's reason code for details.</source>
        <translation>Đã xảy ra lỗi cấp giao thức MQTT 5. Kiểm tra mã lý do của broker để biết chi tiết.</translation>
    </message>
    <message>
        <location filename="../../src/MQTT/Publisher.cpp" line="101"/>
        <source>Unspecified MQTT error (code %1).</source>
        <translation>Lỗi MQTT không xác định (mã %1).</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../../src/Misc/Translator.cpp" line="231"/>
        <source>Failed to load welcome text :(</source>
        <translation>Không thể tải văn bản chào mừng :(</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="192"/>
        <source>Critical</source>
        <translation>Nghiêm Trọng</translation>
    </message>
    <message>
        <location filename="../../src/Misc/ModuleManager.cpp" line="192"/>
        <source>Warning</source>
        <translation>Cảnh Báo</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="585"/>
        <source>Project file not found</source>
        <translation>Không tìm thấy tệp dự án</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="586"/>
        <source>The project file referenced by this shortcut could not be found:

%1</source>
        <translation>Không thể tìm thấy tệp dự án được tham chiếu bởi lối tắt này:

%1</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="589"/>
        <source>Would you like to delete this shortcut?</source>
        <translation>Bạn có muốn xóa lối tắt này không?</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="593"/>
        <source>Delete Shortcut</source>
        <translation>Xóa Lối Tắt</translation>
    </message>
    <message>
        <location filename="../../src/Misc/CLI.cpp" line="595"/>
        <source>Quit</source>
        <translation>Thoát</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1051"/>
        <source>Time (s)</source>
        <translation>Thời Gian (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1109"/>
        <source>Freq: %1</source>
        <translation>Tần Số: %1</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="1112"/>
        <source>Time: −%1</source>
        <translation>Thời Gian: −%1</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1311"/>
        <source>Invalid Bluetooth adapter!</source>
        <translation>Bộ chuyển đổi Bluetooth không hợp lệ!</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1314"/>
        <source>Unsuported platform or operating system</source>
        <translation>Nền tảng hoặc hệ điều hành không được hỗ trợ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1317"/>
        <source>Unsupported discovery method</source>
        <translation>Phương thức khám phá không được hỗ trợ</translation>
    </message>
    <message>
        <location filename="../../src/IO/Drivers/BluetoothLE.cpp" line="1320"/>
        <source>General I/O error</source>
        <translation>Lỗi I/O chung</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/LocalProvider.cpp" line="351"/>
        <source>No local model server URL configured. Open Manage Keys to set one.</source>
        <translation>Chưa cấu hình URL máy chủ mô hình cục bộ. Mở Quản Lý Khóa để thiết lập.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/DeepSeekProvider.cpp" line="146"/>
        <source>No DeepSeek API key set. Open Manage Keys to add one.</source>
        <translation>Chưa đặt khóa API DeepSeek. Mở Quản Lý Khóa để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/MistralProvider.cpp" line="168"/>
        <source>No Mistral API key set. Open Manage Keys to add one.</source>
        <translation>Chưa đặt khóa API Mistral. Mở Quản Lý Khóa để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenAIProvider.cpp" line="362"/>
        <source>No OpenAI API key set. Open Manage Keys to add one.</source>
        <translation>Chưa đặt khóa API OpenAI. Mở Quản Lý Khóa để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/OpenRouterProvider.cpp" line="181"/>
        <source>No OpenRouter API key set. Open Manage Keys to add one.</source>
        <translation>Chưa đặt khóa API OpenRouter. Mở Quản Lý Khóa để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GroqProvider.cpp" line="152"/>
        <source>No Groq API key set. Open Manage Keys to add one.</source>
        <translation>Chưa đặt khóa API Groq. Mở Quản Lý Khóa để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/AnthropicProvider.cpp" line="240"/>
        <source>No Anthropic API key set. Open Manage Keys to add one.</source>
        <translation>Chưa đặt khóa API Anthropic. Mở Quản Lý Khóa để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/AI/Providers/GeminiProvider.cpp" line="285"/>
        <source>No Gemini API key set. Open Manage Keys to add one.</source>
        <translation>Chưa đặt khóa API Gemini. Mở Quản Lý Khóa để thêm.</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="250"/>
        <source>Network error</source>
        <translation>Lỗi mạng</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="253"/>
        <location filename="../../src/Licensing/Trial.cpp" line="269"/>
        <location filename="../../src/Licensing/Trial.cpp" line="301"/>
        <source>Trial Activation Error</source>
        <translation>Lỗi Kích Hoạt Dùng Thử</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="266"/>
        <source>Invalid server response</source>
        <translation>Phản hồi máy chủ không hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="267"/>
        <source>The server returned malformed data: %1</source>
        <translation>Máy chủ trả về dữ liệu không đúng định dạng: %1</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="298"/>
        <source>Unexpected server response</source>
        <translation>Phản hồi không mong đợi từ máy chủ</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/Trial.cpp" line="299"/>
        <source>The server response is missing required fields.</source>
        <translation>Phản hồi từ máy chủ thiếu các trường bắt buộc.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1149"/>
        <source>The frame parser is using more than %1% of CPU time.</source>
        <translation>Bộ phân tích frame đang sử dụng hơn %1% thời gian CPU.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/FrameBuilder.cpp" line="1151"/>
        <source>Serial Studio is dropping frames to keep the application responsive. Please simplify or optimize the frame parser script to reduce its workload.</source>
        <translation>Serial Studio đang loại bỏ các frame để giữ ứng dụng phản hồi. Vui lòng đơn giản hóa hoặc tối ưu hóa script bộ phân tích frame để giảm khối lượng công việc.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="252"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="273"/>
        <source>Frame Parser Disabled</source>
        <translation>Bộ Phân Tích Frame Đã Tắt</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="274"/>
        <source>The Lua frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Bộ phân tích frame Lua cho nguồn %1 đã hết thời gian chờ %2 frame liên tiếp và đã bị tắt để giữ Serial Studio phản hồi.

Nguyên nhân có thể: vòng lặp vô hạn hoặc thao tác cực kỳ chậm trong thân script. Sửa script và tải lại dự án để bật lại phân tích.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="322"/>
        <source>Lua Syntax Error</source>
        <translation>Lỗi Cú Pháp Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="323"/>
        <source>The parser code contains an error:

%1</source>
        <translation>Mã trình phân tích chứa lỗi:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="371"/>
        <source>Lua Runtime Error</source>
        <translation>Lỗi Runtime Lua</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="372"/>
        <source>The parser code triggered an error:

%1</source>
        <translation>Mã trình phân tích đã kích hoạt lỗi:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="478"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="393"/>
        <source>Missing Parse Function</source>
        <translation>Thiếu Hàm Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="394"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) ... end</source>
        <translation>Hàm 'parse' không được định nghĩa trong script.

Vui lòng đảm bảo mã của bạn bao gồm:
function parse(frame) ... end</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="530"/>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="456"/>
        <source>Parse Function Runtime Error</source>
        <translation>Lỗi Runtime Hàm Parse</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/LuaScriptEngine.cpp" line="457"/>
        <source>The parse function contains an error:

%1

Please fix the error in the function body.</source>
        <translation>Hàm parse chứa lỗi:

%1

Vui lòng sửa lỗi trong thân hàm.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="253"/>
        <source>The JavaScript frame parser for source %1 timed out %2 frames in a row and has been disabled to keep Serial Studio responsive.

Most likely cause: an infinite loop or extremely slow operation in the script body. Fix the script and reload the project to re-enable parsing.</source>
        <translation>Trình phân tích frame Javascript cho nguồn %1 đã hết thời gian chờ %2 frame liên tiếp và đã bị vô hiệu hóa để giữ Serial Studio phản hồi.

Nguyên nhân có thể: vòng lặp vô hạn hoặc thao tác cực chậm trong thân script. Sửa script và tải lại dự án để kích hoạt lại phân tích.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="419"/>
        <source>JavaScript Timed Out</source>
        <translation>Javascript Hết Thời Gian Chờ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="420"/>
        <source>The parser code did not finish evaluating within %1 ms and was interrupted.

Most likely cause: an infinite loop at the top level of the script.</source>
        <translation>Mã phân tích cú pháp không hoàn thành đánh giá trong vòng %1 ms và đã bị gián đoạn.

Nguyên nhân có thể: vòng lặp vô hạn ở cấp cao nhất của script.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="437"/>
        <source>JavaScript Syntax Error</source>
        <translation>Lỗi Cú Pháp Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="438"/>
        <source>The parser code contains a syntax error at line %1:

%2</source>
        <translation>Mã trình phân tích chứa lỗi cú pháp tại dòng %1:

%2</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="452"/>
        <source>JavaScript Exception Occurred</source>
        <translation>Xảy Ra Ngoại Lệ Javascript</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="453"/>
        <source>The parser code triggered the following exceptions:

%1</source>
        <translation>Mã trình phân tích đã kích hoạt các ngoại lệ sau:

%1</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="479"/>
        <source>The 'parse' function is not defined in the script.

Please ensure your code includes:
function parse(frame) { ... }</source>
        <translation>Hàm 'parse' không được định nghĩa trong script.

Vui lòng đảm bảo mã của bạn bao gồm:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="531"/>
        <source>The parse function contains an error at line %1:

%2

Please fix the error in the function body.</source>
        <translation>Hàm parse chứa lỗi tại dòng %1:

%2

Vui lòng sửa lỗi trong thân hàm.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="631"/>
        <source>Invalid Function Declaration</source>
        <translation>Khai Báo Hàm Không Hợp Lệ</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="632"/>
        <source>No callable 'parse' export found.

Define one of:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</source>
        <translation>Không tìm thấy hàm xuất 'parse' có thể gọi.

Định nghĩa một trong các dạng sau:
  function parse(frame) { ... }
  const parse = (frame) =&gt; { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="648"/>
        <source>The 'parse' function must accept at least one parameter (the frame payload).</source>
        <translation>Hàm 'parse' phải nhận ít nhất một tham số (payload của frame).</translation>
    </message>
    <message>
        <source>No valid 'parse' function declaration found.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Không tìm thấy khai báo hàm 'parse' hợp lệ.

Định dạng mong đợi:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="647"/>
        <source>Invalid Function Parameter</source>
        <translation>Tham Số Hàm Không Hợp Lệ</translation>
    </message>
    <message>
        <source>The 'parse' function must have at least one parameter.

Expected format:
function parse(frame) { ... }</source>
        <translation type="vanished">Hàm 'parse' phải có ít nhất một tham số.

Định dạng mong đợi:
function parse(frame) { ... }</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="613"/>
        <source>Deprecated Function Signature</source>
        <translation>Chữ Ký Hàm Đã Lỗi Thời</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/JsScriptEngine.cpp" line="614"/>
        <source>The 'parse' function uses the old two-parameter format: parse(%1, %2)

This format is no longer supported. Please update to the new single-parameter format:
function parse(%1) { ... }

The separator parameter is no longer needed.</source>
        <translation>Hàm 'parse' đang sử dụng định dạng hai tham số cũ: parse(%1, %2)

Định dạng này không còn được hỗ trợ. Vui lòng cập nhật sang định dạng một tham số mới:
function parse(%1) { ... }

Không còn cần tham số phân tách nữa.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="386"/>
        <source>Expected %1, got '%2'</source>
        <translation>Mong đợi %1, nhận được '%2'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="435"/>
        <source>Expected enum name after 'enum'</source>
        <translation>Mong đợi tên enum sau 'enum'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="449"/>
        <source>Expected oneof name</source>
        <translation>Mong đợi tên oneof</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="476"/>
        <source>Field tag '%1' out of range (1..%2)</source>
        <translation>Thẻ trường '%1' nằm ngoài phạm vi (1..%2)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="494"/>
        <source>Expected key type in map&lt;&gt;</source>
        <translation>Yêu cầu kiểu khóa trong map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="502"/>
        <source>Expected value type in map&lt;&gt;</source>
        <translation>Yêu cầu kiểu giá trị trong map&lt;&gt;</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="510"/>
        <source>Expected map field name</source>
        <translation>Yêu cầu tên trường map</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="522"/>
        <source>Expected map field tag</source>
        <translation>Yêu cầu thẻ trường map</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="554"/>
        <source>Expected field type, got '%1'</source>
        <translation>Yêu cầu kiểu trường, nhận được '%1'</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="573"/>
        <source>Expected field name after type</source>
        <translation>Yêu cầu tên trường sau kiểu</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="583"/>
        <source>Expected field tag number</source>
        <translation>Yêu cầu số thẻ trường</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="630"/>
        <source>Message nesting too deep (limit %1)</source>
        <translation>Lồng message quá sâu (giới hạn %1)</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="635"/>
        <source>Expected message name</source>
        <translation>Yêu cầu tên thông điệp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="717"/>
        <source>Unexpected token '%1' at file scope</source>
        <translation>Token không mong đợi '%1' ở phạm vi tệp</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Importers/ProtoImporter.cpp" line="763"/>
        <source>Unsupported top-level keyword '%1'</source>
        <translation>Từ khóa cấp cao nhất không được hỗ trợ '%1'</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="162"/>
        <source>Console Output File Error</source>
        <translation>Lỗi Tệp Đầu Ra Console</translation>
    </message>
    <message>
        <location filename="../../src/Console/Export.cpp" line="163"/>
        <source>Cannot open file for writing!</source>
        <translation>Không thể mở tệp để ghi!</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="294"/>
        <source>Automatic (Platform Default)</source>
        <translation>Tự Động (Mặc Định Nền Tảng)</translation>
    </message>
    <message>
        <location filename="../../src/Misc/GraphicsBackend.cpp" line="299"/>
        <source>Software (Fallback)</source>
        <translation>Phần Mềm (Dự Phòng)</translation>
    </message>
    <message>
        <source>Row %1</source>
        <translation type="vanished">Hàng %1</translation>
    </message>
    <message>
        <source>[%1]</source>
        <translation type="vanished">[%1]</translation>
    </message>
    <message>
        <source>Frame %1</source>
        <translation type="vanished">Khung %1</translation>
    </message>
    <message>
        <source>Decoder</source>
        <translation type="vanished">Bộ Giải Mã</translation>
    </message>
    <message>
        <source>Rows</source>
        <translation type="vanished">Hàng</translation>
    </message>
    <message>
        <source>%1 row(s)</source>
        <translation type="vanished">%1 hàng</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="186"/>
        <source>The native parser configuration is not a valid JSON object.</source>
        <translation>Cấu hình trình phân tích cú pháp gốc không phải là đối tượng JSON hợp lệ.</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="196"/>
        <source>Unknown native parser template: "%1".</source>
        <translation>Mẫu trình phân tích cú pháp gốc không xác định: "%1".</translation>
    </message>
    <message>
        <location filename="../../src/DataModel/Scripting/CFrameParser.cpp" line="330"/>
        <source>Built-In Parser Error</source>
        <translation>Lỗi Trình Phân Tích Cú Pháp Built-in</translation>
    </message>
    <message>
        <source>Native Parser Error</source>
        <translation type="vanished">Lỗi Trình Phân Tích Cú Pháp Gốc</translation>
    </message>
    <message>
        <location filename="../../src/Licensing/OfflineLicense.cpp" line="51"/>
        <source>Offline Activation</source>
        <translation>Kích Hoạt Ngoại Tuyến</translation>
    </message>
</context>
<context>
    <name>QuaGzipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="60"/>
        <source>QIODevice::Append is not supported for GZIP</source>
        <translation>QIODevice::Append không được hỗ trợ cho GZIP</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="66"/>
        <source>Opening gzip for both reading and writing is not supported</source>
        <translation>Không hỗ trợ mở gzip cho cả đọc và ghi</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="75"/>
        <source>You can open a gzip either for reading or for writing. Which is it?</source>
        <translation>Bạn có thể mở gzip để đọc hoặc để ghi. Bạn chọn cái nào?</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quagzipfile.cpp" line="81"/>
        <source>Could not gzopen() file</source>
        <translation>Không thể gzopen() tệp</translation>
    </message>
</context>
<context>
    <name>QuaZIODevice</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="178"/>
        <source>QIODevice::Append is not supported for QuaZIODevice</source>
        <translation>QIODevice::Append không được hỗ trợ cho QuaZIODevice</translation>
    </message>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quaziodevice.cpp" line="183"/>
        <source>QIODevice::ReadWrite is not supported for QuaZIODevice</source>
        <translation>QIODevice::ReadWrite không được hỗ trợ cho QuaZIODevice</translation>
    </message>
</context>
<context>
    <name>QuaZipFile</name>
    <message>
        <location filename="../../../lib/QuaZip/quazip/quazipfile.cpp" line="251"/>
        <source>ZIP/UNZIP API error %1</source>
        <translation>Lỗi API ZIP/UNZIP %1</translation>
    </message>
</context>
<context>
    <name>ReportOptionsDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate PDF Report</source>
        <translation>Tạo Báo Cáo PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="24"/>
        <source>Generate Report</source>
        <translation>Tạo Báo Cáo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="69"/>
        <source>Solid</source>
        <translation>Liền Nét</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="70"/>
        <source>Dashed</source>
        <translation>Gạch Đứt</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="71"/>
        <source>Dotted</source>
        <translation>Chấm</translation>
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
        <translation>Letter (8.5 × 11 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="103"/>
        <source>Legal (8.5 × 14 in)</source>
        <translation>Legal (8.5 × 14 in)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="104"/>
        <source>Executive (7.25 × 10.5 in)</source>
        <translation>Executive (7.25 × 10.5 in)</translation>
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
        <translation>%1 — Báo Cáo Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="122"/>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="560"/>
        <source>Session Report</source>
        <translation>Báo Cáo Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="462"/>
        <source>Branding</source>
        <translation>Thương Hiệu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="468"/>
        <source>Page</source>
        <translation>Trang</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="474"/>
        <source>Sections</source>
        <translation>Các Phần</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="480"/>
        <source>Datasets</source>
        <translation>Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="528"/>
        <source>Identity</source>
        <translation>Danh Tính</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="542"/>
        <source>Company</source>
        <translation>Công Ty</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="549"/>
        <source>e.g. Acme Test Systems</source>
        <translation>ví dụ: Hệ Thống Kiểm Tra Acme</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="553"/>
        <source>Document title</source>
        <translation>Tiêu đề tài liệu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="564"/>
        <source>Author</source>
        <translation>Tác Giả</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="571"/>
        <source>Prepared by (optional)</source>
        <translation>Người chuẩn bị (tùy chọn)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="580"/>
        <source>Logo</source>
        <translation>Logo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="593"/>
        <source>File</source>
        <translation>Tệp</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="604"/>
        <source>PNG, JPG or SVG (optional)</source>
        <translation>PNG, JPG hoặc SVG (tùy chọn)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="606"/>
        <source>Browse…</source>
        <translation>Duyệt…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="609"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="650"/>
        <source>Paper</source>
        <translation>Giấy</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="662"/>
        <source>Page size</source>
        <translation>Kích thước trang</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="677"/>
        <source>Plot appearance</source>
        <translation>Giao diện biểu đồ</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="691"/>
        <source>Line width</source>
        <translation>Độ rộng đường</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="723"/>
        <source>Line style</source>
        <translation>Kiểu đường</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="763"/>
        <source>Include</source>
        <translation>Bao Gồm</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="778"/>
        <source>Cover page (logo, document title, test subtitle)</source>
        <translation>Trang bìa (logo, tiêu đề tài liệu, phụ đề kiểm tra)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="781"/>
        <source>Test information (project, timestamps, classification and notes)</source>
        <translation>Thông tin kiểm tra (dự án, dấu thời gian, phân loại và ghi chú)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="784"/>
        <source>Measurement summary (min, max, mean, std. deviation per parameter)</source>
        <translation>Tóm tắt đo lường (min, max, trung bình, độ lệch chuẩn cho mỗi tham số)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="787"/>
        <source>Parameter trends (time-series chart per numeric parameter)</source>
        <translation>Xu hướng tham số (biểu đồ chuỗi thời gian cho mỗi tham số số)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="790"/>
        <source>Annotate min, max, and mean values on plots</source>
        <translation>Chú thích giá trị min, max và trung bình trên biểu đồ</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="822"/>
        <source>Include datasets</source>
        <translation>Bao gồm tập dữ liệu</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="908"/>
        <source>Loading datasets...</source>
        <translation>Đang tải tập dữ liệu...</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="928"/>
        <source>Select at least one dataset to include.</source>
        <translation>Chọn ít nhất một tập dữ liệu để bao gồm.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="937"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="945"/>
        <source>Export PDF</source>
        <translation>Xuất PDF</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportOptionsDialog.qml" line="945"/>
        <source>Export HTML</source>
        <translation>Xuất HTML</translation>
    </message>
</context>
<context>
    <name>ReportProgressDialog</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="20"/>
        <source>Generating Report</source>
        <translation>Đang Tạo Báo Cáo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="69"/>
        <source>Working…</source>
        <translation>Đang Xử Lý…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/ReportProgressDialog.qml" line="86"/>
        <source>This can take a few seconds for sessions with many parameters. The window closes automatically when the report is ready.</source>
        <translation>Quá trình này có thể mất vài giây đối với phiên có nhiều tham số. Cửa sổ sẽ tự động đóng khi báo cáo hoàn tất.</translation>
    </message>
</context>
<context>
    <name>RuntimeReconfigure</name>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="40"/>
        <source>Connection Lost</source>
        <translation>Mất Kết Nối</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="41"/>
        <source>Device Unavailable</source>
        <translation>Thiết Bị Không Khả Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="95"/>
        <source>The connection to your device was lost.</source>
        <translation>Kết nối đến thiết bị đã bị mất.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="96"/>
        <source>Serial Studio couldn't reach your device.</source>
        <translation>Serial Studio không thể kết nối đến thiết bị.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="105"/>
        <source>Check the cable, power, and that no other application has taken over the device. You can try reconnecting, switch to a different device, or quit.</source>
        <translation>Kiểm tra cáp, nguồn điện và đảm bảo không có ứng dụng nào khác đang chiếm dụng thiết bị. Có thể thử kết nối lại, chuyển sang thiết bị khác hoặc thoát.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="108"/>
        <source>Make sure it's plugged in, powered on, and not already in use by another app. You can try again, pick a different device, or quit.</source>
        <translation>Đảm bảo thiết bị đã được cắm, bật nguồn và không bị ứng dụng khác sử dụng. Có thể thử lại, chọn thiết bị khác hoặc thoát.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="120"/>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="189"/>
        <source>Quit</source>
        <translation>Thoát</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="130"/>
        <source>Pick Different Device</source>
        <translation>Chọn Thiết Bị Khác</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Reconnect</source>
        <translation>Kết Nối Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="141"/>
        <source>Try Again</source>
        <translation>Thử Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="157"/>
        <source>Pick the correct device, then press Connect.</source>
        <translation>Chọn thiết bị đúng, sau đó nhấn Kết Nối.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="166"/>
        <source>I/O Interface: %1</source>
        <translation>Giao Diện I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/RuntimeReconfigure.qml" line="199"/>
        <source>Connect</source>
        <translation>Kết Nối</translation>
    </message>
</context>
<context>
    <name>SerialStudio</name>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="338"/>
        <source>Data Grids</source>
        <translation>Lưới Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="341"/>
        <source>Multiple Data Plots</source>
        <translation>Nhiều Biểu Đồ Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="344"/>
        <source>Accelerometers</source>
        <translation>Gia Tốc Kế</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="347"/>
        <source>Gyroscopes</source>
        <translation>Con Quay Hồi Chuyển</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="350"/>
        <source>GPS</source>
        <translation>GPS</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="353"/>
        <source>FFT Plots</source>
        <translation>Biểu Đồ FFT</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="356"/>
        <source>LED Panels</source>
        <translation>Bảng LED</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="359"/>
        <source>Data Plots</source>
        <translation>Biểu Đồ Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="362"/>
        <source>Bars</source>
        <translation>Thanh</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="365"/>
        <source>Gauges</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="368"/>
        <source>Terminal</source>
        <translation>Terminal</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="371"/>
        <source>Clock</source>
        <translation>Đồng Hồ</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="374"/>
        <source>Stopwatch</source>
        <translation>Đồng Hồ Bấm Giờ</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="377"/>
        <source>Compasses</source>
        <translation>La Bàn</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="380"/>
        <source>Meters</source>
        <translation>Đồng Hồ Đo</translation>
    </message>
    <message>
        <source>Thermometers</source>
        <translation type="vanished">Nhiệt Kế</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="383"/>
        <source>3D Plots</source>
        <translation>Biểu Đồ 3D</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="386"/>
        <source>Web Views</source>
        <translation>Khung Xem Web</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="390"/>
        <source>Image Views</source>
        <translation>Khung Xem Hình Ảnh</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="393"/>
        <source>Output Panels</source>
        <translation>Bảng Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="396"/>
        <source>Notifications</source>
        <translation>Thông Báo</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="399"/>
        <source>Waterfalls</source>
        <translation>Thác Nước</translation>
    </message>
    <message>
        <location filename="../../src/SerialStudio.cpp" line="402"/>
        <source>Painter Widgets</source>
        <translation>Widget Vẽ</translation>
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
        <translation>Hệ Thống</translation>
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
        <translation>Chi Tiết Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="88"/>
        <source>Select a session to view details.</source>
        <translation>Chọn một phiên để xem chi tiết.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="130"/>
        <source>Project:</source>
        <translation>Dự Án:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="143"/>
        <source>Started:</source>
        <translation>Bắt Đầu:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="156"/>
        <source>Ended:</source>
        <translation>Kết Thúc:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="162"/>
        <source>(in progress)</source>
        <translation>(đang tiến hành)</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="169"/>
        <source>Frames:</source>
        <translation>Khung:</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="185"/>
        <source>Notes</source>
        <translation>Ghi Chú</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="200"/>
        <source>Add session notes…</source>
        <translation>Thêm ghi chú phiên…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="201"/>
        <source>Notes are read-only for completed sessions.</source>
        <translation>Ghi chú ở chế độ chỉ đọc đối với các phiên đã hoàn thành.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="222"/>
        <source>Tags</source>
        <translation>Thẻ</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="286"/>
        <source>New tag…</source>
        <translation>Thẻ mới…</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="293"/>
        <source>Add</source>
        <translation>Thêm</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="330"/>
        <source>Replay</source>
        <translation>Phát Lại</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="338"/>
        <source>Export CSV</source>
        <translation>Xuất CSV</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="345"/>
        <source>Generate Report</source>
        <translation>Tạo Báo Cáo</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="356"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionDetail.qml" line="362"/>
        <source>Unlock the session file to delete sessions</source>
        <translation>Mở khóa tệp phiên để xóa các phiên</translation>
    </message>
</context>
<context>
    <name>SessionList</name>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="19"/>
        <source>Sessions</source>
        <translation>Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="71"/>
        <source>Search</source>
        <translation>Tìm Kiếm</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="91"/>
        <source>Date</source>
        <translation>Ngày</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="92"/>
        <source>Frames</source>
        <translation>Frame</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="93"/>
        <source>Tags</source>
        <translation>Thẻ</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="193"/>
        <source>No sessions found.</source>
        <translation>Không tìm thấy phiên nào.</translation>
    </message>
    <message>
        <location filename="../../qml/DatabaseExplorer/SessionList.qml" line="194"/>
        <source>No session file open.</source>
        <translation>Chưa mở tệp phiên nào.</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseManager</name>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="428"/>
        <source>Open Session File</source>
        <translation>Mở Tệp Phiên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="430"/>
        <source>Session files (*.db)</source>
        <translation>Tệp phiên (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="486"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="495"/>
        <source>Lock Session File</source>
        <translation>Khóa Tệp Phiên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="487"/>
        <source>Choose a password to lock the session file:</source>
        <translation>Chọn mật khẩu để khóa tệp phiên:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="496"/>
        <source>Confirm the password:</source>
        <translation>Xác nhận mật khẩu:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="504"/>
        <source>Passwords do not match</source>
        <translation>Mật khẩu không khớp</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="505"/>
        <source>The two passwords you entered do not match. The session file was not locked.</source>
        <translation>Hai mật khẩu bạn nhập không khớp. Tệp phiên chưa được khóa.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="541"/>
        <source>Unlock Session File</source>
        <translation>Mở Khóa Tệp Phiên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="542"/>
        <source>Enter the session file password:</source>
        <translation>Nhập mật khẩu tệp phiên:</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="552"/>
        <source>Incorrect password</source>
        <translation>Mật khẩu không đúng</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="553"/>
        <source>The password you entered does not match the one stored in the session file.</source>
        <translation>Mật khẩu bạn nhập không khớp với mật khẩu được lưu trong tệp phiên.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="645"/>
        <source>Session file locked</source>
        <translation>Tệp phiên đã được khóa</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="646"/>
        <source>Unlock the session file before deleting recorded sessions.</source>
        <translation>Mở khóa tệp phiên trước khi xóa các phiên đã ghi.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="655"/>
        <source>Delete session from %1?</source>
        <translation>Xóa phiên từ %1?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="656"/>
        <source>All readings and raw data for this session are permanently removed.</source>
        <translation>Tất cả các số đo và dữ liệu thô cho phiên này sẽ bị xóa vĩnh viễn.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="658"/>
        <source>Delete Session</source>
        <translation>Xóa Phiên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>Export Session to CSV</source>
        <translation>Xuất Phiên sang CSV</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="780"/>
        <source>CSV files (*.csv)</source>
        <translation>Tệp CSV (*.CSV)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="860"/>
        <source>Loading session data…</source>
        <translation>Đang tải dữ liệu phiên…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save PDF Report</source>
        <translation>Lưu Báo Cáo PDF</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="898"/>
        <source>Save HTML Report</source>
        <translation>Lưu Báo Cáo HTML</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>PDF files (*.pdf)</source>
        <translation>Tệp PDF (*.PDF)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="899"/>
        <source>HTML files (*.html)</source>
        <translation>Tệp HTML (*.HTML)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="962"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Failed</source>
        <translation>Thất Bại</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="968"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1008"/>
        <source>Report Failed</source>
        <translation>Báo Cáo Thất Bại</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="970"/>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1009"/>
        <source>Could not generate the report. Check the output path and try again.</source>
        <translation>Không thể tạo báo cáo. Kiểm tra đường dẫn đầu ra và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="998"/>
        <source>Done</source>
        <translation>Hoàn Thành</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1026"/>
        <source>Select logo image</source>
        <translation>Chọn hình ảnh logo</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1028"/>
        <source>Images (*.png *.jpg *.jpeg *.svg)</source>
        <translation>Hình ảnh (*.png *.jpg *.jpeg *.svg)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1086"/>
        <source>No project data</source>
        <translation>Không có dữ liệu dự án</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1087"/>
        <source>This session file does not contain an embedded project.</source>
        <translation>Tệp phiên này không chứa dự án nhúng.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1096"/>
        <source>Invalid project data</source>
        <translation>Dữ liệu dự án không hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1097"/>
        <source>The embedded project JSON is malformed and cannot be restored.</source>
        <translation>JSON dự án nhúng bị lỗi định dạng và không thể khôi phục.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Restore Project</source>
        <translation>Khôi Phục Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1107"/>
        <source>Serial Studio projects (*.ssproj *.json)</source>
        <translation>Dự án Serial Studio (*.ssproj *.json)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Cannot write file</source>
        <translation>Không thể ghi tệp</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1115"/>
        <source>Check file permissions and try again.</source>
        <translation>Kiểm tra quyền truy cập tệp và thử lại.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseManager.cpp" line="1225"/>
        <source>Cannot open session file</source>
        <translation>Không thể mở tệp phiên</translation>
    </message>
</context>
<context>
    <name>Sessions::DatabaseWorker</name>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="77"/>
        <source>Empty file path</source>
        <translation>Đường dẫn tệp trống</translation>
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
        <translation>Cơ sở dữ liệu chưa mở</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="263"/>
        <source>Database not open or empty label</source>
        <translation>Cơ sở dữ liệu chưa mở hoặc nhãn trống</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="331"/>
        <source>Invalid label</source>
        <translation>Nhãn không hợp lệ</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="598"/>
        <source>Cancelled</source>
        <translation>Đã Hủy</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/DatabaseWorker.cpp" line="717"/>
        <source>Could not load session data</source>
        <translation>Không thể tải dữ liệu phiên</translation>
    </message>
</context>
<context>
    <name>Sessions::HtmlReport</name>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="209"/>
        <source>Assembling report…</source>
        <translation>Đang tổng hợp báo cáo…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="217"/>
        <source>Writing output…</source>
        <translation>Đang ghi đầu ra…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="282"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="342"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="700"/>
        <source>Session Report</source>
        <translation>Báo Cáo Phiên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="345"/>
        <source>Untitled project</source>
        <translation>Dự án chưa đặt tên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="352"/>
        <source>Prepared by</source>
        <translation>Được chuẩn bị bởi</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="355"/>
        <source>Generated on %1</source>
        <translation>Được tạo vào %1</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="377"/>
        <source>Test ID</source>
        <translation>ID Kiểm Tra</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="379"/>
        <source>Duration</source>
        <translation>Thời Lượng</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="381"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="493"/>
        <source>Samples</source>
        <translation>Mẫu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="383"/>
        <source>Parameters</source>
        <translation>Tham Số</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="385"/>
        <source>Started</source>
        <translation>Bắt Đầu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="387"/>
        <source>Ended</source>
        <translation>Kết Thúc</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="423"/>
        <source>Project</source>
        <translation>Dự Án</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="425"/>
        <source>Test identifier</source>
        <translation>Mã Định Danh Kiểm Tra</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="426"/>
        <source>Start time</source>
        <translation>Thời Gian Bắt Đầu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="427"/>
        <source>End time</source>
        <translation>Thời Gian Kết Thúc</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="428"/>
        <source>Total duration</source>
        <translation>Tổng Thời Lượng</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="429"/>
        <source>Samples acquired</source>
        <translation>Mẫu Đã Thu Thập</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="430"/>
        <source>Parameters logged</source>
        <translation>Tham Số Đã Ghi</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="446"/>
        <source>Classification</source>
        <translation>Phân Loại</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="453"/>
        <source>Notes</source>
        <translation>Ghi Chú</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="461"/>
        <source>Test Information</source>
        <translation>Thông Tin Kiểm Tra</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="482"/>
        <source>Parameter</source>
        <translation>Tham Số</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="485"/>
        <source>Units</source>
        <translation>Đơn Vị</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="494"/>
        <source>Minimum</source>
        <translation>Tối Thiểu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="495"/>
        <source>Maximum</source>
        <translation>Tối Đa</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="496"/>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="652"/>
        <source>Mean</source>
        <translation>Trung Bình</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="497"/>
        <source>Std. Deviation</source>
        <translation>Độ Lệch Chuẩn</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="542"/>
        <source>Measurement Summary</source>
        <translation>Tóm Tắt Đo Lường</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="543"/>
        <source>click a column to sort</source>
        <translation>nhấp vào cột để sắp xếp</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="568"/>
        <source>%1 samples over %2 seconds</source>
        <translation>%1 mẫu trong %2 giây</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="586"/>
        <source>Combined Parameter View</source>
        <translation>Chế Độ Xem Tham Số Kết Hợp</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="587"/>
        <source>click legend items to toggle signals</source>
        <translation>nhấp vào mục chú giải để bật/tắt tín hiệu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="595"/>
        <source>Parameter Trends</source>
        <translation>Xu Hướng Tham Số</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="650"/>
        <source>Min</source>
        <translation>Tối Thiểu</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="651"/>
        <source>Max</source>
        <translation>Tối Đa</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="724"/>
        <source>Page %1 of %2</source>
        <translation>Trang %1 / %2</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="794"/>
        <source>Loading rendering engine…</source>
        <translation>Đang tải công cụ kết xuất…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="814"/>
        <source>Rendering charts…</source>
        <translation>Đang vẽ biểu đồ…</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/HtmlReport.cpp" line="858"/>
        <source>Generating PDF…</source>
        <translation>Đang Tạo PDF…</translation>
    </message>
</context>
<context>
    <name>Sessions::Player</name>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="261"/>
        <source>Open Session File</source>
        <translation>Mở Tệp Phiên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="263"/>
        <source>Session files (*.db)</source>
        <translation>Tệp phiên (*.db)</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="334"/>
        <source>Device Connection Active</source>
        <translation>Kết Nối Thiết Bị Đang Hoạt Động</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="335"/>
        <source>To use this feature, you must disconnect from the device. Do you want to proceed?</source>
        <translation>Để sử dụng tính năng này, bạn phải ngắt kết nối khỏi thiết bị. Bạn có muốn tiếp tục không?</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="383"/>
        <location filename="../../src/Sessions/Player.cpp" line="462"/>
        <source>Cannot open session file</source>
        <translation>Không thể mở tệp phiên</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="384"/>
        <source>Unknown error</source>
        <translation>Lỗi không xác định</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="400"/>
        <source>No project data</source>
        <translation>Không có dữ liệu dự án</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="401"/>
        <source>This session does not contain an embedded project file — the dashboard falls back to a quick-plot layout.</source>
        <translation>Phiên này không chứa tệp dự án nhúng — bảng điều khiển sẽ sử dụng bố cục vẽ nhanh.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/Player.cpp" line="463"/>
        <source>Check file permissions and try again.</source>
        <translation>Kiểm tra quyền truy cập tệp và thử lại.</translation>
    </message>
</context>
<context>
    <name>Sessions::PlayerLoaderWorker</name>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="69"/>
        <source>This file does not contain any recording sessions.</source>
        <translation>Tệp này không chứa bất kỳ phiên ghi hình nào.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="144"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="205"/>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="224"/>
        <source>Cancelled</source>
        <translation>Đã Hủy</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="168"/>
        <source>Empty file path</source>
        <translation>Đường dẫn tệp trống</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="218"/>
        <source>The selected session is missing its column definitions.</source>
        <translation>Phiên đã chọn thiếu định nghĩa cột.</translation>
    </message>
    <message>
        <location filename="../../src/Sessions/PlayerLoaderWorker.cpp" line="235"/>
        <source>The selected session does not contain any frames.</source>
        <translation>Phiên đã chọn không chứa bất kỳ khung nào.</translation>
    </message>
</context>
<context>
    <name>Settings</name>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="34"/>
        <source>Preferences</source>
        <translation>Tùy Chọn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="61"/>
        <source>General</source>
        <translation>Chung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="67"/>
        <source>Startup</source>
        <translation>Khởi Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="73"/>
        <source>Dashboard</source>
        <translation>Bảng Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="79"/>
        <source>Taskbar</source>
        <translation>Thanh Tác Vụ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="85"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1163"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="92"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1164"/>
        <source>Notifications</source>
        <translation>Thông Báo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="151"/>
        <source>Appearance</source>
        <translation>Giao Diện</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="166"/>
        <source>Language</source>
        <translation>Ngôn Ngữ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="182"/>
        <source>Theme</source>
        <translation>Giao Diện</translation>
    </message>
    <message>
        <source>Files &amp; Updates</source>
        <translation type="vanished">Tệp &amp; Cập Nhật</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="289"/>
        <source>Workspace Folder</source>
        <translation>Thư Mục Workspace</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="585"/>
        <source>Automatically Check for Updates</source>
        <translation>Tự Động Kiểm Tra Cập Nhật</translation>
    </message>
    <message>
        <source>Advanced</source>
        <translation type="vanished">Nâng Cao</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="470"/>
        <source>Rendering Backend</source>
        <translation>Backend Kết Xuất</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="888"/>
        <source>Auto-Hide Toolbar</source>
        <translation>Tự Động Ẩn Thanh Công Cụ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="273"/>
        <source>Files</source>
        <translation>Tệp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="336"/>
        <source>Enable API Server (Port 7777)</source>
        <translation>Bật Máy Chủ API (Cổng 7777)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="354"/>
        <source>Allow External API Connections</source>
        <translation>Cho Phép Kết Nối API từ Bên Ngoài</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="370"/>
        <source>API Access Token</source>
        <translation>Mã Truy Cập API</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="400"/>
        <source>Export Protobuf File</source>
        <translation>Xuất Tệp Protobuf</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="402"/>
        <source>Export…</source>
        <translation>Xuất…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="452"/>
        <source>Graphics</source>
        <translation>Đồ Họa</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="508"/>
        <source>System</source>
        <translation>Hệ Thống</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="524"/>
        <source>Apply Performance Hints</source>
        <translation>Áp Dụng Gợi Ý Hiệu Năng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="538"/>
        <source>Keep Display Awake</source>
        <translation>Giữ Màn Hình Luôn Bật</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="558"/>
        <source>Performance hints raise process priority and opt out of OS power throttling. Changes take effect the next time Serial Studio starts.</source>
        <translation>Gợi ý hiệu năng tăng mức ưu tiên tiến trình và loại trừ điều tiết nguồn của hệ điều hành. Thay đổi có hiệu lực khi Serial Studio khởi động lần tiếp theo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="569"/>
        <source>Updates &amp; News</source>
        <translation>Cập Nhật &amp; Tin Tức</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="599"/>
        <source>Show What's New on Startup</source>
        <translation>Hiển Thị Có Gì Mới Khi Khởi Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="649"/>
        <source>Data Plotting</source>
        <translation>Vẽ Đồ Thị Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="716"/>
        <source>Point Count</source>
        <translation>Số Lượng Điểm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="664"/>
        <source>Time Range</source>
        <translation>Phạm Vi Thời Gian</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="202"/>
        <source>Window</source>
        <translation>Cửa Sổ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="221"/>
        <source>Custom Window Decorations</source>
        <translation>Trang Trí Cửa Sổ Tùy Chỉnh</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="237"/>
        <source>Window Shadow</source>
        <translation>Bóng Cửa Sổ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="264"/>
        <source>Window decoration changes apply after restarting %1.</source>
        <translation>Thay đổi trang trí cửa sổ có hiệu lực sau khi khởi động lại %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="320"/>
        <source>API &amp; Plugins</source>
        <translation>API &amp; Plugin</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="741"/>
        <source>UI Refresh Rate (Hz)</source>
        <translation>Tốc Độ Làm Mới Giao Diện (Hz)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="763"/>
        <source>Dashboard Font</source>
        <translation>Phông Chữ Bảng Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="778"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1326"/>
        <source>Font Family</source>
        <translation>Họ Phông Chữ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="793"/>
        <location filename="../../qml/Dialogs/Settings.qml" line="1341"/>
        <source>Font Size</source>
        <translation>Cỡ Phông Chữ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Small</source>
        <translation>Nhỏ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Normal</source>
        <translation>Bình Thường</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Large</source>
        <translation>Lớn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Extra Large</source>
        <translation>Rất Lớn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="803"/>
        <source>Custom</source>
        <translation>Tùy Chỉnh</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="857"/>
        <source>Layout</source>
        <translation>Bố Cục</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="872"/>
        <source>Show Actions Panel</source>
        <translation>Hiển Thị Bảng Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="904"/>
        <source>Auto-Layout Margin</source>
        <translation>Lề Bố Cục Tự Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="929"/>
        <source>Auto-Layout Spacing</source>
        <translation>Khoảng Cách Bố Cục Tự Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="961"/>
        <source>Video Export</source>
        <translation>Xuất Video</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="979"/>
        <source>Save Videos by Default</source>
        <translation>Lưu Video Theo Mặc Định</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1030"/>
        <source>Behavior</source>
        <translation>Hành Vi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1051"/>
        <source>Always Show Taskbar Buttons</source>
        <translation>Luôn Hiển Thị Các Nút Thanh Tác Vụ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1066"/>
        <source>Show Search Field</source>
        <translation>Hiển Thị Trường Tìm Kiếm</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1081"/>
        <source>Auto-hide Taskbar</source>
        <translation>Tự Động Ẩn Thanh Tác Vụ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1099"/>
        <source>Hide Delay (ms)</source>
        <translation>Độ Trễ Ẩn (ms)</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1123"/>
        <source>Pinned Buttons</source>
        <translation>Các Nút Đã Ghim</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1141"/>
        <source>Drag a pinned button on the taskbar to reorder it.</source>
        <translation>Kéo một nút đã ghim trên thanh tác vụ để sắp xếp lại.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1162"/>
        <source>Settings</source>
        <translation>Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1165"/>
        <source>Clock</source>
        <translation>Đồng Hồ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1166"/>
        <source>Stopwatch</source>
        <translation>Đồng Hồ Bấm Giờ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1167"/>
        <source>Pause / Resume</source>
        <translation>Tạm Dừng / Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1168"/>
        <source>File Transmission</source>
        <translation>Truyền Tệp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1169"/>
        <source>AI Assistant</source>
        <translation>Trợ Lý AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1298"/>
        <source>Display</source>
        <translation>Hiển Thị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1313"/>
        <source>Display Mode</source>
        <translation>Chế Độ Hiển Thị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1358"/>
        <source>Show Timestamps</source>
        <translation>Hiển Thị Dấu Thời Gian</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1377"/>
        <source>Data Transmission</source>
        <translation>Truyền Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1392"/>
        <source>Line Ending</source>
        <translation>Kết Thúc Dòng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1405"/>
        <source>Input Mode</source>
        <translation>Chế Độ Đầu Vào</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1418"/>
        <source>Text Encoding</source>
        <translation>Mã Hóa Văn Bản</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1431"/>
        <source>Checksum</source>
        <translation>Checksum</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1444"/>
        <source>Echo Sent Data</source>
        <translation>Phản Hồi Dữ Liệu Đã Gửi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1463"/>
        <source>Escape Codes</source>
        <translation>Mã Escape</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1478"/>
        <source>VT100 Emulation</source>
        <translation>Mô Phỏng VT100</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1497"/>
        <source>ANSI Colors</source>
        <translation>Màu ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1555"/>
        <source>Delivery</source>
        <translation>Phân Phối</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1570"/>
        <source>System Notifications</source>
        <translation>Thông Báo Hệ Thống</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1591"/>
        <source>Show Warning/Critical events as OS desktop notifications when Serial Studio is not the foreground window.</source>
        <translation>Hiển thị sự kiện Cảnh báo/Nghiêm trọng dưới dạng thông báo desktop của hệ điều hành khi Serial Studio không phải là cửa sổ đang hoạt động.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1601"/>
        <source>Application Logs</source>
        <translation>Nhật Ký Ứng Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1616"/>
        <source>Route Warnings to Notifications</source>
        <translation>Chuyển Cảnh Báo sang Thông Báo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1637"/>
        <source>Off by default — Qt and QML emit warnings frequently and enabling this can drown out real alarms. Critical messages are always routed regardless of this setting.</source>
        <translation>Tắt theo mặc định — QT và QML phát ra cảnh báo thường xuyên và bật tính năng này có thể làm lu mờ các cảnh báo thực sự. Thông báo nghiêm trọng luôn được chuyển bất kể cài đặt này.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1656"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1696"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Settings.qml" line="1704"/>
        <source>Apply</source>
        <translation>Áp Dụng</translation>
    </message>
</context>
<context>
    <name>Setup</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="35"/>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="380"/>
        <source>Device Setup</source>
        <translation>Thiết Lập Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="126"/>
        <source>API Server Active (%1)</source>
        <translation>Máy Chủ API Đang Hoạt Động (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="127"/>
        <source>API Server Ready</source>
        <translation>Máy Chủ API Sẵn Sàng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="128"/>
        <source>API Server Off</source>
        <translation>Máy Chủ API Tắt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="188"/>
        <source>Frame Parsing</source>
        <translation>Phân Tích Frame</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="198"/>
        <source>Console Only (No Parsing)</source>
        <translation>Chỉ Console (Không Phân Tích)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="211"/>
        <source>Quick Plot (Comma Separated Values)</source>
        <translation>Vẽ Đồ Thị Nhanh (Giá Trị Phân Cách bằng Dấu Phẩy)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="222"/>
        <source>Parse via Project File</source>
        <translation>Phân Tích qua Tệp Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="245"/>
        <source>Change Project File (%1)</source>
        <translation>Thay Đổi Tệp Dự Án (%1)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="246"/>
        <source>Select Project File</source>
        <translation>Chọn Tệp Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="261"/>
        <source>Data Export</source>
        <translation>Xuất Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="285"/>
        <source>CSV Spreadsheet</source>
        <translation>Bảng Tính CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="303"/>
        <source>Session Recording</source>
        <translation>Ghi Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="324"/>
        <source>MDF4 Recording</source>
        <translation>Ghi MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="340"/>
        <source>Console Log</source>
        <translation>Nhật Ký Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="392"/>
        <source>I/O Interface: %1</source>
        <translation>Giao Diện I/O: %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="461"/>
        <source>Multi-Device Project</source>
        <translation>Dự Án Đa Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="474"/>
        <source>This project streams data from %1 independent devices.</source>
        <translation>Dự án này truyền dữ liệu từ %1 thiết bị độc lập.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="487"/>
        <source>Each device has its own connection settings. Configure them in the Project Editor under the Sources tab.</source>
        <translation>Mỗi thiết bị có cài đặt kết nối riêng. Cấu hình chúng trong Trình Chỉnh Sửa Dự Án ở tab Nguồn.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Setup.qml" line="506"/>
        <source>Open Project Editor</source>
        <translation>Mở Trình Chỉnh Sửa Dự Án</translation>
    </message>
</context>
<context>
    <name>ShortcutGenerator</name>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="22"/>
        <source>New Deployment</source>
        <translation>Triển Khai Mới</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="93"/>
        <source>Choose an Icon</source>
        <translation>Chọn Biểu Tượng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="109"/>
        <source>Save Deployment</source>
        <translation>Lưu Triển Khai</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="152"/>
        <source>General</source>
        <translation>Chung</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="158"/>
        <source>Taskbar</source>
        <translation>Thanh Tác Vụ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="164"/>
        <source>Logging</source>
        <translation>Ghi Nhật Ký</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="221"/>
        <source>Identity</source>
        <translation>Định Danh</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="277"/>
        <source>Click to choose an icon</source>
        <translation>Nhấp để chọn biểu tượng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="286"/>
        <source>Name:</source>
        <translation>Tên:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="295"/>
        <source>Deployment Name</source>
        <translation>Tên Triển Khai</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="304"/>
        <source>Change Icon…</source>
        <translation>Đổi Biểu Tượng…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="321"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="339"/>
        <source>Project</source>
        <translation>Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="349"/>
        <source>Choose a project file to begin</source>
        <translation>Chọn tệp dự án để bắt đầu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="371"/>
        <source>Behavior</source>
        <translation>Hành Vi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="386"/>
        <source>Theme</source>
        <translation>Giao Diện</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="396"/>
        <source>Same as Serial Studio</source>
        <translation>Giống Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="408"/>
        <source>Fullscreen</source>
        <translation>Toàn Màn Hình</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="418"/>
        <source>Actions Panel</source>
        <translation>Bảng Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="429"/>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="638"/>
        <source>File Transmission</source>
        <translation>Truyền Tệp</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="445"/>
        <source>Double-clicking this deployment takes someone straight to the live dashboard for this project. There's no toolbar or setup pane, just the data, and Serial Studio quits as soon as the device disconnects.</source>
        <translation>Nhấp đúp vào triển khai này sẽ đưa người dùng thẳng đến dashboard trực tiếp cho dự án này. Không có thanh công cụ hay ngăn thiết lập, chỉ có dữ liệu, và Serial Studio thoát ngay khi thiết bị ngắt kết nối.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="491"/>
        <source>Visibility</source>
        <translation>Hiển Thị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="506"/>
        <source>Mode</source>
        <translation>Chế Độ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="515"/>
        <source>Always shown</source>
        <translation>Luôn Hiển Thị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="516"/>
        <source>Auto-hide</source>
        <translation>Tự Động Ẩn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="517"/>
        <source>Hidden</source>
        <translation>Đã Ẩn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="532"/>
        <source>Hiding the taskbar removes window minimize/maximize/close buttons and forces auto-layout, so the dashboard always fills the available area.</source>
        <translation>Ẩn thanh tác vụ sẽ loại bỏ các nút thu nhỏ/phóng to/đóng cửa sổ và buộc bố cục tự động, do đó dashboard luôn lấp đầy vùng khả dụng.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="536"/>
        <source>The taskbar slides in when the user moves the cursor near the bottom edge.</source>
        <translation>Thanh tác vụ trượt vào khi người dùng di chuyển con trỏ gần cạnh dưới.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="538"/>
        <source>The taskbar is permanently visible at the bottom of the dashboard.</source>
        <translation>Thanh tác vụ hiển thị cố định ở cuối dashboard.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="551"/>
        <source>Pinned Buttons</source>
        <translation>Nút Ghim</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="568"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="582"/>
        <source>Notifications</source>
        <translation>Thông Báo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="596"/>
        <source>Clock</source>
        <translation>Đồng Hồ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="610"/>
        <source>Stopwatch</source>
        <translation>Đồng Hồ Bấm Giờ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="624"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="688"/>
        <source>Recorders</source>
        <translation>Bộ Ghi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="703"/>
        <source>CSV File</source>
        <translation>Tệp CSV</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="713"/>
        <source>MDF4 File</source>
        <translation>Tệp MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="723"/>
        <source>Session Database</source>
        <translation>Cơ Sở Dữ Liệu Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="733"/>
        <source>Console Log</source>
        <translation>Nhật Ký Console</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="749"/>
        <source>Recordings are saved in the Serial Studio workspace folder</source>
        <translation>Bản ghi được lưu trong thư mục làm việc của Serial Studio</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="778"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/ShortcutGenerator.qml" line="787"/>
        <source>Save</source>
        <translation>Lưu</translation>
    </message>
</context>
<context>
    <name>SourceFrameParserView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="80"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="317"/>
        <source>Undo</source>
        <translation>Hoàn Tác</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="87"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="328"/>
        <source>Redo</source>
        <translation>Làm Lại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="96"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="347"/>
        <source>Cut</source>
        <translation>Cắt</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="101"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="357"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="106"/>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="367"/>
        <source>Paste</source>
        <translation>Dán</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="113"/>
        <source>Select All</source>
        <translation>Chọn Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="123"/>
        <source>Format Document</source>
        <translation>Định Dạng Tài Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="130"/>
        <source>Format Selection</source>
        <translation>Định Dạng Vùng Chọn</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="297"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="382"/>
        <source>Validate</source>
        <translation>Xác Thực</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="386"/>
        <source>Verify that the script compiles correctly</source>
        <translation>Xác minh rằng script biên dịch chính xác</translation>
    </message>
    <message>
        <source>Reset template parameters to their defaults</source>
        <translation type="vanished">Đặt lại tham số mẫu về giá trị mặc định</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="302"/>
        <source>Reset to the default parsing script</source>
        <translation>Đặt lại về script phân tích mặc định</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="307"/>
        <source>Open</source>
        <translation>Mở</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="312"/>
        <source>Import a script file for data parsing</source>
        <translation>Nhập file script để phân tích dữ liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="322"/>
        <source>Undo the last code edit</source>
        <translation>Hoàn tác chỉnh sửa mã gần nhất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="334"/>
        <source>Redo the previously undone edit</source>
        <translation>Làm lại thao tác vừa hoàn tác</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="352"/>
        <source>Cut selected code to clipboard</source>
        <translation>Cắt mã đã chọn vào clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="362"/>
        <source>Copy selected code to clipboard</source>
        <translation>Sao chép mã đã chọn vào clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="371"/>
        <source>Paste code from clipboard</source>
        <translation>Dán mã từ clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="247"/>
        <source>Help</source>
        <translation>Trợ Giúp</translation>
    </message>
    <message>
        <source>Open help documentation for data parsing</source>
        <translation type="vanished">Mở tài liệu trợ giúp về phân tích dữ liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="196"/>
        <source>Platform:</source>
        <translation>Nền Tảng:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="204"/>
        <source>Built-In</source>
        <translation>Built-in</translation>
    </message>
    <message>
        <source>Language:</source>
        <translation type="vanished">Ngôn Ngữ:</translation>
    </message>
    <message>
        <source>Native</source>
        <translation type="vanished">Gốc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="222"/>
        <source>Select Template…</source>
        <translation>Chọn Template…</translation>
    </message>
    <message>
        <source>Template:</source>
        <translation type="vanished">Mẫu:</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceFrameParserView.qml" line="238"/>
        <source>Test With Sample Data</source>
        <translation>Kiểm Tra với Dữ Liệu Mẫu</translation>
    </message>
    <message>
        <source>Evaluate</source>
        <translation type="vanished">Đánh Giá</translation>
    </message>
</context>
<context>
    <name>SourceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="107"/>
        <source>Duplicate</source>
        <translation>Nhân Bản</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="109"/>
        <source>Create a copy of this data source</source>
        <translation>Tạo bản sao của nguồn dữ liệu này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="121"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="126"/>
        <source>Remove this data source</source>
        <translation>Xóa nguồn dữ liệu này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SourceView.qml" line="127"/>
        <source>The primary data source cannot be removed</source>
        <translation>Nguồn dữ liệu chính không thể xóa</translation>
    </message>
</context>
<context>
    <name>SqlitePlayer</name>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="24"/>
        <source>Session Player</source>
        <translation>Trình Phát Session</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/SqlitePlayer.qml" line="91"/>
        <source>Loading session…</source>
        <translation>Đang tải phiên…</translation>
    </message>
</context>
<context>
    <name>StartMenu</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="101"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="552"/>
        <source>Auto Layout</source>
        <translation>Bố Cục Tự Động</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="109"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="564"/>
        <source>Full Screen</source>
        <translation>Toàn Màn Hình</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="115"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="576"/>
        <source>Add External Window</source>
        <translation>Thêm Cửa Sổ Bên Ngoài</translation>
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
        <translation>Thông Báo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="135"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="765"/>
        <source>Clock</source>
        <translation>Đồng Hồ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="143"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="772"/>
        <source>Stopwatch</source>
        <translation>Đồng Hồ Bấm Giờ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="151"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="780"/>
        <source>Preferences</source>
        <translation>Tùy Chọn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="157"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="861"/>
        <source>Help Center</source>
        <translation>Trung Tâm Trợ Giúp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="163"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="789"/>
        <source>Sessions</source>
        <translation>Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="170"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="798"/>
        <source>File Transmission</source>
        <translation>Truyền Tệp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="177"/>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="806"/>
        <source>AI Assistant</source>
        <translation>Trợ Lý AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="278"/>
        <source>Workspaces</source>
        <translation>Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="352"/>
        <source>Show "%1"</source>
        <translation>Hiển Thị "%1"</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="357"/>
        <source>Show All Hidden Workspaces</source>
        <translation>Hiển Thị Tất Cả Không Gian Làm Việc Ẩn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="366"/>
        <source>New Workspace…</source>
        <translation>Không Gian Làm Việc Mới…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="380"/>
        <source>No Workspaces Available</source>
        <translation>Không Có Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="415"/>
        <source>Actions</source>
        <translation>Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="438"/>
        <source>No Actions Available</source>
        <translation>Không Có Hành Động</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="468"/>
        <source>Plugins</source>
        <translation>Plugin</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="506"/>
        <source>Manage Plugins…</source>
        <translation>Quản Lý Plugin…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="516"/>
        <source>No Plugins Installed</source>
        <translation>Không Có Plugin Nào Được Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="596"/>
        <source>Export</source>
        <translation>Xuất</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="627"/>
        <source>CSV File</source>
        <translation>Tệp CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="633"/>
        <source>MDF4 File</source>
        <translation>Tệp MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="639"/>
        <source>Console Transcript</source>
        <translation>Bản Ghi Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="648"/>
        <source>Session Database</source>
        <translation>Cơ Sở Dữ Liệu Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="662"/>
        <source>No Export Formats Available</source>
        <translation>Không Có Định Dạng Xuất Nào</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="692"/>
        <source>Tools</source>
        <translation>Công Cụ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="821"/>
        <source>No Tools Available</source>
        <translation>Không Có Công Cụ Nào</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="882"/>
        <source>Resume</source>
        <translation>Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="883"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="889"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Quit</source>
        <translation>Thoát</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="910"/>
        <source>Disconnect</source>
        <translation>Ngắt Kết Nối</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="936"/>
        <source>Edit…</source>
        <translation>Chỉnh Sửa…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="947"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/StartMenu.qml" line="948"/>
        <source>Hide</source>
        <translation>Ẩn</translation>
    </message>
</context>
<context>
    <name>Stopwatch</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Stop</source>
        <translation>Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="200"/>
        <source>Start</source>
        <translation>Bắt Đầu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="210"/>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="288"/>
        <source>Lap</source>
        <translation>Vòng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="226"/>
        <source>Reset</source>
        <translation>Đặt Lại</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="279"/>
        <source>#</source>
        <translation>#</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="297"/>
        <source>Total</source>
        <translation>Tổng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="393"/>
        <source>No laps recorded</source>
        <translation>Chưa ghi nhận vòng nào</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Stopwatch.qml" line="401"/>
        <source>Press Lap while the stopwatch is running</source>
        <translation>Nhấn Vòng khi đồng hồ bấm giờ đang chạy</translation>
    </message>
</context>
<context>
    <name>SubMenuCombo</name>
    <message>
        <location filename="../../qml/Widgets/SubMenuCombo.qml" line="160"/>
        <source>No Data Available</source>
        <translation>Không Có Dữ Liệu</translation>
    </message>
</context>
<context>
    <name>SystemDatasetsView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="33"/>
        <source>Dataset Values</source>
        <translation>Giá Trị Dataset</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="158"/>
        <source>Search</source>
        <translation>Tìm Kiếm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="179"/>
        <source>ID</source>
        <translation>ID</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="180"/>
        <source>Group</source>
        <translation>Nhóm</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="181"/>
        <source>Dataset</source>
        <translation>Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="182"/>
        <source>Units</source>
        <translation>Đơn Vị</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="252"/>
        <source>(virtual)</source>
        <translation>(ảo)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="298"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Sao chép mã truy cập %1 vào clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="323"/>
        <source>No datasets defined in this project.</source>
        <translation>Không có tập dữ liệu nào được định nghĩa trong dự án này.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="324"/>
        <source>No datasets match your search.</source>
        <translation>Không có tập dữ liệu nào khớp với tìm kiếm của bạn.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/SystemDatasetsView.qml" line="374"/>
        <source>Dataset access code copied</source>
        <translation>Đã sao chép mã truy cập tập dữ liệu</translation>
    </message>
</context>
<context>
    <name>TableDelegate</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="130"/>
        <source>Parameter</source>
        <translation>Tham Số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="151"/>
        <source>Value</source>
        <translation>Giá Trị</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="514"/>
        <source>(Custom Icon)</source>
        <translation>(Biểu Tượng Tùy Chỉnh)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="639"/>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="645"/>
        <source>Auto</source>
        <translation>Tự Động</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>No</source>
        <translation>Không</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableDelegate.qml" line="813"/>
        <source>Yes</source>
        <translation>Có</translation>
    </message>
</context>
<context>
    <name>TableFolderView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="41"/>
        <source>Folder</source>
        <translation>Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="126"/>
        <source>Add Sub-folder</source>
        <translation>Thêm Thư Mục Con</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="128"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Thêm thư mục con bên trong thư mục này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="136"/>
        <source>Add Shared Table</source>
        <translation>Thêm Bảng Chia Sẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="138"/>
        <source>Add a shared table inside this folder</source>
        <translation>Thêm bảng chia sẻ bên trong thư mục này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="148"/>
        <source>Rename</source>
        <translation>Đổi Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="150"/>
        <source>Rename folder</source>
        <translation>Đổi tên thư mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="158"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="160"/>
        <source>Delete folder</source>
        <translation>Xóa thư mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="173"/>
        <source>Title</source>
        <translation>Tiêu Đề</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="174"/>
        <source>Registers</source>
        <translation>Thanh Ghi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/TableFolderView.qml" line="264"/>
        <source>This folder is empty. Use the toolbar to add a table or sub-folder.</source>
        <translation>Thư mục này trống. Sử dụng thanh công cụ để thêm bảng hoặc thư mục con.</translation>
    </message>
</context>
<context>
    <name>Taskbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="66"/>
        <source>Start Menu</source>
        <translation>Menu Khởi Động</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="194"/>
        <source>Menu</source>
        <translation>Menu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="230"/>
        <source>Search…</source>
        <translation>Tìm Kiếm…</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="496"/>
        <source>Settings</source>
        <translation>Cài Đặt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="497"/>
        <source>Console</source>
        <translation>Console</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="498"/>
        <source>Notifications</source>
        <translation>Thông Báo</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="499"/>
        <source>Clock</source>
        <translation>Đồng Hồ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="500"/>
        <source>Stopwatch</source>
        <translation>Đồng Hồ Bấm Giờ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="501"/>
        <source>File Transmission</source>
        <translation>Truyền Tệp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="502"/>
        <source>AI Assistant</source>
        <translation>Trợ Lý AI</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Resume</source>
        <translation>Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="503"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="986"/>
        <source>MQTT: Connected to %1</source>
        <translation>MQTT: Đã kết nối tới %1</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="987"/>
        <source>MQTT: Not connected</source>
        <translation>MQTT: Chưa kết nối</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1011"/>
        <source>MQTT Publisher</source>
        <translation>MQTT Publisher</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1021"/>
        <source>Status:</source>
        <translation>Trạng Thái:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1029"/>
        <source>Connected</source>
        <translation>Đã Kết Nối</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1030"/>
        <source>Disconnected</source>
        <translation>Đã Ngắt Kết Nối</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1037"/>
        <source>Broker:</source>
        <translation>Broker:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1050"/>
        <source>Mode:</source>
        <translation>Chế Độ:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1063"/>
        <source>Messages sent:</source>
        <translation>Tin nhắn đã gửi:</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/Taskbar.qml" line="1077"/>
        <source>Open MQTT Settings</source>
        <translation>Mở Cài Đặt MQTT</translation>
    </message>
</context>
<context>
    <name>Terminal</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="141"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="149"/>
        <source>Select all</source>
        <translation>Chọn tất cả</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="155"/>
        <source>Clear</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="253"/>
        <source>Send Data to Device</source>
        <translation>Gửi Dữ Liệu đến Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="428"/>
        <source>Show Timestamp</source>
        <translation>Hiển Thị Dấu Thời Gian</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="436"/>
        <source>Echo</source>
        <translation>Echo</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="453"/>
        <source>Emulate VT-100</source>
        <translation>Mô Phỏng VT-100</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="466"/>
        <source>ANSI Colors</source>
        <translation>Màu ANSI</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Terminal.qml" line="486"/>
        <source>Display: %1</source>
        <translation>Hiển Thị: %1</translation>
    </message>
</context>
<context>
    <name>Tips</name>
    <message>
        <source>Did You Know?</source>
        <translation type="vanished">Bạn Có Biết?</translation>
    </message>
    <message>
        <source>Keep your firmware simple by sending raw data and letting Serial Studio parse it in JavaScript, Lua, or code-free Built-In templates.</source>
        <translation type="vanished">Giữ firmware đơn giản bằng cách gửi dữ liệu thô và để Serial Studio phân tích bằng JavaScript, Lua hoặc mẫu tích hợp sẵn không cần mã.</translation>
    </message>
    <message>
        <source>Give each channel its own function to calibrate, filter, or convert units. Offload the math to Serial Studio and keep your firmware lean.</source>
        <translation type="vanished">Gán cho mỗi kênh một hàm riêng để hiệu chuẩn, lọc hoặc chuyển đổi đơn vị. Để Serial Studio xử lý toán học và giữ firmware gọn nhẹ.</translation>
    </message>
    <message>
        <source>Need a value your device never sends? A virtual dataset computes its own channel, like power from voltage and current, plotted and logged as data.</source>
        <translation type="vanished">Cần một giá trị mà thiết bị không gửi? Dataset ảo tự tính toán kênh riêng, như công suất từ điện áp và dòng điện, được vẽ và ghi lại như dữ liệu.</translation>
    </message>
    <message>
        <source>Catch glitches like a bench scope. Time-axis plots have a sweep and trigger mode, and you can drag the trigger level right on the plot.</source>
        <translation type="vanished">Bắt lỗi như một oscilloscope bàn. Biểu đồ theo trục thời gian có chế độ quét và kích hoạt, bạn có thể kéo mức kích hoạt ngay trên biểu đồ.</translation>
    </message>
    <message>
        <source>Stop scrolling to find the right widget. Group them into your own workspaces and jump between them from the taskbar search.</source>
        <translation type="vanished">Không còn cuộn tìm widget phù hợp. Gom nhóm chúng vào workspace riêng và chuyển đổi nhanh từ thanh tìm kiếm taskbar.</translation>
    </message>
    <message>
        <source>Never lose a test run again. Record sessions to a local database, then browse, tag, and replay them whenever you need them.</source>
        <translation type="vanished">Không bao giờ mất phiên kiểm tra nữa. Ghi lại session vào cơ sở dữ liệu cục bộ, sau đó duyệt, gắn thẻ và phát lại bất cứ khi nào cần.</translation>
    </message>
    <message>
        <source>Hand a polished report to your team in seconds. Export any session to HTML or PDF, complete with charts and min/max/mean stats.</source>
        <translation type="vanished">Gửi báo cáo chuyên nghiệp cho nhóm chỉ trong vài giây. Xuất bất kỳ session nào ra HTML hoặc PDF, kèm biểu đồ và thống kê min/max/trung bình.</translation>
    </message>
    <message>
        <source>Close the loop without extra tooling. Output Controls let you send commands back to your device straight from the dashboard.</source>
        <translation type="vanished">Đóng vòng lặp mà không cần công cụ bổ sung. Output Controls cho phép gửi lệnh trở lại thiết bị của bạn trực tiếp từ Dashboard.</translation>
    </message>
    <message>
        <source>Build a visualization nobody else has. The Painter widget runs your own script to draw fully custom graphics from incoming data.</source>
        <translation type="vanished">Xây dựng một trực quan hóa độc đáo. Widget Painter chạy script của bạn để vẽ đồ họa tùy chỉnh hoàn toàn từ dữ liệu nhận được.</translation>
    </message>
    <message>
        <source>One tool for every link. Serial Studio reads from UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT, and Process I/O.</source>
        <translation type="vanished">Một công cụ cho mọi kết nối. Serial Studio đọc từ UART, TCP/UDP, Bluetooth LE, Modbus, CAN Bus, audio, USB, HID, MQTT và Process I/O.</translation>
    </message>
    <message>
        <source>Skip the terminal dance. Send and receive files over your serial link with the built-in XMODEM, YMODEM, and ZMODEM protocols.</source>
        <translation type="vanished">Bỏ qua thao tác thủ công với terminal. Gửi và nhận file qua liên kết serial của bạn với các giao thức XMODEM, YMODEM và ZMODEM tích hợp sẵn.</translation>
    </message>
    <message>
        <source>Already have a Modbus register map or a DBC file? Generate a ready-to-use project from it automatically instead of building one by hand.</source>
        <translation type="vanished">Đã có sơ đồ register Modbus hoặc file DBC? Tạo dự án sẵn sàng sử dụng từ đó tự động thay vì xây dựng thủ công.</translation>
    </message>
    <message>
        <source>Describe what you want and let the AI Assistant build it. It can create and edit projects for you across eight model providers.</source>
        <translation type="vanished">Mô tả điều bạn muốn và để AI Assistant xây dựng. Nó có thể tạo và chỉnh sửa dự án cho bạn trên tám nhà cung cấp mô hình.</translation>
    </message>
    <message>
        <source>Tip %1 of %2</source>
        <translation type="vanished">Mẹo %1 của %2</translation>
    </message>
    <message>
        <source>Learn More</source>
        <translation type="vanished">Tìm Hiểu Thêm</translation>
    </message>
    <message>
        <source>Show Tips on Startup</source>
        <translation type="vanished">Hiển Thị Mẹo Khi Khởi Động</translation>
    </message>
    <message>
        <source>Previous</source>
        <translation type="vanished">Trước</translation>
    </message>
    <message>
        <source>Next</source>
        <translation type="vanished">Tiếp Theo</translation>
    </message>
    <message>
        <source>Close</source>
        <translation type="vanished">Đóng</translation>
    </message>
</context>
<context>
    <name>ToolCallCard</name>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="47"/>
        <source>Awaiting approval</source>
        <translation>Chờ Phê Duyệt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="48"/>
        <source>Done</source>
        <translation>Hoàn Thành</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="49"/>
        <source>Error</source>
        <translation>Lỗi</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="50"/>
        <source>Denied</source>
        <translation>Từ Chối</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="51"/>
        <source>Blocked</source>
        <translation>Đã Chặn</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="52"/>
        <source>Running</source>
        <translation>Đang Chạy</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="152"/>
        <source>Approve</source>
        <translation>Phê Duyệt</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="158"/>
        <source>Deny</source>
        <translation>Từ Chối</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="175"/>
        <source>Arguments</source>
        <translation>Tham Số</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="212"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="272"/>
        <source>Copy</source>
        <translation>Sao Chép</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="217"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="277"/>
        <source>Copy All</source>
        <translation>Sao Chép Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="225"/>
        <location filename="../../qml/AI/ToolCallCard.qml" line="285"/>
        <source>Select All</source>
        <translation>Chọn Tất Cả</translation>
    </message>
    <message>
        <location filename="../../qml/AI/ToolCallCard.qml" line="233"/>
        <source>Result</source>
        <translation>Kết Quả</translation>
    </message>
</context>
<context>
    <name>Toolbar</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="197"/>
        <source>Project Editor</source>
        <translation>Trình Chỉnh Sửa Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="200"/>
        <source>Open the Project Editor to create or modify your JSON layout</source>
        <translation>Mở Trình Chỉnh Sửa Dự Án để tạo hoặc chỉnh sửa bố cục JSON</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="214"/>
        <source>Open Project</source>
        <translation>Mở Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="217"/>
        <source>Open an existing JSON project</source>
        <translation>Mở dự án JSON hiện có</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="226"/>
        <source>Open CSV</source>
        <translation>Mở CSV</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="232"/>
        <source>Play a CSV file as if it were live sensor data</source>
        <translation>Phát lại tệp CSV như thể đang nhận dữ liệu cảm biến trực tiếp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="238"/>
        <source>Open MDF4</source>
        <translation>Mở MDF4</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="243"/>
        <source>Play an MDF4 file as if it were live sensor data (Pro)</source>
        <translation>Phát lại tệp MDF4 như thể đang nhận dữ liệu cảm biến trực tiếp (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <source>Assistant</source>
        <translation>Trợ Lý</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="254"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="260"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="302"/>
        <source>Extensions</source>
        <translation>Tiện Ích Mở Rộng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="264"/>
        <source>Chat with an AI to build and edit your project</source>
        <translation>Trò chuyện với AI để xây dựng và chỉnh sửa dự án của bạn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="265"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="305"/>
        <source>Browse and install extensions</source>
        <translation>Duyệt và cài đặt tiện ích mở rộng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="281"/>
        <source>Deploy</source>
        <translation>Triển Khai</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="286"/>
        <source>Build an operator app for the current project</source>
        <translation>Xây dựng ứng dụng vận hành cho dự án hiện tại</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="292"/>
        <source>Sessions</source>
        <translation>Phiên</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="296"/>
        <source>Browse, replay, and export recorded sessions</source>
        <translation>Duyệt, phát lại và xuất các phiên đã ghi</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="319"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="323"/>
        <source>Preferences</source>
        <translation>Tùy Chọn</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="327"/>
        <source>Open application settings and preferences</source>
        <translation>Mở cài đặt và tùy chọn ứng dụng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="345"/>
        <source>UART</source>
        <translation>UART</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="351"/>
        <source>Select Serial port (UART) communication</source>
        <translation>Chọn giao tiếp cổng nối tiếp (UART)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="362"/>
        <source>Audio</source>
        <translation>Âm Thanh</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="366"/>
        <source>Select audio input device (Pro)</source>
        <translation>Chọn thiết bị đầu vào âm thanh (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="381"/>
        <source>USB</source>
        <translation>USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="386"/>
        <source>Select raw USB communication (Pro)</source>
        <translation>Chọn giao tiếp USB thô (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="395"/>
        <source>Network</source>
        <translation>Mạng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="400"/>
        <source>Select TCP/UDP network communication</source>
        <translation>Chọn giao tiếp mạng TCP/UDP</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="412"/>
        <source>Modbus</source>
        <translation>Modbus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="417"/>
        <source>Select MODBUS communication (Pro)</source>
        <translation>Chọn giao tiếp MODBUS (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="431"/>
        <source>HID</source>
        <translation>HID</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="436"/>
        <source>Select HID device communication (Pro)</source>
        <translation>Chọn giao tiếp thiết bị HID (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="446"/>
        <source>Bluetooth</source>
        <translation>Bluetooth</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="450"/>
        <source>Select Bluetooth Low Energy communication</source>
        <translation>Chọn giao tiếp Bluetooth Low Energy</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="462"/>
        <source>CAN Bus</source>
        <translation>CAN Bus</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="467"/>
        <source>Select CAN Bus communication (Pro)</source>
        <translation>Chọn giao tiếp CAN Bus (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="481"/>
        <source>Process</source>
        <translation>Tiến Trình</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="486"/>
        <source>Select process pipe communication (Pro)</source>
        <translation>Chọn giao tiếp đường ống tiến trình (Pro)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="502"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="506"/>
        <source>About</source>
        <translation>Giới Thiệu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="510"/>
        <source>Show application info and license details</source>
        <translation>Hiển thị thông tin ứng dụng và chi tiết giấy phép</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="522"/>
        <source>Examples</source>
        <translation>Ví Dụ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="525"/>
        <source>Browse example projects</source>
        <translation>Duyệt các dự án ví dụ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="533"/>
        <source>Help Center</source>
        <translation>Trung Tâm Trợ Giúp</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="537"/>
        <source>Browse documentation, FAQ, and wiki</source>
        <translation>Duyệt tài liệu, câu hỏi thường gặp và wiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="543"/>
        <source>AI Wiki &amp; Chat</source>
        <translation>AI Wiki &amp; Chat</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="546"/>
        <source>View detailed documentation and ask questions on DeepWiki</source>
        <translation>Xem tài liệu chi tiết và đặt câu hỏi trên DeepWiki</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="586"/>
        <source>Activate</source>
        <translation>Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="590"/>
        <source>Manage license and activate Serial Studio Pro</source>
        <translation>Quản lý giấy phép và kích hoạt Serial Studio Pro</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="627"/>
        <source>Disconnect</source>
        <translation>Ngắt Kết Nối</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="605"/>
        <source>Connect</source>
        <translation>Kết Nối</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/Toolbar.qml" line="607"/>
        <source>Connect or disconnect from the configured device</source>
        <translation>Kết nối hoặc ngắt kết nối với thiết bị đã cấu hình</translation>
    </message>
</context>
<context>
    <name>TriggerDialog</name>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="50"/>
        <source>Trigger Settings</source>
        <translation>Cài Đặt Kích Hoạt</translation>
    </message>
    <message>
        <source>Hold the waveform stationary by aligning each sweep to a trigger event.</source>
        <translation type="vanished">Giữ dạng sóng cố định bằng cách căn chỉnh mỗi lần quét với sự kiện kích hoạt.</translation>
    </message>
    <message>
        <source>Lock a repeating signal in place, like the Auto button on an oscilloscope. Each sweep starts at the same point on the waveform, so it holds still instead of scrolling past.</source>
        <translation type="vanished">Khóa tín hiệu lặp lại tại chỗ, giống như nút Auto trên máy hiện sóng. Mỗi lần quét bắt đầu tại cùng một điểm trên dạng sóng, do đó nó giữ nguyên thay vì cuộn qua.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="173"/>
        <source>Trigger</source>
        <translation>Tín Hiệu Kích Hoạt</translation>
    </message>
    <message>
        <source>Mode:</source>
        <translation type="vanished">Chế Độ:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="110"/>
        <source>Mode</source>
        <translation>Chế Độ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Auto</source>
        <translation>Tự Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Normal</source>
        <translation>Bình Thường</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="135"/>
        <source>Single</source>
        <translation>Đơn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="158"/>
        <source>Auto free-runs when nothing crosses the level.</source>
        <translation>Auto chạy tự do khi không có gì vượt qua mức.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="159"/>
        <source>Normal waits for a crossing.</source>
        <translation>Normal chờ đợi một lần vượt qua.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="160"/>
        <source>Single captures one sweep, then stops.</source>
        <translation>Single chụp một lần quét, sau đó dừng lại.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="241"/>
        <source>Slope:</source>
        <translation>Độ Dốc:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="273"/>
        <source>Trigger on a downward crossing</source>
        <translation>Kích hoạt khi vượt qua xuống</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="319"/>
        <source>Timebase:</source>
        <translation>Cơ Sở Thời Gian:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="388"/>
        <source>Leave timebase empty to use the plot's time range; lower it to zoom in on a fast signal. Holdoff ignores new triggers for a moment after each.</source>
        <translation>Để trống cơ sở thời gian để sử dụng phạm vi thời gian của biểu đồ; giảm xuống để phóng to tín hiệu nhanh. Holdoff bỏ qua các tín hiệu kích hoạt mới trong một khoảnh khắc sau mỗi lần.</translation>
    </message>
    <message>
        <source>Signal:</source>
        <translation type="vanished">Tín Hiệu:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="230"/>
        <source>Value to cross</source>
        <translation>Giá trị cần vượt qua</translation>
    </message>
    <message>
        <source>Edge:</source>
        <translation type="vanished">Cạnh:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="254"/>
        <source>Rising</source>
        <translation>Lên</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="258"/>
        <source>Trigger on an upward crossing</source>
        <translation>Kích hoạt khi vượt qua lên</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="269"/>
        <source>Falling</source>
        <translation>Xuống</translation>
    </message>
    <message>
        <source>A new sweep begins each time the signal crosses the level in the chosen direction. Auto also free-runs when no crossing is found.</source>
        <translation type="vanished">Một lần quét mới bắt đầu mỗi khi tín hiệu vượt qua mức trong hướng đã chọn. Auto cũng chạy tự do khi không tìm thấy điểm vượt qua.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="292"/>
        <source>Timing</source>
        <translation>Thời Gian</translation>
    </message>
    <message>
        <source>Timebase (ms):</source>
        <translation type="vanished">Cơ sở thời gian (ms):</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="332"/>
        <source>Match time range</source>
        <translation>Khớp phạm vi thời gian</translation>
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
        <translation type="vanished">Cơ sở thời gian đặt khoảng thời gian mà một lần quét hiển thị; để trống để sử dụng phạm vi thời gian của biểu đồ. Giảm xuống để phóng to tín hiệu nhanh. Holdoff bỏ qua các tín hiệu kích hoạt mới trong một khoảnh khắc sau mỗi lần.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="403"/>
        <source>Capture Next</source>
        <translation>Chụp Tiếp Theo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="405"/>
        <source>Arm for one more single-shot capture</source>
        <translation>Kích hoạt cho một lần chụp đơn tiếp theo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="217"/>
        <source>Level:</source>
        <translation>Mức:</translation>
    </message>
    <message>
        <source>Trigger level</source>
        <translation type="vanished">Mức kích hoạt</translation>
    </message>
    <message>
        <source>Holdoff (ms):</source>
        <translation type="vanished">Thời gian giữ (ms):</translation>
    </message>
    <message>
        <source>Holdoff time</source>
        <translation type="vanished">Thời gian giữ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="197"/>
        <source>Source:</source>
        <translation>Nguồn:</translation>
    </message>
    <message>
        <source>Re-arm</source>
        <translation type="vanished">Tái Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/TriggerDialog.qml" line="418"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>UART</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="52"/>
        <source>COM Port</source>
        <translation>Cổng COM</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="79"/>
        <source>Baud Rate</source>
        <translation>Tốc Độ Baud</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="163"/>
        <source>Data Bits</source>
        <translation>Bit Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="184"/>
        <source>Parity</source>
        <translation>Chẵn Lẻ</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="205"/>
        <source>Stop Bits</source>
        <translation>Bit Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="226"/>
        <source>Flow Control</source>
        <translation>Điều Khiển Luồng</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="258"/>
        <source>Auto Reconnect</source>
        <translation>Kết Nối Lại Tự Động</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml" line="276"/>
        <source>Send DTR Signal</source>
        <translation>Gửi Tín Hiệu DTR</translation>
    </message>
</context>
<context>
    <name>UI::AlarmMonitor</name>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="195"/>
        <source>Alarm</source>
        <translation>Cảnh Báo</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>critical</source>
        <translation>nghiêm trọng</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="196"/>
        <source>warning</source>
        <translation>cảnh báo</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="200"/>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation>Giá trị %1%2 đã vào vùng %3 (%4–%5).</translation>
    </message>
    <message>
        <location filename="../../src/UI/AlarmMonitor.cpp" line="205"/>
        <source>Alarms</source>
        <translation>Cảnh Báo</translation>
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
        <translation>Thông Báo</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1733"/>
        <source>Clock</source>
        <translation>Đồng Hồ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1740"/>
        <source>Stopwatch</source>
        <translation>Đồng Hồ Bấm Giờ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1786"/>
        <location filename="../../src/UI/Dashboard.cpp" line="1801"/>
        <source>%1 (Fallback)</source>
        <translation>%1 (Dự Phòng)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Dashboard.cpp" line="1823"/>
        <source>LED Panel (%1)</source>
        <translation>Bảng LED (%1)</translation>
    </message>
</context>
<context>
    <name>UI::DashboardWidget</name>
    <message>
        <location filename="../../src/UI/DashboardWidget.cpp" line="161"/>
        <source>Invalid</source>
        <translation>Không Hợp Lệ</translation>
    </message>
</context>
<context>
    <name>UI::WindowManager</name>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1069"/>
        <source>Select Background Image</source>
        <translation>Chọn Ảnh Nền</translation>
    </message>
    <message>
        <location filename="../../src/UI/WindowManager.cpp" line="1071"/>
        <source>Images (*.png *.jpg *.jpeg *.bmp)</source>
        <translation>Hình Ảnh (*.png *.jpg *.jpeg *.bmp)</translation>
    </message>
</context>
<context>
    <name>USB</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="50"/>
        <source>USB Device</source>
        <translation>Thiết Bị USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="80"/>
        <source>Transfer Mode</source>
        <translation>Chế Độ Truyền</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="89"/>
        <source>Bulk Stream</source>
        <translation>Luồng Bulk</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="90"/>
        <source>Advanced (Bulk + Control)</source>
        <translation>Nâng Cao (Bulk + Control)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="91"/>
        <source>Isochronous</source>
        <translation>Isochronous</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="143"/>
        <source>Connect to USB devices using bulk, control, or isochronous transfers. Suitable for data loggers, custom firmware devices, and USB instruments.</source>
        <translation>Kết nối với thiết bị USB sử dụng truyền bulk, control hoặc isochronous. Phù hợp cho bộ ghi dữ liệu, thiết bị firmware tùy chỉnh và thiết bị đo USB.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="152"/>
        <source>USB specifications (USB.org)</source>
        <translation>Thông số kỹ thuật USB (USB.org)</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="169"/>
        <source>IN Endpoint</source>
        <translation>Điểm Cuối IN</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="205"/>
        <source>OUT Endpoint</source>
        <translation>Điểm Cuối OUT</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="241"/>
        <source>Max Packet Size</source>
        <translation>Kích Thước Gói Tối Đa</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="301"/>
        <source>Control Transfers Enabled</source>
        <translation>Đã Bật Truyền Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="310"/>
        <source>Sending incorrect control requests may crash or damage connected hardware. Use with caution.</source>
        <translation>Gửi yêu cầu điều khiển không đúng có thể làm hỏng hoặc gây hại cho phần cứng đã kết nối. Sử dụng thận trọng.</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="317"/>
        <source>Learn about USB control transfers</source>
        <translation>Tìm hiểu về truyền điều khiển USB</translation>
    </message>
    <message>
        <location filename="../../qml/MainWindow/Panes/SetupPanes/Drivers/USB.qml" line="351"/>
        <source>Packet size should match the maximum transfer size reported by the endpoint. Typical values: 192 B (FS audio), 1024 B (HS).</source>
        <translation>Kích thước gói nên khớp với kích thước truyền tối đa được báo cáo bởi endpoint. Giá trị điển hình: 192 B (FS audio), 1024 B (HS).</translation>
    </message>
</context>
<context>
    <name>Updater</name>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="477"/>
        <source>Would you like to download the update now?</source>
        <translation>Bạn có muốn tải xuống bản cập nhật ngay bây giờ không?</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="479"/>
        <source>Would you like to download the update now?&lt;br /&gt;This is a mandatory update, exiting now will close the application.</source>
        <translation>Bạn có muốn tải xuống bản cập nhật ngay bây giờ không?&lt;br /&gt;Đây là bản cập nhật bắt buộc, thoát ngay bây giờ sẽ đóng ứng dụng.</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="485"/>
        <source>&lt;strong&gt;Change log:&lt;/strong&gt;&lt;br/&gt;%1</source>
        <translation>&lt;strong&gt;Nhật ký thay đổi:&lt;/strong&gt;&lt;br/&gt;%1</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="488"/>
        <source>Version %1 of %2 has been released!</source>
        <translation>Phiên bản %1 của %2 đã được phát hành!</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="520"/>
        <source>No updates are available for the moment</source>
        <translation>Hiện tại không có bản cập nhật nào</translation>
    </message>
    <message>
        <location filename="../../../lib/QSimpleUpdater/src/Updater.cpp" line="522"/>
        <source>Congratulations! You are running the latest version of %1</source>
        <translation>Chúc mừng! Bạn đang chạy phiên bản mới nhất của %1</translation>
    </message>
</context>
<context>
    <name>UserTableView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="172"/>
        <source>Add Register</source>
        <translation>Thêm Thanh Ghi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="175"/>
        <source>Add register</source>
        <translation>Thêm thanh ghi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="182"/>
        <source>Insert Constant</source>
        <translation>Chèn Hằng Số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="185"/>
        <source>Insert constant</source>
        <translation>Chèn hằng số</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="192"/>
        <source>Import</source>
        <translation>Nhập</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="195"/>
        <source>Import registers from CSV</source>
        <translation>Nhập thanh ghi từ CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="202"/>
        <source>Export</source>
        <translation>Xuất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="205"/>
        <source>Export registers to CSV</source>
        <translation>Xuất thanh ghi ra CSV</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="217"/>
        <source>Rename</source>
        <translation>Đổi Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="220"/>
        <source>Rename table</source>
        <translation>Đổi tên bảng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="227"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="230"/>
        <source>Delete table</source>
        <translation>Xóa bảng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="244"/>
        <source>Help</source>
        <translation>Trợ Giúp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="249"/>
        <source>Open help documentation for shared memory</source>
        <translation>Mở tài liệu trợ giúp về bộ nhớ chia sẻ</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="289"/>
        <source>Permissions</source>
        <translation>Quyền</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="290"/>
        <source>Register Name</source>
        <translation>Tên Thanh Ghi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="291"/>
        <source>Default Value</source>
        <translation>Giá Trị Mặc Định</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read-Only</source>
        <translation>Chỉ Đọc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="330"/>
        <source>Read/Write</source>
        <translation>Đọc/ghi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="468"/>
        <source>Copy access code %1 to clipboard</source>
        <translation>Sao chép mã truy cập %1 vào clipboard</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="501"/>
        <source>Delete register</source>
        <translation>Xóa thanh ghi</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="518"/>
        <source>No registers.</source>
        <translation>Không có thanh ghi.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/UserTableView.qml" line="568"/>
        <source>Register access code copied</source>
        <translation>Đã sao chép mã truy cập thanh ghi</translation>
    </message>
</context>
<context>
    <name>Waterfall</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="237"/>
        <source>Show Colorbar</source>
        <translation>Hiển Thị Thanh Màu</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="250"/>
        <source>Show Axes &amp; Grid</source>
        <translation>Hiển Thị Trục &amp; Lưới</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="263"/>
        <source>Show Crosshair</source>
        <translation>Hiển Thị Tâm Ngắm</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Pause</source>
        <translation>Tạm Dừng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/Waterfall.qml" line="277"/>
        <source>Resume</source>
        <translation>Tiếp Tục</translation>
    </message>
</context>
<context>
    <name>WebView</name>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="70"/>
        <source>Web View Unavailable</source>
        <translation>Chế Độ Xem Web Không Khả Dụng</translation>
    </message>
    <message>
        <location filename="../../qml/Widgets/Dashboard/WebView.qml" line="81"/>
        <source>This build of Serial Studio was compiled without Qt WebEngine, so web pages cannot be displayed.</source>
        <translation>Bản build Serial Studio này được biên dịch mà không có QT WebEngine, do đó không thể hiển thị các trang web.</translation>
    </message>
</context>
<context>
    <name>Welcome</name>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="177"/>
        <source>Welcome to %1!</source>
        <translation>Chào mừng đến với %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="188"/>
        <source>Serial Studio is a powerful real-time visualization tool, built for engineers, students, and makers.</source>
        <translation>Serial Studio là công cụ trực quan hóa thời gian thực mạnh mẽ, được xây dựng cho kỹ sư, sinh viên và nhà sáng tạo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="199"/>
        <source>You can start a fully-functional 14-day trial, activate it with your license key, or download and compile the GPLv3 source code yourself.</source>
        <translation>Bạn có thể bắt đầu dùng thử đầy đủ tính năng trong 14 ngày, kích hoạt bằng khóa bản quyền hoặc tự tải và biên dịch mã nguồn GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="209"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="394"/>
        <source>Buying Pro supports the author directly and helps fund future development.</source>
        <translation>Mua Pro hỗ trợ trực tiếp tác giả và giúp tài trợ cho phát triển tương lai.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="217"/>
        <location filename="../../qml/Dialogs/Welcome.qml" line="402"/>
        <source>Building the GPLv3 version yourself helps grow the community and encourages technical contributions.</source>
        <translation>Tự biên dịch phiên bản GPLv3 giúp phát triển cộng đồng và khuyến khích đóng góp kỹ thuật.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="239"/>
        <source>Please wait…</source>
        <translation>Vui lòng đợi…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="275"/>
        <source>%1 days remaining in your trial.</source>
        <translation>Còn %1 ngày dùng thử.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="285"/>
        <source>You're currently using the fully-featured trial of %1 Pro. It's valid for 14 days of personal, non-commercial use.</source>
        <translation>Bạn đang sử dụng phiên bản dùng thử đầy đủ tính năng của %1 Pro. Có hiệu lực trong 14 ngày sử dụng cá nhân, phi thương mại.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="296"/>
        <source>Upgrade to a paid plan to keep using Serial Studio Pro.</source>
        <translation>Nâng cấp lên gói trả phí để tiếp tục sử dụng Serial Studio Pro.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="304"/>
        <source>Or, compile the GPLv3 source code to use it for free.</source>
        <translation>Hoặc biên dịch mã nguồn GPLv3 để sử dụng miễn phí.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="312"/>
        <source>To see available subscription plans, click "Upgrade Now" below.</source>
        <translation>Để xem các gói đăng ký có sẵn, nhấp "Nâng Cấp Ngay" bên dưới.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="333"/>
        <source>Don't nag me about the trial.
I understand that when it ends, I'll need to buy a license or build the GPLv3 version.</source>
        <translation>Đừng nhắc tôi về bản dùng thử.
Tôi hiểu rằng khi kết thúc, tôi cần mua bản quyền hoặc tự biên dịch phiên bản GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="364"/>
        <source>Your %1 trial has expired.</source>
        <translation>Bản dùng thử %1 của bạn đã hết hạn.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="374"/>
        <source>Your trial period has ended. To continue using %1 with all Pro features, please upgrade to a paid plan.</source>
        <translation>Thời gian dùng thử đã kết thúc. Để tiếp tục sử dụng %1 với đầy đủ tính năng Pro, vui lòng nâng cấp lên gói trả phí.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="385"/>
        <source>If you prefer, you can also compile the open-source version under the GPLv3 license.</source>
        <translation>Nếu muốn, bạn cũng có thể biên dịch phiên bản mã nguồn mở theo giấy phép GPLv3.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="413"/>
        <source>Thank you for trying %1!</source>
        <translation>Cảm ơn bạn đã dùng thử %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="455"/>
        <source>Upgrade Now</source>
        <translation>Nâng Cấp Ngay</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="464"/>
        <source>Activate</source>
        <translation>Kích Hoạt</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Open in Limited Mode</source>
        <translation>Mở ở Chế Độ Giới Hạn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Continue</source>
        <translation>Tiếp Tục</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/Welcome.qml" line="479"/>
        <source>Start Trial</source>
        <translation>Bắt Đầu Dùng Thử</translation>
    </message>
</context>
<context>
    <name>WhatsNew</name>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="31"/>
        <source>What's New in %1</source>
        <translation>Có Gì Mới Trong %1</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="58"/>
        <source>Lua &amp; Built-In Parsers</source>
        <translation>Trình Phân Tích Lua &amp; Tích Hợp Sẵn</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="59"/>
        <source>Parse frames in Lua 5.4 or with code-free Built-In templates, alongside JavaScript.</source>
        <translation>Phân tích frame bằng Lua 5.4 hoặc với mẫu Tích Hợp Sẵn không cần mã, cùng với JavaScript.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="67"/>
        <source>AI Assistant</source>
        <translation>Trợ Lý AI</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="68"/>
        <source>An in-app assistant across eight providers that can build and edit projects for you.</source>
        <translation>Trợ lý trong ứng dụng với tám nhà cung cấp có thể tạo và chỉnh sửa dự án cho bạn.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="76"/>
        <source>Device Control Loops</source>
        <translation>Vòng Lặp Điều Khiển Thiết Bị</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="77"/>
        <source>Automate your device with an Arduino-style setup() and loop() routine that can read, write, and drive the dashboard.</source>
        <translation>Tự động hóa thiết bị với quy trình setup() và loop() kiểu Arduino có thể đọc, ghi và điều khiển bảng điều khiển.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="85"/>
        <source>Oscilloscope Sweep &amp; Trigger</source>
        <translation>Quét &amp; Kích Hoạt Dao Động Ký</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="86"/>
        <source>Scope-style sweep with an animated trigger cursor you can drag on the plot.</source>
        <translation>Quét kiểu dao động ký với con trỏ kích hoạt động có thể kéo trên biểu đồ.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="94"/>
        <source>Output Controls</source>
        <translation>Điều Khiển Đầu Ra</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="95"/>
        <source>Transmit back to your device with control widgets and a protocol-helper engine.</source>
        <translation>Truyền dữ liệu ngược lại thiết bị của bạn với các widget điều khiển và một công cụ hỗ trợ giao thức.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="103"/>
        <source>Dashboard Workspaces</source>
        <translation>Không Gian Làm Việc Bảng Điều Khiển</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="104"/>
        <source>Group widgets into your own dashboards and find them from the taskbar search.</source>
        <translation>Gom nhóm các widget vào bảng điều khiển riêng và tìm chúng từ thanh tìm kiếm tác vụ.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="112"/>
        <source>Session Database &amp; Reports</source>
        <translation>Cơ Sở Dữ Liệu Phiên &amp; Báo Cáo</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="113"/>
        <source>Record sessions to SQLite, replay them, and export HTML or PDF reports.</source>
        <translation>Ghi lại các phiên vào SQLITE, phát lại và xuất báo cáo HTML hoặc PDF.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="121"/>
        <source>Operator Deployments</source>
        <translation>Triển Khai cho Người Vận Hành</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="122"/>
        <source>Ship a locked-down, single-purpose build to operators with a runtime-only mode.</source>
        <translation>Phát hành bản dựng khóa chức năng, chỉ một mục đích cho người vận hành với chế độ chỉ chạy.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="130"/>
        <source>New Dashboard Widgets</source>
        <translation>Widget Bảng Điều Khiển Mới</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="131"/>
        <source>Gauge and Meter faces with live readouts, plus Clock, Stopwatch, and Waterfall.</source>
        <translation>Mặt đồng hồ đo và đồng hồ kim với hiển thị trực tiếp, cùng với Đồng Hồ, Bấm Giờ và Waterfall.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="139"/>
        <source>Dataset Transforms</source>
        <translation>Biến Đổi Tập Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="140"/>
        <source>Calibrate, filter, and convert each channel with a per-dataset function, or add virtual datasets that compute new channels.</source>
        <translation>Hiệu chuẩn, lọc và chuyển đổi từng kênh với hàm riêng cho mỗi tập dữ liệu, hoặc thêm các tập dữ liệu ảo để tính toán kênh mới.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="148"/>
        <source>Painter Widget</source>
        <translation>Widget Vẽ</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="149"/>
        <source>Draw fully custom graphics from incoming data with your own drawing script.</source>
        <translation>Vẽ đồ họa tùy chỉnh hoàn toàn từ dữ liệu đầu vào với script vẽ của riêng bạn.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="157"/>
        <source>Data Tables</source>
        <translation>Bảng Dữ Liệu</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="158"/>
        <source>Live register-style tables with virtual datasets and editable cells.</source>
        <translation>Bảng kiểu thanh ghi trực tiếp với tập dữ liệu ảo và ô có thể chỉnh sửa.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="166"/>
        <source>Image Widget</source>
        <translation>Widget Ảnh</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="167"/>
        <source>Decode and display image frames streamed straight from your device.</source>
        <translation>Giải mã và hiển thị các khung hình ảnh truyền trực tiếp từ thiết bị của bạn.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="175"/>
        <source>Notifications &amp; Alarms</source>
        <translation>Thông Báo &amp; Báo Động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="176"/>
        <source>Multi-band dataset alarms with severity tiers, routed to the Notification Center.</source>
        <translation>Báo động tập dữ liệu đa dải với các cấp độ nghiêm trọng, chuyển đến Trung Tâm Thông Báo.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="184"/>
        <source>Project Lock</source>
        <translation>Khóa Dự Án</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="185"/>
        <source>Lock a project to separate operator use from editing, with an access code.</source>
        <translation>Khóa dự án để tách biệt sử dụng vận hành và chỉnh sửa, với mã truy cập.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="193"/>
        <source>MQTT, Protobuf &amp; File Transfer</source>
        <translation>MQTT, Protobuf &amp; Chuyển Tập Tin</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="194"/>
        <source>MQTT input and publishing, Protobuf import, and XMODEM / YMODEM / ZMODEM transfers.</source>
        <translation>Nhập và xuất MQTT, nhập Protobuf, và truyền XMODEM / YMODEM / ZMODEM.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="240"/>
        <source>Welcome to %1!</source>
        <translation>Chào mừng đến với %1!</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="250"/>
        <source>Here's what's new in version %1.</source>
        <translation>Đây là những điểm mới trong phiên bản %1.</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="423"/>
        <source>Show on Startup</source>
        <translation>Hiển thị khi khởi động</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WhatsNew.qml" line="430"/>
        <source>Close</source>
        <translation>Đóng</translation>
    </message>
</context>
<context>
    <name>WidgetDelegate</name>
    <message>
        <location filename="../../qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml" line="345"/>
        <source>Device Disconnected</source>
        <translation>Thiết Bị Đã Ngắt Kết Nối</translation>
    </message>
</context>
<context>
    <name>Widgets::Bar</name>
    <message>
        <source>Alarm</source>
        <translation type="vanished">Cảnh Báo</translation>
    </message>
    <message>
        <source>critical</source>
        <translation type="vanished">nghiêm trọng</translation>
    </message>
    <message>
        <source>warning</source>
        <translation type="vanished">cảnh báo</translation>
    </message>
    <message>
        <source>Value %1%2 entered the %3 band (%4–%5).</source>
        <translation type="vanished">Giá trị %1%2 đã vào dải %3 (%4–%5).</translation>
    </message>
    <message>
        <source>Value %1%2 reached the high alarm %3%2</source>
        <translation type="vanished">Giá trị %1%2 đạt ngưỡng cảnh báo cao %3%2</translation>
    </message>
    <message>
        <source>Value %1%2 reached the low alarm %3%2</source>
        <translation type="vanished">Giá trị %1%2 đạt ngưỡng cảnh báo thấp %3%2</translation>
    </message>
    <message>
        <source>Alarms</source>
        <translation type="vanished">Cảnh Báo</translation>
    </message>
</context>
<context>
    <name>Widgets::Compass</name>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="166"/>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="189"/>
        <source>N</source>
        <translation>B</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="169"/>
        <source>NE</source>
        <translation>ĐB</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="172"/>
        <source>E</source>
        <translation>Đ</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="175"/>
        <source>SE</source>
        <translation>ĐN</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="178"/>
        <source>S</source>
        <translation>N</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="181"/>
        <source>SW</source>
        <translation>TN</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="184"/>
        <source>W</source>
        <translation>T</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Compass.cpp" line="187"/>
        <source>NW</source>
        <translation>TB</translation>
    </message>
</context>
<context>
    <name>Widgets::DataGrid</name>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="132"/>
        <source>Title</source>
        <translation>Tiêu Đề</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/DataGrid.cpp" line="133"/>
        <source>Value</source>
        <translation>Giá Trị</translation>
    </message>
    <message>
        <source>Awaiting data…</source>
        <translation type="vanished">Đang chờ dữ liệu…</translation>
    </message>
</context>
<context>
    <name>Widgets::GPS</name>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery</source>
        <translation>Hình Ảnh Vệ Tinh</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Satellite Imagery with Labels</source>
        <translation>Hình Ảnh Vệ Tinh có Nhãn</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="92"/>
        <source>Street Map</source>
        <translation>Bản Đồ Đường Phố</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Topographic Map</source>
        <translation>Bản Đồ Địa Hình</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Terrain</source>
        <translation>Địa Hình</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="93"/>
        <source>Light Gray Canvas</source>
        <translation>Nền Xám Nhạt</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>Dark Gray Canvas</source>
        <translation>Nền Xám Đậm</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="94"/>
        <source>National Geographic</source>
        <translation>National Geographic</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="373"/>
        <source>Additional map layers are available only for Pro users.</source>
        <translation>Các lớp bản đồ bổ sung chỉ khả dụng cho người dùng Pro.</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/GPS.cpp" line="374"/>
        <source>We can't offer unrestricted access because the ArcGIS API key incurs real costs.</source>
        <translation>Chúng tôi không thể cung cấp quyền truy cập không giới hạn vì khóa API ArcGIS phát sinh chi phí thực tế.</translation>
    </message>
</context>
<context>
    <name>Widgets::MultiPlot</name>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Time (s)</source>
        <translation>Thời Gian (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/MultiPlot.cpp" line="103"/>
        <source>Samples</source>
        <translation>Mẫu</translation>
    </message>
</context>
<context>
    <name>Widgets::Output::Base</name>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="168"/>
        <source>Transmit script timed out after %1 ms</source>
        <translation>Script truyền hết thời gian chờ sau %1 ms</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Output/Base.cpp" line="184"/>
        <source>Payload exceeds maximum size</source>
        <translation>Payload vượt quá kích thước tối đa</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="90"/>
        <source>Time (s)</source>
        <translation>Thời Gian (s)</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Plot.cpp" line="116"/>
        <source>Samples</source>
        <translation>Mẫu</translation>
    </message>
</context>
<context>
    <name>Widgets::Plot3D</name>
    <message>
        <location filename="../../src/UI/Widgets/Plot3D.cpp" line="950"/>
        <source>Grid Interval: %1 unit(s)</source>
        <translation>Khoảng Lưới: %1 đơn vị</translation>
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
        <translation>Nóng</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="409"/>
        <source>Grayscale</source>
        <translation>Thang Độ Xám</translation>
    </message>
    <message>
        <location filename="../../src/UI/Widgets/Waterfall.cpp" line="411"/>
        <source>Unknown</source>
        <translation>Không Xác Định</translation>
    </message>
</context>
<context>
    <name>WorkspaceDialog</name>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="51"/>
        <source>Edit Workspace</source>
        <translation>Chỉnh Sửa Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="52"/>
        <source>New Workspace</source>
        <translation>Không Gian Làm Việc Mới</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="157"/>
        <source>Name:</source>
        <translation>Tên:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="166"/>
        <source>My Workspace</source>
        <translation>Không Gian Làm Việc của Tôi</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="181"/>
        <source>Select widgets to include:</source>
        <translation>Chọn widget để bao gồm:</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="187"/>
        <source>Filter widgets…</source>
        <translation>Lọc widget…</translation>
    </message>
    <message>
        <location filename="../../qml/Dialogs/WorkspaceDialog.qml" line="302"/>
        <source>Cancel</source>
        <translation>Hủy</translation>
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
        <translation>Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="127"/>
        <source>Add Sub-folder</source>
        <translation>Thêm Thư Mục Con</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="130"/>
        <source>Add a sub-folder inside this folder</source>
        <translation>Thêm thư mục con bên trong thư mục này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="138"/>
        <source>Add Workspace</source>
        <translation>Thêm Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="141"/>
        <source>Add a workspace inside this folder</source>
        <translation>Thêm không gian làm việc vào trong thư mục này</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="151"/>
        <source>Rename</source>
        <translation>Đổi Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="153"/>
        <source>Rename folder</source>
        <translation>Đổi tên thư mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="162"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="164"/>
        <source>Delete folder</source>
        <translation>Xóa thư mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="178"/>
        <source>Title</source>
        <translation>Tiêu Đề</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="179"/>
        <source>Contents</source>
        <translation>Nội Dung</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceFolderView.qml" line="269"/>
        <source>This folder is empty. Use the toolbar to add a workspace or sub-folder.</source>
        <translation>Thư mục này trống. Sử dụng thanh công cụ để thêm không gian làm việc hoặc thư mục con.</translation>
    </message>
</context>
<context>
    <name>WorkspaceView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="43"/>
        <source>Workspace</source>
        <translation>Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="144"/>
        <source>Add Widget</source>
        <translation>Thêm Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="146"/>
        <source>Add widget to workspace</source>
        <translation>Thêm widget vào không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="157"/>
        <source>Move Up</source>
        <translation>Di Chuyển Lên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="159"/>
        <source>Move workspace up</source>
        <translation>Di chuyển không gian làm việc lên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="169"/>
        <source>Move Down</source>
        <translation>Di Chuyển Xuống</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="171"/>
        <source>Move workspace down</source>
        <translation>Di chuyển không gian làm việc xuống</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="181"/>
        <source>Change Icon</source>
        <translation>Đổi Biểu Tượng</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="183"/>
        <source>Change workspace icon</source>
        <translation>Đổi biểu tượng không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="198"/>
        <source>Rename</source>
        <translation>Đổi Tên</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="200"/>
        <source>Rename workspace</source>
        <translation>Đổi tên không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="209"/>
        <source>Delete</source>
        <translation>Xóa</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="211"/>
        <source>Delete workspace</source>
        <translation>Xóa không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="233"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="239"/>
        <source>Group</source>
        <translation>Nhóm</translation>
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
        <translation>Loại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="285"/>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="323"/>
        <source>(unknown)</source>
        <translation>(không xác định)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="303"/>
        <source>(group widget)</source>
        <translation>(widget nhóm)</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="353"/>
        <source>Remove widget from workspace</source>
        <translation>Xóa widget khỏi không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspaceView.qml" line="373"/>
        <source>No widgets in this workspace.</source>
        <translation>Không có widget nào trong không gian làm việc này.</translation>
    </message>
</context>
<context>
    <name>WorkspacesView</name>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="33"/>
        <source>Workspaces</source>
        <translation>Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="124"/>
        <source>Add Folder</source>
        <translation>Thêm Thư Mục</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="126"/>
        <source>Add a top-level folder</source>
        <translation>Thêm thư mục cấp cao nhất</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="151"/>
        <source>Customize</source>
        <translation>Tùy Chỉnh</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="153"/>
        <source>Edit workspaces manually</source>
        <translation>Chỉnh sửa không gian làm việc thủ công</translation>
    </message>
    <message>
        <source>Move Up</source>
        <translation type="vanished">Di Chuyển Lên</translation>
    </message>
    <message>
        <source>Move the selected workspace up</source>
        <translation type="vanished">Di chuyển không gian làm việc đã chọn lên</translation>
    </message>
    <message>
        <source>Move Down</source>
        <translation type="vanished">Di Chuyển Xuống</translation>
    </message>
    <message>
        <source>Move the selected workspace down</source>
        <translation type="vanished">Di chuyển không gian làm việc đã chọn xuống</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="135"/>
        <source>Add Workspace</source>
        <translation>Thêm Không Gian Làm Việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="137"/>
        <source>Add workspace</source>
        <translation>Thêm không gian làm việc</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="167"/>
        <source>Cleanup</source>
        <translation>Dọn Dẹp</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="170"/>
        <source>Remove %1 widget reference(s) whose target group or dataset no longer exists</source>
        <translation>Xóa %1 tham chiếu widget có nhóm hoặc tập dữ liệu đích không còn tồn tại</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="173"/>
        <source>No stale widget references in any workspace</source>
        <translation>Không có tham chiếu widget lỗi thời trong bất kỳ không gian làm việc nào</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="188"/>
        <source>Title</source>
        <translation>Tiêu Đề</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="189"/>
        <source>Contents</source>
        <translation>Nội Dung</translation>
    </message>
    <message>
        <source>Widgets</source>
        <translation type="vanished">Widget</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="282"/>
        <source>No workspaces. Add one with the toolbar above, or reset to the auto layout.</source>
        <translation>Không có không gian làm việc. Thêm một không gian bằng thanh công cụ ở trên hoặc đặt lại về bố cục tự động.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="284"/>
        <source>Project has no eligible groups -- add a group with widgets to populate workspaces.</source>
        <translation>Dự án không có nhóm đủ điều kiện -- thêm một nhóm có widget để điền vào không gian làm việc.</translation>
    </message>
    <message>
        <location filename="../../qml/ProjectEditor/Views/WorkspacesView.qml" line="290"/>
        <source>Reset to Auto Layout</source>
        <translation>Đặt Lại về Bố Cục Tự Động</translation>
    </message>
</context>
</TS>