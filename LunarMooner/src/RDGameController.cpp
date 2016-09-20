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

#include <RDGameController.hpp>
#include <RDPlayerController.hpp>
#include <LMCollisionWorld.hpp>
#include <CommandIds.hpp>
#include <GameMessage.hpp>
#include <RDDistanceDrawable.hpp>
#include <LMBulletController.hpp>
#include <BGStarfield.hpp>

#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Command.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/util/Random.hpp>

#include<SFML/Graphics/RectangleShape.hpp>

using namespace rd;

namespace
{
    const sf::FloatRect playerSize(0.f, 0.f, 120.f, 80.f);
    const sf::Vector2f spawnPosition(-playerSize.width, (xy::DefaultSceneSize.y - playerSize.height) / 2.f);
    const float roundTime = 10.f;

    DistanceMeter* distanceMeter = nullptr;

    const float minSpawn = 0.5f;
    const float maxSpawn = 1.f;

    const sf::Vector2f bulletSize(80.f, 4.f);

    lm::Starfield* stars = nullptr;
}

GameController::GameController(xy::MessageBus& mb, xy::Scene& scene, ResourceCollection& rc, lm::CollisionWorld& cw)
    : xy::Component (mb, this),
    m_scene         (scene),
    m_resources     (rc),
    m_collisionWorld(cw),
    m_rockPool      (m_collisionWorld, m_scene, mb),
    m_remainingTime (roundTime),
    m_roundStarted  (false),
    m_spawnTime     (xy::Util::Random::value(minSpawn, maxSpawn))
{
    buildBackground();
    
    spawnPlayer();

    //add a message handler to start timing when round started
    xy::Component::MessageHandler mh;
    mh.id = StateEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<LMStateEvent>();
        switch (data.type)
        {
        default: break;
        case LMStateEvent::RoundBegin:
            m_roundStarted = true;
            break;
        case LMStateEvent::RoundEnd:
            m_roundStarted = false;

            {
                auto gameMessage = xy::Component::create<GameMessage>(getMessageBus(), m_resources.fontResource.get("round end"), "Destination reached...", 58u);
                auto entity = xy::Entity::create(getMessageBus());
                entity->setPosition(xy::DefaultSceneSize / 2.f);
                entity->addComponent(gameMessage);
                entity->addCommandCategories(LMCommandID::UI);

                gameMessage = xy::Component::create<GameMessage>(getMessageBus(), m_resources.fontResource.get("round end"), "Press Fire to Continue", 38u);
                auto subEnt = xy::Entity::create(getMessageBus());
                subEnt->move(0.f, 52.f);
                subEnt->addComponent(gameMessage);
                entity->addChild(subEnt);

                m_scene.addEntity(entity, xy::Scene::Layer::UI);
            }

            break;
        }
    };
    addMessageHandler(mh);

    mh.id = GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<LMGameEvent>();
        switch (data.type)
        {
        default: break;
        case LMGameEvent::PlayerLanded:
            if (!m_roundStarted)
            {
                auto gameMessage = xy::Component::create<GameMessage>(getMessageBus(), m_resources.fontResource.get("round start"), "Press Fire To Begin!", 60u);
                auto entity = xy::Entity::create(getMessageBus());
                entity->setPosition(xy::DefaultSceneSize / 2.f);
                entity->addComponent(gameMessage);
                entity->addCommandCategories(LMCommandID::UI);
                m_scene.addEntity(entity, xy::Scene::Layer::UI);
            }
            else
            {
                //go straight in to playing again
                xy::Command cmd;
                cmd.category = LMCommandID::Player;
                cmd.action = [](xy::Entity& entity, float)
                {
                    entity.getComponent<rd::PlayerController>()->activate();
                };
                m_scene.sendCommand(cmd);
            }
            break;
        }
    };
    addMessageHandler(mh);

    //create distance meter
    auto dm = xy::Component::create<DistanceMeter>(getMessageBus());
    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(0.f, xy::DefaultSceneSize.y - 66.f);
    distanceMeter = entity->addComponent(dm);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
}

//public
void GameController::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_roundStarted)
    {
        //update UI
        float dist = 1.f - (m_remainingTime / roundTime);
        distanceMeter->setDistance(dist);
        
        //update round time
        auto time = m_remainingTime;
        m_remainingTime -= dt;
        if (time > 0 && m_remainingTime < 0)
        {
            xy::Command cmd;
            cmd.category = LMCommandID::Player;
            cmd.action = [](xy::Entity& entity, float)
            {
                entity.getComponent<PlayerController>()->end();
            };
            m_scene.sendCommand(cmd);
        }

        //check for random spawn time
        m_spawnTime -= dt;
        if (m_spawnTime < 0)
        {
            m_spawnTime = xy::Util::Random::value(minSpawn, maxSpawn);
            if (xy::Util::Random::value(0, 6) < 2)
            {
                spawnDoofer();
            }
            else
            {
                spawnRock();
            }
        }
    }
    
    if(!m_roundStarted || m_remainingTime < 0)
    {
        //adjust the background speed relative to the player ship
        xy::Command cmd;
        cmd.category = LMCommandID::Player;
        cmd.action = [](xy::Entity& entity, float)
        {
            stars->setSpeedRatio(entity.getComponent<PlayerController>()->getSpeedRatio() + 0.5f);
        };
        m_scene.sendCommand(cmd);
    }
}

void GameController::fireLaser(const sf::Vector2f& position)
{
    if (!m_roundStarted) return;

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setSize(bulletSize);
    drawable->getDrawable().setFillColor(sf::Color(90u, 255u, 220u, 190u));
    drawable->getDrawable().setOutlineColor(sf::Color(0u, 185u, 140u, 100u));
    drawable->getDrawable().setOutlineThickness(2.f);
    drawable->setBlendMode(sf::BlendAdd);

    auto controller = xy::Component::create<lm::BulletController>(getMessageBus(), LMDirection::Left);
    controller->setSpeed(3200.f);

    auto collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f }, bulletSize }, lm::CollisionComponent::ID::Bullet, true);
    lm::CollisionComponent::Callback cb = std::bind(&lm::BulletController::collisionCallback, controller.get(), std::placeholders::_1);
    collision->setCallback(cb);

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(position);
    entity->setOrigin(bulletSize / 2.f);
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    //raise message
    auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
    msg->type = LMGameEvent::LaserFired;
    msg->posX = position.x;
    msg->posY = position.y;
}

//private
void GameController::buildBackground()
{
    auto starfield = xy::Component::create<lm::Starfield>(getMessageBus(), m_resources.textureResource);
    //starfield->setSpeedRatio(6.f);
    auto entity = xy::Entity::create(getMessageBus());
    stars = entity->addComponent(starfield);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void GameController::spawnPlayer()
{
    auto controller = xy::Component::create<PlayerController>(getMessageBus());

    auto playerCollision = m_collisionWorld.addComponent(getMessageBus(), playerSize, lm::CollisionComponent::ID::Player, true);
    lm::CollisionComponent::Callback callback = std::bind(&rd::PlayerController::collisionCallback, controller.get(), std::placeholders::_1);
    playerCollision->setCallback(callback);

    auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), playerSize);

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setSize({ playerSize.width, playerSize.height });

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(controller);
    entity->addComponent(playerCollision);
    entity->addComponent(qtc);
    entity->addComponent(drawable);
    entity->setWorldPosition(spawnPosition);
    entity->addCommandCategories(LMCommandID::Player);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    auto msg = sendMessage<LMGameEvent>(GameEvent);
    msg->type = LMGameEvent::PlayerSpawned;
    msg->posX = spawnPosition.x;
    msg->posY = spawnPosition.y;
}

void GameController::spawnRock()
{
    m_rockPool.spawn();
}

void GameController::spawnDoofer()
{

}