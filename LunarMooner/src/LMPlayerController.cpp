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

#include <LMPlayerController.hpp>
#include <LMCollisionComponent.hpp>
#include <LMMothershipController.hpp>
#include <LMAsteroidController.hpp>
#include <LMAlienController.hpp>
#include <LMPlayerDrawable.hpp>
#include <BGStarfield.hpp>
#include <CommandIds.hpp>
#include <Game.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Rectangle.hpp>
#include <xygine/Reports.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/AudioSource.hpp>

#include <SFML/Graphics/RectangleShape.hpp>

using namespace lm;
using namespace std::placeholders;

namespace
{
    const sf::Vector2f gravity(0.f, 1.5f);
    const sf::Vector2f thrustX(3.6f, 0.f);
    const sf::Vector2f thrustUp(0.f, -3.8f);

    const float maxDockingVelocity = 8000.f;
    const float maxLandingVelocity = 18000.f ;

    //applied with shielded collisions
    const float damping = 0.65f;

    //distance to top of screen before camera starts to follow
    const float topMargin = 80.f;
    //max distance into space before thrust has no effect
    const float maxSkyDistance = -xy::DefaultSceneSize.y;
}

PlayerController::PlayerController(xy::MessageBus& mb, const MothershipController* ms, const std::vector<sf::Vector2f>& terrain)
    : xy::Component         (mb, this),
    m_mothership            (ms),
    m_inputFlags            (0),
    m_velocity              (ms->getVelocity()),
    m_entity                (nullptr),
    m_carrying              (false),
    m_shield                (false),
    m_thrust                (nullptr),
    m_rcsLeft               (nullptr),
    m_rcsRight              (nullptr),
    m_rcsDown               (nullptr),
    m_rcsEffectLeft         (nullptr),
    m_rcsEffectRight        (nullptr),
    m_rcsEffectDown         (nullptr),
    m_thrustEffect          (nullptr),
    m_highestTerrainPoint   (5000.f),
    m_terrain               (terrain)
{
    m_velocity.y = 2.f;

    updateState = std::bind(&PlayerController::flyingState, this, _1, _2);

    xy::Component::MessageHandler handler;
    handler.id = LMMessageId::GameEvent;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::HumanPickedUp:
            m_velocity = { 0.f, -82.5f };
            updateState = std::bind(&PlayerController::flyingState, this, _1, _2);
            m_carrying = true;
            break;
        }
    };
    addMessageHandler(handler);

    //need to set volume when we receive UI events
    handler.id = xy::Message::UIMessage;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        if (!m_rcsEffectLeft) return; //not been added yet

        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::RequestAudioMute:
            m_rcsEffectLeft->setVolume(0.f);
            m_rcsEffectRight->setVolume(0.f);
            m_thrustEffect->setVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
        case xy::Message::UIEvent::RequestVolumeChange:
        {
            const float vol = std::min(msgData.value * Game::MaxVolume, Game::MaxVolume);
            m_rcsEffectLeft->setVolume(vol);
            m_rcsEffectRight->setVolume(vol);
            m_thrustEffect->setVolume(vol);
        }
        break;
        }
    };
    addMessageHandler(handler);

    handler.id = LMMessageId::TutorialEvent;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        const auto& data = msg.getData<LMTutorialEvent>();
        if (data.action == LMTutorialEvent::Opened)
        {
            m_rcsEffectLeft->pause();
            m_rcsEffectRight->pause();
            m_rcsEffectDown->pause();
            m_thrustEffect->pause();
        }
    };
    addMessageHandler(handler);

    //get the highest point so we only test
    //for collision past this
    for (const auto& p : m_terrain)
    {
        if (p.y < m_highestTerrainPoint)
        {
            m_highestTerrainPoint = p.y;
        }
    }
}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    updateState(entity, dt);

    m_thrust->setInertia(m_velocity);
    m_rcsLeft->setInertia(m_velocity);
    m_rcsRight->setInertia(m_velocity);

    //REPORT("Current Speed", std::to_string(xy::Util::Vector::lengthSquared(m_velocity)));
    entity.getComponent<PlayerDrawable>()->setSpeed(getSpeed());
}

void PlayerController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void PlayerController::onDelayedStart(xy::Entity& entity)
{
    m_thrust = entity.getComponent<xy::ParticleSystem>("thrust");
    m_rcsLeft = entity.getComponent<xy::ParticleSystem>("rcsLeft");
    m_rcsRight = entity.getComponent<xy::ParticleSystem>("rcsRight");
    m_rcsDown = entity.getComponent<xy::ParticleSystem>("rcsDown");
    m_rcsEffectLeft = entity.getComponent<xy::AudioSource>("rcsEffectLeft");
    m_rcsEffectRight = entity.getComponent<xy::AudioSource>("rcsEffectRight");
    m_rcsEffectDown = entity.getComponent<xy::AudioSource>("rcsEffectDown");
    m_thrustEffect = entity.getComponent<xy::AudioSource>("thrustEffect");
}

void PlayerController::setInput(sf::Uint8 input)
{
    if (input != m_inputFlags && m_rcsRight)
    {
        if (input & LMInputFlags::SteerRight)
        {
            m_rcsRight->start();
            m_rcsEffectRight->play(true);
        }
        else
        {
            m_rcsRight->stop();
            m_rcsEffectRight->stop();
        }
        ///////////////////////////////////
        if (input & LMInputFlags::SteerLeft)
        {
            m_rcsLeft->start();
            m_rcsEffectLeft->play(true);
        }
        else
        {
            m_rcsLeft->stop();
            m_rcsEffectLeft->stop();
        }
        ///////////////////////////////////
        if (input & LMInputFlags::ThrustDown)
        {
            m_rcsDown->start();
            m_rcsEffectDown->play(true);
        }
        else
        {
            m_rcsDown->stop();
            m_rcsEffectDown->stop();
        }
        ///////////////////////////////////
        if (input & LMInputFlags::ThrustUp)
        {
            m_thrust->start();
            m_thrustEffect->play(true);
        }
        else
        {
            m_thrust->stop();
            m_thrustEffect->stop();
        }
    }
    
    m_inputFlags = input;
}

sf::Vector2f PlayerController::getPosition() const
{
    return m_entity->getPosition();
}

float PlayerController::getSpeed() const
{
    return std::min(1.f, xy::Util::Vector::lengthSquared(m_velocity) / maxLandingVelocity);
}

void PlayerController::collisionCallback(CollisionComponent* cc)
{
    switch (cc->getID())
    {
    case CollisionComponent::ID::Alien:
        if (!m_shield)
        {
            m_entity->destroy();
            broadcastDeath();
        }
        else
        {
            //deflect the player
            auto manifold = getManifold(cc->globalBounds());
            sf::Vector2f normal(manifold.x, manifold.y);
            m_entity->move(normal * manifold.z);
            m_velocity = xy::Util::Vector::reflect(m_velocity, normal);

            auto alienController = cc->getParentEntity().getComponent<AlienController>();
            if (alienController)
            {                
                m_velocity += alienController->getVelocity();
                m_velocity *= 0.5f;
            }
            else
            {
                auto roidController = cc->getParentEntity().getComponent<AsteroidController>();
                if (roidController)
                {
                    m_velocity += roidController->getVelocity();
                    m_velocity *= 0.15f;
                }
            }
            

            m_shield = false;
            m_entity->getComponent<PlayerDrawable>()->setShield(false);

            auto position = m_entity->getPosition();
            auto shieldMsg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
            shieldMsg->type = LMGameEvent::PlayerLostShield;
            shieldMsg->posX = position.x;
            shieldMsg->posY = position.y;
            shieldMsg->value = LMGameEvent::HitAlien;
        }
        break;
    case CollisionComponent::ID::Ammo:
    {
        auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
        msg->type = LMGameEvent::PlayerGotAmmo;
        auto position = m_entity->getPosition();
        msg->posX = position.x;
        msg->posY = position.y;
        msg->value = cc->getScoreValue();
    }
        break;
    case CollisionComponent::ID::Bounds:
    {
        auto manifold = getManifold(cc->globalBounds());
        sf::Vector2f normal(manifold.x, manifold.y);

        m_entity->move(normal * manifold.z);
        m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
        m_velocity *= 0.3f;// damping;
        //auto force = normal * manifold.z * 0.9f;// damping;
        //m_velocity += force;
    }
        break;
    case CollisionComponent::ID::Mothership:
        //if carrying drop human, raise message
        if (m_carrying && xy::Util::Vector::lengthSquared(m_velocity - m_mothership->getVelocity()) < maxDockingVelocity)
        {
            //we want to be moving slowly enough, and fully contained in mothership area
            if (xy::Util::Rectangle::contains(cc->globalBounds(), m_entity->getComponent<CollisionComponent>()->globalBounds()))
            {
                m_carrying = false;
                auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
                msg->type = LMGameEvent::HumanRescued;
                auto position = m_entity->getPosition();
                msg->posX = position.x;
                msg->posY = position.y;
                msg->value = cc->getScoreValue();
                //LOG("human saved!", xy::Logger::Type::Info);
            }
        }
        break;
    case CollisionComponent::ID::Shield:
    {
        auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
        msg->type = LMGameEvent::PlayerGotShield;
        auto position = m_entity->getPosition();
        msg->posX = position.x;
        msg->posY = position.y;
        msg->value = cc->getScoreValue();

        m_shield = true;
        m_entity->getComponent<PlayerDrawable>()->setShield(true);
    }
    break;
    case CollisionComponent::ID::Tower:
    {
        auto manifold = getManifold(cc->globalBounds());
        auto normal = sf::Vector2f(manifold.x, manifold.y);

        if (manifold.y != 0)
        {
            //we're on top 
            //measure velocity and assplode if too fast
            if (xy::Util::Vector::lengthSquared(m_velocity) > maxLandingVelocity)
            {
                if (!m_shield)
                {
                    //oh noes!
                    m_entity->destroy();
                    broadcastDeath();
                }
                else
                {
                    //BOOOIOIING!
                    m_entity->move(normal * manifold.z);
                    m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
                    m_velocity *= 0.9f;
                    //broke the shield :(
                    m_shield = false;
                    m_entity->getComponent<PlayerDrawable>()->setShield(false);

                    auto position = m_entity->getPosition();
                    auto shieldMsg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
                    shieldMsg->type = LMGameEvent::PlayerLostShield;
                    shieldMsg->posX = position.x;
                    shieldMsg->posY = position.y;
                    shieldMsg->value = LMGameEvent::HitGround;
                }
                break;
            }
           
            m_entity->move(normal * manifold.z);
            m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
            m_velocity.y = -0.1f; //anti-jiggle

            if (!m_carrying)
            {
                //stop and pickup
                updateState = std::bind(&PlayerController::landedState, this, _1, _2);

                auto position = m_entity->getWorldPosition();

                auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
                msg->type = LMGameEvent::PlayerLanded;
                msg->value = cc->getScoreValue();
                msg->posX = position.x;
                msg->posY = position.y;
            }
        }
        else
        {
            //crashded :(
            m_entity->destroy();
            broadcastDeath();
        }
    }
        break;
    default: break;
    }
}

void PlayerController::setSize(const sf::Vector2f& size)
{
    m_collisionSegments = 
    {
        sf::Vector2f(),
        {0.f, size.y},
        size,
        {size.x, 0.f}
    };
}

//private
bool PlayerController::collides(const sf::Vector2f& a1, const sf::Vector2f& a2, const sf::Vector2f& b1, const sf::Vector2f& b2, sf::Vector3f& manifold) const
{
    std::function<float(const sf::Vector2f&, const sf::Vector2f&)> det = [](const sf::Vector2f& a, const sf::Vector2f& b)
    {
        return ((a.y * b.x) - (a.x * b.y));
    };

    sf::Vector2f a(a2 - a1);
    sf::Vector2f b(b2 - b1);

    const float f = det(a, b);
    if (f == 0)
    {
        //lines are parallel
        return false;
    }

    sf::Vector2f c(b2 - a2);
    const float aa = det(a, c);
    const float bb = det(b, c);

    if (f < 0)
    {
        if (aa > 0) return false;
        if (bb > 0) return false;
        if (aa < f) return false;
        if (bb < f) return false;
    }
    else
    {
        if (aa < 0) return false;
        if (bb < 0) return false;
        if (aa > f) return false;
        if (bb > f) return false;
    }

    //calc the normal / penetration
    sf::Vector2f normal;
    float distance = bb / f; //normalised distance from A2 towards A1
    if (distance > 0.5)
    {
        distance = 1.f - distance;
        normal = xy::Util::Vector::normalise({ -b.y, b.x });
    }
    else
    {
        normal = xy::Util::Vector::normalise({ b.y, -b.x });
    }
    manifold.x = normal.x;
    manifold.y = normal.y;

    const float hyp = xy::Util::Vector::length(a * distance);
    const float dot = xy::Util::Vector::dot(a, b);
    const float theta = std::atan2(f, dot);
    manifold.z = std::sin(theta) * hyp; //penetration

    return true;
}

sf::Vector3f PlayerController::getManifold(const sf::FloatRect& worldRect)
{
    sf::FloatRect overlap;
    sf::FloatRect playerBounds = m_entity->getComponent<CollisionComponent>()->globalBounds();

    //we know we intersect, but we want the overlap
    worldRect.intersects(playerBounds, overlap);
    auto collisionNormal = sf::Vector2f(worldRect.left + (worldRect.width / 2.f), worldRect.top + (worldRect.height / 2.f)) - m_entity->getPosition();

    sf::Vector3f manifold;

    if (overlap.width < overlap.height)
    {
        manifold.x = (collisionNormal.x < 0) ? 1.f : -1.f;
        manifold.z = overlap.width;
    }
    else
    {
        manifold.y = (collisionNormal.y < 0) ? 1.f : -1.f;
        manifold.z = overlap.height;
    }

    return manifold;
}

void PlayerController::flyingState(xy::Entity& entity, float dt)
{
    auto entityYPos = entity.getPosition().y;
    
    //apply gravity every frame
    m_velocity += gravity;

    //check input and apply forces
    if (m_inputFlags & LMInputFlags::SteerRight)
    {
        m_velocity += thrustX;
    }

    if (m_inputFlags & LMInputFlags::SteerLeft)
    {
        m_velocity -= thrustX;
    }

    if ((m_inputFlags & LMInputFlags::ThrustUp)
        && entityYPos > maxSkyDistance)
    {
        m_velocity += thrustUp;
    }

    if (m_inputFlags & LMInputFlags::ThrustDown)
    {
        m_velocity -= thrustUp * 0.5f;
    }

    //apply drag
    m_velocity.x *= 0.999f;

    //move ship
    entity.move(m_velocity * dt);

    //if our position < someval vertically, move the camera & background
    if (entityYPos < topMargin)
    {
        xy::Command cmd;
        cmd.category = LMCommandID::Background;
        cmd.action = [this](xy::Entity& e, float dt)
        {
            e.move(0.f, m_velocity.y * dt);

            //REPORT("Speed", std::to_string(m_velocity.y));

            if (auto stars = e.getComponent<Starfield>())
            {
                stars->setVelocity({ 0.f, m_velocity.y });
                stars->setSpeedRatio(m_velocity.y / 400.f);
            }
        };
        entity.getScene()->sendCommand(cmd);
    }

    //see if we're near the gound and do a collision check
    auto position = entity.getPosition();
    if (position.y > m_highestTerrainPoint)
    {
        //TODO we're transforming 2 of the points twice here
        const auto& transform = entity.getTransform();
        for (auto i = 1u; i < m_collisionSegments.size(); ++i)
        {
            auto a1 = transform.transformPoint(m_collisionSegments[i - 1]);
            auto a2 = transform.transformPoint(m_collisionSegments[i]);

            //TODO we could improve this by partitioning segments
            //and only testing nearest to player
            bool collided = false;
            sf::Vector3f manifold;
            for (auto j = 1u; j < m_terrain.size(); ++j)
            {
                if ((collided = collides(a1, a2, m_terrain[j - 1], m_terrain[j], manifold)))
                {
                    if (m_shield)
                    {
                        //bounce off
                        sf::Vector2f normal(manifold.x, manifold.y);
                        m_entity->move(normal * manifold.z);
                        m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
                        m_velocity *= damping;

                        m_shield = false;
                        m_entity->getComponent<PlayerDrawable>()->setShield(false);

                        auto position = m_entity->getPosition();
                        auto shieldMsg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
                        shieldMsg->type = LMGameEvent::PlayerLostShield;
                        shieldMsg->posX = position.x;
                        shieldMsg->posY = position.y;
                        shieldMsg->value = LMGameEvent::HitGround;
                    }
                    else
                    {
                        m_entity->destroy();
                        broadcastDeath();
                    }
                    break;
                }
            }
            //don't test the rest of the player segments
            //else we get more than one collision event
            if (collided) break;
        }
    }
}

void PlayerController::landedState(xy::Entity& entity, float dt)
{
    //wweeee we are empty! do something about this :)
}

void PlayerController::broadcastDeath()
{
    auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
    msg->type = LMGameEvent::PlayerDied;
    //auto bounds = m_entity->globalBounds();
    msg->posX = m_entity->getPosition().x;// +(bounds.width / 2.f);
    msg->posY = m_entity->getPosition().y;// +(bounds.height / 2.f);

    sf::Int16 velX = static_cast<sf::Int16>(m_velocity.x);
    sf::Int16 velY = static_cast<sf::Int16>(m_velocity.y);

    msg->value = (velX << 16) | velY;
}
