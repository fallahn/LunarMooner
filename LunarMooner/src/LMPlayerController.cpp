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
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Rectangle.hpp>
#include <xygine/Reports.hpp>
#include <xygine/components/ParticleSystem.hpp>

using namespace lm;
using namespace std::placeholders;

namespace
{
    const sf::Vector2f gravity(0.f, 1.5f);
    const sf::Vector2f thrustX(3.6f, 0.f);
    const sf::Vector2f thrustUp(0.f, -3.8f);

    const float maxDockingVelocity = 15000.f;
    const float maxLandingVelocity = 16000.f ;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_inputFlags    (0),
    m_entity        (nullptr),
    m_carrying      (false),
    m_thrust        (nullptr),
    m_rcsLeft       (nullptr),
    m_rcsRight      (nullptr)
{
    m_velocity.y = 2.f;

    updateState = std::bind(&PlayerController::flyingState, this, _1, _2);

    xy::Component::MessageHandler handler;
    handler.id = LMMessageId::LMMessage;
    handler.action = [this](xy::Component* c, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMEvent::HumanPickedUp:
            m_velocity = { 0.f, -82.5f };
            updateState = std::bind(&PlayerController::flyingState, this, _1, _2);
            break;
        }
    };
    addMessageHandler(handler);
}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    updateState(entity, dt);

    m_thrust->setInertia(m_velocity);
    m_rcsLeft->setInertia(m_velocity);
    m_rcsRight->setInertia(m_velocity);

    //REPORT("Current Speed", std::to_string(xy::Util::Vector::lengthSquared(m_velocity)));
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
}

void PlayerController::setInput(sf::Uint8 input)
{
    if (input != m_inputFlags)
    {
        (input & LMInputFlags::SteerRight) ? m_rcsRight->start() : m_rcsRight->stop();
        (input & LMInputFlags::SteerLeft) ? m_rcsLeft->start() : m_rcsLeft->stop();
        (input & LMInputFlags::Thrust) ? m_thrust->start() : m_thrust->stop();
    }
    
    m_inputFlags = input;
}

sf::Vector2f PlayerController::getPosition() const
{
    return m_entity->getPosition();
}

float PlayerController::getSpeed() const
{
    return xy::Util::Vector::lengthSquared(m_velocity);
}

void PlayerController::destroy()
{
    Component::destroy();
    auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
    msg->type = LMEvent::PlayerDied;
    //auto bounds = m_entity->globalBounds();
    msg->posX = m_entity->getPosition().x;// +(bounds.width / 2.f);
    msg->posY = m_entity->getPosition().y;// +(bounds.height / 2.f);
}

void PlayerController::collisionCallback(CollisionComponent* cc)
{
    switch (cc->getID())
    {
    case CollisionComponent::ID::Alien:
        m_entity->destroy();
        break;
    case CollisionComponent::ID::Bounds:
    {
        auto manifold = getManifold(cc->globalBounds());
        sf::Vector2f normal(manifold.x, manifold.y);

        m_entity->move(normal * manifold.z);
        m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
        m_velocity *= 0.65f; //some damping
    }
        break;
    case CollisionComponent::ID::Mothership:
        //if carrying drop human, raise message
        if (m_carrying && xy::Util::Vector::lengthSquared(m_velocity) < maxDockingVelocity)
        {
            //we want to be moving slowly enough, and fully contained in mothership area
            if (xy::Util::Rectangle::contains(cc->globalBounds(), m_entity->getComponent<CollisionComponent>()->globalBounds()))
            {
                m_carrying = false;
                auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
                msg->type = LMEvent::HumanRescued;
                LOG("human saved!", xy::Logger::Type::Info);
            }
        }
        break;
    case CollisionComponent::ID::Tower:
    {
        auto manifold = getManifold(cc->globalBounds());
        if (manifold.y != 0)
        {
            //we're on top 
            //measure velocity and assplode if too fast
            if (xy::Util::Vector::lengthSquared(m_velocity) > maxLandingVelocity)
            {
                //oh noes!
                m_entity->destroy();
                break;
            }

            auto normal = sf::Vector2f(manifold.x, manifold.y);
            m_entity->move(normal * manifold.z);
            m_velocity = xy::Util::Vector::reflect(m_velocity, normal);
            m_velocity.y = -0.1f; //anti-jiggle

            if (!m_carrying)
            {
                //stop and pickup
                m_carrying = true;
                updateState = std::bind(&PlayerController::landedState, this, _1, _2);

                auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
                msg->type = LMEvent::PlayerLanded;
                msg->value = cc->getScoreValue();
            }
        }
        else
        {
            //crashded :(
            m_entity->destroy();
        }
    }
        break;
    default: break;
    }
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

    if (m_inputFlags & LMInputFlags::Thrust)
    {
        m_velocity += thrustUp;
    }


    //apply drag
    m_velocity.x *= 0.999f;

    //move ship
    entity.move(m_velocity * dt);
}

void PlayerController::landedState(xy::Entity& entity, float dt)
{
    //wweeee we are empty! do something about this :)
}
