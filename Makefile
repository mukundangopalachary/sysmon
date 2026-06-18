# SYSMON Makefile
# Quick build + common tasks
# For full build, CMakeLists.txt is the source of truth

.PHONY: all build debug release clean install uninstall run test

# Default target
all: build

# Build using CMake
build:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Release
	@cd build && make -j$(nproc)

debug:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
	@cd build && make -j$(nproc)

release:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
	@cd build && make -j$(nproc)

# Install
install: build
	@cd build && make install

uninstall:
	@cd build && make uninstall 2>/dev/null || true

# Run
run: build
	@./build/sysmon

# Test
test: debug
	@cd build && ctest --output-on-failure

# Clean
clean:
	@rm -rf build

# Format code
format:
	@clang-format -i src/core/*.c src/core/*.h
	@clang-format -i src/tui/**/*.cpp src/tui/**/*.h
	@echo "Code formatted"

# Static analysis
analyze:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_BUILD_TYPE=Debug
	@cd build && scan-build make

# Install plugins to user directory
install-plugins:
	@mkdir -p ~/.local/share/sysmon/plugins
	@cp scripts/plugins/*.sh ~/.local/share/sysmon/plugins/
	@chmod +x ~/.local/share/sysmon/plugins/*.sh
	@echo "Plugins installed to ~/.local/share/sysmon/plugins/"

# Generate tags for IDE
tags:
	@ctags -R --languages=C,C++ src/
	@echo "Tags generated"

# Help
help:
	@echo "SYSMON Build System"
	@echo ""
	@echo "Targets:"
	@echo "  build          - Release build"
	@echo "  debug          - Debug build with sanitizers"
	@echo "  release        - Optimized release build"
	@echo "  run            - Build and run"
	@echo "  test           - Build and run tests"
	@echo "  clean          - Remove build directory"
	@echo "  install        - Install to system"
	@echo "  format         - Format source code"
	@echo "  analyze        - Run static analysis"
	@echo "  install-plugins - Install Bash plugins"
	@echo "  tags           - Generate ctags"
