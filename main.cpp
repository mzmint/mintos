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

#include "app.h"

int main() {
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

    //default
    sf::Text apptext(font);
    apptext.setCharacterSize(32);
    apptext.setPosition({10, 58});

    sf::Text headtext(font);
    headtext.setCharacterSize(32);
    headtext.setPosition({10, 12});

    //apps
    App clock(&window, &apptext, &headtext);
    App cmds(&window, &apptext, &headtext);
    App ctrl(&window, &apptext, &headtext);
    App f(&window, &apptext, &headtext);

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
            }

            if (const auto* key =
                event->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Enter) {
                    if (!text.empty()) {
                        cmd = text;
                        text.clear();
                        if (cmd == "clock") {
                            if (clock.isOpen()) clock.close();
                            else clock.open();
                            cmds.close();
                            ctrl.close();
                            f.close();
                            clock.setHeader("Clock");
                        } else if (cmd == "quit") {
                            window.close();
                        } else if (cmd == "exit") {
                            clock.close();
                            cmds.close();
                            ctrl.close();
                            f.close();
                        } else if (cmd == "cmds") {
                            if (cmds.isOpen()) cmds.close();
                            else cmds.open();
                            clock.close();
                            ctrl.close();
                            f.close();
                            cmds.setHeader("Commands");
                            cmds.setText("Commands: \n    quit\n    clock\n    exit\n    ctrl\n    f [file]");
                        } else if (cmd == "ctrl") {
                            if (ctrl.isOpen()) ctrl.close();
                            else ctrl.open();
                            clock.close();
                            f.close();
                            ctrl.setHeader("Control Panel");
                            ctrl.setText("Name: " + Mint::computerName()  + "\nUsername: " + Mint::username() + "\nTotal Ram: " + std::to_string(Mint::totalRAM()));
                        } else if (cmd.rfind("f ", 0) == 0) {
                            std::string contents;

                            if (Mint::File::tryReadText(cmd.substr(2), contents)) {
                                clock.close();
                                cmds.close();
                                ctrl.close();

                                f.setText(contents);
                                f.open();
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

            clock.handleEvent(*event);
            cmds.handleEvent(*event);
            ctrl.handleEvent(*event);
            f.handleEvent(*event);
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

        if (clock.isOpen()) {
            clock.draw();
            clock.setText("Unix Time: " + std::to_string(Mint::unixLocalTimeMs()) + "\nCurrent Date: " + Mint::localDateTimeMs());
        }

        if (cmds.isOpen()) {
            cmds.draw();
        }

        if (ctrl.isOpen()) {
            ctrl.draw();
        }

        if (f.isOpen()) {
            f.draw();
        }

        window.display();
    }
}
