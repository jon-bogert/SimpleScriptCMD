#pragma once

#include <string>

#include <SFML/Graphics.hpp>

struct Character
{
	std::string name;
	std::string notes;
	sf::Color color = sf::Color(255, 0, 0, 255);
};