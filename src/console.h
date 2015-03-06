// Header File
// Created 3/5/2015; 10:55:56 PM

void cprintf_function(const char* func, short category, const char* format, ...);

#define cprintf(_cat, _format, ...) cprintf_function("", _cat, _format, ##__VA_ARGS__)