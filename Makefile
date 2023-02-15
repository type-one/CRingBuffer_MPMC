# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_SOURCE_DIR = /mnt/d/SANDBOX/GITHUB/CRingBuffer_MPMC

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/d/SANDBOX/GITHUB/CRingBuffer_MPMC

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /mnt/d/SANDBOX/GITHUB/CRingBuffer_MPMC/CMakeFiles /mnt/d/SANDBOX/GITHUB/CRingBuffer_MPMC/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /mnt/d/SANDBOX/GITHUB/CRingBuffer_MPMC/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named cringbuffer_mpsc

# Build rule for target.
cringbuffer_mpsc: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 cringbuffer_mpsc
.PHONY : cringbuffer_mpsc

# fast build rule for target.
cringbuffer_mpsc/fast:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/build
.PHONY : cringbuffer_mpsc/fast

main.o: main.c.o

.PHONY : main.o

# target to build an object file
main.c.o:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/main.c.o
.PHONY : main.c.o

main.i: main.c.i

.PHONY : main.i

# target to preprocess a source file
main.c.i:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/main.c.i
.PHONY : main.c.i

main.s: main.c.s

.PHONY : main.s

# target to generate assembly for a file
main.c.s:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/main.c.s
.PHONY : main.c.s

tools/ring_buffer_mpmc.o: tools/ring_buffer_mpmc.c.o

.PHONY : tools/ring_buffer_mpmc.o

# target to build an object file
tools/ring_buffer_mpmc.c.o:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/tools/ring_buffer_mpmc.c.o
.PHONY : tools/ring_buffer_mpmc.c.o

tools/ring_buffer_mpmc.i: tools/ring_buffer_mpmc.c.i

.PHONY : tools/ring_buffer_mpmc.i

# target to preprocess a source file
tools/ring_buffer_mpmc.c.i:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/tools/ring_buffer_mpmc.c.i
.PHONY : tools/ring_buffer_mpmc.c.i

tools/ring_buffer_mpmc.s: tools/ring_buffer_mpmc.c.s

.PHONY : tools/ring_buffer_mpmc.s

# target to generate assembly for a file
tools/ring_buffer_mpmc.c.s:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/tools/ring_buffer_mpmc.c.s
.PHONY : tools/ring_buffer_mpmc.c.s

tools/sync_object.o: tools/sync_object.c.o

.PHONY : tools/sync_object.o

# target to build an object file
tools/sync_object.c.o:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/tools/sync_object.c.o
.PHONY : tools/sync_object.c.o

tools/sync_object.i: tools/sync_object.c.i

.PHONY : tools/sync_object.i

# target to preprocess a source file
tools/sync_object.c.i:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/tools/sync_object.c.i
.PHONY : tools/sync_object.c.i

tools/sync_object.s: tools/sync_object.c.s

.PHONY : tools/sync_object.s

# target to generate assembly for a file
tools/sync_object.c.s:
	$(MAKE) -f CMakeFiles/cringbuffer_mpsc.dir/build.make CMakeFiles/cringbuffer_mpsc.dir/tools/sync_object.c.s
.PHONY : tools/sync_object.c.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... rebuild_cache"
	@echo "... edit_cache"
	@echo "... cringbuffer_mpsc"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
	@echo "... tools/ring_buffer_mpmc.o"
	@echo "... tools/ring_buffer_mpmc.i"
	@echo "... tools/ring_buffer_mpmc.s"
	@echo "... tools/sync_object.o"
	@echo "... tools/sync_object.i"
	@echo "... tools/sync_object.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

