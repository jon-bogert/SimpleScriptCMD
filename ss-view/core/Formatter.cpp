#include "Formatter.h"

#include "Settings.h"

#include <algorithm>

static std::string Tab(uint8_t num)
{
	std::string result;
	for (uint8_t i = 0; i < num; ++i)
	{
		result.append("    ");
	}
	return result;
}

Formatter::Formatter()
{
	m_fontReg.loadFromFile(Settings::Get().fontPath);
}

void Formatter::LoadFromSequence(const Sequence& seq, const CharacterCollection& chars, bool darkMode, bool skipOffsetReset)
{
	m_slugRegions.clear();
	m_blocks.clear();
	if (!skipOffsetReset)
	{
		m_scrollOffset = 0.f;
	}

	std::string lastCharacter = "";
	bool wasLastBlockDialogue = false;

	Paragraph paragraph;

	for (const TextBlock& block : seq.blocks)
	{
		if (block.type == TextBlock::Type::Note)
			continue;

		paragraph = AppendParagraph();

		if (block.type == TextBlock::Type::Parenthetical ||
			block.type == TextBlock::Type::Dialogue)
		{
			if (!wasLastBlockDialogue || block.character != lastCharacter)
			{
				//Empty Line
				paragraph.AppendRun(" ");
				paragraph.SetFont(m_fontReg);
				paragraph.SetFontSize(m_fontSize);

				paragraph = AppendParagraph();

				if (block.character == lastCharacter)
					paragraph.AppendRun(Tab(k_characterTabs) + block.character + " (CONT'D)");
				else
					paragraph.AppendRun(Tab(k_characterTabs) + block.character);

				if (chars.Contains(block.character))
				{
					paragraph.SetColor(chars[block.character].color);
				}
				paragraph.SetFont(m_fontReg);
				paragraph.SetFontSize(m_fontSize);
				paragraph = AppendParagraph();
			}

			if (block.type == TextBlock::Type::Parenthetical)
			{
				std::vector<std::string> formatted = ParentheticalLineBreaks(block.content);
				for (size_t i = 0; i < formatted.size(); ++i)
				{
					if (i > 0)
					{
						formatted[i] = " " + formatted[i];
					}
					else if (i == 0)
					{
						formatted[i] = "(" + formatted[i];
					}
					if (i == formatted.size() - 1)
					{
						formatted[i].push_back(')');
					}
					paragraph.AppendRun(Tab(k_parenthTabs) + formatted[i]);
					paragraph.SetFont(m_fontReg);
					paragraph.SetFontSize(m_fontSize);
					if (i != formatted.size() - 1 )
					{
						paragraph = AppendParagraph();
					}
				}
			}
			else // Dialogue
			{
				std::vector<std::string> formatted = DialogueLineBreaks(block.content);
				for (const std::string& line : formatted)
				{
					paragraph.AppendRun(Tab(k_dialogueTabs) + line);
					paragraph.SetFont(m_fontReg);
					paragraph.SetFontSize(m_fontSize);
					if (line != formatted.back())
					{
						paragraph = AppendParagraph();
					}
				}
			}

			lastCharacter = block.character;
			wasLastBlockDialogue = true;
			continue;
		}

		wasLastBlockDialogue = false;

		//Empty Line
		paragraph.AppendRun(" ");
		paragraph.SetFont(m_fontReg);
		paragraph.SetFontSize(m_fontSize);

		paragraph = AppendParagraph();

		if (block.type == TextBlock::Type::Slug)
		{
			m_slugRegions.push_back(SlugRegion(m_blocks.size() - 1, block.slugCount));
			paragraph.AppendRun(SlugFormat(block.slugCount, block.content));
			paragraph.SetFont(m_fontReg);
			paragraph.SetBold(darkMode);
			paragraph.SetFontSize(m_fontSize);
			continue;
		}

		//Action
		std::vector<std::string> formatted = ActionLineBreaks(block.content);
		for (const std::string& line : formatted)
		{
			paragraph.AppendRun(Tab(k_actionTabs) + line);
			paragraph.SetFont(m_fontReg);
			paragraph.SetFontSize(m_fontSize);
			if (line != formatted.back())
			{
				paragraph = AppendParagraph();
			}
		}
	}

	float cursor = 0.f;
	size_t slugIndex = 0;
	for (size_t i = 0; i < m_blocks.size(); ++i)
	{
		if (slugIndex < m_slugRegions.size() && i == m_slugRegions[slugIndex].objectIndex)
		{
			if (slugIndex != 0)
			{
				m_slugRegions[slugIndex - 1].bounds.y = cursor;
			}
			m_slugRegions[slugIndex++].bounds.x = cursor;
		}
		sf::Text& block = m_blocks[i];
		block.setFillColor((darkMode) ? sf::Color::White : sf::Color::Black);
		block.setPosition({ k_xOffset, cursor - m_scrollOffset });
		cursor += block.getLocalBounds().height + block.getLocalBounds().top;
	}

	if (!m_slugRegions.empty())
	{
		m_slugRegions.back().bounds.y = cursor;
	}
	m_scrollMax = cursor;
}

void Formatter::DrawTo(sf::RenderWindow& window)
{
	for (sf::Text& block : m_blocks)
	{
		window.draw(block);
	}
}

void Formatter::OnScroll(float delta, float windowHeight, float& out_t)
{
	if (GetContentSize() <= windowHeight)
		return;

	m_scrollOffset -= delta;
	m_scrollOffset = std::max(m_scrollOffset, 0.f);
	m_scrollOffset = std::min(m_scrollOffset, m_scrollMax - windowHeight + m_fontSize);

	float cursor = -m_scrollOffset;
	for (sf::Text& block : m_blocks)
	{
		block.setPosition({ k_xOffset, cursor });
		cursor += block.getLocalBounds().height + block.getLocalBounds().top;
	}

	out_t = m_scrollOffset / (m_scrollMax - windowHeight + m_fontSize);
}

void Formatter::SetScroll(float t, float windowHeight)
{
	m_scrollOffset = t * (m_scrollMax - windowHeight + m_fontSize);

	float cursor = -m_scrollOffset;
	for (sf::Text& block : m_blocks)
	{
		block.setPosition({ k_xOffset, cursor });
		cursor += block.getLocalBounds().height + block.getLocalBounds().top;
	}
}

void Formatter::TryOpenFile(const sf::Vector2f& point, const Project& proj)
{
	for (const SlugRegion& region : m_slugRegions)
	{
		if (point.y < region.bounds.x - m_scrollOffset || point.y >= region.bounds.y - m_scrollOffset)
			continue;

		std::string path = proj.FileFromSlug(region.slugNumber).string();
		if (path.find_first_of('\"') == std::string::npos)
			path = "\"" + path + "\"";

		std::string cmd = "start notepad " + path;
		system(cmd.c_str());
	}
}

std::vector<float> Formatter::GetSlugScrollPositions() const
{
	std::vector<float> result(m_slugRegions.size());
	float invContentHeight = 1.f / m_scrollMax;

	for (size_t i = 0; i < result.size(); ++i)
	{
		float slugY = m_slugRegions[i].bounds.x;
		result[i] = slugY * invContentHeight;
	}

	return result;
}

Formatter::Paragraph Formatter::AppendParagraph()
{
	Paragraph p;
	p.text = &m_blocks.emplace_back();
	return p;
}

std::vector<std::string> Formatter::DialogueLineBreaks(const std::string& line)
{
	std::stringstream stream(line);
	std::vector<std::string> result;

	int counter = 0;
	std::string word;
	std::string currLine;

	while (std::getline(stream, word, ' '))
	{
		counter += word.length();
		if (counter + 1 > k_dialogueLimit)
		{
			result.push_back(currLine);
			counter = word.length();
			currLine = word;
			continue;
		}
		if (!currLine.empty())
		{
			currLine.push_back(' ');
			++counter;
		}
		currLine.append(word);
	}
	result.push_back(currLine);

	return result;
}

std::vector<std::string> Formatter::ParentheticalLineBreaks(const std::string& line)
{
	std::stringstream stream(line);
	std::vector<std::string> result;

	int counter = 0;
	std::string word;
	std::string currLine;


	while (std::getline(stream, word, ' '))
	{
		counter += word.length();
		if (counter + 1 > k_parentheticalLimit)
		{
			result.push_back(currLine);
			counter = word.length();
			currLine = word;

			continue;
		}
		if (!currLine.empty())
		{
			currLine.push_back(' ');
			++counter;
		}
		currLine.append(word);
	}
	result.push_back(currLine);

	return result;
}

std::vector<std::string> Formatter::ActionLineBreaks(const std::string& line)
{
	std::stringstream stream(line);
	std::vector<std::string> result;

	int counter = 0;
	std::string word;
	std::string currLine;

	while (std::getline(stream, word, ' '))
	{
		counter += word.length();
		if (counter + 1 > k_actionLimit)
		{
			result.push_back(currLine);
			counter = word.length();
			currLine = word;
			continue;
		}
		if (!currLine.empty())
		{
			currLine.push_back(' ');
			++counter;
		}
		currLine.append(word);
	}
	result.push_back(currLine);

	return result;
}

std::string Formatter::SlugFormat(const uint32_t number, const std::string& line)
{
	std::string numstr = std::to_string(number);
	std::string result = numstr;

	for (size_t i = numstr.length(); i < 4; ++i)
		result.push_back(' ');

	result.append(line);
	for (int i = 0; i < k_actionLimit - line.length() - numstr.length(); ++i)
	{
		result.push_back(' ');
	}
	result.append(numstr);

	return result;
}
