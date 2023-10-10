export CXX := g++
export CXXFLAGS := -Wall -Wextra -std=c++17

RELEASE_FLAGS := -DNDEBUG -O3 -march=native -flto -fno-exceptions -fno-rtti
DEBUG_FLAGS := -g -DDEBUG -fno-omit-frame-pointer

# Define the bin directory
BIN_PATH = bin

# Define the top-level directory where the Makefile is located
TOP_DIR := $(shell basename $(CURDIR))

# Find all subdirectories containing a "Makefile" file
SUBDIRS := $(shell find . -type f -name Makefile ! -path "./Makefile" -exec dirname {} \;)

# Filter out subdirectories that don't contain a Makefile
PROJECT_DIRS := $(filter-out $(TOP_DIR), $(SUBDIRS))

# Set the default target to build all projects
.DEFAULT_GOAL := all

# Define targets for each project direc	tory
define PROJECT_TARGET
.PHONY: $(1)
$(1):
	@$(MAKE) -C $(1)
	@mkdir -p $(BIN_PATH)
	@cp $(1)/build/bin/* bin
endef

# Create targets for each project directory
$(foreach dir,$(PROJECT_DIRS),$(eval $(call PROJECT_TARGET,$(dir))))

# Define the 'all' target to build all projects
all: export CXXFLAGS := $(CXXFLAGS) $(RELEASE_FLAGS)
all: $(PROJECT_DIRS)

.PHONY: debug
debug: export CXXFLAGS := $(CXXFLAGS) $(DEBUG_FLAGS)
debug: $(PROJECT_DIRS)

.PHONY: install
install: 
	@for dir in $(PROJECT_DIRS); do \
		$(MAKE) -C $$dir install; \
	done

# Define a clean target to clean all projects
.PHONY: clean
clean:
	@for dir in $(PROJECT_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	@rm -rf $(BIN_PATH)

.PHONY: qemu
qemu: all
	@qemu-system-x86_64 -bios uefi/ovmf-x64/OVMF-pure-efi.fd -net none -drive file=disk.img,format=raw

# Other common targets can be added here, e.g., 'install', 'test', etc.

# Disable built-in rules and variables to avoid unexpected behavior
.SUFFIXES:
MAKEFLAGS += --no-builtin-rules
