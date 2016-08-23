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

#include <PlanetHoppingState.hpp>
#include <PHGameController.hpp>

#include <xygine/App.hpp>
#include <xygine/components/SoundPlayer.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

PlanetHoppingState::PlanetHoppingState(xy::StateStack& stack, Context context)
    : xy::State     (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_collisionWorld(m_scene)
{
    m_loadingSprite.setTexture(m_resources.textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();

    m_scene.setView(context.defaultView);
    m_scene.drawDebug(true);
    //TODO post processes

    buildScene();

    quitLoadingScreen();
}

//public
bool PlanetHoppingState::update(float dt)
{
    m_collisionWorld.update();
    m_scene.update(dt);
    
    return false;
}

bool PlanetHoppingState::handleEvent(const sf::Event& evt)
{
    return false;
}

void PlanetHoppingState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& data = msg.getData<xy::Message::UIEvent>();
        switch (data.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            m_scene.setView(getContext().defaultView);
            break;
        }
    }
}

void PlanetHoppingState::draw()
{
    auto& rw = getContext().renderWindow;

    rw.draw(m_scene);
}

//private
void PlanetHoppingState::buildScene()
{
    auto gameController = xy::Component::create<ph::GameController>(m_messageBus, m_resources, m_scene, m_collisionWorld);
    auto soundPlayer = xy::Component::create<xy::SoundPlayer>(m_messageBus, m_resources.soundResource);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(gameController);
    entity->addComponent(soundPlayer);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void PlanetHoppingState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}