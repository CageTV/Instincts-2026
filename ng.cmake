# BeastHarmony.dll on alandtse's CommonLibSSE-NG; included by E:/WorkSpace/ng-build/CMakeLists.txt (see ng_plugin there)
ng_plugin(TARGET BeastHarmony NAME BeastHarmony VERSION 0.1.0 AUTHOR "CageTV" ROOT "${CMAKE_CURRENT_LIST_DIR}"
    SOURCES src/main.cpp src/Config.cpp src/Subdue.cpp
    INCLUDES "${CMAKE_CURRENT_LIST_DIR}/src"
    PCH "${CMAKE_CURRENT_LIST_DIR}/src/PCH.h"
    LIBS nlohmann_json::nlohmann_json)
