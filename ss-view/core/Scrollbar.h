#pragma once

#include <SFML/Graphics.hpp>

class ScrollBar
{
public:
	void SetSize(sf::Vector2f size)
	{
		m_visual.setSize(size);
	}

	void SetColor(sf::Color color)
	{
		m_visual.setFillColor(color);
	}

	void SetWindowDimensions(sf::Vector2f dims)
	{
		m_windowDimenisons = dims;
	}

	void SetScrollPoint(float t)
	{
		m_t = t;
	}

	void DrawTo(sf::RenderTarget& target)
	{
		if (!m_isVisible)
			return;

		m_visual.setPosition({
			m_windowDimenisons.x - m_visual.getSize().x,
			(m_windowDimenisons.y - m_visual.getSize().y) * m_t
		});

		target.draw(m_visual);
	}

	bool TestOverlap(const sf::Vector2f& point)
	{
		if (!m_isVisible)
			return false;

		float offsetPixels = m_t * (m_windowDimenisons.y - m_visual.getSize().y);
		return point.x >= m_windowDimenisons.x - m_visual.getSize().x
			&& point.y >= offsetPixels
			&& point.x < m_windowDimenisons.x
			&& point.y < offsetPixels + m_visual.getSize().y;
	}

	void SetIsDragging(const bool isDragging) { m_isDragging = isDragging; }
	bool GetIsDragging() const { return m_isDragging; }

	float GetScrollFactor() const { return m_t; }

	void SetIsVisible(const bool isVisible) { m_isVisible = isVisible; }
	bool GetIsVisible() const { return m_isVisible; }

	void DoScroll(sf::Vector2f delta)
	{
		// Not clicked on scroll bar
		if (!m_isVisible || !m_isDragging)
			return;

		// Test Deficites
		if (delta.y < 0 && m_scrollDeficite > 0)
		{
			float newDef = m_scrollDeficite + delta.y;
			if (newDef >= 0)
			{
				delta.y = newDef;
				m_scrollDeficite = 0;
			}
			else
			{
				m_scrollDeficite = newDef;
			}
		}

		else if (delta.y > 0 && m_scrollDeficite < 0)
		{
			float newDef = m_scrollDeficite + delta.y;
			if (newDef <= 0)
			{
				delta.y = newDef;
				m_scrollDeficite = 0;
			}
			else
			{
				m_scrollDeficite = newDef;
			}
		}

		float prevY = m_visual.getPosition().y;

		float y = m_visual.getPosition().y + delta.y;
		if (y > m_windowDimenisons.y - m_visual.getSize().y)
		{
			float newY = m_windowDimenisons.y - m_visual.getSize().y;
			m_scrollDeficite += y - newY;
			y = newY;
		}
		if (y < 0)
		{
			m_scrollDeficite += y;
			y = 0;
		}

		m_visual.setPosition({ m_visual.getPosition().x,  y });

		float scrollDelta = ((y - prevY) / (m_windowDimenisons.y - m_visual.getSize().y));
		m_t += scrollDelta;
	}

private:

	sf::RectangleShape m_visual;
	sf::Vector2f m_windowDimenisons;
	float m_t = 0.f;

	bool m_isVisible = false;
	bool m_isDragging = false;
	float m_scrollDeficite = 0.f;
};