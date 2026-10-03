#ifndef PLATFORM_TEXT_INPUT_H
#define PLATFORM_TEXT_INPUT_H

void add_char_to_buffer(char text);
void add_text_to_buffer(const char* text);
void empty_keypress_buffer();

bool has_text_input();
char pop_text_input();

#endif
