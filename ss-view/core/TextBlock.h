#pragma once
#include <string>

struct TextBlock
{
	enum Type
	{
		Unassigned = -1,
		Slug = 0,
		Action,
		Parenthetical,
		Dialogue,
		Note
	};

	Type type = Type::Unassigned;
	std::string character = "";
	std::string content;
	uint32_t slugCount = 1;

};

