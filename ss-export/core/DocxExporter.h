#pragma once

#include "Project.h"

#include <minidocx/minidocx.hpp>

#define DIALOGUE_LIMIT 36
#define PARENTH_LIMIT 31
#define ACTION_LIMIT 57
#define LINE_LIMIT 52

class DocxExporter
{
public:
	void Export(const std::filesystem::path& filePath, Project& proj)
	{
		m_document = new docx::Document();

		m_lineCount = 0;
		m_slugCount = 1;
		m_lastCharacter = "";
		m_wasLastBlockDialogue = false;

		proj.ForEach([&](TextBlock& b, TextBlock* n) { return WriteBlock(b, n); });

		docx::Section sec = m_document->FirstSection();
		sec.SetPageMargin(
			docx::Inch2Twip(1.),
			docx::Inch2Twip(1.),
			docx::Inch2Twip(1.),
			docx::Inch2Twip(1.));

		m_document->Save(filePath.string());

		delete m_document;
	}

private:
	void EmptyLine()
	{
		docx::Paragraph paragraph = m_document->AppendParagraph();
		paragraph.AppendRun("");
		paragraph.SetFont("CourierPrime");
		paragraph.SetFontSize(12.);
		paragraph.SetBeforeSpacing(0);
		paragraph.SetAfterSpacing(0);
		paragraph.SetLineSpacingLines(1.);
	}

	void PageBreak()
	{
		if (m_lineCount != LINE_LIMIT)
			m_document->AppendPageBreak();

		m_lineCount = 0;
	}

	void AddLine(const std::string& content, bool isBold = false)
	{
		docx::Paragraph paragraph = m_document->AppendParagraph();
		paragraph.AppendRun(content);
		paragraph.SetFont("CourierPrime");
		paragraph.SetFontSize(12.);
		paragraph.SetBeforeSpacing(0);
		paragraph.SetAfterSpacing(0);
		paragraph.SetLineSpacingLines(1.);
		if (isBold)
		{
			paragraph.SetFontStyle(docx::Run::Bold);
		}
	}
	
	bool WriteBlock(TextBlock& block, TextBlock* next)
	{
		if (block.type == TextBlock::Type::Note)
			return false;

		if (block.type == TextBlock::Type::Parenthetical ||
			block.type == TextBlock::Type::Dialogue)
		{
			std::vector<std::string> formatted;
			if (block.type == TextBlock::Type::Parenthetical)
			{
				formatted = ParentheticalLineBreaks(block.content);
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
				}
			}
			else // Dialogue
			{
				formatted = DialogueLineBreaks(block.content);
			}

			// Line Count is == LINE_LIMIT, we will be writing on the next page anyways
			//bool lineBreakOverride = false;
			//if (m_lineCount + formatted.size() > LINE_LIMIT)
			//{
			//	lineBreakOverride = true;
			//	EmptyLine();
			//}
			bool lineBreakOverride = (m_lineCount + formatted.size() > LINE_LIMIT);

			if (lineBreakOverride || !m_wasLastBlockDialogue || block.character != m_lastCharacter)
			{
				if (m_lineCount + formatted.size() + 2 > LINE_LIMIT)
				{
					PageBreak();
				}
				else
				{
					EmptyLine();
					++m_lineCount;
				}

				if (block.character == m_lastCharacter)
					AddLine("\t\t\t\t\t" + block.character + " (CONT'D)");
				else
					AddLine("\t\t\t\t\t" + block.character);

				++m_lineCount;
			}

			for (const std::string& line : formatted)
			{
				AddLine(((block.type == TextBlock::Type::Parenthetical) ? "\t\t\t\t" : "\t\t\t") + line);
			}
			m_lineCount += formatted.size();

			m_lastCharacter = block.character;
			m_wasLastBlockDialogue = true;
			return false;
		}

		m_wasLastBlockDialogue = false;

		if (m_lineCount == LINE_LIMIT)
		{
			PageBreak();
		}
		else if (m_lineCount != 0)
		{
			EmptyLine();
			++m_lineCount;
		}

		if (block.type == TextBlock::Type::Slug)
		{
			if (next == nullptr)
			{
				std::cout << "Warning: Action missing after slug line "
					<< std::to_string(m_slugCount) << " : " << block.content << std::endl;
				if (m_lineCount + 1 > LINE_LIMIT)
				{
					PageBreak();
				}
				return false;
			}

		    //Slug Action (next block)
		    std::vector<std::string> formatted = ActionLineBreaks(next->content);
		    if (m_lineCount + formatted.size() + 2 > LINE_LIMIT)
		    {
		    	PageBreak();
		    }
			AddLine(SlugFormat(m_slugCount++, block.content), true);
			EmptyLine();
		    for (const std::string& line : formatted)
		    {
		    	AddLine("\t" + line);
		    }
		    m_lineCount += formatted.size() + 2;
			return true;
		}

		//Action
		std::vector<std::string> formatted = ActionLineBreaks(block.content);
		if (m_lineCount + formatted.size() > LINE_LIMIT)
		{
			PageBreak();
		}
		for (const std::string& line : formatted)
		{
			AddLine("\t" + line);
		}

		m_lineCount += formatted.size();
		return false;
	}

	std::vector<std::string> DialogueLineBreaks(const std::string& line)
	{
		std::stringstream stream(line);
		std::vector<std::string> result;

		int counter = 0;
		std::string word;
		std::string currLine;

		while (std::getline(stream, word, ' '))
		{
			counter += word.length();
			if (counter + 1 > DIALOGUE_LIMIT)
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

	std::vector<std::string> ParentheticalLineBreaks(const std::string& line)
	{
		std::stringstream stream(line);
		std::vector<std::string> result;

		int counter = 0;
		std::string word;
		std::string currLine;


		while (std::getline(stream, word, ' '))
		{
			counter += word.length();
			if (counter + 1 > PARENTH_LIMIT)
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

	std::vector<std::string> ActionLineBreaks(const std::string& line)
	{
		std::stringstream stream(line);
		std::vector<std::string> result;

		int counter = 0;
		std::string word;
		std::string currLine;

		while (std::getline(stream, word, ' '))
		{
			counter += word.length();
			if (counter + 1 > ACTION_LIMIT)
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

	std::string SlugFormat(const uint32_t number, const std::string& line)
	{
		std::string numstr = std::to_string(number);
		std::string result = numstr;

		result.push_back('\t');
		result.append(line);
		for (int i = 0; i < ACTION_LIMIT - line.length() - numstr.length(); ++i)
		{
			result.push_back(' ');
		}
		result.append(numstr);

		return result;
	}

	docx::Document* m_document = nullptr;

	bool m_wasLastBlockDialogue = false;
	uint32_t m_slugCount = 0;
	std::string m_lastCharacter;

	int m_lineCount;
};