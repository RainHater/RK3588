#项目信息
PROJECT_NAME = app
VERSION = 1.0.0

ROOT_DIR = $(PWD)
BUILD_DIR = $(ROOT_DIR)/build
INSTALL_DIR = $(ROOT_DIR)/target
THIRD_PARTY_DIR = $(ROOT_DIR)/3rdparty
SCRIPTS_DIR = $(ROOT_DIR)/scripts
THIRD_PARTY_PYTHON_DIR = $(SCRIPTS_DIR)/python/3rdparty

#编译选项
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Debug -DAPP_VERSION=$(VERSION)
THREAD_NUM := 14

.ONESHELL:
setenvs = \
    set_envs() { \
        export ROOT_DIR=$(ROOT_DIR) \
		export BUILD_DIR=$(BUILD_DIR); \
        export THIRD_PARTY_DIR=$(THIRD_PARTY_DIR); \
        export INSTALL_DIR=$(INSTALL_DIR); \
		export THREAD_NUM=$(THREAD_NUM); \
		export THIRD_PARTY_PYTHON_DIR=$(THIRD_PARTY_PYTHON_DIR); \
		export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:$(INSTALL_DIR)/target/lib; \
		export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig:$(INSTALL_DIR)/lib/pkgconfig; \
    }; \
    set_envs
