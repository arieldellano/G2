CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -std=c++17
LDFLAGS :=

# path #
SRC_PATH = src
BUILD_PATH = build
BIN_PATH = $(BUILD_PATH)/bin

# executable # 
TARGET = $(shell basename $(CURDIR))

# code lists #
# Find all source files in the source directory, sorted by
# most recently modified
SOURCES = $(shell find $(SRC_PATH) -name '*.cpp' | sort -k 1nr | cut -f2-)
# Set the object file names, with the source directory stripped
# from the path, and the build path prepended in its place
OBJECTS = $(SOURCES:$(SRC_PATH)/%.cpp=$(BUILD_PATH)/%.o)
# Set the dependency files that will be used to add header dependencies
DEPS = $(OBJECTS:.o=.d)

# flags #
INCLUDES = -I include/ -I /usr/local/include -I ../../include
# Space-separated pkg-config libraries used by this project
LIBS =

.PHONY: default_target
default_target: release

.PHONY: release
release: export CXXFLAGS := $(CXXFLAGS)
release: dirs
	@$(MAKE) all

.PHONY: dirs
dirs:
	@mkdir -p $(dir $(OBJECTS))
	@mkdir -p $(BIN_PATH)

.PHONY: install
install:
	@echo "TODO: Installing $(BIN_PATH)/$(TARGET)..."

.PHONY: clean
clean:
	@$(RM) $(TARGET)
	@$(RM) -r $(BUILD_PATH)
	@$(RM) -r $(BIN_PATH)

# checks the executable and symlinks to the output
.PHONY: all
all: $(BIN_PATH)/$(TARGET)

# Creation of the executable
$(BIN_PATH)/$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ ${LIBS}

# Add dependency files, if they exist
-include $(DEPS)

# Source file rules
$(BUILD_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MP -MMD -c $< -o $@