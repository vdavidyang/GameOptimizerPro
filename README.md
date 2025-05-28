# 🚀 GameOptimizer Pro - Windows游戏性能优化工具箱

<div style="text-align:center">   ![icon](./resources/app.png) </div>

🎮**为Windows游戏玩家打造的性能加速神器 | 基于GPL 3.0开源协议**✨

![version](https://img.shields.io/badge/version-1.0.0-blue)![License](https://img.shields.io/badge/license-GPLv3-blue)![Branch](https://img.shields.io/github/actions/workflow/build.yml?branch=main)![Downloads](https://img.shields.io/github/downloads/vdavidyang/GameOptimizerPro/total)

## 🔍 项目简介

专为PC游戏玩家设计的系统级优化工具，通过智能调整系统资源分配、进程优先级和网络设置，显著提升游戏帧率稳定性，降低卡顿和延迟。采用Qt框架开发，支持Windows 10/11系统。

## ⚡ 核心功能

- 🎮 **进程优化**：优化游戏/反作弊进程，动态调整优先级和CPU亲和性
- 🔋 **电源性能模式**：一键切换至高性能电源计划
- 🌐 **网络延迟优化**：禁用Nagle算法，加速TCP传输
- 🛑 **后台抑制**：游戏运行时自动限制后台程序资源占用
- ⚙️ **系统服务管理**：禁用非必要系统服务释放资源
- 📈 **调度优化**：提升游戏进程的线程调度优先级

## 📦 安装指南

### 二进制安装

1. 下载最新安装包：进入[Release页面](https://github.com/yourusername/GameOptimizerPro/releases)，下载`GameOptimizerPro_Setup.exe`
2. 运行安装程序
3. 根据向导完成安装

```powershell
# 静默安装命令（管理员权限）
./GameBoostPro_Setup.exe /S
```

## 🚦 快速使用

1. 启动程序后进入系统托盘
2. 双击托盘图标打开主窗口
3. 开启目标游戏进程优化
4. 开启游戏电源优化
5. 开启自动限制反作弊
6. 启动游戏享受流畅体验！

## 🔨 构建指南

### 环境要求

- Visual Studio 2022/VS Code
- Qt 6.8.3 (msvc2022_64)
- CMake 3.25+
- NSIS 3.0+

```
git clone https://github.com/vdavidyang/GameOptimizerPro.git
cd GameOptimizerPro
build_all.bat
# 生成文件在 build 目录
```

## ⚠️ 注意事项

1. 需要以管理员权限运行
3. 建议关闭其他优化软件避免冲突
4. 修改系统服务前请创建还原点

## ❓ 常见问题

**Q：会封号吗？**
A：绝对安全！仅调整系统调度规则，不修改游戏文件，主播长期实测。

**Q：帧率提升明显吗？**
A：多数用户反馈有效，实测可提升10-20帧，显著改善团战卡顿。

**Q：脚本报错怎么办？**
A：请检查是否以管理员身份运行，或提交Issue反馈问题。

**Q：杀毒软件报毒？** 
A：因涉及注册表修改，部分杀软可能误报，请添加信任或[查看源码](src/)验证安全性。  

**Q：支持Steam/Uplay平台吗？**
A：支持所有Windows平台的游戏进程

## 🤝 贡献指南

欢迎通过以下方式参与：

1. 提交Issue报告问题或建议
2. 发起Pull Request改进代码
3. 完善中文/英文翻译文件（在translations目录）
4. 编写技术文档或使用教程

## ⚠️ 免责声明

本程序仅供学习和技术研究使用，作者不对使用此程序导致的任何后果负责。使用本程序即表示您已了解并接受相关风险。

## 📧 联系作者

- 邮箱：vdavidyang@gmail.com
- GitHub Issues: [提交问题](https://github.com/vdavidyang/GameOptimizerPro/issues)

## 📜 开源协议

本项目基于 GPL-3.0 License 开源，二次开发请遵守协议要求。