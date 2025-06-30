cmake_minimum_required(VERSION 3.29)
project(Common)

set(CMAKE_CXX_STANDARD 26)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")



include_directories(Common)

add_library(Common
        Common/AutoSocket.cpp
        Common/AutoSocket.h
        Common/Finally.h
        Common/required.h
        Common/ServerSocket.cpp
        Common/ServerSocket.h
        Common/Default.h
        Common/Never.h
        Common/Logger.cpp
        Common/Logger.h
        Common/StringConvert.cpp
        Common/StringConvert.h
)

add_executable(ChatServerApp
        ChatServerApp/ClientLogic.cpp
        ChatServerApp/Logic.cpp
        ChatServerApp/main.cpp
        ChatServerApp/SocketLogic.cpp
        ChatServerApp/PipeLogic.cpp
)

add_executable(ChatClientApp
        ChatClientApp/main.cpp
)

add_executable(ServerStatusApp
        ServerStatusApp/main.cpp
        ServerStatusApp/QueryPipe.cpp
)

target_link_libraries(ChatServerApp PRIVATE Common Ws2_32 msxml6)
target_link_libraries(ChatClientApp PRIVATE Common Ws2_32)
#target_link_libraries(ServerStatusApp PRIVATE Common)

set_target_properties(ChatServerApp PROPERTIES
        WIN32_EXECUTABLE OFF
)