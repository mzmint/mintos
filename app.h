#ifndef APP_H
#define APP_H

#include <SFML/Graphics.hpp>
#include <string>

class App {
private:
    sf::RenderWindow* window;
    sf::Text* text;

public:
    App(sf::RenderWindow* w, sf::Text* t);

    void setText(std::string str);
    void setPosition(sf::Vector2f pos);
    void setCharacterSize(unsigned int size);
    void setFont(sf::Font *font);

    void draw();
};

#endif