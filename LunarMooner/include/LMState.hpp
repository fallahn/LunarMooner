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

#ifndef LUNAR_MOONER_STATE_HPP_
#define LUNAR_MOONER_STATE_HPP_

#include <StateIds.hpp>
#include <ResourceCollection.hpp>
#include <LMCollisionWorld.hpp>
#include <LMPlayerState.hpp>
#include <OLOverlay.hpp>

#include <xygine/State.hpp>
#include <xygine/Scene.hpp>
#include <xygine/mesh/MeshRenderer.hpp>

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>

class PlayerProfile;
class LunarMoonerState final : public xy::State
{
public:
    LunarMoonerState(xy::StateStack&, Context, sf::Uint8, PlayerProfile&);
    ~LunarMoonerState() = default;

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

    xy::StateID stateID() const { return (m_playerCount == 1) ? States::SinglePlayer : States::MultiPlayer; }
private:
    sf::Uint8 m_playerCount;
    xy::Scene m_scene;
    xy::MessageBus& m_messageBus;

    xy::MeshRenderer m_meshRenderer;

    sf::Uint8 m_inputFlags;
    sf::Uint8 m_prevInputFlags;

    ResourceCollection m_resources;

    lm::CollisionWorld m_collisionWorld;
    lm::Overlay m_overlay;

    bool m_useController;
    void parseControllerInput();
    
    void initGameController(sf::Uint8, sf::Uint8, lm::SpecialWeapon);
    void initSounds();
    void initParticles();
    void initMeshes();

    void buildBackground();

    sf::Sprite m_loadingSprite;
    void updateLoadingScreen(float, sf::RenderWindow&) override;
};

#endif //LUNAR_MOONER_STATE_HPP_