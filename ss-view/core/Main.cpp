#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#pragma comment(lib, "dwmapi.lib")

#define DCLICK_TIME 0.5f

#include <windows.h>
#include <dwmapi.h>

#include <filesystem>

#include "FileChecker.h"
#include "Formatter.h"
#include "Project.h"
#include "Scrollbar.h"
#include "Settings.h"
#include "SlugScrollPositions.h"
#include "ToolBar.h"

#include "WindowMeasure.h"

bool g_darkMode = true;

int main(int argc, char** argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            std::cout << "SimpleScript - Viewer\n  Run in SimpleScript project root directory.\n\n  Shortcuts\n    Ctrl+Left/Right -- Move to next/prev squence\n    Ctrl+M -- Switch dark/light mode\n    Double-click -- Open contents under cursor in notepad\n\n";
        }
        return 0;
    }

    Settings::Get().Load();

    sf::RenderWindow window(sf::VideoMode(800, 1080), "SimpleScript - Viewer");

    BOOL useDark = TRUE;
    DwmSetWindowAttribute(window.getSystemHandle(), DWMWA_USE_IMMERSIVE_DARK_MODE, &useDark, sizeof(useDark));

    COLORREF borderColor = RGB(20, 20, 20);
    DwmSetWindowAttribute(window.getSystemHandle(), DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));

    ScrollBar mainScrollbar;
    mainScrollbar.SetColor({ 110, 110, 110, 200 });
    mainScrollbar.SetSize({ 15, 35 });
    mainScrollbar.SetWindowDimensions({ 800, 1080 });

    ScrollBar toolbarScrollbar;
    toolbarScrollbar.SetColor({ 110, 110, 110, 255 });
    toolbarScrollbar.SetSize({ 15, 35 });
    toolbarScrollbar.SetWindowDimensions({ 300, 1080 });

    Project proj;
#ifdef _DEBUG
    proj.Load(std::filesystem::current_path() / "prj");
#else
    proj.Load(std::filesystem::current_path());
#endif // _DEBUG
    size_t sequenceIndex = 0;

    window.setTitle("SimpleScript Viewer - " + proj.GetSequence(sequenceIndex).name + " (" + std::to_string(sequenceIndex + 1) + "/" + std::to_string(proj.GetNumberOfSequences()) + ")");

    Formatter formatter;
    formatter.SetFontSize(WindowMeasure(window.getSize().x));
    formatter.LoadFromSequence(proj.GetSequence(sequenceIndex), proj.Characters(), g_darkMode);
    mainScrollbar.SetIsVisible(formatter.GetContentSize() > window.getSize().y);

    Toolbar toolbar;
    toolbar.SetMenuSize(window.getSize(), 300);
    toolbar.SetIconSize(10.f);
    toolbar.SetIconColor({ 127, 127, 127 });
    toolbar.SetBackgroundColor({ 0, 0, 0, 180 });
    toolbar.SetClearColor({ 20, 20, 20, 200 });
    toolbar.SetTextProperties(formatter.GetFont());
    toolbar.SetHighlightColor({ 35,35, 35, 180 });
    for (size_t i = 0; i < proj.GetNumberOfSequences(); ++i)
    {
        toolbar.AddMenuItem(std::to_string(i + 1) + " : " + proj.GetSequence(i).name);
    }
    toolbar.Format();
    toolbar.SetIndexToBold(0);
    toolbarScrollbar.SetIsVisible(toolbar.GetContentSize() > window.getSize().y);

    SlugScrollPositions slugPositions;
    slugPositions.SetWindowSize((sf::Vector2f)window.getSize());
    slugPositions.SetLineColor({ 127, 127, 127 });
    slugPositions.SetLineSize({ 15, 3 });
    slugPositions.Calculate(formatter.GetSlugScrollPositions());

    FileChecker fileChecker;
    fileChecker.CheckFiles();

    sf::Vector2f mousePositionLast;
    sf::Vector2f mouseDelta;

    sf::Clock clickTimer;
    bool doDoubleClick = false;

    while (window.isOpen())
    {
        bool clickWasPressed = false;
        bool clickWasReleased = false;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::Resized)
            {
                window.setView(sf::View({ (float)event.size.width * 0.5f, (float)event.size.height * 0.5f}, { (float)event.size.width, (float)event.size.height }));
                mainScrollbar.SetWindowDimensions((sf::Vector2f)window.getSize());
                mainScrollbar.SetIsVisible(formatter.GetContentSize() > window.getSize().y);


                formatter.SetFontSize(WindowMeasure(window.getSize().x));
                formatter.LoadFromSequence(proj.GetSequence(sequenceIndex), proj.Characters(), g_darkMode);
                toolbar.SetMenuSize(window.getSize(), 300);
                toolbar.SetTextProperties(formatter.GetFont());
                toolbar.Format();
                slugPositions.SetWindowSize((sf::Vector2f)window.getSize());
                slugPositions.Calculate(formatter.GetSlugScrollPositions());

                toolbarScrollbar.SetWindowDimensions({ 300.f, (float)window.getSize().y });
                toolbarScrollbar.SetIsVisible(toolbar.GetContentSize() > window.getSize().y);

                if (formatter.GetContentSize() <= window.getSize().y)
                {
                    mainScrollbar.SetScrollPoint(0.f);
                    formatter.SetScroll(0.f, window.getSize().y);
                }
                else
                {
                    formatter.SetScroll(mainScrollbar.GetScrollFactor(), window.getSize().y);
                }
            }

            if (event.type == sf::Event::MouseWheelScrolled)
            {
                float t = 0.f;
                if (toolbar.IsOpen())
                {
                    toolbar.OnScroll(event.mouseWheelScroll.delta * 50.f, t);
                    toolbarScrollbar.SetScrollPoint(t);
                }
                else
                {
                    formatter.OnScroll(event.mouseWheelScroll.delta * 50.f, window.getSize().y, t);
                    mainScrollbar.SetScrollPoint(t);
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                clickWasPressed = true;
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                clickWasReleased = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl))
            {
                if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::Right && sequenceIndex < proj.GetNumberOfSequences() - 1)
                    {
                        ++sequenceIndex;
                        window.setTitle("SimpleScript Viewer - " + proj.GetSequence(sequenceIndex).name + " (" + std::to_string(sequenceIndex + 1) + "/" + std::to_string(proj.GetNumberOfSequences()) + ")");
                        formatter.LoadFromSequence(proj.GetSequence(sequenceIndex), proj.Characters(), g_darkMode);
                        mainScrollbar.SetIsVisible(formatter.GetContentSize() > window.getSize().y);
                        mainScrollbar.SetScrollPoint(0.f);
                        slugPositions.Calculate(formatter.GetSlugScrollPositions());
                    }
                    if (event.key.code == sf::Keyboard::Left && sequenceIndex > 0)
                    {
                        --sequenceIndex;
                        window.setTitle("SimpleScript Viewer - " + proj.GetSequence(sequenceIndex).name + " (" + std::to_string(sequenceIndex + 1) + "/" + std::to_string(proj.GetNumberOfSequences()) + ")");
                        formatter.LoadFromSequence(proj.GetSequence(sequenceIndex), proj.Characters(), g_darkMode);
                        mainScrollbar.SetIsVisible(formatter.GetContentSize() > window.getSize().y);
                        mainScrollbar.SetScrollPoint(0.f);
                        slugPositions.Calculate(formatter.GetSlugScrollPositions());
                    }
                    if (event.key.code == sf::Keyboard::M)
                    {
                        g_darkMode = !g_darkMode;
                        formatter.LoadFromSequence(proj.GetSequence(sequenceIndex), proj.Characters(), g_darkMode, true);
                        slugPositions.Calculate(formatter.GetSlugScrollPositions());
                    }
                }
            }

            if (event.type == sf::Event::GainedFocus)
            {
                if (fileChecker.CheckFiles())
                {
#ifdef _DEBUG
                    proj.Load(std::filesystem::current_path() / "prj");
#else
                    proj.Load(std::filesystem::current_path());
#endif // _DEBUG
                    bool resetScroll = false;
                    if (sequenceIndex >= proj.GetNumberOfSequences())
                    {
                        sequenceIndex = 0;
                        resetScroll = true;
                    }

                    formatter.LoadFromSequence(proj.GetSequence(sequenceIndex), proj.Characters(), g_darkMode, !resetScroll);
                    mainScrollbar.SetIsVisible(formatter.GetContentSize() > window.getSize().y);
                    slugPositions.Calculate(formatter.GetSlugScrollPositions());

                    toolbar.ClearMenuItems();
                    for (size_t i = 0; i < proj.GetNumberOfSequences(); ++i)
                    {
                        toolbar.AddMenuItem(std::to_string(i + 1) + " : " + proj.GetSequence(i).name);
                    }
                    toolbar.Format();
                    toolbar.SetIndexToBold(sequenceIndex);
                    toolbarScrollbar.SetIsVisible(toolbar.GetContentSize() > window.getSize().y);
                }
            }
        }

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        mouseDelta = (sf::Vector2f)mousePos - mousePositionLast;
        mousePositionLast = (sf::Vector2f)mousePos;

        if (toolbar.CheckPoint(mousePositionLast))
        {
            if (!toolbar.IsOpen() && clickWasPressed)
            {
                toolbar.SetIsOpen(true);
            }
            else if (toolbar.IsOpen() && clickWasPressed)
            {
                int index = toolbar.GetHighlightIndex();
                if (index >= 0 && index != sequenceIndex)
                {
                    sequenceIndex = index;
                    window.setTitle("SimpleScript Viewer - " + proj.GetSequence(sequenceIndex).name + " (" + std::to_string(sequenceIndex + 1) + "/" + std::to_string(proj.GetNumberOfSequences()) + ")");
                    formatter.LoadFromSequence(proj.GetSequence(sequenceIndex), proj.Characters(), g_darkMode);
                    mainScrollbar.SetIsVisible(formatter.GetContentSize() > window.getSize().y);
                    mainScrollbar.SetScrollPoint(0.f);
                    slugPositions.Calculate(formatter.GetSlugScrollPositions());
                    toolbar.SetIsOpen(false);
                    toolbar.SetIndexToBold(sequenceIndex);
                }
            }
        }
        else if (clickWasPressed && toolbar.IsOpen())
        {
            toolbar.SetIsOpen(false);
        }

        if (toolbar.IsOpen())
        {
            if (clickWasPressed && toolbarScrollbar.TestOverlap(mousePositionLast))
            {
                toolbarScrollbar.SetIsDragging(true);
            }

            if (toolbarScrollbar.GetIsDragging() && mouseDelta.y != 0.f)
            {
                toolbarScrollbar.DoScroll(mouseDelta);
                toolbar.SetScroll(toolbarScrollbar.GetScrollFactor());
            }
        }
        if (clickWasReleased && toolbarScrollbar.GetIsDragging())
        {
            toolbarScrollbar.SetIsDragging(false);
        }

        if (clickWasPressed && mainScrollbar.TestOverlap(mousePositionLast))
        {
            mainScrollbar.SetIsDragging(true);
        }
        if (clickWasReleased && mainScrollbar.GetIsDragging())
        {
            mainScrollbar.SetIsDragging(false);
        }

        if (mainScrollbar.GetIsDragging() && mouseDelta.y != 0.f)
        {
            mainScrollbar.DoScroll(mouseDelta);
            formatter.SetScroll(mainScrollbar.GetScrollFactor(), window.getSize().y);
        }

        if (clickWasPressed)
        {
            if (!doDoubleClick)
            {
                clickTimer.restart();
                doDoubleClick = true;
            }
            else
            {
                if (clickTimer.getElapsedTime().asSeconds() <= DCLICK_TIME)
                {
                    doDoubleClick = false;
                    //DO Double click stuff
                    if (!toolbar.IsOpen())
                    {
                        formatter.TryOpenFile(mousePositionLast, proj);
                    }
                }
            }
        }
        if (doDoubleClick && clickTimer.getElapsedTime().asSeconds() > DCLICK_TIME)
        {
            doDoubleClick = false;
        }

        window.clear((g_darkMode) ? sf::Color(20, 20, 20) : sf::Color(235, 235, 235));
        formatter.DrawTo(window);
        if (mainScrollbar.GetIsVisible())
        {
            slugPositions.DrawTo(window);
        }
        mainScrollbar.DrawTo(window);
        toolbar.DrawTo(window);
        if (toolbar.IsOpen())
        {
            toolbarScrollbar.DrawTo(window);
        }
        window.display();
    }

    return 0;
}