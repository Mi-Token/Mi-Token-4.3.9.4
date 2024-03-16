#pragma once

#include "common.h"

bool WriteBypassSettings(BYTE* data, int length, const char** errorMessage, bool* rollback);