# 工程分层重构报告

## 重构结果

工程自有代码已按 `app / service / device / bsp / platform / component` 分层，并为各层建立独立 CMake target。当前依赖方向为：

```text
main（组合根） -> app_core/platform
app_core -> component
service -> 通用 OpenCV 能力
device -> bsp
bsp -> component/Linux API/厂商 SDK
platform -> component/Linux API
component -> 标准库/通用第三方库
```

现有代码没有发现反向依赖、循环依赖或上层直接访问 Linux 设备接口、RKNN、V4L2、FFmpeg 的情况。

## 模块归属

| 层级 | 模块 | 说明 |
| --- | --- | --- |
| app | `Application`、`main` | 命令行与应用启动流程；`main` 仅负责对象组装 |
| service | `FaceProcessing` | 人脸图像预处理、特征归一化与相似度计算 |
| device | `InferenceDevice` | 使用项目自有张量类型隔离 RKNN |
| device | `CameraDevice`、`FrameStreamerDevice` | 使用项目自有视频帧类型隔离 V4L2、OpenCV 和 FFmpeg |
| bsp | `RknnInference` | Rockchip RKNN SDK 适配 |
| bsp | `V4L2Capture` | Linux V4L2 摄像头适配 |
| bsp | `FFmpegStreamer` | Rockchip FFmpeg 编码与推流适配 |
| platform | `SharedMemory`、`Tools` | POSIX 共享内存和 Linux 进程路径能力 |
| component | `logging::GetLogger`、`SafeQueue`、`ThreadPool` | 通用日志和并发组件；无状态日志入口使用命名空间 |

## 可靠性修复

- `SharedMemory` 改为文件锁初始化、进程共享 robust mutex 和事务式 `Write/ReadIfNew`，避免 ARM 多进程下的数据撕裂与发布乱序。
- `ThreadPool` 禁止停止后重新启动，重复停止仍会正确回收线程，避免析构时触发 `terminate`。
- V4L2 使用驱动实际分配的缓冲数量，增加非阻塞采集、`poll` 超时、`EINTR/EAGAIN` 处理和失败资源回收。
- FFmpeg 初始化失败会统一释放半初始化资源；编码、取包和写流错误会返回到 device 层。
- RKNN 初始化会正确传播加载或查询失败，并保证输出缓冲在所有错误路径释放。
- `--version` 在日志初始化前处理，保持无副作用行为。
- 将原 `LoggerWithTag` 单例式静态类改为 `logging` 命名空间函数，保留线程安全复用，并拒绝外部同名日志器冲突。
- 日志模块新增 `logging::SetLogPath` 接口，可在首次获取日志器前设置完整日志文件路径，并自动创建父目录；未设置时仍使用原时间戳路径。
- 删除重构后形成的孤儿 `Dynamic.cmake` 和 `Singleton.h`。

## 验证结果

验证环境：`lima-ubuntu22`。

| 验证项 | 结果 |
| --- | --- |
| `make` | 通过 |
| 默认 `make test` | 6/6 通过 |
| ThreadPool 与共享内存测试重复 30 次 | 通过 |
| `/proc` 目录执行 `build/app --version` | 输出 `1.0.0` |
| `git diff --check` | 通过 |
| code-reviewer 最终审查 | 无阻断问题 |

## 未执行与注意事项

- Lima 环境没有 RKNN 驱动、真实 V4L2 摄像头和 RTSP 服务，因此硬件成功链路没有实机执行。
- `facenet_test` 仍会参与编译，但默认不注册到 CTest；需要目标硬件时使用 `-DRKPLATFORM_ENABLE_HARDWARE_TESTS=ON` 显式启用。
- 本次重构调整了内部头文件路径、命名空间和 CMake target 名称。应用命令行行为保持不变，但仓库外若直接引用旧内部头文件，需要迁移到新的 device 公共接口。
- 未修改 `3rdparty/`、模型、图片和部署资源。
