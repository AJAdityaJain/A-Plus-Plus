//
// Created by Aditya Jain on 9/3/24.
//

#pragma once
#include "Compiler.h"
#include "WFile.h"

inline int stackKB = 256;
inline auto files = vector<string>();
inline auto symbols = map<string, unsigned int>();;
inline string msvcrt = "msvcrt.dll";
inline string output;
inline string midput;
const filesystem::path base = exeDirectory();
const filesystem::path TEMPFOLDER = base.parent_path().string().append("\\TEMP\\");
const filesystem::path libd = base.parent_path().string().append("\\libd.asm");
const filesystem::path fasmdir = base.parent_path().string().append("\\FASM\\");

void linkerConfig(const filesystem::path& file);

void pipeline();