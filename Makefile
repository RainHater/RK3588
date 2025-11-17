# === 项目信息 ===
PROJECT_NAME := app

ROOT_DIR := ${PWD}
THIRD_PARTY_DIR := ${ROOT_DIR}/3rdparty
BUILD_DIR := ${ROOT_DIR}/build
INSTALL_DIR  := ${${ROOT_DIR}}install


# === 编译选项 ===
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=Release
THREAD_NUM := 14

# === 默认目标 ===
.PHONY: all
all: configure build

# === 配置阶段 ===
.PHONY: configure
configure:
	@echo "==> Configuring project $(PROJECT_NAME)..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DPROJECT_NAME=$(PROJECT_NAME) ..

# === 编译阶段 ===
.PHONY: build
build:
	@echo "==> Building project..."
	@cd $(BUILD_DIR) && $(MAKE)

# === 安装阶段 ===
.PHONY: install
install:
	@echo "==> Installing to $(INSTALL_DIR)..."
	@cd $(BUILD_DIR) && $(MAKE) install DESTDIR=$(abspath $(INSTALL_DIR))

# === 清理构建文件 ===
.PHONY: clean
clean:
	@echo "==> Cleaning build directory..."
	@rm -rf $(BUILD_DIR) $(INSTALL_DIR)

# === 运行程序 ===
.PHONY: run
run: build
	@echo "==> Running $(PROJECT_NAME)..."
	@$(BUILD_DIR)/$(PROJECT_NAME)

.PHONY: opencv
opencv:
	@[ -e ${BUILD_DIR}/$@/.build_ok ] && echo "$@ compilation completed..." || mkdir -p ${BUILD_DIR}/$@

	cd ${BUILD_DIR}/$@ && \
	cmake -DCMAKE_BUILD_TYPE=Release \
		-DSMALL_LOCALSIZE=ON -DENABLE_FAST_MATH=ON -DWITH_IPP=OFF \
		-DUSE_O3=ON -DENABLE_CXX11=ON -DWITH_TBB=ON -DWITH_OPENMP=ON -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DWITH_WEBP=OFF \
		-DWITH_OPENCL=ON -DWITH_OPENGL=OFF -DWITH_QT=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=ON -DWITH_CUDA=OFF \
		-DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_opencv_apps=OFF -DBUILD_ZLIB=OFF \
		-DWITH_FFMPEG=OFF -DOPENCV_FFMPEG_SKIP_BUILD_CHECK=OFF -DBUILD_opencv_objdetect=ON \
		-DBUILD_opencv_calib3d=ON -DBUILD_opencv_dnn=ON -DBUILD_opencv_features2d=ON \
		-DBUILD_opencv_flann=ON -DBUILD_opencv_gapi=OFF -DBUILD_opencv_ml=OFF \
		-DWITH_GSTREAMER=OFF -DWITH_JAVA=OFF -DOPENCV_ENABLE_FREE=ON \
		-DWITH_JPEG=ON \
		-DBUILD_opencv_stitching=OFF -DBUILD_opencv_python2=OFF -DBUILD_opencv_python3=OFF \
		-DWITH_FREETYPE=ON -DOPENCV_EXTRA_MODULES_PATH=$(THIRD_PARTY_DIR)/opencv_contrib-4.x/modules \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
		-DINSTALL_DIR=${INSTALL_DIR} ${BUILD_DIR}/$@ $(THIRD_PARTY_DIR)/$@ && \
	make -j${THREAD_NUM} && make install && cd -
	touch ${BUILD_DIR}/$@/.build_ok
