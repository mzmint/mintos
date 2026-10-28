#include "app.h"

#include <algorithm>
#include <sstream>

App::App(sf::RenderWindow* w, sf::Text* t)
    : window(w), text(t) {
}

void App::setText(std::string str) {
    content = str;
    lines.clear();

    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) {
        lines.emplace_back();
    }

    // Keep scrolling within the available lines.
    int maxScroll = std::max(
        0,
        static_cast<int>(lines.size()) - visibleLines
    );

    scrollLine = std::clamp(scrollLine, 0, maxScroll);

    updateText();
}

void App::setPosition(sf::Vector2f pos) {
    text->setPosition(pos);
}

void App::setCharacterSize(unsigned int size) {
    text->setCharacterSize(size);
}

void App::setFont(sf::Font* font) {
    text->setFont(*font);
}

void App::setVisibleLines(int count) {
    visibleLines = std::max(1, count);

    int maxScroll = std::max(
        0,
        static_cast<int>(lines.size()) - visibleLines
    );

    scrollLine = std::clamp(scrollLine, 0, maxScroll);
    updateText();
}

void App::setScrollable(bool enabled) {
    scrollEnabled = enabled;
}

void App::handleEvent(const sf::Event& event) {
    if (!ison || !scrollEnabled) {
        return;
    }

    const auto* key = event.getIf<sf::Event::KeyPressed>();

    if (!key) {
        return;
    }

    int maxScroll = std::max(
        0,
        static_cast<int>(lines.size()) - visibleLines
    );

    if (key->scancode == sf::Keyboard::Scancode::Up) {
        if (scrollLine > 0) {
            --scrollLine;
            updateText();
        }
    }
    else if (key->scancode == sf::Keyboard::Scancode::Down) {
        if (scrollLine < maxScroll) {
            ++scrollLine;
            updateText();
        }
    }
}

void App::updateText() {
    std::string visibleText;

    int endLine = std::min(
        scrollLine + visibleLines,
        static_cast<int>(lines.size())
    );

    for (int i = scrollLine; i < endLine; ++i) {
        if (i > scrollLine) {
            visibleText += '\n';
        }

        visibleText += lines[i];
    }

    text->setString(visibleText);
}

bool App::isOpen() {
    return ison;
}

void App::close() {
    ison = false;
}

void App::open() {
    ison = true;
}

void App::draw() {
    if (ison) {
        window->draw(*text);
    }
}
