include $(PWD)/scripts/make/env.mk

THIRD_PARTY_TARGETS := \
	opencv \
	mpp \
	rga \
	ffmpeg-rockchip \
	spdlog \
	rknpu2 \
	json \
	jpeg_turbo \
	stb_image \
	fftw

.PHONY: all
all: info configure build

.PHONY: info
info:
	@echo "项目名: $(PROJECT_NAME)"
	@echo "构建目录: $(BUILD_DIR)"
	@echo "安装目录: $(INSTALL_DIR)"
	@echo "Python 目录: $(THIRD_PARTY_PYTHON_DIR)"

.PHONY: configure
configure:
	@echo "==> Configuring project $(PROJECT_NAME)..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake $(CMAKE_FLAGS) -DCMAKE_PROJECT_NAME=$(PROJECT_NAME) -DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) ..

.PHONY: build
build:
	@echo "==> Building project..."
	@cd $(BUILD_DIR) && $(MAKE) &&  \
		cp $(BUILD_DIR)/$(PROJECT_NAME) $(INSTALL_DIR)/bin/$(PROJECT_NAME) && \
		patchelf --set-rpath $(INSTALL_DIR)/lib/ $(INSTALL_DIR)/bin/$(PROJECT_NAME) && \
		echo "==> Building finish!"
	
.PHONY: install
install:
	@echo "==> Installing to $(INSTALL_DIR)..."
	@cd $(BUILD_DIR) && $(MAKE) install && echo "==> Install finish!"

.PHONY: clean
clean:
	@echo "==> Cleaning build directory..."
	@rm -rf $(BUILD_DIR) $(INSTALL_DIR)

.PHONY: test
test: test-configure test-build test-run

.PHONY: test-configure
test-configure:
	@echo "==> Configuring tests..."
	@cmake \
		-S $(ROOT_DIR) \
		-B $(BUILD_DIR) \
		$(CMAKE_FLAGS) \
		-DCMAKE_PROJECT_NAME=$(PROJECT_NAME) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
		-DBUILD_TESTING=ON


.PHONY: test-build
test-build: test-configure
	@echo "==> Building facenet_test..."
	@cmake --build $(BUILD_DIR) \
		-j$(THREAD_NUM)

.PHONY: test-run
test-run: test-build
	@echo "==> Running facenet tests..."
	@LD_LIBRARY_PATH=$(INSTALL_DIR)/lib:$$LD_LIBRARY_PATH \
		ctest \
			--test-dir $(BUILD_DIR) \
			-V

.PHONY: run
run: build
	@echo "==> Running $(PROJECT_NAME)..."
	export LD_LIBRARY_PATH=$(ROOT_DIR)/target/lib:$LD_LIBRARY_PATH
	@cd $(INSTALL_DIR)/bin && ./$(PROJECT_NAME)

.PHONY: package
package:
	$(setenvs)
	python3 $(SCRIPTS_DIR)/python/package_ota.py

.PHONY: $(THIRD_PARTY_TARGETS)
$(THIRD_PARTY_TARGETS):
	$(setenvs)
	python3 $(THIRD_PARTY_PYTHON_DIR)/$@/run.py
