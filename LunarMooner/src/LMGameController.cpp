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

#include <LMGameController.hpp>
#include <LMPlayerController.hpp>
#include <LMMothershipController.hpp>
#include <LMCollisionWorld.hpp>
#include <LMHumanController.hpp>
#include <LMAlienController.hpp>
#include <LMBulletController.hpp>
#include <LMAsteroidController.hpp>
#include <LMSpeedMeter.hpp>
#include <LMPlayerInfoDisplay.hpp>
#include <LMRoundSummary.hpp>
#include <LMTerrain.hpp>
#include <LMEarlyWarning.hpp>
#include <LMShieldDrawable.hpp>
#include <LMLaserSight.hpp>
#include <LMSpriteBatch.hpp>
#include <CommandIds.hpp>
#include <StateIds.hpp>
#include <Game.hpp>

#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/AudioSource.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Math.hpp>
#include <xygine/Resource.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>

using namespace lm;
using namespace std::placeholders;

namespace
{
    const sf::Vector2f playerSize(32.f, 42.f);
    const sf::Vector2f bulletSize(2.f, 24.f);

    const sf::Vector2f mothershipBounds(386.f, 1534.f);
    const sf::Vector2f mothershipStart(386.f, 46.f);

    const sf::Uint32 rescueScore = 50u;
    const sf::Uint32 extraLifeScore = 5000u;
    const sf::Uint8 minAsteroidLevel = 2;

    const sf::Uint8 ammoPerHuman = 3u;
    const sf::Uint8 maxItems = 4;

    const float meteorThreshold = 540.f; //player must be below this to spawn meteor

    //humans to rescue per level
    std::array<sf::Uint8, 10u> humanCounts = 
    {
        3, 4, 4, 5, 6, 6, 8, 8, 10, 12
    };

    //aliens per level
    const std::array<sf::Uint8, 10u> alienCounts =
    {
        12, 14, 16, 18, 21, 24, 27, 31, 35, 38
    };

    const sf::FloatRect alienArea(280.f, 200.f, 1360.f, 480.f);
    const std::array<sf::FloatRect, 4u> alienSizes =
    {
        sf::FloatRect(0.f, 0.f, 20.f, 16.f),
        { 0.f, 0.f, 28.f, 30.f },
        { 0.f, 0.f, 14.f, 16.f },
        { 0.f, 0.f, 18.f, 26.f }
    };

    //time limits per level
    const std::array<float, 10u> roundTimes =
    {
        75.f, 90.f, 100.f, 110.f, 120.f, 
        130.f, 145.f, 150.f, 155.f, 160.f
    };
}

GameController::GameController(xy::MessageBus& mb, xy::Scene& scene, CollisionWorld& cw, const xy::App::AudioSettings& as,
    xy::SoundResource& sr, xy::TextureResource& tr, xy::FontResource& fr)
    : xy::Component     (mb, this),
    m_scene             (scene),
    m_collisionWorld    (cw),
    m_audioSettings     (as),
    m_soundResource     (sr),
    m_textureResource   (tr),
    m_fontResource      (fr),
    m_inputFlags        (0),
    m_spawnReady        (true),
    m_player            (nullptr),
    m_timeRound         (false),
    m_mothership        (nullptr),
    m_terrain           (nullptr),
    m_alienBatch        (nullptr),
    m_speedMeter        (nullptr),
    m_scoreDisplay      (nullptr),
    m_currentPlayer     (0),
    m_flushRoidEvents   (false),
    m_itemCount         (0u)
{
    xy::Component::MessageHandler handler;
    handler.id = LMMessageId::GameEvent;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::PlayerGotAmmo:
            m_playerStates[m_currentPlayer].ammo += ammoPerHuman;
        case LMGameEvent::PlayerGotShield:
            m_playerStates[m_currentPlayer].score += msgData.value;
            m_scoreDisplay->showScore(msgData.value, m_player->getPosition());
            break;
        case LMGameEvent::ItemCollected:
            m_itemCount--;
            break;
        case LMGameEvent::PlayerDied:
        {
            m_playerStates[m_currentPlayer].lives--;
            
            //if died carrying last human move up to next level
            if (m_playerStates[m_currentPlayer].lives > -1
                && m_humans.empty())
            {
                if (m_playerStates[m_currentPlayer].humansSaved > 0)
                {
                    moveToNextRound();
                }
                else
                {
                    //you killed everyone! no more game for you.
                    m_playerStates[m_currentPlayer].lives = -1;
                }
            }
            else
            {
                storePlayerState();
                m_pendingDelayedEvents.emplace_back();
                auto& de = m_pendingDelayedEvents.back();
                de.time = 1.6f;
                de.action = [this]() 
                {
                    showRoundSummary(false);
                };
            }
            
            m_player = nullptr;
        }
            break;
        case LMGameEvent::PlayerLanded:
        {
            if (!m_player) break;

            auto pos = m_player->getPosition();

            //get nearest human
            auto dist = xy::Util::Vector::lengthSquared(m_humans[0]->getPosition() - pos);
            std::size_t index = 0;
            if (m_humans.size() > 1)
            {
                for (auto i = 1u; i < m_humans.size(); ++i)
                {
                    auto newDist = xy::Util::Vector::lengthSquared(m_humans[i]->getPosition() - pos);
                    if (newDist < dist)
                    {
                        index = i;
                        dist = newDist;
                    }
                }
            }
            m_humans[index]->getComponent<HumanController>()->setDestination(pos);

            m_playerStates[m_currentPlayer].score += msgData.value;
            m_scoreDisplay->showScore(msgData.value, m_player->getPosition());
        }
            break;
        case LMGameEvent::HumanPickedUp:
            m_playerStates[m_currentPlayer].ammo += ammoPerHuman;
            break;
        case LMGameEvent::HumanRescued:
            addRescuedHuman();
            m_playerStates[m_currentPlayer].humansSaved++;
            m_playerStates[m_currentPlayer].score += rescueScore;
            m_scoreDisplay->showScore(rescueScore, m_player->getPosition(), sf::Color::Magenta);

            if (m_humans.empty())
            {
                moveToNextRound();
            }
            break;
        case LMGameEvent::AlienDied:
            m_playerStates[m_currentPlayer].alienCount--;
            m_playerStates[m_currentPlayer].score += msgData.value;
            m_scoreDisplay->showScore(msgData.value, { msgData.posX, msgData.posY });
            //50/50 chance we spawn a new alien
            //if (xy::Util::Random::value(0, 1) == 1)
            {
                spawnAlien({ alienArea.left, xy::Util::Random::value(alienArea.top, alienArea.top + alienArea.height) });
                m_playerStates[m_currentPlayer].alienCount++;
            }

            //see if we can spawn a new item
            if (m_itemCount < maxItems
                && xy::Util::Random::value(0, 1) == 1)
            {
                spawnCollectable({ msgData.posX, msgData.posY });
            }
            break;
        }
    };
    addMessageHandler(handler);

    handler.id = LMMessageId::StateEvent;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMStateEvent::SummaryFinished:
            addDelayedRespawn();
            LOG("Summary finished", xy::Logger::Type::Info);
            break;
        }
    };
    addMessageHandler(handler);

    handler.id = xy::Message::UIMessage;
    handler.action = [this](xy::Component*, const xy::Message& msg)
    {
        //if game over screen opened broadcast scores for each player
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        if (msgData.type == xy::Message::UIEvent::MenuOpened
            && msgData.stateId == States::ID::GameOver)
        {
            for (auto i = 0u; i < m_playerStates.size(); ++i)
            {
                auto message = getMessageBus().post<LMMenuEvent>(LMMessageId::MenuEvent);
                message->playerId = i;
                message->score = m_playerStates[i].score;
            }
        }
    };
    addMessageHandler(handler);

    //precache player effects
    m_particleDefs[LMParticleID::Thruster].loadFromFile("assets/particles/thrust.xyp", m_textureResource);
    m_particleDefs[LMParticleID::RcsLeft].loadFromFile("assets/particles/rcs_left.xyp", m_textureResource);
    m_particleDefs[LMParticleID::RcsRight].loadFromFile("assets/particles/rcs_right.xyp", m_textureResource);
    m_particleDefs[LMParticleID::RoidTrail].loadFromFile("assets/particles/roid_trail.xyp", m_textureResource);

    m_soundCache.insert(std::make_pair(LMSoundID::Engine, m_soundResource.get("assets/sound/fx/thrust.wav")));
    m_soundCache.insert(std::make_pair(LMSoundID::RCS, m_soundResource.get("assets/sound/fx/rcs.wav")));
    m_soundCache.insert(std::make_pair(LMSoundID::RoundCountEnd, m_soundResource.get("assets/sound/fx/score_count_end.wav")));
    m_soundCache.insert(std::make_pair(LMSoundID::RoundCountLoop, m_soundResource.get("assets/sound/fx/score_count_loop.wav")));

    //create sprite batch for aliens
    auto sprBatch = xy::Component::create<SpriteBatch>(getMessageBus());
    auto ent = xy::Entity::create(getMessageBus());
    m_alienBatch = ent->addComponent(sprBatch);
    m_scene.addEntity(ent, xy::Scene::Layer::BackMiddle);
}

//public
void GameController::entityUpdate(xy::Entity&, float dt)
{   
    //append any waiting events
    m_delayedEvents.splice(m_delayedEvents.end(), m_pendingDelayedEvents);

    //execute then remove expired events
    m_delayedEvents.erase(std::remove_if(m_delayedEvents.begin(), m_delayedEvents.end(),
        [this, dt](DelayedEvent& de)
    {
        de.time -= dt;
        if (de.time <= 0)
        {
            de.action();
            return true;
        }

        return false;
    }), m_delayedEvents.end());

    if (m_flushRoidEvents)
    {
        m_flushRoidEvents = false;
        m_delayedEvents.erase(std::remove_if(m_delayedEvents.begin(), m_delayedEvents.end(),
            [](const DelayedEvent& de)
        {
            return de.id == EventID::SpawnRoid;
        }), m_delayedEvents.end());
    }

    //remove humans which have reached the ship
    m_humans.erase(std::remove_if(m_humans.begin(), m_humans.end(),
        [](const xy::Entity* ent)
    {
        return ent->destroyed();
    }), m_humans.end());

    //clear dead aliens
    m_aliens.erase(std::remove_if(m_aliens.begin(), m_aliens.end(), [](const xy::Entity* entity) 
    {
        return entity->destroyed();
    }), m_aliens.end());

    //update UI
    if (m_player)
    {
        m_speedMeter->setValue(m_player->getSpeed());
    }

    //count down round time - TODO we can neaten this up a bit
    if (m_timeRound)
    {
        const float oldTime = m_playerStates[m_currentPlayer].timeRemaining;
        m_playerStates[m_currentPlayer].timeRemaining = std::max(0.f, m_playerStates[m_currentPlayer].timeRemaining - dt);
        
        if (oldTime > 31 &&
            m_playerStates[m_currentPlayer].timeRemaining <= 31)
        {
            auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
            msg->type = LMStateEvent::CountDownWarning;
        }
        else if (oldTime > 11 &&
            m_playerStates[m_currentPlayer].timeRemaining <= 11)
        {
            auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
            msg->type = LMStateEvent::CountDownStarted;
        }
        else if (oldTime > 6 &&
            m_playerStates[m_currentPlayer].timeRemaining <= 6)
        {
            auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
            msg->type = LMStateEvent::CountDownInProgress;
        }
        else if (oldTime > 1 &&
            m_playerStates[m_currentPlayer].timeRemaining <= 1)
        {
            //we timed out, end round
            restartRound();
        }
    }

    //check for extra lives
    if ((m_playerStates[m_currentPlayer].previousScore / extraLifeScore) <
        (m_playerStates[m_currentPlayer].score / extraLifeScore))
    {
        m_playerStates[m_currentPlayer].lives++;
        m_scoreDisplay->showMessage("EXTRA LIFE!");

        auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
        msg->type = LMGameEvent::ExtraLife;
    }
    m_playerStates[m_currentPlayer].previousScore = m_playerStates[m_currentPlayer].score;
}

void GameController::setInput(sf::Uint8 input)
{    
    bool shoot = ((input & LMInputFlags::Shoot) != 0
        && (m_inputFlags & LMInputFlags::Shoot) == 0);

    if (m_player)
    {
        //set player input
        m_player->setInput(input);

        //hook player shoot event here
        //as bullets are technically entities in their own right
        if (shoot && m_playerStates[m_currentPlayer].ammo > 0)
        {
            spawnBullet();
            m_playerStates[m_currentPlayer].ammo--;
        }
    }
    else
    {        
        if (shoot)
        {
            //assume there's some UI which wants commanding
            xy::Command cmd;
            cmd.category = LMCommandID::UI;
            cmd.action = [](xy::Entity& entity, float)
            {
                entity.getComponent<RoundSummary>()->completeSummary();
            };
            m_scene.sendCommand(cmd);

            //try spawning
            spawnPlayer();
        }
    }

    m_inputFlags = input;
}

void GameController::addPlayer()
{
    m_playerStates.emplace_back();

    auto& state = m_playerStates.back();

    auto& humans = state.humansRemaining;
    for (auto i = 0; i < humanCounts[0]; ++i)
    {
        humans.emplace_back(xy::Util::Random::value(290.f, 1600.f), xy::Util::Random::value(1045.f, 1060.f));
    }

    state.alienCount = alienCounts[0];
    state.timeRemaining = roundTimes[0];
}

void GameController::start()
{
    createTerrain();
    createMothership();
    spawnAliens();
    spawnHumans();
    createUI();

    m_scoreDisplay->showMessage("Start!");

    auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
    msg->type = LMStateEvent::RoundBegin;
}

//private
void GameController::spawnPlayer()
{
    if (!m_player && m_spawnReady)
    {
        const auto& children = m_mothership->getChildren();
        auto spawnPos = children.back()->getWorldPosition();
        xy::Command cmd;
        cmd.category = LMCommandID::DropShip;
        cmd.action = [](xy::Entity& ent, float) {ent.destroy(); };
        m_scene.sendCommand(cmd);

        auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
        dropshipDrawable->getDrawable().setSize(playerSize);
        dropshipDrawable->getDrawable().setOutlineColor(sf::Color::Cyan);

        auto playerController = xy::Component::create<lm::PlayerController>(getMessageBus(),
            m_mothership->getComponent<MothershipController>(), m_terrain->getChain());
        playerController->setSize(playerSize);

        auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, playerSize }, CollisionComponent::ID::Player);
        CollisionComponent::Callback cb = std::bind(&PlayerController::collisionCallback, playerController.get(), _1);
        collision->setCallback(cb);

        auto thrust = m_particleDefs[LMParticleID::Thruster].createSystem(getMessageBus());
        thrust->setName("thrust");

        auto rcsLeft = m_particleDefs[LMParticleID::RcsLeft].createSystem(getMessageBus());
        rcsLeft->setName("rcsLeft");

        auto rcsRight = m_particleDefs[LMParticleID::RcsRight].createSystem(getMessageBus());
        rcsRight->setName("rcsRight");

        auto sfx1 = xy::Component::create<xy::AudioSource>(getMessageBus(), m_soundResource);
        sfx1->setSoundBuffer(m_soundCache[LMSoundID::RCS]);
        sfx1->setFadeInTime(0.1f);
        sfx1->setFadeOutTime(0.3f);
        sfx1->setName("rcsEffectLeft");
        sfx1->setVolume(m_audioSettings.muted ? 0.f : m_audioSettings.volume * Game::MaxVolume);

        auto sfx2 = xy::Component::create<xy::AudioSource>(getMessageBus(), m_soundResource);
        sfx2->setSoundBuffer(m_soundCache[LMSoundID::RCS]);
        sfx2->setFadeInTime(0.1f);
        sfx2->setFadeOutTime(0.3f);
        sfx2->setName("rcsEffectRight");
        sfx2->setVolume(m_audioSettings.muted ? 0.f : m_audioSettings.volume * Game::MaxVolume);

        auto sfx3 = xy::Component::create<xy::AudioSource>(getMessageBus(), m_soundResource);
        sfx3->setSoundBuffer(m_soundCache[LMSoundID::Engine]);
        sfx3->setFadeInTime(0.1f);
        sfx3->setFadeOutTime(0.3f);
        sfx3->setName("thrustEffect");
        sfx3->setVolume(m_audioSettings.muted ? 0.f : m_audioSettings.volume * Game::MaxVolume);

        auto entity = xy::Entity::create(getMessageBus());
        entity->setPosition(spawnPos);
        entity->setOrigin(playerSize / 2.f);
        entity->addComponent(dropshipDrawable);
        m_player = entity->addComponent(playerController);
        entity->addComponent(collision);
        entity->addComponent(thrust);
        entity->addComponent(rcsLeft);
        entity->addComponent(rcsRight);
        entity->addComponent(sfx1);
        entity->addComponent(sfx2);
        entity->addComponent(sfx3);
        entity->addCommandCategories(LMCommandID::Player);
        m_scene.addEntity(entity, xy::Scene::BackFront);

        m_spawnReady = false;
        m_timeRound = true;

        auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
        msg->type = LMGameEvent::PlayerSpawned;
        msg->posX = spawnPos.x;
        msg->posY = spawnPos.y;
        if (m_playerStates[m_currentPlayer].timeRemaining < 10.f)
        {
            sf::Uint32 speed = static_cast<sf::Uint32>(xy::Util::Math::round(10.f / m_playerStates[m_currentPlayer].timeRemaining));
            msg->value = speed;
        }
    }
}

void GameController::createMothership()
{
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(getMessageBus());
    drawable->getDrawable().setRadius(10.f);
    drawable->getDrawable().setScale(12.f, 4.f);
    drawable->getDrawable().setFillColor(sf::Color::Yellow);

    auto controller = xy::Component::create<lm::MothershipController>(getMessageBus(), mothershipBounds);

    auto bounds = drawable->getDrawable().getGlobalBounds();
    auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, {bounds.width, bounds.height} }, CollisionComponent::ID::Mothership);

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);
    entity->setPosition(960.f - (bounds.width / 2.f), mothershipStart.y);
    entity->addCommandCategories(LMCommandID::Mothership);

    m_mothership = m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);

    auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
    dropshipDrawable->getDrawable().setSize(playerSize);
    xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

    entity = xy::Entity::create(getMessageBus());    
    entity->setPosition(bounds.width / 2.f, bounds.height / 2.f + 10.f);
    entity->addComponent(dropshipDrawable);
    entity->addCommandCategories(LMCommandID::DropShip);
    m_mothership->addChild(entity);
}

namespace
{
    std::unique_ptr<xy::SfDrawableComponent<sf::CircleShape>> getHumanDrawable(xy::MessageBus& mb)
    {
        auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(mb);
        drawable->getDrawable().setRadius(10.f);
        drawable->getDrawable().setFillColor(sf::Color::Blue);
        drawable->getDrawable().setScale(1.f, 1.4f);
        drawable->getDrawable().setOrigin(drawable->getDrawable().getGlobalBounds().width / 2.f, 0.f);
        return std::move(drawable);
    }
}

void GameController::spawnHuman(const sf::Vector2f& position)
{
    auto drawable = getHumanDrawable(getMessageBus());

    auto controller = xy::Component::create<HumanController>(getMessageBus());

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->setPosition(position);
    entity->addCommandCategories(LMCommandID::Human);

    m_humans.push_back(m_scene.addEntity(entity, xy::Scene::Layer::FrontRear));
}

void GameController::spawnHumans()
{
    for(auto& h : m_playerStates[m_currentPlayer].humansRemaining)
    {
        spawnHuman(h);
    }
}

void GameController::spawnAlien(const sf::Vector2f& position)
{
    auto size = alienSizes[xy::Util::Random::value(0, alienSizes.size() - 1)];

    auto drawable = m_alienBatch->addSprite(getMessageBus());
    drawable->setTextureRect(size);
    drawable->setColour(sf::Color::Red);

    auto controller = xy::Component::create<AlienController>(getMessageBus(), alienArea);

    auto collision = m_collisionWorld.addComponent(getMessageBus(), size, lm::CollisionComponent::ID::Alien);
    lm::CollisionComponent::Callback cb = std::bind(&AlienController::collisionCallback, controller.get(), std::placeholders::_1);
    collision->setCallback(cb);
    //smaller are worth more because they arew harder to hit
    //this cludge assumes max size is 50 x 50
    collision->setScoreValue(100 - static_cast<sf::Uint16>(size.width + size.height));

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);
    entity->setPosition(position);
    entity->addCommandCategories(LMCommandID::Alien);

    m_aliens.push_back(m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle));
}

void GameController::spawnAliens()
{
    for (auto i = 0; i < m_playerStates[m_currentPlayer].alienCount; ++i)
    {
        auto position = sf::Vector2f(xy::Util::Random::value(alienArea.left, alienArea.left + alienArea.width),
            xy::Util::Random::value(alienArea.top, alienArea.top + alienArea.height));
        spawnAlien(position);
    }
}

void GameController::createTerrain()
{
    //walls
    auto collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ 40.f, 1080.f } }, lm::CollisionComponent::ID::Bounds);
    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(collision);
    entity->setPosition(alienArea.left - 40.f, 0.f);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ 40.f, 1080.f } }, lm::CollisionComponent::ID::Bounds);
    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(collision);
    entity->setPosition(alienArea.left + alienArea.width, 0.f);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ alienArea.width, 40.f } }, lm::CollisionComponent::ID::Bounds);
    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(collision);
    entity->setPosition(alienArea.left, -40.f);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ alienArea.width, 40.f } }, lm::CollisionComponent::ID::Bounds);
    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(collision);
    entity->setPosition(alienArea.left, 1080.f);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    //flame effects - we know these work so lets make part of the map file
    /*xy::ParticleSystem::Definition pd;
    pd.loadFromFile("assets/particles/fire.xyp", m_textureResource);
    
    auto ps = pd.createSystem(getMessageBus());
    ps->setLifetimeVariance(0.3f);
    ps->start(pd.releaseCount);

    entity = xy::Entity::create(getMessageBus());
    entity->setPosition(550.f, 860.f);
    entity->addComponent(ps);
    m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);*/

    //death zone at bottom
    auto terrain = xy::Component::create<Terrain>(getMessageBus());
    terrain->load("assets/maps/01.lmm", m_textureResource);

    entity = xy::Entity::create(getMessageBus());
    entity->setPosition(alienArea.left, 1080.f - 320.f); //TODO fix these numbers
    m_terrain = entity->addComponent(terrain);
    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);

    //need to do this after adding to entity to get correct transforms
    auto platforms = m_terrain->getPlatforms();
    for (const auto& p : platforms)
    {
        collision = m_collisionWorld.addComponent(getMessageBus(), { {}, p.size }, lm::CollisionComponent::ID::Tower);
        collision->setScoreValue(p.value);

        entity = xy::Entity::create(getMessageBus());
        entity->addComponent(collision);
        entity->setPosition(p.position);

        m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    }

    entity = xy::Entity::create(getMessageBus());
    auto shieldDrawable = xy::Component::create<ShieldDrawable>(getMessageBus(), 3000.f);
    shieldDrawable->setPosition(960.f, 3700.f);
    shieldDrawable->rotate(-180.f);
    shieldDrawable->setTexture(m_textureResource.get("assets/images/game/shield_noise.png"));
    //shieldDrawable->setScale(0.1f, 0.1f);
    entity->addComponent(shieldDrawable);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
}

void GameController::addRescuedHuman()
{
    auto drawable = getHumanDrawable(getMessageBus());
    
    float width = m_mothership->globalBounds().width;
    width -= 8.f; //4 px padding each end

    const float humanCount = static_cast<float>(humanCounts[std::min(static_cast<std::size_t>(m_playerStates[m_currentPlayer].level) - 1u, humanCounts.size() - 1)]);
    float offset = width / humanCount;
    const float pad = offset / 2.f;
    offset *= m_playerStates[m_currentPlayer].humansSaved;

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(pad + offset + 4.f, 24.f);
    entity->addComponent(drawable);
    m_mothership->addChild(entity);
}

void GameController::spawnBullet()
{
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setSize(bulletSize);
    drawable->getDrawable().setFillColor(sf::Color(90u, 255u, 120u, 190u));
    drawable->getDrawable().setOutlineColor(sf::Color(0u, 185u, 0u, 100u));
    drawable->getDrawable().setOutlineThickness(2.f);
    drawable->setBlendMode(sf::BlendAdd);
    
    auto controller = xy::Component::create<BulletController>(getMessageBus());

    auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, bulletSize }, CollisionComponent::ID::Bullet);
    CollisionComponent::Callback cb = std::bind(&BulletController::collisionCallback, controller.get(), _1);
    collision->setCallback(cb);

    auto position = m_player->getPosition();

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(position);
    entity->setOrigin(bulletSize / 2.f);
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);

    m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);

    //raise message
    auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
    msg->type = LMGameEvent::LaserFired;
    msg->posX = position.x;
    msg->posY = position.y;
}

void GameController::createUI()
{
    //velocity meter
    auto speedMeter = xy::Component::create<SpeedMeter>(getMessageBus(), 50000.f);
    auto entity = xy::Entity::create(getMessageBus());
    m_speedMeter = entity->addComponent(speedMeter);

    entity->rotate(-90.f);
    entity->setPosition(alienArea.left + ((alienArea.width - m_speedMeter->getSize().y) / 2.f), m_speedMeter->getSize().x + 10.f);

    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    //score / lives display etc
    auto scores = xy::Component::create<ScoreDisplay>(getMessageBus(), m_fontResource, m_playerStates);
    entity = xy::Entity::create(getMessageBus());
    m_scoreDisplay = entity->addComponent(scores);

    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    m_scoreDisplay->setPlayerActive(m_currentPlayer);
}

void GameController::storePlayerState()
{
    //store positions for current player
    m_playerStates[m_currentPlayer].humansRemaining.clear();

    for (auto& h : m_humans)
    {
        m_playerStates[m_currentPlayer].humansRemaining.push_back(h->getPosition());
    }
}

void GameController::swapPlayerState()
{
    //restore next (living) player's state
    std::size_t count = 0;
    std::size_t lastPlayer = m_currentPlayer;
    do
    {
        m_currentPlayer = (m_currentPlayer + 1) % m_playerStates.size();
    } while (m_playerStates[m_currentPlayer].lives < 0 && ++count < m_playerStates.size());

    LOG("Switched to player " + std::to_string(m_currentPlayer + 1), xy::Logger::Type::Info);

    if (count >= m_playerStates.size())
    {
        //everyone is dead! request end game
        auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
        msg->type = LMStateEvent::GameOver;
        return;
    }

    //restore new state 
    if (lastPlayer != m_currentPlayer
        || m_playerStates[m_currentPlayer].startNewRound)
    {
        restorePlayerState();
        m_playerStates[m_currentPlayer].startNewRound = false;
    }
}

void GameController::restorePlayerState()
{
    m_scoreDisplay->showMessage("Player " + std::to_string(m_currentPlayer + 1) + "!");

    //clear and restore aliens
    for (auto a : m_aliens) a->destroy();
    m_aliens.clear();
    spawnAliens();

    //clear humans and restore
    for (auto h : m_humans) h->destroy();
    m_humans.clear();
    spawnHumans();
        
    //clear rescued humans from ship
    auto& children = m_mothership->getChildren();
    for (auto& c : children) c->destroy();
    //and restore from state
    std::size_t count = m_playerStates[m_currentPlayer].humansSaved;
    //we count these out again so adding drawables to mship are spaced correctly
    m_playerStates[m_currentPlayer].humansSaved = 0;
    for (auto i = 0u; i < count; ++i, ++m_playerStates[m_currentPlayer].humansSaved)
    {
        addRescuedHuman();
    }

    //throw in some random roids at higher levels.
    //as roid events are recursive we need to clear any pending
    //events else we get a cascade of them very quickly...
    m_flushRoidEvents = true;

    auto& ps = m_playerStates[m_currentPlayer];
    if (ps.level > minAsteroidLevel)
    {
        addDelayedAsteroid();
        if (ps.level > (minAsteroidLevel * 2))
        {
            addDelayedAsteroid(); //even moar!!
            if (ps.level > (minAsteroidLevel * 3))
            {
                addDelayedAsteroid(); //MOOOOAAAR!! :P
            }
        }
    }

    //clear floating items
    xy::Command cmd;
    cmd.category = LMCommandID::Item;
    cmd.action = [](xy::Entity& ent, float) { ent.destroy(); };
    m_scene.sendCommand(cmd);
    m_itemCount = 0;

    //lose ammo - TODO should this be restored with state?
    m_playerStates[m_currentPlayer].ammo = 0;

    //reset round time if new round
    if (ps.startNewRound)
    {
        if (ps.level <= roundTimes.size())
        {
            ps.timeRemaining = roundTimes[ps.level - 1];
        }
        else //2 seconds fewer for each level above 10
        {
            ps.timeRemaining = roundTimes.back() - ((ps.level - 10) * 2.f);
        }
    }

    //update display
    m_scoreDisplay->setPlayerActive(m_currentPlayer);
}

void GameController::moveToNextRound()
{  
    //display a round summary
    //do this first so player info is current
    //when the summary board is constructed - this means we can't delay it though :(
    showRoundSummary(true);
    
    //remove player from scene
    xy::Command cmd;
    cmd.category = LMCommandID::Player;
    cmd.action = [](xy::Entity& entity, float)
    {
        entity.destroy();
    };
    m_scene.sendCommand(cmd);
    m_player = nullptr;
    
    //update state with new round values
    auto& ps = m_playerStates[m_currentPlayer];
    ps.humansSaved = 0;
        
    //new human count
    auto& humans = ps.humansRemaining;
    humans.clear();
    for (auto i = 0; i < humanCounts[std::min(ps.level, static_cast<sf::Uint8>(humanCounts.size() - 1))]; ++i)
    {
        humans.emplace_back(xy::Util::Random::value(290.f, 1600.f), xy::Util::Random::value(1045.f, 1060.f));
    }

    //TODO gen a new terrain?

    //increase aliens
    ps.alienCount = alienCounts[std::min(ps.level, static_cast<sf::Uint8>(alienCounts.size() - 1))];
    ps.ammo = 0;
    ps.level++;
    ps.startNewRound = true;
}

void GameController::restartRound()
{
    //remove player from scene
    xy::Command cmd;
    cmd.category = LMCommandID::Player;
    cmd.action = [](xy::Entity& entity, float)
    {
        entity.destroy();
    };
    m_scene.sendCommand(cmd);
    m_player = nullptr;

    //poor colonists got nuked :(
    for (auto h : m_humans) h->destroy();
    m_humans.clear();

    //update state with new round values
    auto& ps = m_playerStates[m_currentPlayer];
    ps.humansSaved = 0;

    //reset human count
    auto& humans = ps.humansRemaining;
    humans.clear();
    for (auto i = 0; i < humanCounts[std::min(static_cast<std::size_t>(ps.level - 1), humanCounts.size() - 1)]; ++i)
    {
        humans.emplace_back(xy::Util::Random::value(290.f, 1600.f), xy::Util::Random::value(1045.f, 1060.f));
    }

    //TODO load a new terrain?

    //increase aliens
    ps.alienCount = alienCounts[std::min(static_cast<std::size_t>(ps.level - 1), alienCounts.size() - 1)];
    ps.ammo = 0;
    ps.lives--;
    ps.startNewRound = true;

    //display a round summary
    m_pendingDelayedEvents.emplace_back();
    auto& de = m_pendingDelayedEvents.back();
    de.time = 1.6f;
    de.action = [this]() 
    {
        showRoundSummary(false);
    };
}

void GameController::addDelayedRespawn()
{
    m_pendingDelayedEvents.emplace_back();
    auto& de = m_pendingDelayedEvents.back();
    de.id = EventID::SpawnPlayer;
    de.time = 1.f;
    de.action = [this]()
    {
        swapPlayerState();
        m_spawnReady = true;

        auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
        msg->type = LMStateEvent::RoundBegin;

        auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
        dropshipDrawable->getDrawable().setSize(playerSize);
        xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

        auto entity = xy::Entity::create(getMessageBus());
        auto bounds = m_mothership->getComponent<xy::SfDrawableComponent<sf::CircleShape>>()->getDrawable().getGlobalBounds();
        entity->setPosition(bounds.width / 2.f, bounds.height / 2.f + 10.f);
        entity->addComponent(dropshipDrawable);
        entity->addCommandCategories(LMCommandID::DropShip);
        m_mothership->addChild(entity);
    };
}

void GameController::spawnEarlyWarning(const sf::Vector2f& dest)
{
    auto laser = xy::Component::create<LaserSight>(getMessageBus(), (dest.x < 960.f) ? 45.f : 135.f);
    auto laserEnt = xy::Entity::create(getMessageBus());
    laserEnt->addComponent(laser);
    laserEnt->setWorldPosition(dest);
    m_scene.addEntity(laserEnt, xy::Scene::Layer::BackFront);
    
    /*auto ew = xy::Component::create<EarlyWarning>(getMessageBus(), dest);
    auto ent = xy::Entity::create(getMessageBus());
    ent->addComponent(ew);
    ent->setPosition(960.f, 0.f);
    m_scene.addEntity(ent, xy::Scene::Layer::UI);*/

    auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
    msg->type = LMGameEvent::EarlyWarning;
}

void GameController::spawnAsteroid(const sf::Vector2f& position)
{
    auto size = alienSizes[xy::Util::Random::value(0, alienSizes.size() - 1)];

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setFillColor(sf::Color(255, 127, 0));
    drawable->getDrawable().setSize({ size.width, size.height });
    drawable->getDrawable().setOrigin({ size.width / 2.f, size.height / 2.f });

    const float dir = (position.x < 960.f) ? 1.f : -1.f;
    auto controller = xy::Component::create<AsteroidController>(getMessageBus(), alienArea, sf::Vector2f(dir, 1.f));

    auto collision = m_collisionWorld.addComponent(getMessageBus(), size, lm::CollisionComponent::ID::Alien);
    //lm::CollisionComponent::Callback cb = std::bind(&AlienController::collisionCallback, controller.get(), std::placeholders::_1);
    //collision->setCallback(cb);
    collision->setScoreValue(100);

    auto ps = m_particleDefs[LMParticleID::RoidTrail].createSystem(getMessageBus());
    ps->setLifetimeVariance(0.55f);

    auto as = xy::Component::create<xy::AudioSource>(getMessageBus(), m_soundResource);
    as->setPitch(0.7f);
    as->setVolume(m_audioSettings.muted ? 0.f : m_audioSettings.volume * Game::MaxVolume);
    as->setSoundBuffer(m_soundCache[LMSoundID::Engine]);
    as->setFadeInTime(1.5f);
    as->play(true);

    //TODO gui setting callback

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(ps);
    entity->addComponent(controller);
    entity->addComponent(collision);
    entity->addComponent(as);
    entity->setPosition(position);

    m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
}

void GameController::addDelayedAsteroid()
{
    if (m_player && (m_player->getPosition().y > meteorThreshold))
    {
        sf::Vector2f position(xy::Util::Random::value(alienArea.left, alienArea.left + alienArea.width), -30.f);

        m_pendingDelayedEvents.emplace_back();
        auto& de = m_pendingDelayedEvents.back();
        de.id = EventID::SpawnRoid;
        de.time = xy::Util::Random::value(10.f, 20.f);
        de.action = [this, position]()
        {
            spawnAsteroid(position);
            addDelayedAsteroid();
        };

        //we know when and where it's coming from so let's
        //tell it to the early warning system! :D
        m_pendingDelayedEvents.emplace_back();
        auto& de2 = m_pendingDelayedEvents.back();
        de2.id = EventID::SpawnRoid;
        de2.time = de.time - 2.f; //magic const here. need to relate it to early warning system
        de2.action = [this, position]()
        {
            spawnEarlyWarning(position);
        };
    }
    else
    {
        m_pendingDelayedEvents.emplace_back();
        auto& de = m_pendingDelayedEvents.back();
        de.id = EventID::SpawnRoid;
        de.time = 5.f;
        de.action = [this]()
        {
            addDelayedAsteroid();
        };
    }
}

void GameController::spawnCollectable(const sf::Vector2f& position)
{
    CollisionComponent::ID type = 
        (xy::Util::Random::value(0, 4) < 2) ? CollisionComponent::ID::Shield : CollisionComponent::ID::Ammo;

    auto drawable = getHumanDrawable(getMessageBus());
    if (type == CollisionComponent::ID::Shield)
    {
        drawable->getDrawable().setFillColor(sf::Color::Cyan);
    }
    else
    {
        drawable->getDrawable().setFillColor(sf::Color::Yellow);
    }

    auto bounds = drawable->getDrawable().getLocalBounds();
    auto collision = m_collisionWorld.addComponent(getMessageBus(), bounds, type);
    collision->setScoreValue(20);

    //for now we expect the same behaviour as aliens
    auto controller = xy::Component::create<AlienController>(getMessageBus(), alienArea);

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(position);
    entity->addComponent(drawable);
    auto cc = entity->addComponent(collision);
    entity->addComponent(controller);
    entity->addCommandCategories(LMCommandID::Item);

    auto ep = m_scene.addEntity(entity, xy::Scene::Layer::BackFront);

    //add our callback here :)
    cc->setCallback([ep, this](CollisionComponent* col) 
    {
        switch (col->getID())
        {
        default: break;
        //case CollisionComponent::ID::Bullet:
        case CollisionComponent::ID::Player:
        {
            auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
            msg->type = LMGameEvent::ItemCollected;
            msg->posX = ep->getPosition().x;
            msg->posY = ep->getPosition().y;
            msg->value = ep->getComponent<CollisionComponent>()->getScoreValue();
            ep->destroy();
        }
        break;
        }
    });

    m_itemCount++;
}

void GameController::showRoundSummary(bool doScores)
{
    auto summary = xy::Component::create<RoundSummary>(getMessageBus(), m_playerStates[m_currentPlayer], m_textureResource, m_fontResource, doScores);
    summary->setSoundBuffer(LMSoundID::RoundCountEnd, m_soundCache[LMSoundID::RoundCountEnd], m_audioSettings.muted ? 0.f : m_audioSettings.volume * Game::MaxVolume);
    summary->setSoundBuffer(LMSoundID::RoundCountLoop, m_soundCache[LMSoundID::RoundCountLoop], m_audioSettings.muted ? 0.f : m_audioSettings.volume * Game::MaxVolume);
    
    auto entity = xy::Entity::create(getMessageBus());
    entity->addCommandCategories(LMCommandID::UI);
    entity->addComponent(summary);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    m_timeRound = false;

    auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
    msg->type = LMStateEvent::RoundEnd;
}
