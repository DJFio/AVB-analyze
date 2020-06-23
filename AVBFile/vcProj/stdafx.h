#pragma once
// Handle Windows cases - Michael Haephrati
// ----------------------------------------

#ifdef _MSC_VER
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <stdio.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#endif

typedef struct _MatchResult
{
	std::string FileName;
	std::string ProjectName;
	std::string filepackageUID;
	std::string SourcePackageUID;
} MatchResult;
