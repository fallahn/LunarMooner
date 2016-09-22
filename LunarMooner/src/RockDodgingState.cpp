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

#include <RockDodgingState.hpp>
#include <RDGameController.hpp>
#include <RDPlayerController.hpp>
#include <CommandIds.hpp>
#include <GameMessage.hpp>
#include <LMBulletController.hpp>
#include <LMPostBlur.hpp>

#include <xygine/App.hpp>
#include <xygine/KeyBinds.hpp>
#include <xygine/Command.hpp>
#include <xygine/PostChromeAb.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
    rd::GameController* gameController = nullptr;
}

RockDodgingState::RockDodgingState(xy::StateStack& stack, Context context)
    : xy::State         (stack, context),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_scene             (m_messageBus),
    m_collisionWorld    (m_scene)
{
    m_loadingSprite.setTexture(m_resources.textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();

    m_scene.setView(context.defaultView);
    //post processes
    auto pp = xy::PostProcess::create<lm::PostBlur>();
    m_scene.addPostProcess(pp);
    pp = xy::PostProcess::create<xy::PostChromeAb>();
    m_scene.addPostProcess(pp);

    //m_scene.drawDebug(true);

    //TODO cache resources
    initGameController();
    
    quitLoadingScreen();
}

//public
bool RockDodgingState::update(float dt)
{
    m_collisionWorld.update();
    m_scene.update(dt);
    
    return false;
}

bool RockDodgingState::handleEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyPressed)
    {
        if (evt.key.code == xy::Input::getKey(xy::Input::ActionOne))
        {
            xy::Command cmd;
            cmd.category = LMCommandID::Player;
            cmd.action = [](xy::Entity& entity, float)
            {
                entity.getComponent<rd::PlayerController>()->activate();

                auto bounds = entity.getComponent<xy::QuadTreeComponent>()->localBounds();
                sf::Vector2f offset(bounds.width / 2.f, bounds.height / 2.f);
                gameController->fireLaser(entity.getWorldPosition() + offset);
            };
            m_scene.sendCommand(cmd);

            cmd.category = LMCommandID::UI;
            cmd.action = [](xy::Entity& entity, float)
            {
                if (auto c = entity.getComponent<GameMessage>())
                {
                    c->clear();
                }
            };
            m_scene.sendCommand(cmd);

            if (gameController->gameEnded())
            {
                requestStackPop();
                requestStackPush(States::ID::SinglePlayer);
            }
        }

        //handle other inputs
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
        case sf::Keyboard::Escape:
            requestStackPush(States::ID::Pause);
            break;
        }

    }
    else if (evt.type == sf::Event::KeyReleased)
    {

    }

    //TODO controller input

    return false;
}

void RockDodgingState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

void RockDodgingState::draw()
{
    auto& rt = getContext().renderWindow;
    rt.draw(m_scene);
    rt.setView(getContext().defaultView);
}

//private
void RockDodgingState::initGameController()
{
    auto gc = xy::Component::create<rd::GameController>(m_messageBus, m_scene, m_resources, m_collisionWorld);
    auto entity = xy::Entity::create(m_messageBus);
    gameController = entity->addComponent(gc);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void RockDodgingState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}