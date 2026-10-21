#include <SFML/Graphics.hpp>
#include <string>
#include <filesystem>
#include <iostream>
#include <optional>
#include "MintLibClock.h"
#include "MintLibSystem.h"
#include "MintLibFile.h"
#include <vector>
#include <sstream>
#include <algorithm>

int main() {
    bool isclock = false;
    bool iscmds = false;
    bool isctrl = false;
    bool isf = false;

    int scrollLine = 0;
    const float lineHeight = 40.f;
    const float fileTop = 10.f;
    const float fileBottom = 1010.f;
    std::vector<std::string> fileLines;
    const int visibleLines = 35;

    sf::Clock flashclock;
    bool fvisible = true;
    const sf::Time interval = sf::milliseconds(1000);


    sf::RenderWindow window(
        sf::VideoMode({1920, 1080}),
        "MintOS"
    );

    std::string text;
    std::string cmd;

    sf::Font font;

    if (!font.openFromFile("assets/font.ttf")) {
        std::cout << std::filesystem::current_path() << '\n';
        return 1;
    }

    sf::Text cmdtext(font);
    cmdtext.setCharacterSize(32);
    cmdtext.setPosition({100, 1028});

    sf::Text deftext(font);
    deftext.setCharacterSize(32);
    deftext.setPosition({10, 1028});
    deftext.setString("mint:");

    sf::Text ftext(font);
    ftext.setCharacterSize(32);
    ftext.setPosition({80, 1028});
    ftext.setString(">");

    //clock
    sf::Text clocktext(font);
    clocktext.setCharacterSize(32);
    clocktext.setPosition({10, 10});

    //cmds
    sf::Text cmdstext(font);
    cmdstext.setCharacterSize(32);
    cmdstext.setPosition({10, 10});
    cmdstext.setString("Commands: \n    quit\n    clock\n    exit");

    //ctrl
    sf::Text ctrltext(font);
    ctrltext.setCharacterSize(32);
    ctrltext.setPosition({10, 10});
    ctrltext.setString("Name: " + Mint::computerName() + "\nTotal Ram: " + std::to_string(Mint::totalRAM()));

    //default
    sf::Text dfltext(font);
    dfltext.setCharacterSize(32);
    dfltext.setPosition({10, 10});
    dfltext.setString("Commands: \n    quit\n    clock\n    exit\n    f [file]");

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (event->is<sf::Event::FocusGained>()) {
                window.setMouseCursorVisible(false);
            }

            if (event->is<sf::Event::FocusLost>()) {
                window.setMouseCursorVisible(true);
            }

            if (const auto* key =
                event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Backspace) {
                    if (!text.empty()) {
                        text.pop_back();
                    }
                }
                if (isf) {
                    if (key->scancode == sf::Keyboard::Scancode::Up) {
                        if (scrollLine > 0) {
                            scrollLine--;
                        }
                    }

                    if (key->scancode == sf::Keyboard::Scancode::Down) {
                        int maxScroll = std::max(0, static_cast<int>(fileLines.size()) - visibleLines);
                        if (scrollLine < maxScroll) {
                            scrollLine++;
                        }
                    }
                }
            }

            if (const auto* key =
                event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Enter) {
                    if (!text.empty()) {
                        cmd = text;
                        text.clear();
                        if (cmd == "clock") {
                            iscmds = false;
                            isctrl = false;
                            isf = false;
                            isclock = !isclock;
                        } else if (cmd == "quit") {
                            window.close();
                        } else if (cmd == "exit") {
                            isclock = false;
                            iscmds = false;
                            isctrl = false;
                            isf = false;
                        } else if (cmd == "cmds") {
                            isclock = false;
                            isctrl = false;
                            iscmds = !iscmds;
                            isf = false;
                        } else if (cmd == "ctrl") {
                            isclock = false;
                            isctrl = !isctrl;
                            iscmds = false;
                            isf = false;
                        } else if (cmd.substr(0, 1) == "f" && cmd.substr(1, 1) == " ") {
                            std::string temp;
                            isclock = false;
                            isctrl = false;
                            iscmds = false;
                            if (Mint::File::tryReadText(cmd.substr(2), temp)) {
                                std::string contents = Mint::File::readText(cmd.substr(2));

                                fileLines.clear();
                                std::istringstream stream(contents);
                                std::string line;

                                while (std::getline(stream, line)) {
                                    fileLines.push_back(line);
                                }

                                scrollLine = 0;
                                isf = true;
                            }
                        }
                    }
                }
            }

            if (const auto* entered =
                event->getIf<sf::Event::TextEntered>()) {
                char32_t c = entered->unicode;

                if (c >= 32 && c != 127) {
                    if (c < 128) {
                        text += static_cast<char>(c);
                    }
                }
            }
        }

        cmdtext.setString(text);

        if (text == "") {
            if (flashclock.getElapsedTime() >= interval) {
                fvisible = !fvisible;
                flashclock.restart();
            }
        } else {
            fvisible = true;
            flashclock.restart();
        }

        window.clear(sf::Color::Black);
        window.draw(deftext);
        window.draw(cmdtext);

        if (text != "" || fvisible)
            window.draw(ftext);

        if (isclock) {
            window.draw(clocktext);
            clocktext.setString("Unix Time: " + std::to_string(Mint::unixLocalTimeMs()) + "\nCurrent Date: " + Mint::localDateTimeMs());
        }

        if (iscmds) {
            window.draw(cmdstext);
        }

        if (isctrl) {
            window.draw(ctrltext);
        }

        if (isf) {
            std::string visibleText;

            int endLine = std::min(
                scrollLine + visibleLines,
                static_cast<int>(fileLines.size())
            );

            for (int i = scrollLine; i < endLine; i++) {
                visibleText += fileLines[i];

                if (i < endLine - 1) {
                    visibleText += '\n';
                }
            }

            dfltext.setString(visibleText);
            dfltext.setPosition({10.f, 10.f});

            window.draw(dfltext);
        }

        window.display();
    }
}