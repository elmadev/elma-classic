#ifndef TEXT_INPUT_H
#define TEXT_INPUT_H

constexpr int MaxKeycode = 256;

void add_char_to_buffer(char text);
void add_text_to_buffer(const char* text);
void empty_keypress_buffer();

bool has_text_input();
char pop_text_input();

#endif
