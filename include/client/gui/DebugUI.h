#pragma once

void DebugUI_Init();
void DebugUI_Deinit();

void DebugUI_TextAt(u8 line, const char* text, ...);
void DebugUI_Text(const char* text, ...);

void DebugUI_Log(const char* text, ...);

void DebugUI_Draw();
