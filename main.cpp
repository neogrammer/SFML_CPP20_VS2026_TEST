#include <SFML/Graphics.hpp>

#include <iostream>
#define IMGUI_ENABLE_FREETYPE
#include "imgui.h"
#include "imgui_freetype.h"
#include "imgui-SFML.h"


void centerTextV(sf::Text& txt_, uint32_t scrH_);
void centerTextH(sf::Text& txt_, uint32_t scrW_);
void centerText(sf::Text& txt_, const sf::Vector2u& screenSize_);

int main()
{
    //////////////////////////////
    // Constants
    constexpr uint32_t SCRW = 1600;
    constexpr uint32_t SCRH = 900;
    //  End of Constants
    ///////////////////////////////
   
    //////////////////////////////
    // Initialize Game
    sf::RenderWindow window(sf::VideoMode({ SCRW, SCRH }), "SFML works!");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window, false)) throw std::runtime_error("Bad ImGui::Init()");
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    auto* f0 = io.Fonts->AddFontFromFileTTF("assets/fonts/bubbly.ttf", 48.f);
    auto* f1 = io.Fonts->AddFontFromFileTTF("assets/fonts/blue_winter.ttf", 48.f);
    auto* f2 = io.Fonts->AddFontFromFileTTF("assets/fonts/Dino.ttf", 64.f);
    auto* f3 = io.Fonts->AddFontFromFileTTF("assets/fonts/faith.ttf", 48.f);
    auto* f4 = io.Fonts->AddFontFromFileTTF("assets/fonts/hard.otf", 24.f);
    auto* f5 = io.Fonts->AddFontFromFileTTF("assets/fonts/Nova.otf", 24.f);
    auto* f6 = io.Fonts->AddFontFromFileTTF("assets/fonts/Rebel.ttf", 24.f);
    auto* f7 = io.Fonts->AddFontFromFileTTF("assets/fonts/Steel.otf", 16.f);
    auto* f8 = io.Fonts->AddFontFromFileTTF("assets/fonts/Sum.ttf", 16.f);
    auto* f9 = io.Fonts->AddFontFromFileTTF("assets/fonts/Techno.otf", 16.f);
    auto* f10 = io.Fonts->AddFontFromFileTTF("assets/fonts/Venite.ttf", 32.f);
    auto* f11 = io.Fonts->AddFontFromFileTTF("assets/fonts/VeniteStr8.ttf", 32.f);
    auto* f12 = io.Fonts->AddFontFromFileTTF("assets/fonts/Warr.otf", 24.f);

    if (!f0 || !f1 || !f2) throw std::runtime_error("Font load failed");
    io.FontDefault = f0;                    // optional but nice
    io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
    io.Fonts->Build();
    if (!ImGui::SFML::UpdateFontTexture())  // once
        throw std::runtime_error("UpdateFontTexture failed");
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    sf::Font bubblyFnt{ "assets/fonts/faith.ttf" };
    sf::Text testTxt{ bubblyFnt };
    testTxt.setCharacterSize(44u);
    testTxt.setFillColor(sf::Color::White);
    testTxt.setOutlineColor(sf::Color::Black);
    testTxt.setOutlineThickness(8.f);
    testTxt.setString("Hello ImGui-SFML!");
    testTxt.setPosition({100.f,80.f});
    centerText(testTxt, { SCRW,SCRH });
    std::cout << testTxt.getCharacterSize() * testTxt.getString().getSize() << std::endl;
    sf::CircleShape shape(10.f);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin({5.f,5.f});
    shape.setPosition({ (float)SCRW / 2.f, (float)SCRH / 2.f });
    sf::Clock deltaClock;
    // End of initialization
    //////////////////////////


    ////////////////////////////
    //  Main Loop
    while (window.isOpen())
    {

        //////////////////////////////
        // Poll for Events
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }
        //  End of Polling
        //////////////////////////////


        //////////////////////////////
        // Update Game
        ImGui::SFML::Update(window, deltaClock.restart());
        
       
        ImGui::PushFont(f6);
        ImGui::Begin("YAY");
        ImGui::PopFont();
        
        ImGui::PushFont(f8);
        ImGui::Button("Test1");
        ImGui::PopFont();

        ImGui::PushFont(f10);
        ImGui::Button("Test2");
        ImGui::PopFont();

        ImGui::PushFont(f9);
        ImGui::Button("Test3");
        ImGui::PopFont();

        ImGui::PushFont(f1);
        ImGui::Button("Test4");
        ImGui::PopFont();

        ImGui::PushFont(f3);
        ImGui::Button("Test");
        ImGui::PopFont();

        ImGui::PushFont(f5);
        ImGui::Button("Test");
        ImGui::PopFont();

        ImGui::End();


        // End of Update
        ///////////////////////////////

        //////////////////////////////
        // Render Game
        window.clear();
        window.draw(testTxt);
       // window.draw(shape);
        ImGui::SFML::Render(window);
        window.display();
        //  End Of Rendering
        /////////////////////////////
    }
    //  End of Main Loop
    /////////////////////////////////
    
    ////////////////////////////////
    //   Shutdown Game
    ImGui::SFML::Shutdown();
    //  End of Shutdown
    /////////////////////////////////

    return 0;
}

void centerTextH(sf::Text& txt_, uint32_t scrW_)
{
    sf::FloatRect textBounds = txt_.getLocalBounds();

    // Set origin to the center of the text
    txt_.setOrigin(
        { textBounds.position.x + textBounds.size.x / 2.0f,
        txt_.getOrigin().y}
    );

    // Position text at the center of the window
    txt_.setPosition({ (float)scrW_ / 2.0f, txt_.getPosition().y });


}

void centerTextV(sf::Text& txt_, uint32_t scrH_)
{
    sf::FloatRect textBounds = txt_.getLocalBounds();

    // Set origin to the center of the text
    txt_.setOrigin(
        { txt_.getOrigin().x,
        textBounds.position.y + textBounds.size.y / 2.0f }
    );

    // Position text at the center of the window
    txt_.setPosition({ txt_.getPosition().x, (float)scrH_ / 2.0f});


}

void centerText(sf::Text& txt_, const sf::Vector2u& screenSize_)
{
    sf::FloatRect textBounds = txt_.getLocalBounds();

    // Set origin to the center of the text
    txt_.setOrigin(
        { textBounds.position.x + textBounds.size.x / 2.0f,
        textBounds.position.y + textBounds.size.y / 2.0f }
    );

    // Position text at the center of the window
    txt_.setPosition({ screenSize_.x / 2.0f, screenSize_.y / 2.0f });


}
