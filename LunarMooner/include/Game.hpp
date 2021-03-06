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

#ifndef GAME_HPP_
#define GAME_HPP_

#include <PlayerProfile.hpp>

#include <xygine/App.hpp>
#include <xygine/Resource.hpp>

class Game final : public xy::App
{
public:
    Game();
    ~Game() = default;
    Game(const Game&) = delete;
    Game& operator = (const Game&) = delete;

    static const float MaxVolume;

private:

    xy::StateStack m_stateStack;
    PlayerProfile m_profile;
    
    //these are shared by menu states
    //to speed up times between switching menus
    xy::TextureResource m_menuTextures;
    xy::FontResource m_menuFonts;

    void handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;

    void registerStates() override;
    void updateApp(float dt) override;
    void draw() override;

    void initialise() override;
    void finalise() override;
};


#endif //GAME_HPP_