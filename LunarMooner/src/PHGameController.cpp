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
#include <LMAlienController.hpp>
#include <PHPlanetRotation.hpp>
#include <LMShaderIds.hpp>
#include <GameMessage.hpp>
#include <PHHeadsUp.hpp>
#include <PHBeamDrawable.hpp>
#include <PHHumanController.hpp>

#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/SpriteBatch.hpp>
#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/CircleShape.hpp>

using namespace ph;

namespace
{
    //changing these affects wrapOffset in PHPlayerController
    const float boundsOffset = 40.f;
    const float boundsMargin = (boundsOffset * 4.f);

    const sf::Vector2f playerStart(100.f, xy::DefaultSceneSize.y / 2.f);
    const sf::FloatRect playerSize({ -10.f, -10.f }, { 20.f, 20.f });

    const float masterRadius = 100.f; //for start / end planets
    const float minBodySize = 20.f;
    const float maxBodySize = 30.f;

    const std::array<sf::FloatRect, 4u> debrisSizes =
    {
        sf::FloatRect(0.f, 0.f, 40.f, 38.f),
        { 40.f, 0.f, 56.f, 60.f },
        { 96.f, 0.f, 28.f, 32.f },
        { 124.f, 0.f, 36.f, 52.f }
    };
    const std::size_t debrisCount = 8u;

    const float targetOrbitTime = 2.f; //time in seconds to orbit final moon for, to get a round end

    HeadsUpDisplay* hud = nullptr;
}

GameController::GameController(xy::MessageBus& mb, ResourceCollection& rc, xy::Scene& scene, lm::CollisionWorld& cw, xy::MeshRenderer& mr)
    : xy::Component     (mb, this),
    m_resources         (rc),
    m_scene             (scene),
    m_collisionWorld    (cw),
    m_meshRenderer      (mr),
    m_playerSpawned     (false),
    m_targetUID         (0u),
    m_currentParent     (0u),
    m_lastOrbitTime     (0.f),
    m_gameEnded         (false)
{
    buildScene();
    spawnDebris();
    buildUI();
    addMessageHandlers();
}

//public
void GameController::entityUpdate(xy::Entity&, float)
{
    //REPORT("Current Parent", std::to_string(m_currentParent));
    //REPORT("Target", std::to_string(m_targetUID));
    
    //check we're in orbit around target
    if (m_currentParent == m_targetUID)
    {
        const float time = m_targetClock.getElapsedTime().asSeconds();
        if (m_lastOrbitTime < targetOrbitTime
            && time > targetOrbitTime)
        {
            showMessage("Success!", "Press Fire to Continue");
            m_gameEnded = true;

            auto msg = sendMessage<LMStateEvent>(StateEvent);
            msg->type = LMStateEvent::RoundEnd;
            msg->stateID = States::ID::PlanetHopping;
            XY_ASSERT(hud, "HUD pointer not assigned");
            msg->value = static_cast<sf::Int32>(hud->getTime());


        }
        m_lastOrbitTime = time;
    }
}

void GameController::spawnPlayer()
{
    if (m_playerSpawned) return;
    
    auto controller = xy::Component::create<ph::PlayerController>(getMessageBus());

    auto playerCollision = m_collisionWorld.addComponent(getMessageBus(), playerSize, lm::CollisionComponent::ID::Player, true);
    lm::CollisionComponent::Callback callback = std::bind(&ph::PlayerController::collisionCallback, controller.get(), std::placeholders::_1);
    playerCollision->setCallback(callback);

    auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), playerSize);

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(getMessageBus());
    drawable->getDrawable().setRadius(playerSize.width / 2.f);
    drawable->getDrawable().setOrigin(playerSize.width / 2.f, playerSize.height / 2.f);
    drawable->getDrawable().setFillColor(sf::Color::Red);

    auto orbit = xy::Component::create<OrbitComponent>(getMessageBus(), playerSize.width / 2.f);
    auto ccOrbit = m_collisionWorld.addComponent(getMessageBus(), playerSize, lm::CollisionComponent::ID::Gravity, true);
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
    ent->addCommandCategories(LMCommandID::Player);
    m_scene.addEntity(ent, xy::Scene::Layer::FrontMiddle);

    m_playerSpawned = true;

    auto msg = sendMessage<LMGameEvent>(LMMessageId::GameEvent);
    msg->type = LMGameEvent::PlayerSpawned;
    msg->posX = m_spawnPosition.x;
    msg->posY = m_spawnPosition.y;
}

bool GameController::gameEnded() const
{
    return m_gameEnded;
}

//private
void GameController::buildScene()
{
    //set bounds
    m_scene.setSize({
        { -(boundsOffset + boundsMargin), -(boundsOffset + boundsMargin) },
        xy::DefaultSceneSize + (sf::Vector2f(boundsOffset + boundsMargin, boundsOffset + boundsMargin) * 2.f) });

    auto ct = m_collisionWorld.addComponent(getMessageBus(),
    { {},{ xy::DefaultSceneSize.x + ((boundsOffset + boundsMargin) * 2.f), boundsOffset } },
        lm::CollisionComponent::ID::Bounds, true); //top
    auto qt = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), ct->localBounds());

    auto cb = m_collisionWorld.addComponent(getMessageBus(),
    { { 0.f, xy::DefaultSceneSize.y + boundsOffset + (boundsMargin * 2.f) },{ xy::DefaultSceneSize.x + ((boundsOffset + boundsMargin) * 2.f), boundsOffset } },
        lm::CollisionComponent::ID::Bounds, true); //bottom
    auto qb = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), cb->localBounds());

    auto cl = m_collisionWorld.addComponent(getMessageBus(),
    { { 0.f, boundsOffset },{ boundsOffset, xy::DefaultSceneSize.y + (boundsMargin * 2.f) } },
        lm::CollisionComponent::ID::Bounds, true); //left
    auto ql = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), cl->localBounds());

    auto cr = m_collisionWorld.addComponent(getMessageBus(),
    { { xy::DefaultSceneSize.x + boundsOffset + (boundsMargin * 2.f), boundsOffset },{ boundsOffset, xy::DefaultSceneSize.y + (boundsMargin * 2.f) } },
        lm::CollisionComponent::ID::Bounds, true); //right
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
    m_targetUID = endBody->getUID();
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
                    xy::Util::Random::value(minBodySize, maxBodySize));
                bodies.push_back(body);
            }
        }
    }
    //move and remove overlapping bodies 
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
    auto bodyCount = bodies.size();
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
                body->setPosition(-1000.f, -1000.f);
                body->addCommandCategories(LMCommandID::TerrainObject); //use this as for delayed destroy command
                bodyCount--;
                //LOG("Destroyed Body", xy::Logger::Type::Info);
                break;
            }
        }
        if (startBody->globalBounds().intersects(body->globalBounds())
            || endBody->globalBounds().intersects(body->globalBounds()))
        {
            body->setPosition(-1000.f, -1000.f);
            body->addCommandCategories(LMCommandID::TerrainObject);
            bodyCount--;
            //LOG("Destroyed Body", xy::Logger::Type::Info);
        }
    }
    m_collisionWorld.flush();

    //spawn a few doofers
    std::size_t dooferCount = 0;
    for (auto b : bodies)
    {
        if (xy::Util::Random::value(0, 2) == 0 && b->getWorldPosition().y > 0)
        {
            auto drawable = xy::Component::create<xy::AnimatedDrawable>(getMessageBus(), m_resources.textureResource.get("assets/images/game/doofer_01.png"));
            drawable->loadAnimationData("assets/images/game/doofer_01.xya");
            drawable->playAnimation(1);
            drawable->setOrigin(drawable->getFrameSize().x / 2.f, static_cast<float>(drawable->getFrameSize().y));
            
            auto controller = xy::Component::create<HumanController>(getMessageBus(), *drawable);
            
            b->addComponent(drawable);
            b->addComponent(controller);
            
            m_populatedPlanetIDs.push_back(b->getUID());
            //LOG("Spawned Doofer", xy::Logger::Type::Info);
            dooferCount++;
        }
    }

    auto msg = sendMessage<LMStateEvent>(StateEvent);
    msg->stateID = States::ID::PlanetHopping;
    msg->type = LMStateEvent::GameStart;
    msg->value = (dooferCount << 16) | bodyCount;

    //setup player
    m_spawnPosition = startBody->getWorldPosition() + sf::Vector2f((masterRadius * 1.6f), 0.f);

    //show a message
    showMessage("Get To The Next Moon!", "Press Fire to Start");
}

xy::Entity* GameController::addBody(const sf::Vector2f& position, float radius)
{
    auto ccSmall = m_collisionWorld.addComponent(getMessageBus(), { { -radius, -radius },{ radius * 2.f, radius * 2.f } }, lm::CollisionComponent::ID::Body);
    
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(getMessageBus());
    drawable->getDrawable().setRadius(radius);
    drawable->getDrawable().setOrigin(radius, radius);
    drawable->getDrawable().setFillColor(sf::Color::Transparent);

    auto orbit = xy::Component::create<OrbitComponent>(getMessageBus(), radius);
    drawable->getDrawable().setOutlineThickness(orbit->getInfluenceRadius() - radius);
    drawable->getDrawable().setOutlineColor({ 0, 12, 155, 10 });

    const auto influenceRad = orbit->getInfluenceRadius();
    auto ccLarge = m_collisionWorld.addComponent(getMessageBus(), { {-influenceRad, -influenceRad}, {influenceRad * 2.f, influenceRad * 2.f} }, lm::CollisionComponent::ID::Gravity, true);

    auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), ccLarge->localBounds());

    auto model = m_meshRenderer.createModel(Mesh::Moon, getMessageBus());
    model->setScale({ radius, radius, radius });
    model->setRotation({ xy::Util::Random::value(1.f, 360.f), xy::Util::Random::value(1.f, 360.f), xy::Util::Random::value(1.f, 360.f) });
    model->setBaseMaterial(m_resources.materialResource.get(radius > 50 ? Material::Moon : Material::DesertPlanet));

    auto rotator = xy::Component::create<PlanetRotation>(getMessageBus());

    auto ent = xy::Entity::create(getMessageBus());
    ent->addComponent(ccSmall);
    ent->addComponent(drawable);
    ent->addComponent(orbit);
    ent->addComponent(ccLarge);
    ent->addComponent(qtc);
    ent->addComponent(model);
    ent->addComponent(rotator);
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
        case LMGameEvent::TimerExpired:
            m_gameEnded = true;
            showMessage("Time's Up!", "Press Fire to Continue");
            break;
        case LMGameEvent::PlayerDied:
            if (!m_gameEnded)
            {
                showMessage("You Died", "Press Fire to Continue");
            }
            m_playerSpawned = false;
            m_currentParent = 0;           
            break;
        case LMGameEvent::PlayerSpawned:
            //clean up scene
        {
            xy::Command cmd;
            cmd.category = LMCommandID::TerrainObject;
            cmd.action = [](xy::Entity& entity, float) {entity.destroy(); };
            m_scene.sendCommand(cmd);
        }
            break;
        case LMGameEvent::EnteredOrbit:
            m_currentParent = data.value;
            if (m_currentParent == m_targetUID)
            {
                //we're in orbit around the target planet!
                m_targetClock.restart();
            }

            /*
            check if planet has doofer
            if it does send message to player 
            */
            if(std::find(m_populatedPlanetIDs.begin(), m_populatedPlanetIDs.end(), data.value) != m_populatedPlanetIDs.end())
            {
                xy::Command cmd;
                cmd.category = LMCommandID::Player;
                cmd.action = [this](xy::Entity& entity, float dt) 
                {
                    auto beam = xy::Component::create<BeamDrawable>(getMessageBus());
                    entity.addComponent(beam);
                };
                m_scene.sendCommand(cmd);
            }
            break;
        case LMGameEvent::LeftOrbit:
            m_currentParent = 0;
            
            break;
        case LMGameEvent::HumanRescued:
            //remove from populated planets
            auto it = std::find(std::begin(m_populatedPlanetIDs), std::end(m_populatedPlanetIDs), m_currentParent);
            if (it != m_populatedPlanetIDs.end())
            {
                m_populatedPlanetIDs.erase(it);
            }
            //display a little message
            hud->addTag("+XP", { data.posX, data.posY });
            break;
        }
    };
    addMessageHandler(mh);
}

void GameController::spawnDebris()
{
    auto sb = xy::Component::create<xy::SpriteBatch>(getMessageBus());
    sb->setTexture(&m_resources.textureResource.get("assets/images/game/debris_large.png"));
    auto entity = xy::Entity::create(getMessageBus());
    auto* spriteBatch = entity->addComponent(sb);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    std::function<void(const sf::Vector2f&)> spawn = [this, spriteBatch](const sf::Vector2f& position)
    {
        auto size = debrisSizes[xy::Util::Random::value(0, debrisSizes.size() - 1)];

        auto drawable = spriteBatch->addSprite(getMessageBus());
        drawable->setTextureRect(size);

        auto controller = xy::Component::create<lm::AlienController>(getMessageBus(), sf::FloatRect({}, xy::DefaultSceneSize));

        auto collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ size.width, size.height } }, lm::CollisionComponent::ID::Alien);
        lm::CollisionComponent::Callback cb = std::bind(&lm::AlienController::collisionCallback, controller.get(), std::placeholders::_1);
        collision->setCallback(cb);

        auto qtc = xy::Component::create<xy::QuadTreeComponent>(getMessageBus(), sf::FloatRect({ 0.f, 0.f }, { size.width, size.height }));

        auto entity = xy::Entity::create(getMessageBus());
        entity->addComponent(drawable);
        entity->addComponent(controller);
        entity->addComponent(collision);
        entity->addComponent(qtc);
        entity->setPosition(position);
        entity->addCommandCategories(LMCommandID::Alien);

        m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
    };

    for (auto i = 0u; i < debrisCount; ++i)
    {
        auto position = sf::Vector2f(xy::Util::Random::value(0.f, xy::DefaultSceneSize.x),
            xy::Util::Random::value(0.f, xy::DefaultSceneSize.y));
        spawn(position);
    }
}

void GameController::buildUI()
{
    auto display = xy::Component::create<HeadsUpDisplay>(getMessageBus(), m_resources);
    auto entity = xy::Entity::create(getMessageBus());
    hud = entity->addComponent(display);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
}

void GameController::showMessage(const std::string& lineOne, const std::string& lineTwo)
{
    auto displayMsg = xy::Component::create<GameMessage>(getMessageBus(), m_resources.fontResource.get("PHGC_420"), lineOne);
    xy::Component::MessageHandler handler;
    handler.id = GameEvent;
    handler.action = [](xy::Component* c, const xy::Message& msg)
    {
        const auto& data = msg.getData<LMGameEvent>();
        switch (data.type)
        {
        default: break;
        case LMGameEvent::PlayerSpawned:
            dynamic_cast<GameMessage*>(c)->clear();
            break;
        }
    };
    displayMsg->addMessageHandler(handler);

    auto topEnt = xy::Entity::create(getMessageBus());
    topEnt->addComponent(displayMsg);
    topEnt->setPosition(xy::DefaultSceneSize / 2.f);
    topEnt->move(0.f, -60.f);

    displayMsg = xy::Component::create<GameMessage>(getMessageBus(), m_resources.fontResource.get("PHGC_450"), lineTwo, 60u);
    auto bottomEnt = xy::Entity::create(getMessageBus());
    bottomEnt->addComponent(displayMsg);
    bottomEnt->setPosition(0.f, 120.f);

    topEnt->addChild(bottomEnt);
    m_scene.addEntity(topEnt, xy::Scene::Layer::UI);
}