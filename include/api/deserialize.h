#ifndef API_DESERIALIZE
#define API_DESERIALIZE

#include <string>
#include <vector>

template <typename T> std::pair<T, std::string> deserialize(const std::vector<unsigned char>& data);

#endif
