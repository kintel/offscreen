#pragma once

#include <vector>

#include "state.h"

void setupModernOGL2(std::vector<MyState> &states, const std::string &glslVersion);
void renderModernOGL2(const std::vector<MyState>& state);
