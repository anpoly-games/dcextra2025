#pragma once

using ColoredText = std::pair<std::string, Color>;

void push_rolling_text(eecs::Registry& reg, const char* text, Color col);
