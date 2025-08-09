#pragma once

#include <filesystem>
#include <chrono>
#include <unordered_map>

class FileChecker
{
	struct Time
	{
		std::filesystem::file_time_type timepoint;
		bool visited = false;
	};

public:
	// false -> no change
	bool CheckFiles()
	{
		bool result = false;
		m_fileDataNew->clear();

		for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(std::filesystem::current_path()))
		{
			std::filesystem::path filename = entry.path().filename();

			if (filename == ".git")
				continue;
			if (filename == ".backup")
				continue;
			if (filename == ".gitignore")
				continue;
			if (filename == ".gitattributes")
				continue;

			Time& time = m_fileDataNew->operator[](filename);
			time.timepoint = entry.last_write_time();

			if (m_fileDataOld->find(filename) == m_fileDataOld->end())
			{
				result = true;
				continue;
			}

			Time& old = m_fileDataOld->operator[](filename);
			old.visited = true;
			if (old.timepoint != time.timepoint)
			{
				result = true;
			}
		}

		if (!result)
		{
			for (auto& pair : *m_fileDataOld)
			{
				if (!pair.second.visited)
				{
					result = true;
					break;
				}
			}
		}

		std::swap(m_fileDataOld, m_fileDataNew);
		return result;
	}

	FileChecker()
	{
		m_dataA = std::make_unique<std::unordered_map <std::filesystem::path, Time>>();
		m_dataB = std::make_unique<std::unordered_map <std::filesystem::path, Time>>();

		m_fileDataOld = m_dataA.get();
		m_fileDataNew = m_dataB.get();
	}

private:
	std::unordered_map <std::filesystem::path, Time>* m_fileDataOld;
	std::unordered_map <std::filesystem::path, Time>* m_fileDataNew;
	std::unique_ptr<std::unordered_map <std::filesystem::path, Time>> m_dataA;
	std::unique_ptr<std::unordered_map <std::filesystem::path, Time>> m_dataB;

};
