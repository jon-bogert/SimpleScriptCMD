#pragma once

#include "Settings.h"

#include <SFML/Graphics.hpp>

uint32_t WindowMeasure(uint32_t windowWidth)
{
	sf::Font font;
	font.loadFromFile(Settings::Get().fontPath);

	sf::Text text;
	text.setFont(font);
	text.setString("_________________________________________________________________");
	uint32_t size = 1;
	text.setCharacterSize(size);
	while (text.getLocalBounds().width + text.getLocalBounds().left < windowWidth)
	{
		text.setCharacterSize(++size);
	}

	return size - 1;
}