# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/latkins/Desktop/srcs

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/latkins/Desktop/srcs/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/srcs.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/srcs.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/srcs.dir/flags.make

CMakeFiles/srcs.dir/Client.cpp.o: CMakeFiles/srcs.dir/flags.make
CMakeFiles/srcs.dir/Client.cpp.o: ../Client.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/latkins/Desktop/srcs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/srcs.dir/Client.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/srcs.dir/Client.cpp.o -c /Users/latkins/Desktop/srcs/Client.cpp

CMakeFiles/srcs.dir/Client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/srcs.dir/Client.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/latkins/Desktop/srcs/Client.cpp > CMakeFiles/srcs.dir/Client.cpp.i

CMakeFiles/srcs.dir/Client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/srcs.dir/Client.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/latkins/Desktop/srcs/Client.cpp -o CMakeFiles/srcs.dir/Client.cpp.s

CMakeFiles/srcs.dir/ListeningSocket.cpp.o: CMakeFiles/srcs.dir/flags.make
CMakeFiles/srcs.dir/ListeningSocket.cpp.o: ../ListeningSocket.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/latkins/Desktop/srcs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/srcs.dir/ListeningSocket.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/srcs.dir/ListeningSocket.cpp.o -c /Users/latkins/Desktop/srcs/ListeningSocket.cpp

CMakeFiles/srcs.dir/ListeningSocket.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/srcs.dir/ListeningSocket.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/latkins/Desktop/srcs/ListeningSocket.cpp > CMakeFiles/srcs.dir/ListeningSocket.cpp.i

CMakeFiles/srcs.dir/ListeningSocket.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/srcs.dir/ListeningSocket.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/latkins/Desktop/srcs/ListeningSocket.cpp -o CMakeFiles/srcs.dir/ListeningSocket.cpp.s

CMakeFiles/srcs.dir/main.cpp.o: CMakeFiles/srcs.dir/flags.make
CMakeFiles/srcs.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/latkins/Desktop/srcs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/srcs.dir/main.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/srcs.dir/main.cpp.o -c /Users/latkins/Desktop/srcs/main.cpp

CMakeFiles/srcs.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/srcs.dir/main.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/latkins/Desktop/srcs/main.cpp > CMakeFiles/srcs.dir/main.cpp.i

CMakeFiles/srcs.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/srcs.dir/main.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/latkins/Desktop/srcs/main.cpp -o CMakeFiles/srcs.dir/main.cpp.s

CMakeFiles/srcs.dir/Parser.cpp.o: CMakeFiles/srcs.dir/flags.make
CMakeFiles/srcs.dir/Parser.cpp.o: ../Parser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/latkins/Desktop/srcs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/srcs.dir/Parser.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/srcs.dir/Parser.cpp.o -c /Users/latkins/Desktop/srcs/Parser.cpp

CMakeFiles/srcs.dir/Parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/srcs.dir/Parser.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/latkins/Desktop/srcs/Parser.cpp > CMakeFiles/srcs.dir/Parser.cpp.i

CMakeFiles/srcs.dir/Parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/srcs.dir/Parser.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/latkins/Desktop/srcs/Parser.cpp -o CMakeFiles/srcs.dir/Parser.cpp.s

CMakeFiles/srcs.dir/Socket.cpp.o: CMakeFiles/srcs.dir/flags.make
CMakeFiles/srcs.dir/Socket.cpp.o: ../Socket.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/latkins/Desktop/srcs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/srcs.dir/Socket.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/srcs.dir/Socket.cpp.o -c /Users/latkins/Desktop/srcs/Socket.cpp

CMakeFiles/srcs.dir/Socket.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/srcs.dir/Socket.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/latkins/Desktop/srcs/Socket.cpp > CMakeFiles/srcs.dir/Socket.cpp.i

CMakeFiles/srcs.dir/Socket.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/srcs.dir/Socket.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/latkins/Desktop/srcs/Socket.cpp -o CMakeFiles/srcs.dir/Socket.cpp.s

# Object files for target srcs
srcs_OBJECTS = \
"CMakeFiles/srcs.dir/Client.cpp.o" \
"CMakeFiles/srcs.dir/ListeningSocket.cpp.o" \
"CMakeFiles/srcs.dir/main.cpp.o" \
"CMakeFiles/srcs.dir/Parser.cpp.o" \
"CMakeFiles/srcs.dir/Socket.cpp.o"

# External object files for target srcs
srcs_EXTERNAL_OBJECTS =

srcs: CMakeFiles/srcs.dir/Client.cpp.o
srcs: CMakeFiles/srcs.dir/ListeningSocket.cpp.o
srcs: CMakeFiles/srcs.dir/main.cpp.o
srcs: CMakeFiles/srcs.dir/Parser.cpp.o
srcs: CMakeFiles/srcs.dir/Socket.cpp.o
srcs: CMakeFiles/srcs.dir/build.make
srcs: CMakeFiles/srcs.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/latkins/Desktop/srcs/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable srcs"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/srcs.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/srcs.dir/build: srcs
.PHONY : CMakeFiles/srcs.dir/build

CMakeFiles/srcs.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/srcs.dir/cmake_clean.cmake
.PHONY : CMakeFiles/srcs.dir/clean

CMakeFiles/srcs.dir/depend:
	cd /Users/latkins/Desktop/srcs/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/latkins/Desktop/srcs /Users/latkins/Desktop/srcs /Users/latkins/Desktop/srcs/cmake-build-debug /Users/latkins/Desktop/srcs/cmake-build-debug /Users/latkins/Desktop/srcs/cmake-build-debug/CMakeFiles/srcs.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/srcs.dir/depend
