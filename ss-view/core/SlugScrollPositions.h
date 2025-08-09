#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

class SlugScrollPositions
{
public:
	void SetLineColor(const sf::Color& color) { m_color = color; }
	void SetLineSize(const sf::Vector2f size) { m_lineSize = size; }
	void SetWindowSize(const sf::Vector2f& windowSize) { m_windowSize = windowSize; }

	void Calculate(const std::vector<float>& scrollPositions)
	{
		m_visuals.resize(scrollPositions.size());
		m_scrollPositions = scrollPositions;

		for (size_t i = 0; i < m_visuals.size(); ++i)
		{
			sf::RectangleShape& rect = m_visuals[i];
			rect.setSize(m_lineSize);
			rect.setFillColor(m_color);
			rect.setPosition(sf::Vector2f(
				m_windowSize.x - m_lineSize.x,
				scrollPositions[i] * m_windowSize.y
			));
		}
	}

	void DrawTo(sf::RenderTarget& target)
	{
		for (const sf::RectangleShape& rect : m_visuals)
		{
			target.draw(rect);
		}
	}

private:
	std::vector<float> m_scrollPositions;
	std::vector<sf::RectangleShape> m_visuals;
	sf::Vector2f m_windowSize;
	sf::Vector2f m_lineSize;
	sf::Color m_color;
};