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
CMAKE_SOURCE_DIR = /home/youliang/code/vx-mpp

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/youliang/code/vx-mpp/build/linux/aarch64

# Include any dependencies generated for this target.
include app/src/components/av1/CMakeFiles/vx_av1.dir/depend.make

# Include the progress variables for this target.
include app/src/components/av1/CMakeFiles/vx_av1.dir/progress.make

# Include the compile flags for this target's objects.
include app/src/components/av1/CMakeFiles/vx_av1.dir/flags.make

app/src/components/av1/CMakeFiles/vx_av1.dir/library_entry_point.c.o: app/src/components/av1/CMakeFiles/vx_av1.dir/flags.make
app/src/components/av1/CMakeFiles/vx_av1.dir/library_entry_point.c.o: ../../../app/src/components/av1/library_entry_point.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/youliang/code/vx-mpp/build/linux/aarch64/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object app/src/components/av1/CMakeFiles/vx_av1.dir/library_entry_point.c.o"
	cd /home/youliang/code/vx-mpp/build/linux/aarch64/app/src/components/av1 && /home/youliang/code/sdk/aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/vx_av1.dir/library_entry_point.c.o   -c /home/youliang/code/vx-mpp/app/src/components/av1/library_entry_point.c

app/src/components/av1/CMakeFiles/vx_av1.dir/library_entry_point.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/vx_av1.dir/library_entry_point.c.i"
	cd /home/youliang/code/vx-mpp/build/linux/aarch64/app/src/components/av1 && /home/youliang/code/sdk/aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/youliang/code/vx-mpp/app/src/components/av1/library_entry_point.c > CMakeFiles/vx_av1.dir/library_entry_point.c.i

app/src/components/av1/CMakeFiles/vx_av1.dir/library_entry_point.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/vx_av1.dir/library_entry_point.c.s"
	cd /home/youliang/code/vx-mpp/build/linux/aarch64/app/src/components/av1 && /home/youliang/code/sdk/aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/youliang/code/vx-mpp/app/src/components/av1/library_entry_point.c -o CMakeFiles/vx_av1.dir/library_entry_point.c.s

# Object files for target vx_av1
vx_av1_OBJECTS = \
"CMakeFiles/vx_av1.dir/library_entry_point.c.o"

# External object files for target vx_av1
vx_av1_EXTERNAL_OBJECTS =

bin/libvx_av1.so: app/src/components/av1/CMakeFiles/vx_av1.dir/library_entry_point.c.o
bin/libvx_av1.so: app/src/components/av1/CMakeFiles/vx_av1.dir/build.make
bin/libvx_av1.so: app/src/components/av1/CMakeFiles/vx_av1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/youliang/code/vx-mpp/build/linux/aarch64/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library ../../../../bin/libvx_av1.so"
	cd /home/youliang/code/vx-mpp/build/linux/aarch64/app/src/components/av1 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/vx_av1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
app/src/components/av1/CMakeFiles/vx_av1.dir/build: bin/libvx_av1.so

.PHONY : app/src/components/av1/CMakeFiles/vx_av1.dir/build

app/src/components/av1/CMakeFiles/vx_av1.dir/clean:
	cd /home/youliang/code/vx-mpp/build/linux/aarch64/app/src/components/av1 && $(CMAKE_COMMAND) -P CMakeFiles/vx_av1.dir/cmake_clean.cmake
.PHONY : app/src/components/av1/CMakeFiles/vx_av1.dir/clean

app/src/components/av1/CMakeFiles/vx_av1.dir/depend:
	cd /home/youliang/code/vx-mpp/build/linux/aarch64 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/youliang/code/vx-mpp /home/youliang/code/vx-mpp/app/src/components/av1 /home/youliang/code/vx-mpp/build/linux/aarch64 /home/youliang/code/vx-mpp/build/linux/aarch64/app/src/components/av1 /home/youliang/code/vx-mpp/build/linux/aarch64/app/src/components/av1/CMakeFiles/vx_av1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : app/src/components/av1/CMakeFiles/vx_av1.dir/depend

