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

#ifndef LM_ROCK_DODGER_HPP_
#define LM_ROCK_DODGER_HPP_

#include <StateIds.hpp>
#include <ResourceCollection.hpp>
#include <LMCollisionWorld.hpp>

#include <xygine/State.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Graphics/Sprite.hpp>

class RockDodgingState final : public xy::State
{
public:
    RockDodgingState(xy::StateStack&, Context);
    ~RockDodgingState() = default;

    bool update(float) override;
    void draw() override;
    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;

    xy::StateID stateID() const override { return States::RockDodging; }

private:
    xy::MessageBus& m_messageBus;
    xy::Scene m_scene;
    ResourceCollection m_resources;
    lm::CollisionWorld m_collisionWorld;

    void initGameController();
    
    sf::Sprite m_loadingSprite;
    void updateLoadingScreen(float, sf::RenderWindow&) override;
};

#endif //LM_ROCK_DODGER_HPP_