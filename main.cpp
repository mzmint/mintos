#include <SFML/Graphics.hpp>
#include <string>
#include <filesystem>
#include <iostream>
#include <optional>
#include "MintLibClock.h"
#include "MintLibSystem.h"
#include "MintLibFile.h"

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
    std::string path = "root/";
    long long startuptime = Mint::unixLocalTimeMs();
    bool isctrlfull = false;

    sf::Font font;

    if (!font.openFromFile("assets/font.ttf")) {
        std::cout << std::filesystem::current_path() << '\n';
        return 1;
    }

    sf::Text deftext(font);
    deftext.setCharacterSize(32);
    deftext.setPosition({10, 1028});
    deftext.setString("mint@" + path + ":");

    sf::Text cmdtext(font);
    cmdtext.setCharacterSize(32);
    cmdtext.setPosition({deftext.getLocalBounds().size.x + 32, 1028});

    sf::Text ftext(font);
    ftext.setCharacterSize(32);
    ftext.setPosition({16 + deftext.getLocalBounds().size.x, 1028});
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

    Mint::File::createDirectory("root");

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
                            isctrlfull = false;
                            clock.toggle();
                            cmds.close();
                            ctrl.close();
                            f.close();
                            clock.setHeader("Clock");
                        } else if (cmd == "quit") {
                            window.close();
                        } else if (cmd == "exit") {
                            isctrlfull = false;
                            clock.close();
                            cmds.close();
                            ctrl.close();
                            f.close();
                        } else if (cmd == "cmds") {
                            isctrlfull = false;
                            cmds.toggle();
                            clock.close();
                            ctrl.close();
                            f.close();
                            cmds.setHeader("Commands");
                            cmds.setText("Commands: \n    quit\n    clock\n    exit\n    ctrl\n    f [file]");
                        } else if (cmd == "ctrl") {
                            isctrlfull = false;
                            ctrl.toggle();
                            clock.close();
                            f.close();
                            ctrl.setHeader("Control Panel");
                        } else if (cmd.rfind("f ", 0) == 0) {
                            std::string contents;

                            if (Mint::File::tryReadText(path + cmd.substr(2), contents)) {
                                isctrlfull = false;
                                clock.close();
                                cmds.close();
                                ctrl.close();

                                f.setText(contents);
                                f.setHeader(cmd.substr(2));
                                f.open();
                            }
                        } else if (cmd.rfind("nf ", 0) == 0) {
                            clock.close();
                            cmds.close();
                            ctrl.close();
                            f.close();
                            Mint::File::createFile(path + cmd.substr(3));
                        } else if (cmd.rfind("d ", 0) == 0) {
                            std::string dir = cmd.substr(2);
                            std::string newPath;

                            while (path.length() > 1 && (path.back() == '/' || path.back() == '\\')) {
                                path.pop_back();
                            }
                            if (dir == "..") {
                                newPath = Mint::File::parentPath(path);
                            } else {
                                while (!dir.empty() && (dir.front() == '/' || dir.front() == '\\')) {
                                    dir.erase(0, 1);
                                }
                                newPath = path + "/" + dir;
                            }
                            if (Mint::File::exists(newPath) &&
                                Mint::File::isDirectory(newPath)) {
                                clock.close();
                                cmds.close();
                                ctrl.close();
                                f.close();
                                path = newPath;

                                if (path.back() != '/' && path.back() != '\\') path += "/";
                                deftext.setString("mint@" + path + ":");
                                ftext.setPosition({16 + deftext.getLocalBounds().size.x, 1028});
                                cmdtext.setPosition({deftext.getLocalBounds().size.x + 32, 1028});
                                }
                        } else if (cmd.rfind("nd ", 0) == 0) {
                            std::string dir = cmd.substr(3);
                            while (path.length() > 1 && (path.back() == '/' || path.back() == '\\')) {
                                path.pop_back();
                            }
                            while (!dir.empty() && (dir.front() == '/' || dir.front() == '\\')) {
                                dir.erase(0, 1);
                            }
                            while (!dir.empty() && (dir.back() == '/' || dir.back() == '\\')) {
                                dir.pop_back();
                            }

                            std::string newPath = path + "/" + dir;

                            if (!dir.empty() && !Mint::File::exists(newPath)) {
                                clock.close();
                                cmds.close();
                                ctrl.close();
                                f.close();

                                Mint::File::createDirectory(newPath);
                            }
                        } else if (cmd == "ctrl full") {
                            isctrlfull = !isctrlfull;

                            if (isctrlfull) {
                                ctrl.open();
                            } else {
                                ctrl.close();
                            }
                            clock.close();
                            cmds.close();
                            f.close();

                            ctrl.setHeader("Control Panel");
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
            if (isctrlfull) {
                ctrl.setText("Name: " + Mint::System::computerName()
                                + "\nUsername: " + Mint::System::username()
                                + "\nTotal Ram: " + std::to_string(Mint::System::totalRAM())
                                + "\nAvailable Ram: " + std::to_string(Mint::System::availableRAM())
                                + "\nRam Usage: " + std::to_string(Mint::System::ramUsagePercent())
                                + "\nHost Name: " + Mint::System::hostname()
                                + "\nIP Address: " + Mint::System::localIPAddress()
                                + "\nTime Since Startup: " + std::to_string(Mint::unixLocalTimeMs() - startuptime)
                                + "\nCurrent Path: " + path
                                + "\nCPU Architecture: " + Mint::System::cpuArchitecture()
                                + "\nCPU Cores: " + std::to_string(Mint::System::cpuCores())
                                + "\nCPU Name: " + Mint::System::cpuName()
                                + "\nCPU Threads: " + std::to_string(Mint::System::cpuThreads())
                                + "\nCPU Usage: " + std::to_string(Mint::System::cpuUsagePercent())
                                + "\nCPU Vendor: " + Mint::System::cpuVendor()
                                + "\nScreen Refresh Rate: " + std::to_string(Mint::System::screenRefreshRate())
                                + "\nScreen Width: " + std::to_string(Mint::System::screenWidth())
                                + "\nScreen Height: " + std::to_string(Mint::System::screenHeight())
                                + "\nUser Home Directory: " + Mint::System::userHomeDirectory());
            } else {
                ctrl.setText("Name: " + Mint::System::computerName()
                                + "\nUsername: " + Mint::System::username()
                                + "\nTotal Ram: " + std::to_string(Mint::System::totalRAM())
                                + "\nAvailable Ram: " + std::to_string(Mint::System::availableRAM())
                                + "\nRam Usage: " + std::to_string(Mint::System::ramUsagePercent())
                                + "\nHost Name: " + Mint::System::hostname()
                                + "\nIP Address: " + Mint::System::localIPAddress()
                                + "\nTime Since Startup: " + std::to_string(Mint::unixLocalTimeMs() - startuptime)
                                + "\nCurrent Path: " + path
                                + "\nCPU Usage: " + std::to_string(Mint::System::cpuUsagePercent()));
            }
            ctrl.draw();
        }

        if (f.isOpen()) {
            f.draw();
        }

        window.display();
    }
}
