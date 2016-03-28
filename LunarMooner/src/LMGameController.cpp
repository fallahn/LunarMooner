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
#include <CommandIds.hpp>

#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/ParticleController.hpp>
#include <xygine/util/Position.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>

using namespace lm;
using namespace std::placeholders;

namespace
{
    const sf::Vector2f playerSize(32.f, 42.f);
    const sf::Vector2f bulletSize(6.f, 10.f);

    const sf::Vector2f mothershipBounds(386.f, 1534.f);
    const sf::Vector2f mothershipStart(386.f, 46.f);

    const sf::Uint32 rescueScore = 50u;
    const sf::Uint8 minAsteroidLevel = 2;

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
        130.f, 300.f, 300.f, 300.f, 300.f,
        300.f, 300.f, 300.f, 310.f, 320.f
    };

    sf::Clock tempClock;
}

GameController::GameController(xy::MessageBus& mb, xy::Scene& scene, CollisionWorld& cw)
    : xy::Component (mb, this),
    m_scene         (scene),
    m_collisionWorld(cw),
    m_inputFlags    (0),
    m_spawnReady    (true),
    m_player        (nullptr),
    m_timeRound     (false),
    m_mothership    (nullptr),
    m_speedMeter    (nullptr),
    m_scoreDisplay  (nullptr),
    m_currentPlayer (0)
{
    xy::Component::MessageHandler handler;
    handler.id = LMMessageId::LMMessage;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMEvent::SummaryFinished:
            addDelayedRespawn();
            break;
        case LMEvent::PlayerDied:
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
                showRoundSummary(false);
            }
            
            m_player = nullptr;
        }
            break;
        case LMEvent::PlayerLanded:
        {
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
        case LMEvent::HumanRescued:
            addRescuedHuman();
            m_playerStates[m_currentPlayer].humansSaved++;
            m_playerStates[m_currentPlayer].score += rescueScore;
            m_scoreDisplay->showScore(rescueScore, m_player->getPosition(), sf::Color::Magenta);

            if (m_humans.empty())
            {
                moveToNextRound();
            }
            break;
        case LMEvent::AlienDied:
            m_playerStates[m_currentPlayer].alienCount--;
            m_playerStates[m_currentPlayer].score += msgData.value;
            m_scoreDisplay->showScore(msgData.value, { msgData.posX, msgData.posY });
            //1/5 chance we spawn a new alien
            if (xy::Util::Random::value(0, 5) == 3)
            {
                spawnAlien({ alienArea.left, xy::Util::Random::value(alienArea.top, alienArea.top + alienArea.height) });
                m_playerStates[m_currentPlayer].alienCount++;
            }
            break;
        }
    };
    addMessageHandler(handler);

    //particle spawner
    auto particleManager = xy::Component::create<xy::ParticleController>(getMessageBus());
    handler.action = [](xy::Component* c, const xy::Message& msg)
    {
        auto component = dynamic_cast<xy::ParticleController*>(c);
        auto& msgData = msg.getData<LMEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMEvent::AlienDied:
        case LMEvent::PlayerDied:
            component->fire(LMParticleID::SmallExplosion, { msgData.posX, msgData.posY });
            break;
        }
    };
    particleManager->addMessageHandler(handler);

    auto entity = xy::Entity::create(getMessageBus());
    auto pc = entity->addComponent(particleManager);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    xy::ParticleSystem::Definition pd;
    pd.loadFromFile("assets/particles/small_explosion.xyp", m_textureResource);
    pc->addDefinition(LMParticleID::SmallExplosion, pd);

    m_particleDefs[LMParticleID::Thruster].loadFromFile("assets/particles/thrust.xyp", m_textureResource);
    m_particleDefs[LMParticleID::RcsLeft].loadFromFile("assets/particles/rcs_left.xyp", m_textureResource);
    m_particleDefs[LMParticleID::RcsRight].loadFromFile("assets/particles/rcs_right.xyp", m_textureResource);
    m_particleDefs[LMParticleID::RoidTrail].loadFromFile("assets/particles/roid_trail.xyp", m_textureResource);

}

//public
void GameController::entityUpdate(xy::Entity&, float dt)
{
    for (auto& de : m_delayedEvents) de.time -= dt;
    
    //execute then remove expired events
    m_delayedEvents.erase(std::remove_if(m_delayedEvents.begin(), m_delayedEvents.end(),
        [](const DelayedEvent& de)
    {
        if (de.time <= 0)
        {
            de.action();
            return true;
        }
        return false;
    }),
        m_delayedEvents.end());

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

    //count down round time
    if (m_timeRound)
    {
        const float oldTime = m_playerStates[m_currentPlayer].timeRemaining;
        m_playerStates[m_currentPlayer].timeRemaining = std::max(0.f, m_playerStates[m_currentPlayer].timeRemaining - dt);
        if (oldTime > 0 &&
            m_playerStates[m_currentPlayer].timeRemaining == 0)
        {
            //we timed out, end round
            restartRound();
        }
    }
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
        if (shoot && m_player->carryingHuman())
        {
            spawnBullet();
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
}

//private
void GameController::spawnPlayer()
{
    if (!m_player && m_spawnReady)
    {
        const auto& children = m_mothership->getChildren();
        auto spawnPos = children.back()->getWorldPosition();
        children.back()->destroy();

        auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
        dropshipDrawable->getDrawable().setSize(playerSize);

        auto playerController = xy::Component::create<lm::PlayerController>(getMessageBus());

        auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, playerSize }, CollisionComponent::ID::Player);
        CollisionComponent::Callback cb = std::bind(&PlayerController::collisionCallback, playerController.get(), _1);
        collision->setCallback(cb);

        auto thrust = m_particleDefs[LMParticleID::Thruster].createSystem(getMessageBus());
        thrust->setName("thrust");

        auto rcsLeft = m_particleDefs[LMParticleID::RcsLeft].createSystem(getMessageBus());
        rcsLeft->setName("rcsLeft");

        auto rcsRight = m_particleDefs[LMParticleID::RcsRight].createSystem(getMessageBus());
        rcsRight->setName("rcsRight");

        auto entity = xy::Entity::create(getMessageBus());
        entity->setPosition(spawnPos);
        entity->setOrigin(playerSize / 2.f);
        entity->addComponent(dropshipDrawable);
        m_player = entity->addComponent(playerController);
        entity->addComponent(collision);
        entity->addComponent(thrust);
        entity->addComponent(rcsLeft);
        entity->addComponent(rcsRight);
        entity->addCommandCategories(LMCommandID::Player);

        m_scene.addEntity(entity, xy::Scene::BackFront);

        m_spawnReady = false;
        m_timeRound = true;
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
    entity->setPosition(mothershipStart);
    entity->addCommandCategories(LMCommandID::Mothership);

    m_mothership = m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);

    auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
    dropshipDrawable->getDrawable().setSize(playerSize);
    xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

    entity = xy::Entity::create(getMessageBus());    
    entity->setPosition(bounds.width / 2.f, bounds.height / 2.f + 10.f);
    entity->addComponent(dropshipDrawable);
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

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setFillColor(sf::Color::Red);
    drawable->getDrawable().setSize({ size.width, size.height });

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
    auto backgroundDrawable = xy::Component::create<xy::SfDrawableComponent<sf::Sprite>>(getMessageBus());
    backgroundDrawable->getDrawable().setTexture(m_textureResource.get("assets/images/background.png"));

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(alienArea.left, 0.f);
    entity->addComponent(backgroundDrawable);

    m_scene.addEntity(entity, xy::Scene::BackRear);


    //death zone at bottom
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setFillColor(sf::Color::Red);
    drawable->getDrawable().setSize({ alienArea.width, 40.f });

    auto collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ alienArea.width, 40.f } }, lm::CollisionComponent::ID::Alien);

    entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(collision);
    entity->setPosition(alienArea.left, 1040.f);

    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);


    //walls
    collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f },{ 40.f, 1080.f } }, lm::CollisionComponent::ID::Bounds);
    entity = xy::Entity::create(getMessageBus());
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


    //towers to land on
    std::array<std::pair<sf::Vector2f, sf::Vector2f>, 3u> positions =
    {
        std::make_pair(sf::Vector2f(180.f, 210.f), sf::Vector2f(20.f, 1080.f - 210.f)),
        std::make_pair(sf::Vector2f(150.f, 290.f), sf::Vector2f(520.f, 1080.f - 290.f)),
        std::make_pair(sf::Vector2f(220.f, 60.f), sf::Vector2f(900.f, 1080.f - 60.f))
    };
    //hack in some scores for now until we decide a better way to generate terrain
    std::array<sf::Uint16, 3u> scores = {30, 10, 50};
    int i = 0;

    for (const auto& p : positions)
    {
        drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        drawable->getDrawable().setFillColor(sf::Color::Green);
        drawable->getDrawable().setSize(p.first);

        collision = m_collisionWorld.addComponent(getMessageBus(), { { 0.f, 0.f }, p.first }, lm::CollisionComponent::ID::Tower);
        collision->setScoreValue(scores[i++]);

        entity = xy::Entity::create(getMessageBus());
        entity->addComponent(drawable);
        entity->addComponent(collision);
        entity->setPosition(alienArea.left + p.second.x, p.second.y);

        m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    }
}

void GameController::addRescuedHuman()
{
    auto drawable = getHumanDrawable(getMessageBus());

    float width = m_mothership->globalBounds().width;
    width -= 8.f; //4 px padding each end

    float offset = width / static_cast<float>(humanCounts[std::min(m_playerStates[m_currentPlayer].level, static_cast<sf::Uint8>(humanCounts.size() - 1))]);
    offset *= m_playerStates[m_currentPlayer].humansSaved;
    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(offset + 4.f, 24.f);
    entity->addComponent(drawable);
    m_mothership->addChild(entity);
}

void GameController::spawnBullet()
{
    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setSize(bulletSize);
    drawable->getDrawable().setFillColor(sf::Color::Cyan);

    auto controller = xy::Component::create<BulletController>(getMessageBus());

    auto collision = m_collisionWorld.addComponent(getMessageBus(), { {0.f, 0.f}, bulletSize }, CollisionComponent::ID::Bullet);
    CollisionComponent::Callback cb = std::bind(&BulletController::collisionCallback, controller.get(), _1);
    collision->setCallback(cb);

    auto entity = xy::Entity::create(getMessageBus());
    entity->setPosition(m_player->getPosition());
    entity->setOrigin(bulletSize / 2.f);
    entity->addComponent(drawable);
    entity->addComponent(controller);
    entity->addComponent(collision);

    m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
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

    if (count >= m_playerStates.size())
    {
        //everyone is dead! request end game
        auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
        msg->type = LMEvent::GameOver;
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
    for (auto i = 0; i < m_playerStates[m_currentPlayer].humansSaved; ++i)
    {
        addRescuedHuman();
    }

    //throw in some random roids at higher levels
    auto& ps = m_playerStates[m_currentPlayer];
    if (ps.level > minAsteroidLevel)
    {
        addDelayedAsteroid();
        if (ps.level > 6)
        {
            addDelayedAsteroid(); //even moar!!
        }
    }

    //reset round time if new round - TODO adjust times once beyond level 10
    //something like: times[9] - (level - 10 * 2)
    if (ps.startNewRound)
    {
        ps.timeRemaining = roundTimes[std::min(static_cast<std::size_t>(ps.level - 1), roundTimes.size() - 1)];
    }
}

void GameController::moveToNextRound()
{
    xy::Logger::log("Level: " + std::to_string(m_playerStates[m_currentPlayer].level) 
        + ", time: " + std::to_string(tempClock.restart().asSeconds()),
        xy::Logger::Type::Info, xy::Logger::Output::File);
        
    //display a round summary
    //do this first so player info is current
    //when the summary board is constructed
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

    //update state with new round values
    auto& ps = m_playerStates[m_currentPlayer];
    ps.humansSaved = 0;

    //new human count
    auto& humans = ps.humansRemaining;
    humans.clear();
    for (auto i = 0; i < humanCounts[std::min(static_cast<std::size_t>(ps.level - 1), humanCounts.size() - 1)]; ++i)
    {
        humans.emplace_back(xy::Util::Random::value(290.f, 1600.f), xy::Util::Random::value(1045.f, 1060.f));
    }

    //TODO gen a new terrain?

    //increase aliens
    ps.alienCount = alienCounts[std::min(static_cast<std::size_t>(ps.level - 1), alienCounts.size() - 1)];

    ps.lives--;
    ps.startNewRound = true;

    //display a round summary
    showRoundSummary(false);
}

void GameController::addDelayedRespawn()
{
    m_delayedEvents.emplace_back();
    auto& de = m_delayedEvents.back();
    de.time = 1.f;
    de.action = [this]()
    {
        swapPlayerState();
        m_spawnReady = true;

        auto dropshipDrawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
        dropshipDrawable->getDrawable().setFillColor(sf::Color::Blue);
        dropshipDrawable->getDrawable().setSize(playerSize);
        xy::Util::Position::centreOrigin(dropshipDrawable->getDrawable());

        auto entity = xy::Entity::create(getMessageBus());
        auto bounds = m_mothership->getComponent<xy::SfDrawableComponent<sf::CircleShape>>()->getDrawable().getGlobalBounds();
        entity->setPosition(bounds.width / 2.f, bounds.height / 2.f + 10.f);
        entity->addComponent(dropshipDrawable);
        m_mothership->addChild(entity);
    };
}

void GameController::spawnAsteroid()
{
    auto size = alienSizes[xy::Util::Random::value(0, alienSizes.size() - 1)];
    sf::Vector2f position(xy::Util::Random::value(alienArea.left + (alienArea.width / 2.f), alienArea.left + alienArea.width), -size.height * 2);

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(getMessageBus());
    drawable->getDrawable().setFillColor(sf::Color(255, 127, 0));
    drawable->getDrawable().setSize({ size.width, size.height });
    drawable->getDrawable().setOrigin({ size.width / 2.f, size.height / 2.f });

    auto controller = xy::Component::create<AsteroidController>(getMessageBus(), alienArea);

    auto collision = m_collisionWorld.addComponent(getMessageBus(), size, lm::CollisionComponent::ID::Alien);
    //lm::CollisionComponent::Callback cb = std::bind(&AlienController::collisionCallback, controller.get(), std::placeholders::_1);
    //collision->setCallback(cb);
    collision->setScoreValue(100);

    auto ps = m_particleDefs[LMParticleID::RoidTrail].createSystem(getMessageBus());
    ps->setLifetimeVariance(0.55f);

    auto entity = xy::Entity::create(getMessageBus());
    entity->addComponent(drawable);
    entity->addComponent(ps);
    entity->addComponent(controller);
    entity->addComponent(collision);
    entity->setPosition(position);

    m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
}

void GameController::addDelayedAsteroid()
{
    m_delayedEvents.emplace_back();
    auto& de = m_delayedEvents.back();
    de.time = xy::Util::Random::value(18.f, 28.f);
    de.action = [this]()
    {       
        if (m_playerStates[m_currentPlayer].level > minAsteroidLevel)
        {
            spawnAsteroid();
            addDelayedAsteroid();
        }
    };
}

void GameController::showRoundSummary(bool doScores)
{
    auto summary = xy::Component::create<RoundSummary>(getMessageBus(), m_playerStates[m_currentPlayer], m_textureResource, m_fontResource, doScores);
    auto entity = xy::Entity::create(getMessageBus());
    entity->addCommandCategories(LMCommandID::UI);
    entity->addComponent(summary);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    m_timeRound = false;
}