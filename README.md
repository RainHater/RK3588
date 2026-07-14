# C++ 嵌入式工程模版仓库

一个简洁现代的嵌入式工程模版仓库, 带Python 构建编译脚本, 使用 CMake + Makefile 构建, 适合快速启动中小型 C++ 工程或嵌入式项目。

---

## 已完成功能

- [x] V4L2 采集摄像头画面
- [x] ffmpeg 推流
- [x] 模块化 CMake
- [x] facenet 推理测试例子

---

## 环境与工具要求
- 操作系统：[Ubuntu 22.04 LTS](https://github.com/Joshua-Riek/ubuntu-rockchip)
- 开发板：[Rock 5b](https://docs.radxa.com/rock5/rock5b)

## 项目的环境构建
1. 安装的依赖
```bash
sudo apt install libdrm-dev libspdlog-dev libopencv-dev v4l-utils patchelf -y
```

2. 下载子仓库
```bash
git submodule update --init --recursive --depth 1
```

## 构建与运行

1. 构建编译
```bash
#配置 + 编译
make

#运行程序
make run

#清理
make clean
```

2. FFMPEG 拉流
```bash
ffplay -fflags nobuffer -flags low_delay -framedrop -strict experimental rtsp://<ip>>:8554/live
```
