
# 在文件开头添加工具链强制检查
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Using MSVC toolchain")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(FATAL_ERROR "MinGW toolchain detected but project requires MSVC")
endif()

# 在文件开头添加架构检测
if(NOT DEFINED TARGET_ARCH)
    # 显式处理Ninja生成器
    if(CMAKE_GENERATOR MATCHES "Ninja")
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
            set(TARGET_ARCH "x64" CACHE STRING "Target architecture" FORCE)
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
            set(TARGET_ARCH "arm64")
        else()
            set(TARGET_ARCH "x86")
        endif()
    
    # 保留原有Visual Studio生成器检测
    elseif(CMAKE_GENERATOR_PLATFORM)
        string(TOLOWER "${CMAKE_GENERATOR_PLATFORM}" TARGET_ARCH)
    
    # 增强MSVC编译器检测
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        if(CMAKE_CL_64)
            set(TARGET_ARCH "x64")
        else()
            set(TARGET_ARCH "x86")
        endif()
    
    # 保留原有系统处理器检测
    elseif(CMAKE_SYSTEM_PROCESSOR)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
            set(TARGET_ARCH "x64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
            set(TARGET_ARCH "arm64")
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86|i86pc")
            set(TARGET_ARCH "x86")
        endif()
    
    # 最后检查指针大小
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(TARGET_ARCH "x64")
        else()
            set(TARGET_ARCH "x86")
        endif()
    endif()

    # 增加缓存变量和验证
    set(SUPPORTED_ARCHS x64 x86 arm64)

    # 设置默认值
    if(NOT TARGET_ARCH)
        # set(TARGET_ARCH "x64" CACHE STRING "Target architecture (x86/x64/arm64)" FORCE)
        # 增加架构验证
        if(NOT TARGET_ARCH IN_LIST SUPPORTED_ARCHS)
            message(FATAL_ERROR "Unsupported architecture: ${TARGET_ARCH}. Supported: ${SUPPORTED_ARCHS}")
        endif()

        # 设为缓存变量以便后续修改
        set(TARGET_ARCH ${TARGET_ARCH} CACHE STRING "Target architecture (x86/x64/arm64)" FORCE)
        set_property(CACHE TARGET_ARCH PROPERTY STRINGS ${SUPPORTED_ARCHS})
    endif()

    # 增强日志输出
    message(STATUS "========================================")
    message(STATUS "Architecture Detection Report:")
    message(STATUS "Generator: ${CMAKE_GENERATOR}")
    message(STATUS "Generator Platform: ${CMAKE_GENERATOR_PLATFORM}")
    message(STATUS "VS Platform: ${CMAKE_VS_PLATFORM_NAME}")
    message(STATUS "System Processor: ${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "Detected Architecture: ${TARGET_ARCH}")
    message(STATUS "========================================")
endif()

# 设置默认架构
set(TARGET_ARCH "x64")

# 定义架构相关路径
set(ARCH_SUFFIX "")
if(TARGET_ARCH STREQUAL "x64")
    set(ARCH_SUFFIX "64")
    set(VC_TOOLSET "host=x64")
# elseif(TARGET_ARCH STREQUAL "arm64")
#     set(ARCH_SUFFIX "arm64")
#     set(VC_TOOLSET "host=arm64")
# else()
#     set(ARCH_SUFFIX "32")
#     set(VC_TOOLSET "host=x86")
endif()

# 配置Qt路径
set(QT_ROOT "D:/Qt/6.8.3/msvc2022_${ARCH_SUFFIX}")  # 确保使用MSVC版本

# if(NOT QT_ROOT)
#     # 尝试自动查找Qt安装路径
#     get_filename_component(QT_DEFAULT_PATH "D:/Qt" ABSOLUTE)
#     file(GLOB QT_VERSIONS LIST_DIRECTORIES true "${QT_DEFAULT_PATH}/*")
    
#     # 按版本排序找到最新的
#     list(SORT QT_VERSIONS)
#     list(REVERSE QT_VERSIONS)
    
#     foreach(QT_PATH IN LISTS QT_VERSIONS)
#         if(IS_DIRECTORY "${QT_PATH}/msvc2022_${ARCH_SUFFIX}")
#             set(QT_ROOT "${QT_PATH}/msvc2022_${ARCH_SUFFIX}")
#             break()
#         endif()
#     endforeach()
    
#     if(NOT QT_ROOT)
#         message(FATAL_ERROR "Qt for MSVC 2022 (${TARGET_ARCH}) not found. Please set QT_ROOT manually")
#     endif()
# endif()

# if(NOT EXISTS ${QT_ROOT})
#     message(FATAL_ERROR "Qt for ${TARGET_ARCH} not found at ${QT_ROOT}")
# endif()

# 添加到CMAKE_PREFIX_PATH
list(PREPEND CMAKE_PREFIX_PATH ${QT_ROOT})

# 设置第三方库路径
set(SWITCHBUTTON_LIB_ROOT "${CMAKE_SOURCE_DIR}/lib/${TARGET_ARCH}")

# 修改库查找路径
set(SWITCHBUTTON_RELEASE_DIR "${SWITCHBUTTON_LIB_ROOT}/release")
set(SWITCHBUTTON_DEBUG_DIR "${SWITCHBUTTON_LIB_ROOT}/debug")

# 从环境变量读取 vcpkg 路径
if (DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    # 显式添加 vcpkg 的安装路径到 CMAKE_PREFIX_PATH
    list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows")
endif()

cmake_minimum_required(VERSION 3.21)

# 设置可执行文件名后缀
set(EXECUTABLE_NAME "GameOptimizerPro_${TARGET_ARCH}")

# 设置项目名称和版本 使用带架构后缀的名称
set(PROJECT_NAME_EN ${EXECUTABLE_NAME})

string(TIMESTAMP TIMESTAMP "%Y-%m-%d %H:%M:%S")

# 版本完整定义
set(PROJECT_VERSION_MAJOR 0)    # 主版本号（重大功能更新）
set(PROJECT_VERSION_MINOR 1)    # 次版本号（功能改进）
set(PROJECT_VERSION_PATCH 0)    # 修订号（Bug修复）
set(PROJECT_VERSION_TWEAK 0)    # 内部版本号
set(TIMESTAMP $<CONFIGURATION_TIMESTAMP>)

# 1. 查找Git可执行文件
find_package(Git QUIET)
if(NOT Git_FOUND)
    message(WARNING "Git not found - version information will be incomplete")
    set(GIT_COMMIT_HASH "unknown")
    set(GIT_COMMIT_HASH_SHORT "unknown")
    set(GIT_BRANCH "unknown")
    set(GIT_TAG "unknown")
    set(GIT_IS_DIRTY 0)
else()
    # 2. 获取Git提交哈希
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # 3. 获取短哈希（前7位）
    if(GIT_COMMIT_HASH)
        string(SUBSTRING ${GIT_COMMIT_HASH} 0 7 GIT_COMMIT_HASH_SHORT)
    else()
        set(GIT_COMMIT_HASH_SHORT "unknown")
    endif()
    
    # 4. 获取当前分支名称
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # 5. 获取最近的标签
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # 6. 检查是否有未提交的修改
    execute_process(
        COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD --
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_IS_DIRTY
    )
    if(GIT_IS_DIRTY EQUAL 0)
        set(GIT_IS_DIRTY 0)
    else()
        set(GIT_IS_DIRTY 1)
    endif()
endif()

project(
    ${PROJECT_NAME_EN}
    VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
    LANGUAGES CXX
)

# 确保正确处理资源文件
if(MSVC)
    # 对于MSVC编译器，设置资源编译器选项
    set(CMAKE_RC_COMPILER_INIT rc)
    enable_language(RC)
    
    # 添加资源编译定义
    set_source_files_properties(${RC_FILE} PROPERTIES
        COMPILE_DEFINITIONS "PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR};PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR};PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH};PROJECT_VERSION=\"${PROJECT_VERSION}\";PROJECT_NAME=\"${PROJECT_NAME_EN}\""
    )
endif()

# 版本文件生成
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/version.h.in
    ${CMAKE_BINARY_DIR}/version.h
    @ONLY
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器特定的扩展

# --- Windows 特定配置 ---
# 检查是否在 Windows 上构建
if(NOT WIN32)
    message(FATAL_ERROR "此项目只能在 Windows 上构建，因为它依赖 Windows SDK, WMI 和 ATL。")
endif()

# 启用异常处理
add_compile_options(/EHsc)

# 设置 Unicode 字符集 (根据代码需求)

add_definitions(-DUNICODE -D_UNICODE)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

# 查找 nlohmann_json
find_package(nlohmann_json REQUIRED)

# 查找 WinToast 库
# find_package(WinToast REQUIRED)

file(GLOB TS_FILES "${CMAKE_SOURCE_DIR}/Translations/*.ts")
qt_add_translation(QM_FILES ${TS_FILES})

# 安装翻译文件
install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/translations
)

# 添加 rc 文件
set(RC_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/GameOptimizerPro.rc"
)

# 确保资源文件被正确处理
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <DEFINES> <INCLUDES> <FLAGS> /fo <OBJECT> <SOURCE>")
endif()

# 添加资源文件
set(RESOURCES
    resources/res.qrc
)
qt_add_resources(RESOURCE_FILES 
    ${RESOURCES}
)

set(PROJECT_SOURCES
    src/main.cpp

    src/ui/mainwnd.cpp
    src/ui/tray_app.cpp
    src/ui/mainwnd.ui

    src/log/logging.cpp
    
    src/core/application.cpp
    src/core/config_manager.cpp
    src/core/optimizer.cpp
    src/core/process_manager.cpp
    src/core/power_manager.cpp
    src/core/registry_manager.cpp
    src/core/service_manager.cpp

    src/utils/system_utils.cpp
    src/utils/event_sink.cpp

    src/config/app_config.cpp
    src/config/optimism_config.cpp
    src/config/power_plan.cpp
    src/config/process_config.cpp
    src/config/process_info.cpp
    src/config/system_info.cpp

    include/ui/mainwnd.h
    include/ui/tray_app.h

    include/log/logging.h

    include/core/application.h
    include/core/config_manager.h
    include/core/optimizer.h
    include/core/power_manager.h
    include/core/process_manager.h
    include/core/registry_manager.h
    include/core/service_manager.h

    include/utils/system_utils.h
    include/utils/event_sink.h

    include/config/app_config.h
    include/config/optimism_config.h
    include/config/power_plan.h
    include/config/process_config.h
    include/config/process_info.h
    include/config/system_info.h

    # rc 文件
    ${RC_FILE}

    # 资源文件
    ${RESOURCE_FILES}

    # 语言翻译文件
    ${TS_FILES}
)

# 定义不同构建类型的库名-静态库
set(SWITCHBUTTON_STATIC_RELEASE_NAME "switchbutton")
set(SWITCHBUTTON_STATIC_DEBUG_NAME "switchbuttond")

# 查找 Release 版本的静态库
find_library(SWITCHBUTTON_STATIC_LIBRARY_RELEASE  # 存储查找结果的变量
    NAMES ${SWITCHBUTTON_STATIC_RELEASE_NAME}    # 要查找的库名 (不含扩展名)
    PATHS ${SWITCHBUTTON_RELEASE_DIR}           # 在哪个目录下查找
    NO_DEFAULT_PATH                            # 不在系统默认路径查找
    DOC "Path to the release version of the SwitchButton static library (.lib)" # 添加文档说明
)

# 查找 Debug 版本的静态库
find_library(SWITCHBUTTON_STATIC_LIBRARY_DEBUG # 存储查找结果的变量
    NAMES ${SWITCHBUTTON_STATIC_DEBUG_NAME}    # 要查找的库名 (不含扩展名)
    PATHS ${SWITCHBUTTON_DEBUG_DIR}            # 在哪个目录下查找
    NO_DEFAULT_PATH
    DOC "Path to the debug version of the SwitchButton static library (.lib)" # 添加文档说明
)

# 如果使用 Debug 名称 (${SWITCHBUTTON_STATIC_DEBUG_NAME}) 没有找到 Debug 库
if(NOT SWITCHBUTTON_STATIC_LIBRARY_DEBUG)
    message(STATUS "Debug library '${SWITCHBUTTON_STATIC_DEBUG_NAME}' not found in ${SWITCHBUTTON_DEBUG_DIR}. Trying fallback name '${SWITCHBUTTON_STATIC_RELEASE_NAME}'...")
    # 尝试使用 Release 名称 (${SWITCHBUTTON_STATIC_RELEASE_NAME}) 在 Debug 目录查找
    find_library(SWITCHBUTTON_STATIC_LIBRARY_DEBUG # 仍然存储到这个变量
        NAMES ${SWITCHBUTTON_STATIC_RELEASE_NAME} # 使用备用名称查找
        PATHS ${SWITCHBUTTON_STATIC_LIBRARY_DIR}
        NO_DEFAULT_PATH
    )
endif()

# 检查是否至少找到了一个版本的库-静态库
if(NOT SWITCHBUTTON_STATIC_LIBRARY_RELEASE AND NOT SWITCHBUTTON_STATIC_LIBRARY_DEBUG)
    # 同时提供搜索的名称和路径，方便调试
    message(FATAL_ERROR "Could not find SwitchButton static library.\n"
                        " Searched for Release name '${SWITCHBUTTON_STATIC_RELEASE_NAME}' in '${SWITCHBUTTON_RELEASE_DIR}'.\n"
                        " Searched for Debug names '${SWITCHBUTTON_STATIC_DEBUG_NAME}' and '${SWITCHBUTTON_STATIC_RELEASE_NAME}' in '${SWITCHBUTTON_DEBUG_DIR}'.\n"
                        " Please ensure the library is built as STATIC, check the paths and names.")
elseif(NOT SWITCHBUTTON_STATIC_LIBRARY_RELEASE)
     message(WARNING "SwitchButton Release static library not found (Searched for '${SWITCHBUTTON_STATIC_RELEASE_NAME}' in ${SWITCHBUTTON_RELEASE_DIR}). Release builds might fail or link incorrectly.")
elseif(NOT SWITCHBUTTON_STATIC_LIBRARY_DEBUG)
     message(WARNING "SwitchButton Debug static library not found (Searched for '${SWITCHBUTTON_STATIC_DEBUG_NAME}' and '${SWITCHBUTTON_STATIC_RELEASE_NAME}' in ${SWITCHBUTTON_DEBUG_DIR}). Debug builds might fail or link incorrectly.")
else()
    message(STATUS "Found SwitchButton Release static library: ${SWITCHBUTTON_STATIC_LIBRARY_RELEASE}")
    message(STATUS "Found SwitchButton Debug static library: ${SWITCHBUTTON_STATIC_LIBRARY_DEBUG}")
endif()

# 设置UI文件的生成规则
set_property(GLOBAL APPEND PROPERTY AUTOGEN_SOURCE_GROUP "UI Files")
set_source_files_properties(src/QtUI/mainwnd.ui PROPERTIES QT_SOURCE_GENERATOR UIC)

# 创建翻译文件
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# 添加可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${EXECUTABLE_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
endif()

# 添加必要的预处理器定义
# _ATL_FREE_THREADED 通常与 CComMultiThreadModel 一起使用
# 根据需要添加其他宏，例如 UNICODE, _UNICODE (MSVC 通常默认设置)
# 添加宏定义-静态库
# 在 add_executable 或 qt_add_executable 之后
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
    SWITCHBUTTON_STATIC
    _ATL_FREE_THREADED
    UNICODE
    _UNICODE
)

# 添加包含目录
target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR} # AUTOUIC/AUTOMOC 生成的文件通常在这里
)

# 编译定义传递版本信息
target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
    "PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}"
    "PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}"
    "PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}"
    "PROJECT_VERSION=\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}\""
    "PROJECT_NAME=\"${PROJECT_NAME_EN}\""
    "GIT_COMMIT_HASH_SHORT=\"${GIT_COMMIT_HASH_SHORT}\""
    "GIT_BRANCH=\"${GIT_BRANCH}\""
)

# 添加版本信息到安装目标
install(FILES ${CMAKE_BINARY_DIR}/version.h
    DESTINATION include/${PROJECT_NAME_EN}
)

# 添加链接库
target_link_libraries(${EXECUTABLE_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    
    # WinToast::WinToast

    nlohmann_json::nlohmann_json

    wbemuuid.lib
    ole32.lib
    oleaut32.lib
    powrprof.lib
    Advapi32.lib

    # $<CONFIG:Debug> 会在 Debug 构建时展开为后面的内容
    $<$<CONFIG:Debug>:${SWITCHBUTTON_STATIC_LIBRARY_DEBUG}>
    # $<CONFIG:Release> 会在 Release 构建时展开
    $<$<CONFIG:Release>:${SWITCHBUTTON_STATIC_LIBRARY_RELEASE}>
    # $<CONFIG:RelWithDebInfo> 会在 RelWithDebInfo 构建时展开 (通常链接 Release 库)
    $<$<CONFIG:RelWithDebInfo>:${SWITCHBUTTON_STATIC_LIBRARY_RELEASE}>
    # $<CONFIG:MinSizeRel> 会在 MinSizeRel 构建时展开 (通常链接 Release 库)
    $<$<CONFIG:MinSizeRel>:${SWITCHBUTTON_STATIC_LIBRARY_RELEASE}>
)

# 获取可执行文件的完整路径 (这在 add_custom_command 中可能不直接工作，最好获取目录)
# 一个更可靠的方法是直接指定输出目录，CMake 通常是 CMAKE_BINARY_DIR 下的某个位置
# 或者假设它在 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR} (取决于生成器)
# 为了简单起见，我们先尝试复制到 CMAKE_RUNTIME_OUTPUT_DIRECTORY
# 如果你的可执行文件直接生成在构建目录下，可以简化路径

# 设置可执行文件输出目录 (如果未设置)
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0 OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.GameOptimizerPro)
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 安装可执行文件
install(TARGETS ${EXECUTABLE_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)

# 添加打包目标
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "${EXECUTABLE_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "vdavidyang")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
include(CPack)

# 添加更严格的编译选项
target_compile_options(${EXECUTABLE_NAME} PRIVATE
    /W4         # 高警告级别
    /WX         # 视警告为错误
    /permissive- # 标准一致性模式
    
    # 调试信息
    $<$<CONFIG:Debug>:/Zi /Od>
    $<$<NOT:$<CONFIG:Debug>>:/O2>
    
    # 安全相关
    /sdl        # 启用附加安全检查
    /guard:cf   # 启用控制流防护
)
