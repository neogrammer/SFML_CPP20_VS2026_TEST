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
    delta = 0.f;


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

    stateMap[eStateID::Splash] = SplashState{};
    stateMap[eStateID::Title] = TitleState{};
    stateMap[eStateID::Play] = PlayState{};

    currState = &stateMap[eStateID::Splash];
    std::visit([this](auto& s) {
        s.enter();
        }, *currState);

    switchTo = eStateID::None;



    return true;
}

void Game::Shutdown()
{
    std::visit([this](auto& s) {
        s.leave();
        }, *currState);

    ImGui::SFML::Shutdown();
}

void Game::Run()
{
    sf::Clock secondaryClock;
    float secondaryTime{ 0.f };
    sf::Clock finalClock;

    secondaryClock.restart();
    finalClock.restart();
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
                std::visit([&](auto& s) {
                    s.handleKeyEvent(keyPressed->code, true);
                    }, *currState);
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyReleased>())
            {
                std::visit([&](auto& s) {
                    s.handleKeyEvent(keyPressed->code, false);
                    }, *currState);

                if (keyPressed->code == sf::Keyboard::Key::Escape)
                    mWindow.close();
            }
        }
        delta += mDeltaClock.restart().asSeconds();




        bool repaint = false;
        secondaryClock.restart();
        while (delta >= 0.016667f)
        {
            repaint = true;
            delta -= 0.016667f;
            if (currState) {
                switchTo = std::visit([this](auto& s) -> eStateID {
                    return s.update(0.016667f);
                    }, *currState);
            }
            if (switchTo != eStateID::None) break;
        }

        if (repaint && switchTo == eStateID::None)
        {
            if (currState) {
                secondaryTime = secondaryClock.getElapsedTime().asSeconds();
                finalClock.restart();
                while (secondaryTime > 0.016667f)
                {
                    secondaryTime -= 0.016667f;
                }

                switchTo = std::visit([this, secondaryTime](auto& s) -> eStateID {
                    return s.update(delta + secondaryTime);
                    }, *currState);
                delta = std::clamp(finalClock.getElapsedTime().asSeconds(), 0.f, 0.016667f);
            }
        }

        // Check if switching state
        if (switchTo != eStateID::None && switchTo != eStateID::Count)
        {
            if (currState)
            {
                // 's' automatically becomes the active type (e.g., SplashState)
                std::visit([this](auto& s) {
                    s.leave();
                    }, *currState);

                currState = &stateMap[switchTo];

                std::visit([this](auto& s) {
                    s.enter();
                    }, *currState);
            }
            switchTo = eStateID::None;
        }

        

        if (repaint)
        {
            mWindow.clear();
            if (currState) {
                std::visit([this](auto& s) {
                    s.render(mWindow);
                    }, *currState);
            }
            mWindow.display();
        }
    }
}

Game::Game()
{
}

Game::~Game()
{
}

