#include "app.h"

App::App(sf::RenderWindow* w, sf::Text* t)
    : window(w), text(t) {
}

void App::setText(std::string str) {
    text->setString(str);
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
    window->draw(*text);
}