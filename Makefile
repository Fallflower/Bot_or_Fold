.PHONY: clean all run

CXX = g++

# ---- Platform detection ----
ifeq ($(OS),Windows_NT)
    TARGET_EXT = .exe
    MKDIR = mkdir
    RMDIR = rmdir /s /q
    DEL = del /f /q
else
    TARGET_EXT =
    MKDIR = mkdir -p
    RMDIR = rm -rf
    DEL = rm -f
endif

# CFLAGS = -std=c++17 -g   # for debug
CURL_DIR = C:/Users/xy673/Documents/Cpp_libs/curl-win64-mingw
CFLAGS = -Wall -Wextra -std=c++17 -MMD -MP -Iinclude -I$(CURL_DIR)/include
targets = main$(TARGET_EXT)
BUILD_DIR = build

VPATH = src

sources = $(wildcard *.cpp) $(wildcard src/*.cpp)
objects = $(addprefix $(BUILD_DIR)/, $(addsuffix .o,$(basename $(notdir $(sources)))))
deps = $(objects:.o=.d)

all: $(targets)
	@echo "Build complete."

run: $(targets)
	./$(targets) lhy 9 200 0 1

$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

LDLIBS = -L$(CURL_DIR)/lib -lcurl -lssl -lcrypto -lz -lssh2 -lnghttp2 -lbrotlicommon -lbrotlidec -lzstd -lpsl

$(targets): $(objects)
	$(CXX) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CFLAGS) -c $< -o $@

-include $(deps)

clean:
	$(RMDIR) $(BUILD_DIR)
	$(DEL) $(targets)
