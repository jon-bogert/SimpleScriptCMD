#pragma once

#include "TextBlock.h"
#include "Character.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

struct Sequence
{
	std::string name;
	std::vector<TextBlock> blocks;
};

class Project
{
public:
	void ForEach(std::function<bool(TextBlock&, TextBlock*)> callback)
	{
		for (Sequence& seq : m_sequences)
		{
			for (size_t i = 0; i < seq.blocks.size(); ++i)
			{
				TextBlock* next = (i < seq.blocks.size() - 1) ? &seq.blocks[i + 1] : nullptr;
				if (callback(seq.blocks[i], next))
					i++;
			}
		}
	}

	void MsgCallback(const std::function<void(const std::string&)> msgCallback) { m_print = msgCallback; }
	void Load(const std::filesystem::path& projDirectory)
	{
		if (!std::filesystem::exists(projDirectory))
		{
			Print("Project Directory does not exist");
			return;
		}

		if (std::filesystem::exists(projDirectory / "_char.txt"))
		{
			LoadCharacters(projDirectory / "_char.txt");
		}
		else
		{
			Print("Note -- '_char.txt' was not found.");
		}

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(projDirectory))
		{
			if (entry.is_regular_file())
				continue;

#ifdef _DEBUG
			if (entry.path().filename() == "int")
				continue;
#endif // _DEBUG

			if (entry.path().filename() == ".git")
				continue;

			if (entry.path().filename() == ".backup")
				continue;

			LoadSequence(entry.path());
		}
	}

	void Save(const std::filesystem::path& projPath)
	{
		NewBackup(projPath);

		SaveCharacters(projPath / "_char.txt");

		size_t fileCounter = 0;

		for (size_t i = 0; i < m_sequences.size(); ++i)
		{
			std::string name = TwoDig(i) + "_" + m_sequences[i].name;
			std::filesystem::create_directories(projPath / name);
			SaveSequence(projPath / name, m_sequences[i], fileCounter);
		}
	}

private:
	void Print(const std::string& msg)
	{
		if (m_print == nullptr)
		{
			std::cout << msg << std::endl;
			return;
		}

		m_print(msg);
	}

	void LoadCharacters(const std::filesystem::path& charPath)
	{
		std::ifstream file(charPath);

		std::string charName = "";
		Color charColor{};

		std::string line;
		while(std::getline(file, line))
		{
			Trim(line);
			if (line.empty())
				continue;

			if (line[0] == '[')
			{
				size_t nameEnd = line.find_first_of(']');
				size_t colBegin = line.find_first_of('{');
				size_t colEnd = line.find_first_of('}');

				Character* c = nullptr;

				if (nameEnd == std::string::npos)
				{
					Print("No Character end point fount for line: " + line);
					if (colBegin == std::string::npos)
					{
						charName = line.substr(1);
					}
					else
					{
						charName = line.substr(colBegin - 1);
					}

					ToCaps(charName);
					Trim(charName);
					c = &m_characters[charName];
					c->name = charName;
				}
				else
				{
					charName = line.substr(1, nameEnd - 1);
					ToCaps(charName);
					Trim(charName);
					c = &m_characters[charName];
					c->name = charName;
				}

				if (colBegin == std::string::npos)
				{
					c->color = { 255, 255, 255, 255 };
				}
				else
				{
					std::stringstream colorStream;
					if (colEnd == std::string::npos)
					{
						Print("No Color end point fount for line: " + line);
						colorStream << line.substr(colBegin + 1);
					}
					else
					{
						colorStream << line.substr(colBegin + 1, colEnd - (colBegin + 1));
					}
					int count = 0;
					std::string colCell;
					while(std::getline(colorStream, colCell, ',') && count < 4)
					{
						Trim(colCell);
						uint8_t* channel = nullptr;
						switch (count)
						{
						case 0:
							channel = &c->color.r;
							break;
						case 1:
							channel = &c->color.g;
							break;
						case 2:
							channel = &c->color.b;
							break;
						case 3:
							channel = &c->color.a;
							break;
						default:
							Print(std::string("count is larger than 4"));
						}
						try
						{
							*channel = (uint8_t)std::stoi(colCell);
						}
						catch (std::exception)
						{
							Print("Could not parse color integer: " + colCell);
							*channel = 255;
						}
						++count;
					}

					while (count < 4)
					{
						uint8_t* channel = nullptr;
						switch (count)
						{
						case 0:
							channel = &c->color.r;
							break;
						case 1:
							channel = &c->color.g;
							break;
						case 2:
							channel = &c->color.b;
							break;
						case 3:
							channel = &c->color.a;
							break;
						}
						*channel = 255;
						++count;
					}
				}
				continue;
			}

			// No special character
			if (charName.empty())
			{
				Print("Fatal Error -- No current character name for line: " + line);
				exit(1);
			}

			if (!m_characters[charName].notes.empty())
			{
				m_characters[charName].notes += "\n" + line;
			}
			else
			{
				m_characters[charName].notes = line;
			}
		}
	}

	void LoadSequence(const std::filesystem::path& sequencePath)
	{
		std::string name = sequencePath.filename().string();
		std::size_t underscoreIndex = name.find_first_of('_');
		if (underscoreIndex != std::string::npos)
		{
			name = name.substr(underscoreIndex + 1);
		}

		Sequence& seq = m_sequences.emplace_back();
		seq.name = name;

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(sequencePath))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".txt")
				continue;

			LoadScene(entry.path(), seq);
		}
	}

	void LoadScene(const std::filesystem::path& scenePath, Sequence& seq)
	{
		std::ifstream file(scenePath);

		if (!file.is_open())
		{
			Print("Could not open file: " + scenePath.string());
			return;
		}

		std::string lastCharacter = "";

		std::string line{};
		while (std::getline(file, line))
		{
			Trim(line);
			if (line.empty())
				continue;

			if (line[0] == '#')
			{
				ToCaps(line);
				TextBlock& block = seq.blocks.emplace_back();
				block.type = TextBlock::Slug;
				block.content = line.substr(1);
				Trim(block.content);
				continue;
			}
			if (line[0] == '[')
			{
				size_t closeIndex = line.find_first_of(']');
				if (closeIndex == std::string::npos)
				{
					Print("Expecting close bracket for character specifier on line: " + line);
					lastCharacter = line.substr(1);
					ToCaps(lastCharacter);
					continue;
				}
				lastCharacter = line.substr(1, closeIndex - 1);
				ToCaps(lastCharacter);
				Trim(lastCharacter);
				continue;
			}
			if (line[0] == '*')
			{
				TextBlock& block = seq.blocks.emplace_back();
				block.type = TextBlock::Action;
				block.content = line.substr(1);
				Trim(block.content);
				continue;
			}
			if (line[0] == '(')
			{
				if (lastCharacter.empty())
				{
					Print("Fatal Error -- No Character assigned for parenthetical: " + line);
					exit(1);
				}

				TextBlock& block = seq.blocks.emplace_back();
				block.type = TextBlock::Parenthetical;
				block.character = lastCharacter;

				size_t endIndex = line.find_last_of(')');
				if (endIndex == std::string::npos)
				{
					Print("Expecting close parethesis for character specifier on line: " + line);
					block.content = line.substr(1);
					Trim(block.content);
					continue;
				}

				block.content = line.substr(1, endIndex - 1);
				Trim(block.content);

				continue;
			}
			if (line.length() >= 2 && line.substr(0, 2) == "//")
			{
				std::string note = line.substr(2);
				Trim(note);
				if (!note.empty())
				{
				    TextBlock& block = seq.blocks.emplace_back();
				    block.type = TextBlock::Note;
					block.content = note;
					Trim(block.content);
				}
				continue;
			}

			if (lastCharacter.empty())
			{
				Print("Fatal Error -- No Character assigned for dialogue: " + line);
				exit(1);
			}

			TextBlock& block = seq.blocks.emplace_back();
			block.type = TextBlock::Dialogue;
			block.character = lastCharacter;
			block.content = line;
		}
	}

	void NewBackup(const std::filesystem::path projPath)
	{
		if (std::filesystem::exists(projPath / ".backup"))
		{
			std::filesystem::remove_all(projPath / ".backup");
		}
		std::filesystem::create_directories(projPath / ".backup");

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(projPath))
		{
			if (entry.path().filename() == ".git")
				continue;

			if (entry.path().filename() == ".gitattributes")
				continue;

			if (entry.path().filename() == ".gitignore")
				continue;

			if (entry.path().filename() == ".backup")
				continue;

			if (entry.path().has_extension() && entry.path().extension() != ".txt")
				continue;

#ifdef _DEBUG
			if (entry.path().filename() == "int")
				continue;
#endif // _DEBUG

			std::filesystem::copy(entry.path(), projPath / ".backup" / entry.path().filename(), std::filesystem::copy_options::recursive);

			std::filesystem::remove_all(entry.path());
		}
	}

	void SaveCharacters(const std::filesystem::path& charPath)
	{
		std::ofstream file(charPath);

		for (const Character& c : m_characters.data)
		{
			file << '[' << c.name << "]{ "
				<< std::to_string((int)c.color.r) << ", "
				<< std::to_string((int)c.color.g) << ", "
				<< std::to_string((int)c.color.b) << ", "
				<< std::to_string((int)c.color.a) << " }" << std::endl;

			if (c.notes.empty())
			{
				file << std::endl;
				continue;
			}

			file << c.notes << std::endl << std::endl;
		}
	}

	void SaveSequence(const std::filesystem::path& sequencePath, const Sequence& seq, size_t& fileCounter)
	{
		std::ofstream file;
		std::string lastCharName = "";
		for (const TextBlock& block : seq.blocks)
		{
			if (block.type == TextBlock::Slug)
			{
				if (file.is_open())
					file.close();

				std::string filename = ThreeDig(fileCounter++) + "_" + NameFromSlug(block.content) + ".txt";

				file.open(sequencePath / filename);

				file << "# " << block.content << std::endl << std::endl;
				continue;
			}

			if (!file.is_open())
			{
				Print("Fatal Error -- Sequence doesn't begin with a slug line");
				exit(1);
			}

			switch (block.type)
			{
			case TextBlock::Action:
				file << "* " << block.content << std::endl << std::endl;
				break;
			case TextBlock::Note:
				file << "// " << block.content << std::endl << std::endl;
				break;
			case TextBlock::Parenthetical:
				if (block.character != lastCharName)
				{
					file << '[' << block.character << ']' << std::endl;
					lastCharName = block.character;
				}
				file << '(' << block.content << ')' << std::endl << std::endl;
				break;
			case TextBlock::Dialogue:
				if (block.character != lastCharName)
				{
					file << '[' << block.character << ']' << std::endl;
					lastCharName = block.character;
				}
				file << block.content << std::endl << std::endl;
				break;
			default:
				Print(std::string("Save Sequence -- `TextBlock` enum not implemented"));
				break;
			}
		}
	}

	void Trim(std::string& str)
	{
		std::unordered_set<char> whitespace = { '\n', '\t', '\r', ' ' };
		while (!str.empty() && whitespace.find(str.front()) != whitespace.end())
		{
			str.erase(0, 1);
		}
		while (!str.empty() && whitespace.find(str.back()) != whitespace.end())
		{
			str.pop_back();
		}
	}

	void ToCaps(std::string& str)
	{
		for (char& c : str)
		{
			if (c >= 'a' && c <= 'z')
			{
				c -= 32;
			}
		}
	}

	std::string TwoDig(size_t val)
	{
		if (val < 10)
			return "0" + std::to_string(val);

		return std::to_string(val);
	}

	std::string ThreeDig(size_t val)
	{
		if (val < 10)
			return "00" + std::to_string(val);
		if (val < 100)
			return "0" + std::to_string(val);

		return std::to_string(val);
	}

	std::string NameFromSlug(const std::string& line)
	{
		std::stringstream result;
		bool lastWasSpecial = false;
		for (const char& c : line)
		{
			if (c >= 'A' && c <= 'Z')
			{
				lastWasSpecial = false;
				result << c;
				continue;
			}
			if (c >= 'a' && c <= 'z')
			{
				lastWasSpecial = false;
				result << (char)(c - 32);
				continue;
			}

			if (lastWasSpecial)
				continue;

			lastWasSpecial = true;
			result << '_';
		}
		return result.str();
	}

private:
	struct CharacterCollection
	{
		std::vector<Character> data;
		Character& operator[](std::string name)
		{
			auto result = std::find_if(data.begin(), data.end(), [&](const Character& c) { return c.name == name; });
			if (result == data.end())
			{
				Character& newEntry = data.emplace_back();
				newEntry.name = name;
				return newEntry;
			}
			return *result;
		}
	};

	std::vector<Sequence> m_sequences;
	CharacterCollection m_characters;
	std::function<void(const std::string&)> m_print = nullptr;
};