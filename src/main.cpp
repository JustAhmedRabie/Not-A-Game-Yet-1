#include <iostream>
#include <memory>
#include <fstream>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

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

    Shape(ShapeType type, std::string name, sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f scale, sf::Color color)
        :type(type), name(name), position(position), velocity(velocity), scale(scale), color(color)
    {

    }
};


int main(int argc, char* argv[])
{
    std::vector<Shape> shapes = {};
    std::string fontPath;
    int fontSize;
    sf::Color fontColor;
    int wWidth;
    int wHeight;

    std::ifstream file("build/config.txt");

    if (!file)
    {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    std::string word;

    while (file >> word)
    {
        if (word == "Window")
        {
            file >> wWidth >> wHeight;
        }
        else if (word == "Font")
        {
            file >> fontPath;
            file >> fontSize >> fontColor.r >> fontColor.g >> fontColor.b;
        }
        else
        {
            ShapeType type = word == "Circle" ? ShapeType::CIRCLE : ShapeType::RECTANGLE;
            std::string name;
            float px, py, vx, vy, r, g, b, sx, sy;

            file >> name;
            file >> px >> py >> vx >> vy >> r >> g >> b >> sx;
            if (type == ShapeType::RECTANGLE) file >> sy;
            else sy = sx;

            shapes.emplace_back(type, name, sf::Vector2f(px, py), sf::Vector2f(vx, vy), sf::Vector2f(sx, sy), sf::Color(r, g, b, 255));
        }

    }


    // Create a new window of size 1280x720 pixels
    // Top-left is (0, 0), bottom-right is (width, height)
    sf::RenderWindow window(sf::VideoMode({ (unsigned int)wWidth, (unsigned int)wHeight }), "SFML Works!");
    window.setFramerateLimit(60);

    // Initialize ImGui-SFML
    if (!ImGui::SFML::Init(window))
    {
        return -1;
    }

    sf::Clock deltaClock;

    // Scale UI elements and fonts for high-DPI displays
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui::GetIO().FontGlobalScale = 2.0f;

    // Color array for ImGui ColorEdit3 (floats from 0.0 to 1.0)
    float c[3] = { 0.0f, 1.0f, 1.0f };

    // Initial shape properties
    float circleRadius = 50.0f;
    int circleSegments = 32;
    float circleSpeedX = 1.0f;
    float circleSpeedY = 1.0f;
    bool drawCircle = true;
    bool drawText = true;

    // Set up the initial circle shape
    sf::CircleShape circle(circleRadius, circleSegments);
    circle.setPosition({ 100.0f, 100.0f });

    // Load font
    sf::Font font;
    if (!font.openFromFile(fontPath))
    {
        std::cerr << "Could not load font!\n";
        return -1;
    }

    // Set up sample text
    sf::Text text(font, "Sample Text", 24);
    text.setPosition({ 0.0f, (float)wHeight - (float)text.getCharacterSize() });

    // Character buffer for ImGui text input
    char nameBuffer[255] = "Sample Text";

    // Main game loop
    while (window.isOpen())
    {
        // Handle SFML & ImGui events
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
        ImGui::Checkbox("Draw Circle", &drawCircle);
        ImGui::Checkbox("Draw Text", &drawText);
        ImGui::SliderFloat("Radius", &circleRadius, 10.0f, 300.0f);
        ImGui::SliderInt("Sides", &circleSegments, 3, 64);
        ImGui::ColorEdit3("Color Circle", c);
        ImGui::InputText("Text", nameBuffer, 255);

        if (ImGui::Button("Set Text"))
        {
            text.setString(nameBuffer);
        }

        if (ImGui::Button("Reset Circle"))
        {
            circle.setPosition({ 0.0f, 0.0f });
        }

        ImGui::End();

        // Update circle properties based on GUI input
        circle.setRadius(circleRadius);
        circle.setPointCount(circleSegments);
        circle.setFillColor(sf::Color(
            (uint8_t)(c[0] * 255),
            (uint8_t)(c[1] * 255),
            (uint8_t)(c[2] * 255)
        ));

        // Movement / Position update
        circle.setPosition({
            circle.getPosition().x + circleSpeedX,
            circle.getPosition().y + circleSpeedY
            });

        // Clear, draw, render, display
        window.clear(sf::Color::Black);

        if (drawCircle)
        {
            window.draw(circle);
        }

        if (drawText)
        {
            window.draw(text);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}