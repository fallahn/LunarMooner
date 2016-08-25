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
#include <PHPlayerController.hpp>
#include <PHOrbitComponent.hpp>
#include <CommandIds.hpp>

#include <xygine/App.hpp>
#include <xygine/components/SoundPlayer.hpp>
#include <xygine/components/ParticleController.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

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
    //m_scene.drawDebug(true);
    //TODO post processes

    buildScene();

    tmpTxt.setFont(m_resources.fontResource.get("temptext"));
    tmpTxt.setFillColor(sf::Color(120, 120, 255));
    tmpTxt.setString("Space: Launch from orbit or respawn - A/D: Steer");
    tmpTxt.setPosition(10.f, 10.f);
    tmpTxt.setCharacterSize(16);

    quitLoadingScreen();
}

//public
bool PlanetHoppingState::update(float dt)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        xy::Command cmd;
        cmd.category = LMCommandID::Player;
        cmd.action = [](xy::Entity& entity, float)
        {
            entity.getComponent<ph::PlayerController>()->moveLeft();
        };
        m_scene.sendCommand(cmd);
    }
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        xy::Command cmd;
        cmd.category = LMCommandID::Player;
        cmd.action = [](xy::Entity& entity, float)
        {
            entity.getComponent<ph::PlayerController>()->moveRight();
        };
        m_scene.sendCommand(cmd);
    }

    m_collisionWorld.update();
    m_scene.update(dt);
    
    return false;
}

bool PlanetHoppingState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default:break;
        case sf::Keyboard::Space:
        {
            xy::Command cmd;
            cmd.category = LMCommandID::Player;
            cmd.action = [](xy::Entity& entity, float)
            {
                entity.getComponent<ph::PlayerController>()->leaveOrbit(entity.getComponent<ph::OrbitComponent>()->removeParent());
            };
            m_scene.sendCommand(cmd);

            //do this second so we don't launch the player on spawn
            cmd.category = LMCommandID::GameController;
            cmd.action = [](xy::Entity& entity, float)
            {
                entity.getComponent<ph::GameController>()->spawnPlayer();
            };
            m_scene.sendCommand(cmd);
        }
            break;
        case sf::Keyboard::BackSpace:
            /*requestStackPop();
            requestStackPush(States::PlanetHopping);*/
            break;
        }
    }

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

    rw.setView(rw.getDefaultView());
    rw.draw(tmpTxt);
}

//private
void PlanetHoppingState::buildScene()
{
    auto gameController = xy::Component::create<ph::GameController>(m_messageBus, m_resources, m_scene, m_collisionWorld);
    //auto soundPlayer = xy::Component::create<xy::SoundPlayer>(m_messageBus, m_resources.soundResource);


    //TODO move particle init to own function
    auto particleController = xy::Component::create<xy::ParticleController>(m_messageBus);
    xy::Component::MessageHandler handler;
    handler.id = LMMessageId::GameEvent;
    handler.action = [](xy::Component* c, const xy::Message& msg)
    {
        auto component = dynamic_cast<xy::ParticleController*>(c);
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::PlayerDied:
            component->fire(LMParticleID::LargeExplosion, { msgData.posX, msgData.posY });
            component->fire(LMParticleID::SmallExplosion, { msgData.posX, msgData.posY });
            break;
        }
    };
    particleController->addMessageHandler(handler);

    auto entity = xy::Entity::create(m_messageBus);
    auto pc = entity->addComponent(particleController);
    xy::ParticleSystem::Definition pd;
    pd.loadFromFile("assets/particles/small_explosion.xyp", m_resources.textureResource);
    pc->addDefinition(LMParticleID::SmallExplosion, pd);

    pd.loadFromFile("assets/particles/large_explosion.xyp", m_resources.textureResource);
    pc->addDefinition(LMParticleID::LargeExplosion, pd);
    
    entity->addComponent(gameController);
    entity->addCommandCategories(LMCommandID::GameController);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void PlanetHoppingState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}