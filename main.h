#ifndef MAIN_H
#define MAIN_H

#include <string>

constexpr double STOPWATCH_MULTIPLIER = 0.182;
extern bool ErrorGraphicsLoaded;

void quit();

double stopwatch();
void stopwatch_reset();
void delay(int milliseconds);

void internal_error(const char* text1, const char* text2 = nullptr, const char* text3 = nullptr);
void external_error(const char* text1, const char* text2 = nullptr, const char* text3 = nullptr);
void internal_error(const std::string& message);
void external_error(const std::string& message);

int random_range(int maximum);

#endif
