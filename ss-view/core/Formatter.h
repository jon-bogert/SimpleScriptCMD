#pragma once

#include "Project.h"

#include <SFML/Graphics.hpp>

#include <vector>

class Formatter
{
	struct Paragraph
	{
		sf::Text* text;

		void SetFontSize(float size)
		{
			text->setCharacterSize(size);
		}

		void SetFont(sf::Font& font)
		{
			text->setFont(font);
		}

		void AppendRun(const std::string& str)
		{
			std::string contents = text->getString();
			text->setString(contents + str);
		}

		void SetColor(const sf::Color& color)
		{
			text->setOutlineColor(color);
			text->setOutlineThickness(0.5f);
		}

		void SetBold(bool darkMode)
		{
			text->setOutlineThickness(0.5f);
			text->setOutlineColor((darkMode) ? sf::Color::White : sf::Color::Black);
		}
	};

	struct SlugRegion
	{
		uint32_t slugNumber = 0;
		uint32_t objectIndex = 0;
		sf::Vector2f bounds{};

		SlugRegion(const uint32_t i, const uint32_t s) : objectIndex(i), slugNumber(s) {}
	};

public:
	Formatter();

	void LoadFromSequence(const Sequence& proj, const CharacterCollection& chars, bool darkMode, bool skipOffsetReset = false);
	void DrawTo(sf::RenderWindow& window);

	void OnScroll(float delta, float windowHeight, float& out_t);

	void SetScroll(float t, float windowHeight);

	void TryOpenFile(const sf::Vector2f& point, const Project& proj);

	float GetContentSize() const { return m_scrollMax + m_fontSize; }

	const sf::Font& GetFont() { return m_fontReg; }

	void SetFontSize(uint32_t size) { m_fontSize = size; }
	uint32_t GetFontSize() const { return m_fontSize; }

	std::vector<float> GetSlugScrollPositions() const;

private:
	Paragraph AppendParagraph();

	std::vector<std::string> DialogueLineBreaks(const std::string& line);
	std::vector<std::string> ParentheticalLineBreaks(const std::string& line);
	std::vector<std::string> ActionLineBreaks(const std::string& line);
	std::string SlugFormat(const uint32_t number, const std::string& line);

private:

	sf::Font m_fontReg;

	std::vector<sf::Text> m_blocks;
	std::vector<SlugRegion> m_slugRegions;

	float m_scrollOffset = 0.f;
	float m_scrollMax = 0.f;
	uint32_t m_fontSize = 20;

	const float k_xOffset = 20.f;
	const int k_dialogueLimit = 36;
	const int k_parentheticalLimit = 31;
	const int k_actionLimit = 57;
	const int k_actionTabs = 1;
	const int k_characterTabs = 5;
	const int k_parenthTabs = 4;
	const int k_dialogueTabs = 3;
};