//
// Created by Aditya Jain on 9/3/24.
//

#pragma once
#include "Compiler.h"
#include "WFile.h"

inline int stackKB = 256;
inline auto files = vector<string>();
inline auto msvcrt = "msvcrt.dll";
inline auto symbols = map<string, unsigned int>();;
inline string output;
inline string midput;
const filesystem::path base = exeDirectory();
const filesystem::path TEMPFOLDER = base.parent_path().string().append("\\TEMP\\");
const filesystem::path fasmdir = base.parent_path().string().append("\\FASM\\");

void linkerConfig(const filesystem::path& file);

void pipeline();