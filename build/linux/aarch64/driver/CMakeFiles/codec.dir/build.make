# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/youliang/code/media/vx-mpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/youliang/code/media/vx-mpp/build/linux/aarch64

# Utility rule file for codec.

# Include the progress variables for this target.
include driver/CMakeFiles/codec.dir/progress.make

driver/CMakeFiles/codec:
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver && cp -rf /home/youliang/code/media/vx-mpp/driver/* /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver/
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver && echo compiling\ module\ codec.ko...

codec: driver/CMakeFiles/codec
codec: driver/CMakeFiles/codec.dir/build.make
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver && echo obj-m\ :=\ codec.o > /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver/Makefile
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver && echo codec-objs:=dmabuf/mpp_dma_alloc.o\ \ \ \ 						dmabuf/mpp_dmabuf.o\ 						mpp_core.o\ \ 						 >>/home/youliang/code/media/vx-mpp/build/linux/aarch64/driver/Makefile
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver && make -C /home/youliang/code/sdk/kernel M=/home/youliang/code/media/vx-mpp/build/linux/aarch64/driver modules ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu-
.PHONY : codec

# Rule to build all files generated by this target.
driver/CMakeFiles/codec.dir/build: codec

.PHONY : driver/CMakeFiles/codec.dir/build

driver/CMakeFiles/codec.dir/clean:
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver && $(CMAKE_COMMAND) -P CMakeFiles/codec.dir/cmake_clean.cmake
.PHONY : driver/CMakeFiles/codec.dir/clean

driver/CMakeFiles/codec.dir/depend:
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/youliang/code/media/vx-mpp /home/youliang/code/media/vx-mpp/driver /home/youliang/code/media/vx-mpp/build/linux/aarch64 /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver /home/youliang/code/media/vx-mpp/build/linux/aarch64/driver/CMakeFiles/codec.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : driver/CMakeFiles/codec.dir/depend

