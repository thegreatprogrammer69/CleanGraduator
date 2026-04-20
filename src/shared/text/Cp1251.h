#ifndef CLEANGRADUATOR_SHARED_TEXT_CP1251_H
#define CLEANGRADUATOR_SHARED_TEXT_CP1251_H

#include <string>
#include <string_view>

namespace shared::text {

std::string cp1251ToUtf8(std::string_view cp1251);
std::string utf8ToCp1251(std::string_view utf8);

} // namespace shared::text

#endif // CLEANGRADUATOR_SHARED_TEXT_CP1251_H
