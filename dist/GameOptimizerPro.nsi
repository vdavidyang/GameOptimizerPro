
; 指定插件目录
!addplugindir ".\Plugins"

; 添加版本定义
!define VERSION "1.0.0.0"

Var res_hash_file
Var HASH_VALUE_EXPECT
Var HASH_VALUE_NOW

; 添加架构定义
!ifndef ARCH
    !ifdef BUILD_ARCH
        !define ARCH "${BUILD_ARCH}"
    !else
        !define ARCH "x64" ; 默认架构为x64
    !endif
!endif

; 设置源文件目录和输出目录
!define SOURCE_DIR "..\build\${ARCH}\Release" ; 源文件目录
!define OUTPUT_DIR "${ARCH}"                  ; 输出目录

Name "鱼腥味的游戏优化工具箱 (${ARCH})"
OutFile "${OUTPUT_DIR}\GameOptimizerPro_${ARCH}_Setup.exe"
InstallDir "$PROGRAMFILES\GameOptimizerPro_${ARCH}"
RequestExecutionLevel admin

; 界面设置
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "StrFunc.nsh"
${StrCase}
!define MUI_ICON "..\resources\app.ico"
!define MUI_UNICON "..\resources\uninstall.ico"

; 安装页面流程
; 欢迎页面
!insertmacro MUI_PAGE_WELCOME
; 许可协议
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
; 选择安装位置
!insertmacro MUI_PAGE_DIRECTORY
; 安装过程
!insertmacro MUI_PAGE_INSTFILES
; 完成页面
!insertmacro MUI_PAGE_FINISH

; 卸载页面流程
; 确认卸载
!insertmacro MUI_UNPAGE_CONFIRM
; 卸载过程
!insertmacro MUI_UNPAGE_INSTFILES

; 语言设置
!insertmacro MUI_LANGUAGE "SimpChinese"

Function .onInit
    
    ; 读取哈希文件
    IfFileExists "$EXEDIR\GameOptimizerPro_${ARCH}_Setup.exe.sha256" +2 0
    Goto skip_hash_check

    ; 执行certutil读取安装包自身哈希值
    nsExec::ExecToStack 'cmd /c "certutil -hashfile "$EXEDIR\GameOptimizerPro_${ARCH}_Setup.exe" SHA256 | findstr /v "SHA256" | findstr /v "CertUtil""'
    Pop $0 ; 获取返回值（0=成功）
    Pop $1 ; 获取命令输出（哈希值）
    ${If} $0 != 0
        MessageBox MB_ICONSTOP "计算哈希失败！错误代码：$0"
        ; Abort
    ${EndIf}

    StrCpy $HASH_VALUE_NOW $1
    
    ; 清理哈希值 去除首尾空格/换行
    Push $HASH_VALUE_NOW
    Call Trim
    Pop $HASH_VALUE_NOW

    ; 将哈希值转换为小写
    ${StrCase} $HASH_VALUE_NOW $HASH_VALUE_NOW "L"

    ; 读取预期哈希值
    ClearErrors
    FileOpen $res_hash_file "$EXEDIR\GameOptimizerPro_${ARCH}_Setup.exe.sha256" r
    ${If} ${Errors}
        MessageBox MB_ICONSTOP "无法打开哈希文件！"
        Abort
    ${EndIf}
    
    FileRead $res_hash_file $HASH_VALUE_EXPECT  ; 读取第一行
    FileClose $res_hash_file

    ; 清理哈希值 去除首尾空格/换行
    Push $HASH_VALUE_EXPECT
    Call Trim
    Pop $HASH_VALUE_EXPECT

    ; 将哈希值转换为小写
    ${StrCase} $HASH_VALUE_EXPECT $HASH_VALUE_EXPECT "L"

    ; 比较提取到的哈希值和预期哈希值（不区分大小写）,如果不相等则弹出提示
    StrCmp $HASH_VALUE_NOW $HASH_VALUE_EXPECT equal not_equal

    equal:
        ; MessageBox MB_OK "哈希值匹配！"
        Goto done

    not_equal:
        MessageBox MB_ICONSTOP "安装程序校验失败!$\n文件可能被篡改或损坏。请前往官方地址重新下载。"
        Abort

    done:

    ; ${If} $HASH_VALUE_NOW != $HASH_VALUE_EXPECT
    ;     MessageBox MB_ICONSTOP "安装程序校验失败!$\n文件可能被篡改或损坏。请前往官方地址重新下载。"
    ;     Abort
    ; ${EndIf}

    skip_hash_check:
FunctionEnd

; 定义Trim函数（处理首尾空格/换行符）
Function Trim
    Exch $R0
    Push $R1
    Push $R2

    ; 去除左侧空白字符
    StrCpy $R1 0
    left_loop:
        StrCpy $R2 $R0 1 $R1
        StrCmp $R2 "" right_trim
        StrCmp $R2 " " left_trim
        StrCmp $R2 "$\t" left_trim
        StrCmp $R2 "$\r" left_trim
        StrCmp $R2 "$\n" left_trim
        Goto right_trim
    left_trim:
        IntOp $R1 $R1 + 1
        Goto left_loop

    right_trim:
    StrCpy $R0 $R0 "" $R1
    StrLen $R1 $R0
    IntOp $R1 $R1 - 1
    right_loop:
        StrCmp $R1 -1 done
        StrCpy $R2 $R0 1 $R1
        StrCmp $R2 " " right_cut
        StrCmp $R2 "$\t" right_cut
        StrCmp $R2 "$\r" right_cut
        StrCmp $R2 "$\n" right_cut
        Goto done
    right_cut:
        IntOp $R1 $R1 - 1
        Goto right_loop

    done:
    IntOp $R1 $R1 + 1
    StrCpy $R0 $R0 $R1

    Pop $R2
    Pop $R1
    Exch $R0
FunctionEnd

; 从命令行参数读取哈希值
!ifdef EXE_HASH
    !define EXPECTED_HASH "${EXE_HASH}"
!else
    !define EXPECTED_HASH "SKIP_CHECK" ; 默认跳过
!endif

Section "主程序" SEC_MAIN
    SetOutPath $INSTDIR
    
    ; 先校验源文件再复制
    !if "${EXPECTED_HASH}" != "SKIP_CHECK"
        Crypto::HashFile "SHA256" "${SOURCE_DIR}\GameOptimizerPro_${ARCH}.exe"
        Pop $0
        StrCmp $0 "${EXPECTED_HASH}" +3 0
        MessageBox MB_ICONSTOP "文件校验失败，安装程序可能损坏或被篡改！"
        Abort
    !endif

    ; 从源目录复制文件
    File /r "${SOURCE_DIR}\GameOptimizerPro_${ARCH}.exe"
    File /r "${SOURCE_DIR}\vc_redist.x64.exe"
    File /r "${SOURCE_DIR}\*.dll"
    File /r "${SOURCE_DIR}\platforms\*.*"
    File /r "${SOURCE_DIR}\config\*.*"
    ; File /r "${SOURCE_DIR}\translations\*.*"

    ; 创建快捷方式
    CreateDirectory "$SMPROGRAMS\鱼腥味的游戏优化工具箱 (${ARCH})"
    CreateShortcut "$SMPROGRAMS\鱼腥味的游戏优化工具箱 (${ARCH})\游戏优化工具箱.lnk" "$INSTDIR\GameOptimizerPro_${ARCH}.exe"
    CreateShortcut "$SMPROGRAMS\鱼腥味的游戏优化工具箱 (${ARCH})\卸载.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortcut "$DESKTOP\鱼腥味的游戏优化工具箱 (${ARCH}).lnk" "$INSTDIR\GameOptimizerPro_${ARCH}.exe"
    
    ; 写入注册表
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameOptimizerPro" "DisplayName" "游戏优化工具箱 (${ARCH})"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameOptimizerPro" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameOptimizerPro" "DisplayVersion" "${VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameOptimizerPro" "Publisher" "鱼腥味工作室"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameOptimizerPro" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameOptimizerPro" "NoRepair" 1
    WriteRegStr HKLM "Software\GameOptimizerPro" "Architecture" "${ARCH}"
        
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "运行环境检测"
    ; 检查是否已安装 VC++ 运行库
    IfFileExists "$SYSDIR\vcruntime140.dll" vc_ok
    
    ; 如果未安装，尝试从安装包目录运行 vc_redist.x64.exe
    MessageBox MB_YESNO "需要安装 VC++ 2015-2022 运行库，是否立即安装？" IDNO vc_ok
    SetOutPath "$PLUGINSDIR" ; 临时目录
    File "${SOURCE_DIR}\vc_redist.x64.exe" ; 从源目录提取安装包
    ExecWait '"$PLUGINSDIR\vc_redist.x64.exe" /install /quiet /norestart' $0
    ${If} $0 != 0
        MessageBox MB_ICONSTOP "VC++ 运行库安装失败，错误代码：$0"
    ${EndIf}
    Delete "$PLUGINSDIR\vc_redist.x64.exe" ; 清理临时文件
    
    vc_ok:
SectionEnd

Section "Uninstall"
    RMDir /r $INSTDIR
    Delete "$DESKTOP\鱼腥味的游戏优化工具箱 (${ARCH}).lnk"
    RMDir /r "$SMPROGRAMS\鱼腥味的游戏优化工具箱 (${ARCH})"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GameOptimizerPro"
    DeleteRegKey HKLM "Software\GameOptimizerPro"
SectionEnd

; 版本信息补全
VIProductVersion "${VERSION}"
VIAddVersionKey "ProductName" "鱼腥味的游戏优化工具箱 (${ARCH})"
VIAddVersionKey "FileDescription" "鱼腥味的游戏优化工具箱安装程序"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "LegalCopyright" "© 2025 鱼腥味"
VIAddVersionKey "OriginalFilename" "GameOptimizerPro_${ARCH}_Setup.exe"