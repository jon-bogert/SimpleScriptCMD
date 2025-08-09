#pragma once

#include "Settings.h"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

class Toolbar
{
public:
	Toolbar()
	{
		m_closedVisual.setPointCount(3);
	}

	void SetIconSize(float size)
	{
		m_size = size;
		m_closedVisual.setPoint(0, { 0, 0 });
		m_closedVisual.setPoint(1, { size, 0 });
		m_closedVisual.setPoint(2, { 0, size });
	}

	void SetMenuSize(const sf::Vector2u& windowSize, float menuWidth)
	{
		m_renderTexture.create(menuWidth, windowSize.y);
		m_renderSprite.setTexture(m_renderTexture.getTexture(), true);
		m_background.setSize((sf::Vector2f)windowSize);
	}
	
	void SetBackgroundColor(const sf::Color& color)
	{
		m_background.setFillColor(color);
	}

	void SetClearColor(const sf::Color& color)
	{
		m_clearColor = color;
	}

	void SetIconColor(const sf::Color& color)
	{
		m_closedVisual.setFillColor(color);
	}

	void SetScaleFactor(float scaleFactor)
	{
		m_scaleFactor = scaleFactor;
	}

	bool CheckPoint(const sf::Vector2f& point)
	{
		if (m_isOpen)
		{
			return CheckPointOpen(point);
		}

		return CheckPointClosed(point);
	}

	void SetIsOpen(const bool isOpen)
	{
		m_isOpen = isOpen;
	}

	void SetTextProperties(const sf::Font& font)
	{
		m_font = &font;
	}

	void SetHighlightColor(const sf::Color& color)
	{
		m_highlight.setFillColor(color);
		m_highlight.setSize({ (float)m_renderTexture.getSize().x, (float)m_textSize * 1.5f });
	}

	void AddMenuItem(const std::string& name)
	{
		if (m_font == nullptr)
		{
			std::cout << "Toolbar -- Font was not set" << std::endl;
			return;
		}

		sf::Text& item = m_menuItems.emplace_back();
		item.setFillColor(sf::Color::White);
		item.setFont(*m_font);
		item.setCharacterSize(m_textSize);

		for (size_t i = 1; i <= name.length(); ++i)
		{
			std::string substr = name.substr(0, i);
			item.setString(name.substr(0, i));
			if (item.getLocalBounds().width + item.getLocalBounds().left > (m_renderTexture.getSize().x - m_textSize))
			{
				size_t length = (size_t)std::max((int)i - 6, 1);
				item.setString(name.substr(0, length) + "...");
				break;
			}
		}
	}

	void ClearMenuItems()
	{
		m_menuItems.clear();
	}

	void Format()
	{
		m_scrollOffset = 0;
		float cursor = m_scrollOffset;
		for (sf::Text& item : m_menuItems)
		{
			item.setPosition({ (float)m_textSize, cursor });
			cursor += item.getLocalBounds().height + item.getLocalBounds().top + m_textSize * 0.5f;
		}
		m_scrollMax = cursor;
	}

	void OnScroll(float delta, float& out_t)
	{
		if (GetContentSize() <= m_renderTexture.getSize().y)
			return;

		m_scrollOffset -= delta;
		m_scrollOffset = std::max(m_scrollOffset, 0.f);
		m_scrollOffset = std::min(m_scrollOffset, m_scrollMax - m_renderTexture.getSize().y + m_textSize);

		float cursor = -m_scrollOffset;
		for (sf::Text& item : m_menuItems)
		{
			item.setPosition({ (float)m_textSize, cursor });
			cursor += item.getLocalBounds().height + item.getLocalBounds().top + m_textSize * 0.5f;
		}

		out_t = m_scrollOffset / (m_scrollMax - m_renderTexture.getSize().y + m_textSize);
	}

	void SetScroll(float scrollFactor)
	{
		m_scrollOffset = scrollFactor * (m_scrollMax - m_renderTexture.getSize().y + m_textSize);

		float cursor = -m_scrollOffset;
		for (sf::Text& item : m_menuItems)
		{
			item.setPosition({ (float)m_textSize, cursor });
			cursor += item.getLocalBounds().height + item.getLocalBounds().top + m_textSize * 0.5f;
		}
	}

	bool IsOpen() const { return m_isOpen; }
	int GetHighlightIndex() const { return m_highlightIndex; }

	float GetContentSize() const
	{
		return m_scrollMax + m_textSize;
	}

	void SetIndexToBold(size_t index)
	{
		for (size_t i = 0; i < m_menuItems.size(); ++i)
		{
			if (i == index)
			{
				m_menuItems[i].setFillColor(sf::Color::White);
				m_menuItems[i].setOutlineColor(sf::Color::White);
				m_menuItems[i].setOutlineThickness(0.5f);
				continue;
			}
			m_menuItems[i].setFillColor({200, 200, 200});
			m_menuItems[i].setOutlineThickness(0.f);
		}
	}

	void DrawTo(sf::RenderTarget& target)
	{
		if (m_isOpen)
		{
			target.draw(m_background);
			m_renderTexture.clear(m_clearColor);

			if (m_highlightIndex >= 0)
			{
				m_highlight.setPosition({ 0.f,
					m_menuItems[m_highlightIndex].getGlobalBounds().top
					- m_textSize * 0.5f
				});
				m_renderTexture.draw(m_highlight);
			}

			for (sf::Text& item : m_menuItems)
			{
				m_renderTexture.draw(item);
			}

			m_renderTexture.display();
			target.draw(m_renderSprite);

			return;
		}
		target.draw(m_closedVisual);
	}

private:

	bool CheckPointClosed(const sf::Vector2f& point)
	{
		if (point.x >= 0 && point.y >= 0
			&& point.x < m_size && point.y < m_size)
		{
			m_closedVisual.setPoint(0, { 0, 0 });
			m_closedVisual.setPoint(1, { m_size * m_scaleFactor, 0 });
			m_closedVisual.setPoint(2, { 0, m_size * m_scaleFactor });
			return true;
		}

		m_closedVisual.setPoint(0, { 0, 0 });
		m_closedVisual.setPoint(1, { m_size, 0 });
		m_closedVisual.setPoint(2, { 0, m_size });
		return false;
	}

	bool CheckPointOpen(const sf::Vector2f& point)
	{
		if (point.x >= 0 && point.y >= 0
			&& point.x < m_renderTexture.getSize().x && point.y < m_renderTexture.getSize().y)
		{
			for (size_t i = 0; i < m_menuItems.size(); ++i)
			{
				if (CheckPointMenuItem(point, m_menuItems[i]))
				{
					m_highlightIndex = (int)i;
					return true;
				}
			}
			m_highlightIndex = -1;
			return true;
		}

		m_highlightIndex = -1;
		return false;
	}

	bool CheckPointMenuItem(const sf::Vector2f& point, const sf::Text& item)
	{
		return item.getGlobalBounds().contains(point);
	}

	bool m_isOpen = false;
	float m_size = 0.f;
	float m_scaleFactor = 1.5f;
	float m_scrollOffset = 0.f;
	int m_highlightIndex = -1; // -1 == null
	uint32_t m_textSize = 20;
	float m_scrollMax = 0.f;

	const sf::Font* m_font;
	sf::Color m_clearColor = sf::Color::Black;
	sf::ConvexShape m_closedVisual;
	sf::RenderTexture m_renderTexture;
	sf::Sprite m_renderSprite;
	sf::RectangleShape m_background;
	sf::RectangleShape m_highlight;

	std::vector<sf::Text> m_menuItems;

};