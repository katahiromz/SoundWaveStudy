# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008" .
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles" .
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

# project name and language
project(SoundWaveStudy CXX)

##############################################################################

# wav2txt.exe
add_executable(wav2txt wav2txt.cpp)
target_compile_definitions(wav2txt PRIVATE -DWAV2TXT)

# txt2wav.exe
add_executable(txt2wav txt2wav.cpp)
target_compile_definitions(txt2wav PRIVATE -DTXT2WAV)

##############################################################################
