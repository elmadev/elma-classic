#ifndef KEYS_H
#define KEYS_H

typedef int Keycode;
constexpr int MaxKeycode = 256;

void add_key_to_buffer(Keycode keycode);
void add_text_to_buffer(const char* text);

bool has_text_input();
char pop_text_input();

#endif
