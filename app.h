#ifndef APP_H
#define APP_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class App {
private:
    sf::RenderWindow* window;
    sf::Text* text;
    sf::Text* header;
    bool ison = false;
    std::string content;
    std::vector<std::string> lines;
    int scrollLine = 0;
    int visibleLines = 34;
    bool scrollEnabled = true;
    void updateText();
    std::string htext;
    std::string atext;

public:
    App(sf::RenderWindow* w, sf::Text* t, sf::Text* h);

    void setText(std::string str);
    void setPosition(sf::Vector2f pos);
    void setCharacterSize(unsigned int size);
    void setFont(sf::Font* font);
    void setVisibleLines(int count);
    void setScrollable(bool enabled);
    void handleEvent(const sf::Event& event);
    bool isOpen();
    void close();
    void open();
    void draw();
    void setHeader(std::string headertext);
    std::string getText();
    std::string getHeader();
};

#endif