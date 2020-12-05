#pragma once

#include <string>

#include "../common/defs.h"

size_t extractAndPopulateField (config& cfg, bunkeringData& data, char *source , size_t index, std::string& result);
void populateData (config& cfg, bunkeringData& data, char *docPath);
void generateReport (config& cfg, bunkeringData& data);
