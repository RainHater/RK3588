# RK3588 C++ 仓库

一个简洁现代的 RK3588 C++ 仓库, 带python 构建编译脚本, 使用 CMake + Makefile 构建。
适合快速启动中小型 C++ 工程或嵌入式项目。

---

## 已完成功能

- ✅ V4L2 采集摄像头画面

---

## 环境与工具要求
- 操作系统：Ubuntu 22.04 LTS
- 开发板：[Rock 5b](https://docs.radxa.com/rock5/rock5b)

## 项目的环境构建
1. 安装的依赖
```bash
sudo apt install libdrm-dev libopencv-dev v4l-utils -y
```

2. 下载子仓库
```bash
git submodule update --init --recursive --depth 1
```

## 构建与运行

```bash
#配置 + 编译
make

#运行程序
make run

#清理
make clean
```

## 注意

1. 当出现 brltty 占用 USB 端口请卸载它
```bash
sudo systemctl stop brltty
sudo systemctl disable brltty

sudo apt-get remove --purge brltty

sudo apt-get autoremove
```
