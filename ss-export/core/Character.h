#pragma once

#include <string>

#ifndef SS_COLOR
#define SS_COLOR
struct Color
{
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;
};
#endif // SS_COLOR

struct Character
{
	std::string name;
	std::string notes;
	Color color = { 255, 0, 0, 255 };
};