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

# Include any dependencies generated for this target.
include app/test/CMakeFiles/mpp_mem_test.dir/depend.make

# Include the progress variables for this target.
include app/test/CMakeFiles/mpp_mem_test.dir/progress.make

# Include the compile flags for this target's objects.
include app/test/CMakeFiles/mpp_mem_test.dir/flags.make

app/test/CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.o: app/test/CMakeFiles/mpp_mem_test.dir/flags.make
app/test/CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.o: ../../../app/test/mpp_mem_test.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/youliang/code/media/vx-mpp/build/linux/aarch64/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object app/test/CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.o"
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/app/test && /home/youliang/code/sdk/aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.o   -c /home/youliang/code/media/vx-mpp/app/test/mpp_mem_test.c

app/test/CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.i"
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/app/test && /home/youliang/code/sdk/aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/youliang/code/media/vx-mpp/app/test/mpp_mem_test.c > CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.i

app/test/CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.s"
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/app/test && /home/youliang/code/sdk/aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/youliang/code/media/vx-mpp/app/test/mpp_mem_test.c -o CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.s

# Object files for target mpp_mem_test
mpp_mem_test_OBJECTS = \
"CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.o"

# External object files for target mpp_mem_test
mpp_mem_test_EXTERNAL_OBJECTS =

app/test/mpp_mem_test: app/test/CMakeFiles/mpp_mem_test.dir/mpp_mem_test.c.o
app/test/mpp_mem_test: app/test/CMakeFiles/mpp_mem_test.dir/build.make
app/test/mpp_mem_test: app/src/lib/libvx-mpp.a
app/test/mpp_mem_test: app/test/CMakeFiles/mpp_mem_test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/youliang/code/media/vx-mpp/build/linux/aarch64/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable mpp_mem_test"
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/app/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/mpp_mem_test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
app/test/CMakeFiles/mpp_mem_test.dir/build: app/test/mpp_mem_test

.PHONY : app/test/CMakeFiles/mpp_mem_test.dir/build

app/test/CMakeFiles/mpp_mem_test.dir/clean:
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64/app/test && $(CMAKE_COMMAND) -P CMakeFiles/mpp_mem_test.dir/cmake_clean.cmake
.PHONY : app/test/CMakeFiles/mpp_mem_test.dir/clean

app/test/CMakeFiles/mpp_mem_test.dir/depend:
	cd /home/youliang/code/media/vx-mpp/build/linux/aarch64 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/youliang/code/media/vx-mpp /home/youliang/code/media/vx-mpp/app/test /home/youliang/code/media/vx-mpp/build/linux/aarch64 /home/youliang/code/media/vx-mpp/build/linux/aarch64/app/test /home/youliang/code/media/vx-mpp/build/linux/aarch64/app/test/CMakeFiles/mpp_mem_test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : app/test/CMakeFiles/mpp_mem_test.dir/depend

