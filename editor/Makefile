PROJECT_NAME=editor

# Compiler and flags
CC = g++
FLAGS = -Wall -Wextra -std=c++23
FLAGS_DEBUG = -g
FLAGS_RELEASE = -O3

# Directories
SRC_DIR = src
BIN_DIR = bin
DEBUG_DIR = bin/debug
RELEASE_DIR = bin/release
OBJ_DIR = .obj

# Target executable name
TARGET_DEBUG = $(DEBUG_DIR)/${PROJECT_NAME}
TARGET_RELEASE = $(RELEASE_DIR)/${PROJECT_NAME}

# Find all .cpp files in the src directory
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# Convert .cpp source files to .o object files
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

# Default target
all: debug

debug: FLAGS += $(FLAGS_DEBUG)
debug: $(TARGET_DEBUG)

release: FLAGS += $(FLAGS_RELEASE)
release: $(TARGET_RELEASE)

$(TARGET_DEBUG): $(OBJECTS)
	@mkdir -p $(DEBUG_DIR)
	$(CC) $(FLAGS) -o $@ $(OBJECTS)

$(TARGET_RELEASE): $(OBJECTS)
	@mkdir -p $(RELEASE_DIR)
	$(CC) $(FLAGS) -o $@ $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

run: debug
	@${TARGET_DEBUG}

gdb: debug
	@gdb ${TARGET_DEBUG}

run-release:
	@${TARGET_RELEASE}

.PHONY: all clean release debug

