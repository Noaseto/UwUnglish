#pragma once
#include <cstdint>
#include <string>

namespace uwunglish::util {
/**
 * merge high byte and low byte into a full id
 *
 * @param leftId high byte
 * @param rightId low byte
 * @return the concatenated id
 */
uint16_t computeId(uint8_t leftId, uint8_t rightId);

/**
 * Convert an utf8 string into a cp1252 one
 * parameter is left untouched
 *
 * @param utf8String some utf8 encoded string
 * @return a cp1252 encoded string
 */
std::string UTF8ToCP1252(const std::string& utf8String);
}