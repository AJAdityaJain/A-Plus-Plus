cmake_minimum_required (VERSION 3.8)

project (
        appx
        VERSION 0.1.9
        DESCRIPTION "APlusPlus Programming Language Compiler"
        HOMEPAGE_URL "https://github.com/AJAdityaJain/A-Plus-Plus"
)

# Add source to this project's executable.
add_executable (appx
        src/APlusPlus.cpp
        src/Lexer.h
        src/Lexer.cpp
        src/Parser.h
        src/Parser.cpp
        src/Enums.h
        src/Compiler.cpp
        src/Compiler.h
        src/AError.h
        src/AError.cpp
        src/WFile.h
        src/WFile.cpp
        src/DebugLogger.h
        src/FATMaker.h
        src/FATMaker.cpp
        src/Linker.h
        src/Linker.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET appx PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
