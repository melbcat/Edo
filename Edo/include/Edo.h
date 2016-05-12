// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the EDO_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// EDO_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef EDO_EXPORTS
#define EDO_API __declspec(dllexport)
#else
#define EDO_API __declspec(dllimport)
#endif

#ifndef _EDO_H
#define _EDO_H

#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <chrono>

#endif

