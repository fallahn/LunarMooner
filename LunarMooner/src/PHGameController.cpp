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

#include <PHGameController.hpp>
#include <ResourceCollection.hpp>
#include <LMCollisionWorld.hpp>
#include <PHOrbitComponent.hpp>
#include <PHPlayerController.hpp>
#include <CommandIds.hpp>

#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

using namespace ph;

namespace
{
    const float boundsOffset = 40.f;
    const float boundsMargin = boundsOffset;

    const sf::Vector2f playerStart(100.f, xy::DefaultSceneSize.y / 2.f);
    const sf::FloatRect playerSize({ -10.f, -10.f }, { 20.f, 20.f });

    const float masterRadius = 140.f; //for start / end planets
}

GameController::GameController(xy::MessageBus& mb, ResourceCollection& rc, xy::Scene& scene, lm::CollisionWorld& cw)
    : xy::Component(mb, this),
    m_resources(rc),
    m_scene(scene),
    m_collisionWorld(cw)
{
    buildScene();
    addMessageHandlers();
}

//public
void GameController::entityUpdate(xy::Entity&, float)
{

}

//private
void GameController::spawnPlayer()
{
    auto controller = xy::Component::create<ph::PlayerController>(getMessageBus());

    auto playerCollision = m_collisionWorld.addComponent(getMessageBus(), playerSize, lm::CollisionComponent::ID::Player);
    lm::CollisionComponent::Callback callback = std::bind(&ph::PlayerController::collisionCallback, controller.get(), std::placeholders::_1);
    playerCollision->setCallback(callback);

    auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), playerSize);

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setSize({ playerSize.width, playerSize.height });
    drawable->getDrawable().setOrigin(playerSize.width / 2.f, playerSize.height / 2.f);
    drawable->getDrawable().setFillColor(sf::Color::Red);

    auto orbit = xy::Component::create<OrbitComponent>(getMessageBus(), playerSize.width / 2.f);
    auto ccOrbit = m_collisionWorld.addComponent(getMessageBus(), playerSize, lm::CollisionComponent::ID::Gravity);
    callback = std::bind(&OrbitComponent::collisionCallback, orbit.get(), std::placeholders::_1);
    ccOrbit->setCallback(callback);

    auto ent = xy::Entity::create(getMessageBus());
    ent->addComponent(controller);
    ent->addComponent(playerCollision);
    ent->addComponent(qtc);
    ent->addComponent(drawable);
    ent->addComponent(orbit);
    ent->addComponent(ccOrbit);
    ent->setPosition(m_spawnPosition);
    m_scene.addEntity(ent, xy::Scene::Layer::FrontMiddle);
}

void GameController::buildScene()
{
    //set bounds
    m_scene.setSize({
        { -(boundsOffset + boundsMargin), -(boundsOffset + boundsMargin) },
        xy::DefaultSceneSize + (sf::Vector2f(boundsOffset + boundsMargin, boundsOffset + boundsMargin) * 2.f) });

    auto ct = m_collisionWorld.addComponent(getMessageBus(),
    { {},{ xy::DefaultSceneSize.x + ((boundsOffset + boundsMargin) * 2.f), boundsOffset } },
        lm::CollisionComponent::ID::Bounds); //top
    auto qt = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), ct->localBounds());

    auto cb = m_collisionWorld.addComponent(getMessageBus(),
    { { 0.f, xy::DefaultSceneSize.y + boundsOffset + (boundsMargin * 2.f) },{ xy::DefaultSceneSize.x + ((boundsOffset + boundsMargin) * 2.f), boundsOffset } },
        lm::CollisionComponent::ID::Bounds); //bottom
    auto qb = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), cb->localBounds());

    auto cl = m_collisionWorld.addComponent(getMessageBus(),
    { { 0.f, boundsOffset },{ boundsOffset, xy::DefaultSceneSize.y + (boundsMargin * 2.f) } },
        lm::CollisionComponent::ID::Bounds); //left
    auto ql = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), cl->localBounds());

    auto cr = m_collisionWorld.addComponent(getMessageBus(),
    { { xy::DefaultSceneSize.x + boundsOffset + (boundsMargin * 2.f), boundsOffset },{ boundsOffset, xy::DefaultSceneSize.y + (boundsMargin * 2.f) } },
        lm::CollisionComponent::ID::Bounds); //right
    auto qr = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), cr->localBounds());

    auto ent = xy::Entity::create(getMessageBus());
    ent->addComponent(ct);
    ent->addComponent(cb);
    ent->addComponent(cl);
    ent->addComponent(cr);
    ent->addComponent(qt);
    ent->addComponent(qb);
    ent->addComponent(ql);
    ent->addComponent(qr);
    ent->setPosition(-(boundsOffset + boundsMargin), -(boundsOffset + boundsMargin));
    m_scene.addEntity(ent, xy::Scene::Layer::BackRear);


    //starting planet
    const float startOffset = masterRadius * 1.6f;
    auto startBody = addBody({ startOffset, xy::Util::Random::value(startOffset, xy::DefaultSceneSize.y - startOffset) }, masterRadius);
    startBody->update(0.f);

    //ending planet
    auto endBody = addBody({ xy::DefaultSceneSize.x - startOffset, xy::Util::Random::value(startOffset, xy::DefaultSceneSize.y - startOffset) }, masterRadius);
    endBody->update(0.f);

    //create intermediate bodies - evenly distribute this by repeating 4 times
    //once for each quadrant of the area
    std::vector<xy::Entity*> bodies;
    for (auto x = 0u; x < 2u; ++x)
    {
        float startX = x * (xy::DefaultSceneSize.x / 2.f);
        float endX = startX + (xy::DefaultSceneSize.x / 2.f);

        for (auto y = 0u; y < 2u; ++y)
        {
            float startY = y * (xy::DefaultSceneSize.y / 2.f);
            float endY = startY + (xy::DefaultSceneSize.y / 2.f);

            for (auto i = 0u; i < 3u; ++i)
            {
                auto body = addBody({ xy::Util::Random::value(startX, endX), xy::Util::Random::value(startY, endY) },
                    xy::Util::Random::value(10.f, 60.f));
                bodies.push_back(body);
            }
        }
    }
    //mvoe and remove overlapping bodies 
    for (auto body : bodies)
    {
        //move bodies in from far edges
        if (body->getWorldPosition().x < startOffset) body->move(startOffset, 0.f);
        if (body->getWorldPosition().x >(xy::DefaultSceneSize.x - startOffset)) body->move(-startOffset, 0.f);

        body->update(0.f); //to correctly build bounds
        auto bounds = body->globalBounds();

        //use this to move away from start / end planets
        std::function<void(xy::Entity*, const xy::Entity*)> checkMaster = [&bounds](xy::Entity* body, const xy::Entity* master)
        {
            const float moveDistance = masterRadius * 4.f;
            if (bounds.intersects(master->globalBounds()))
            {
                if (master->getWorldPosition().y < xy::DefaultSceneSize.y / 2.f)
                {
                    //move the body to bottom half of screen
                    body->move(0.f, moveDistance);
                }
                else
                {
                    body->move(0.f, -moveDistance);
                }
                body->update(0.f); //update bounds
                bounds = body->globalBounds();
            }
        };
        checkMaster(body, startBody);
        checkMaster(body, endBody);

        //move bodies apart
        for (auto other : bodies)
        {
            other->update(0.f);
            if (body != other &&
                bounds.intersects(other->globalBounds()))
            {
                if (body->getWorldPosition().y < xy::DefaultSceneSize.y / 2.f)
                {
                    //move down
                    body->move(0.f, xy::DefaultSceneSize.y / 4.f);
                    if (body->getWorldPosition().y > xy::DefaultSceneSize.y)
                    {
                        body->move(0.f, -(bounds.height * 0.75f));
                    }
                }
                else
                {
                    //move up
                    body->move(0.f, -(xy::DefaultSceneSize.y / 4.f));
                    if (body->getWorldPosition().y < 0)
                    {
                        body->move(0.f, bounds.height * 0.75f);
                    }                  
                }
                body->update(0.f); //recalc bounds after moving
                bounds = body->globalBounds();
            }
        }
    }

    //remove any still overlapping
    for (auto body : bodies)
    {
        body->update(0.f);
        for (auto other : bodies)
        {
            other->update(0.f);
            if (body != other && !other->destroyed() &&
                (body->globalBounds().intersects(other->globalBounds())
                    || body->globalBounds().contains(other->getWorldPosition())))
            {
                body->destroy();
                LOG("Destroyed Body", xy::Logger::Type::Info);
                break;
            }
        }
        if (startBody->globalBounds().intersects(body->globalBounds())
            || endBody->globalBounds().intersects(body->globalBounds()))
        {
            body->destroy();
            LOG("Destroyed Body", xy::Logger::Type::Info);
        }
    }

    //create player
    m_spawnPosition = startBody->getWorldPosition() + sf::Vector2f((masterRadius * 1.4f), 0.f);
    spawnPlayer();
}

xy::Entity* GameController::addBody(const sf::Vector2f& position, float radius)
{
    auto ccSmall = m_collisionWorld.addComponent(getMessageBus(), { { -radius, -radius },{ radius * 2.f, radius * 2.f } }, lm::CollisionComponent::ID::Body);
    
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(getMessageBus());
    drawable->getDrawable().setRadius(radius);
    drawable->getDrawable().setOrigin(radius, radius);

    auto orbit = xy::Component::create<OrbitComponent>(getMessageBus(), radius);
    drawable->getDrawable().setOutlineThickness(orbit->getInfluenceRadius() - radius);
    drawable->getDrawable().setOutlineColor({ 0, 120, 255, 30 });

    const auto influenceRad = orbit->getInfluenceRadius();
    auto ccLarge = m_collisionWorld.addComponent(getMessageBus(), { {-influenceRad, -influenceRad}, {influenceRad * 2.f, influenceRad * 2.f} }, lm::CollisionComponent::ID::Gravity);
    /*auto callback = std::bind(&OrbitComponent::collisionCallback, orbit.get(), std::placeholders::_1);
    ccLarge->setCallback(callback);*/

    auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), ccLarge->localBounds());

    auto ent = xy::Entity::create(getMessageBus());
    ent->addComponent(ccSmall);
    ent->addComponent(drawable);
    ent->addComponent(orbit);
    ent->addComponent(ccLarge);
    ent->addComponent(qtc);
    ent->setPosition(position);

    return m_scene.addEntity(ent, xy::Scene::Layer::FrontMiddle);
}

void GameController::addMessageHandlers()
{
    xy::Component::MessageHandler mh;
    mh.id = GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& data = msg.getData<LMGameEvent>();
        switch (data.type)
        {
        default: break;
        case LMGameEvent::PlayerDied:
            spawnPlayer();
            break;
        }
    };
    addMessageHandler(mh);
}