#include <Game.h>
#include "Globs.h"
#include "misc/util.h"
#include <iostream>
void Game::processEvents()
{}

void Game::update(float dt)
{
    
    
}

void Game::handleKeyEvent(sf::Keyboard::Key key, bool isPressed)
{
    // user let go of the escape key
    if (key == sf::Keyboard::Key::Escape && isPressed == false)
        mWindow.close();
}

void Game::render()
{
    ImGui::SFML::Update(mWindow, sf::seconds(0.016667f));

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

    mWindow.clear();
    mWindow.draw(testTxt);

    ImGui::SFML::Render(mWindow);
    mWindow.display();
}

void Game::resizeBackground()
{}

bool Game::Initialize()
{
    mWindow.create(sf::VideoMode({ glb::WW, glb::WH }), "SFML works!");
    mWindow.setFramerateLimit(60);
    if (!ImGui::SFML::Init(mWindow, false)) throw std::runtime_error("Bad ImGui::Init()");
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    f0 = io.Fonts->AddFontFromFileTTF("assets/fonts/bubbly.ttf", 48.f);
    f1 = io.Fonts->AddFontFromFileTTF("assets/fonts/blue_winter.ttf", 48.f);
    f2 = io.Fonts->AddFontFromFileTTF("assets/fonts/Dino.ttf", 64.f);
    f3 = io.Fonts->AddFontFromFileTTF("assets/fonts/faith.ttf", 48.f);
    f4 = io.Fonts->AddFontFromFileTTF("assets/fonts/hard.otf", 24.f);
    f5 = io.Fonts->AddFontFromFileTTF("assets/fonts/Nova.otf", 24.f);
    f6 = io.Fonts->AddFontFromFileTTF("assets/fonts/Rebel.ttf", 24.f);
    f7 = io.Fonts->AddFontFromFileTTF("assets/fonts/Steel.otf", 16.f);
    f8 = io.Fonts->AddFontFromFileTTF("assets/fonts/Sum.ttf", 16.f);
    f9 = io.Fonts->AddFontFromFileTTF("assets/fonts/Techno.otf", 16.f);
    f10 = io.Fonts->AddFontFromFileTTF("assets/fonts/Venite.ttf", 32.f);
    f11 = io.Fonts->AddFontFromFileTTF("assets/fonts/VeniteStr8.ttf", 32.f);
    f12 = io.Fonts->AddFontFromFileTTF("assets/fonts/Warr.otf", 24.f);

    if (!f0 || !f1 || !f2) throw std::runtime_error("Font load failed");
    io.FontDefault = f0;                    // optional but nice
    io.Fonts->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
    io.Fonts->Build();
    if (!ImGui::SFML::UpdateFontTexture())  // once
        throw std::runtime_error("UpdateFontTexture failed");
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
  
    testTxt.setCharacterSize(44u);
    testTxt.setFillColor(sf::Color::White);
    testTxt.setOutlineColor(sf::Color::Black);
    testTxt.setOutlineThickness(8.f);
    testTxt.setString("Hello ImGui-SFML!");
    testTxt.setPosition({ 100.f,80.f });
    util::centerText(testTxt, { glb::WW,glb::WH });
    std::cout << testTxt.getCharacterSize() * testTxt.getString().getSize() << std::endl;

	return true;
}

void Game::Shutdown()
{
    ImGui::SFML::Shutdown();
}

void Game::Run()
{
    while (mWindow.isOpen())
    {

        //////////////////////////////
        // Poll for Events
        while (const std::optional event = mWindow.pollEvent())
        {
            ImGui::SFML::ProcessEvent(mWindow, *event);
            if (event->is<sf::Event::Closed>())
            {
                mWindow.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                handleKeyEvent(keyPressed->code, true);
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
            {
                handleKeyEvent(keyPressed->code, false);
            }
        }
        static float delta = 0.f;
        delta += mDeltaClock.restart().asSeconds();
        
        bool repaint = false;
        while (delta >= 0.016667f)
        {
            repaint = true;
            delta -= 0.016667f;
            update(0.016667f);
        }

        if (repaint)
            render();
    }
}

Game::Game()
{
}

Game::~Game()
{
}

