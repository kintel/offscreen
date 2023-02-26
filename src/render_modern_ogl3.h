#pragma once

#include <vector>

#include "state.h"

void setupModernOGL3(std::vector<MyState> &state, const std::string &glslVersion);
void renderModernOGL3(const std::vector<MyState>& states);
