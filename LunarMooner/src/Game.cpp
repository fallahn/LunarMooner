/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

LunarMooner - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <Game.hpp>
#include <StateIds.hpp>
#include <MenuMainState.hpp>
#include <MenuOptionState.hpp>
#include <GameOverState.hpp>
#include <MenuPauseState.hpp>
#include <MenuHighScores.hpp>
#include <MenuAchievementState.hpp>
#include <MenuWeaponSelect.hpp>
#include <LMState.hpp>
#include <MenuBackgroundState.hpp>
#include <PlanetHoppingState.hpp>
#include <RockDodgingState.hpp>
#include <IntroState.hpp>
#include <TutorialState.hpp>

#include <xygine/util/String.hpp>

#include <SFML/Window/Event.hpp>


namespace
{
    //as variadic template captures by ref
    //these need to remain here...
    const sf::Uint8 onePlayer = 1;
    const sf::Uint8 twoPlayer = 2;
    //this is rather bad design...
    const bool imFalse = false;
    const bool imTrue = true;
    const xy::StateID sp = States::ID::SinglePlayer;
}

const float Game::MaxVolume = 100.f;

Game::Game()
    : m_stateStack  ({ getRenderWindow(), *this }),
    m_profile       (getMessageBus())
{
    //register a couple of console commands for loading game modes
    xy::Console::addCommand("play", [this](const std::string& param)
    {
        auto p = xy::Util::String::toLower(param);
        if (p == "rescue")
        {
            m_stateStack.clearStates();
            m_stateStack.pushState(States::ID::SinglePlayer);
        }
        else if (p == "orbit")
        {
            m_stateStack.clearStates();
            m_stateStack.pushState(States::ID::PlanetHopping);
        }
        else
        {
            xy::Console::print(param + " not a valid game mode. Game modes are rescue or orbit");
        }
    });
}

//private
void Game::handleEvent(const sf::Event& evt)
{
    /*if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        }
    }*/    
    
    m_stateStack.handleEvent(evt);
}

void Game::handleMessage(const xy::Message& msg)
{
    switch (msg.id)
    {
    case xy::Message::Type::UIMessage:
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        case xy::Message::UIEvent::ResizedWindow:
            m_stateStack.updateView();
            break;
        default: break;
        }
        break;
    }
    default: break;
    }
    
    m_stateStack.handleMessage(msg);
    m_profile.handleMessage(msg);
}

void Game::updateApp(float dt)
{
    m_stateStack.update(dt);
}

void Game::draw()
{
    m_stateStack.draw();
}

void Game::initialise()
{
    registerStates();
#ifdef _DEBUG_
    m_stateStack.pushState(States::ID::MenuBackground);
#else
    m_stateStack.pushState(States::ID::Intro);
#endif //_DEBUG_
    getRenderWindow().setKeyRepeatEnabled(false);

    m_profile.load();
}

void Game::finalise()
{
    m_stateStack.clearStates();
    m_stateStack.applyPendingChanges();

    m_profile.save();
}

void Game::registerStates()
{
    m_stateStack.registerState<MenuMainState>(States::ID::MenuMain, m_menuTextures, m_menuFonts);
    m_stateStack.registerState<MenuOptionState>(States::ID::MenuOptions, m_menuTextures, m_menuFonts);
    m_stateStack.registerState<MenuOptionState>(States::ID::PausedOptions, m_menuTextures, m_menuFonts, imTrue);
    m_stateStack.registerState<LunarMoonerState>(States::ID::SinglePlayer, onePlayer, m_profile);
    m_stateStack.registerState<LunarMoonerState>(States::ID::MultiPlayer, twoPlayer, m_profile);
    m_stateStack.registerState<GameOverState>(States::ID::GameOver, m_menuTextures, m_menuFonts);
    m_stateStack.registerState<MenuPauseState>(States::ID::Pause, m_menuTextures, m_menuFonts);
    m_stateStack.registerState<MenuHighScoreState>(States::ID::HighScoresMenu, m_menuTextures, m_menuFonts);
    m_stateStack.registerState<MenuHighScoreState>(States::ID::HighScoresEnd, m_menuTextures, m_menuFonts, imTrue);
    m_stateStack.registerState<MenuBackgroundState>(States::ID::MenuBackground);
    m_stateStack.registerState<MenuAchievementState>(States::ID::MenuAchievement, m_menuTextures, m_menuFonts, m_profile);
    m_stateStack.registerState<MenuWeaponState>(States::ID::MenuWeapon, m_menuTextures, m_menuFonts, m_profile);
    m_stateStack.registerState<PlanetHoppingState>(States::PlanetHopping);
    m_stateStack.registerState<RockDodgingState>(States::RockDodging);
    m_stateStack.registerState<IntroState>(States::ID::Intro);
    m_stateStack.registerState<TutorialState>(States::ID::Tutorial, sp, m_menuFonts);
}
