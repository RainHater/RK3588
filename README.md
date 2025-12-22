# RK3588 C++ 仓库

一个简洁现代的 **RK3588 C++ 仓库**, 带python 构建编译脚本, 使用 **CMake + Makefile** 构建。
适合快速启动中小型 C++ 工程或嵌入式项目。

---

## 功能特性

- 支持 CMake 自动配置与构建  
- 使用 Makefile 一键构建、运行、清理  
- 可配置编译类型（Debug / Release）  
- 支持模块化扩展（库 + 主程序）
- Python 脚本可快速子模块安装

---

## 环境与工具要求
- 操作系统：Ubuntu 22.04 LTS
- 开发板：[Rock 5b](https://docs.radxa.com/rock5/rock5b)

## 项目的环境构建
1. 安装的依赖
```bash
sudo apt install libdrm-dev
```

2. 下载子仓库
```bash
git submodule update --init --recursive --depth 1
```

## ⚙️ 构建与运行

### 使用 Makefile

```bash
#配置 + 编译
make

#运行程序
make run

#清理
make clean
```
