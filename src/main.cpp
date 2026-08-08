#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <optional> //? suggested to add it since we used it in the code

enum ShapeType
{
    CIRCLE,
    RECTANGLE
};

class Shape
{
private:
    /* data */
public:
    ShapeType type;
    std::string name;
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    sf::Vector2f scale;

    Shape(ShapeType type, std::string name, sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f scale,
          sf::Color color)
        : type(type),
          name(name),
          position(position),
          velocity(velocity),
          scale(scale),
          color(color)
    {
    }
};

//* A new Unit
struct GameConfig
{
    std::vector<Shape> shapes = {};
    std::string fontPath;
    int fontSize;
    sf::Color fontColor;
    int wWidth;
    int wHeight;
};

//* The SanityCheck Object for sake of testing
struct SaneCheck
{
    // Initial shape properties
    float circleRadius = 50.0f;
    int circleSegments = 32;
    float circleSpeedX = 3.0f;
    float circleSpeedY = 1.0f;
    bool drawCircle = true;
    bool drawText = true;
};

// TODO : i think it may be cleaner in some way
bool getConfig(GameConfig& conf)
{
    std::ifstream file("build/config.txt");

    if (!file)
    {
        std::cerr << "Failed to open file\n";
        return false;
    }

    std::string word;

    while (file >> word)
    {
        if (word == "Window")
        {
            file >> conf.wWidth >> conf.wHeight;
        }
        else if (word == "Font")
        {
            file >> conf.fontPath;
            file >> conf.fontSize >> conf.fontColor.r >> conf.fontColor.g >> conf.fontColor.b;
        }
        else
        {
            ShapeType type = word == "Circle" ? ShapeType::CIRCLE : ShapeType::RECTANGLE;
            std::string name;
            float px, py, vx, vy, r, g, b, sx, sy;

            file >> name;
            file >> px >> py >> vx >> vy >> r >> g >> b >> sx;
            if (type == ShapeType::RECTANGLE)
                file >> sy;
            else
                sy = sx;

            conf.shapes.emplace_back(type, name, sf::Vector2f(px, py), sf::Vector2f(vx, vy), sf::Vector2f(sx, sy),
                                     sf::Color(r, g, b, 255));
        }
    }
    return true;
}

void GameLoop(sf::RenderWindow& window, GameConfig& conf, SaneCheck& check, sf::CircleShape& circle, sf::Font& font)
{
    sf::Clock deltaClock;
    /*
    ! from here , a tight coupling that needs resolving , but kept that way for the purposes of testing
    */
    // Color array for ImGui ColorEdit3 (floats from 0.0 to 1.0)
    float c[3] = {0.0f, 1.0f, 1.0f};
    // Set up sample text
    sf::Text text(font, "Sample Text", 24);
    text.setPosition({0.0f, (float) conf.wHeight - (float) text.getCharacterSize()});

    // Character buffer for ImGui text input
    char nameBuffer[255] = "Sample Text";
    //* the actual function logic
    // Main game loop
    while (window.isOpen())
    {
        // Handle SFML & ImGui events
        //! the compiler might throw an error since sometimes it require a template to give to optinal
        //! it may be given as auto , but the test will tell us the results
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        // Update ImGui
        ImGui::SFML::Update(window, deltaClock.restart());

        // Construct ImGui Controls Window
        ImGui::Begin("Window Title");
        ImGui::Text("Text Label");
        ImGui::Checkbox("Draw Circle", &check.drawCircle);
        ImGui::Checkbox("Draw Text", &check.drawText);
        ImGui::SliderFloat("Radius", &check.circleRadius, 10.0f, 300.0f);
        ImGui::SliderInt("Sides", &check.circleSegments, 3, 64);
        ImGui::ColorEdit3("Color Circle", c);
        ImGui::InputText("Text", nameBuffer, 255);

        if (ImGui::Button("Set Text"))
        {
            text.setString(nameBuffer);
        }

        if (ImGui::Button("Reset Circle"))
        {
            circle.setPosition({0.0f, 0.0f});
        }

        ImGui::End();

        // Update circle properties based on GUI input
        circle.setRadius(check.circleRadius);
        circle.setPointCount(check.circleSegments);
        circle.setFillColor(sf::Color((uint8_t) (c[0] * 255), (uint8_t) (c[1] * 255), (uint8_t) (c[2] * 255)));

        if (circle.getGlobalBounds().position.x + circle.getRadius() * 2 >= conf.wWidth ||
            circle.getGlobalBounds().position.x <= 0.f)
        {
            check.circleSpeedX *= -1;
        }

        if (circle.getGlobalBounds().position.y + circle.getRadius() * 2 >= conf.wHeight ||
            circle.getGlobalBounds().position.y <= 0.f)
        {
            check.circleSpeedY *= -1;
        }
        // Movement / Position update
        circle.setPosition({circle.getPosition().x + check.circleSpeedX, circle.getPosition().y + check.circleSpeedY});

        // Clear, draw, render, display
        window.clear(sf::Color::Black);

        if (check.drawCircle)
        {
            window.draw(circle);
        }

        if (check.drawText)
        {
            const auto bounds = text.getLocalBounds();

            text.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
            text.setPosition(
                {circle.getPosition().x + circle.getRadius(), circle.getPosition().y + circle.getRadius()});
            window.draw(text);
        }

        ImGui::SFML::Render(window);
        window.display();
    }
}

int main(int argc, char* argv[])
{
    // the unit that holds the Game Config
    GameConfig conf;
    // the unit tht belong to the Sanity Test object
    SaneCheck check;
    // the return if the window is not opened
    bool isOpened = getConfig(conf);
    if (!isOpened)
    {
        return -1;
    }

    // Create a new window of size 1280x720 pixels
    // Top-left is (0, 0), bottom-right is (width, height)
    sf::RenderWindow window(sf::VideoMode({(unsigned int) conf.wWidth, (unsigned int) conf.wHeight}), "SFML Works!");
    window.setFramerateLimit(60);

    // Initialize ImGui-SFML
    if (!ImGui::SFML::Init(window))
    {
        return -1;
    }

    // Scale UI elements and fonts for high-DPI displays
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui::GetIO().FontGlobalScale = 2.0f;

    // Set up the initial circle shape
    sf::CircleShape circle(check.circleRadius, check.circleSegments);
    circle.setPosition({100.0f, 100.0f});

    // Load font
    sf::Font font;
    if (!font.openFromFile(conf.fontPath))
    {
        std::cerr << "Could not load font!\n";
        return -1;
    }
    /*
    ! a very ugly style , but kept for the sake of only refactor the code
    TODO we should make it ckeaner which it will take args from other functions
    TODO -> also make the SanityCheck object a hardcoded unit like making it a struct then init , so we can use it
    anytime
    TODO -> i could've done that but got tired :p

    */
    GameLoop(window, conf, check, circle, font);

    ImGui::SFML::Shutdown();
    return 0;
}