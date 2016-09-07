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

#include <PHPlayerController.hpp>
#include <LMCollisionComponent.hpp>
#include <PHBeamDrawable.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace ph;

namespace
{
    const float rcsStrength = 230.f;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_inOrbit       (true),
    m_input         (0)
{
    //need to set the initial velocity so we're rotating in the correct direction
    m_velocity.y = 400.f;
    m_rightVector.x = m_velocity.y;
    
    xy::Component::MessageHandler mh;
    mh.id = GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& msgData = msg.getData<LMGameEvent>();
        switch(msgData.type)
        {
        default: break;
        case LMGameEvent::EnteredOrbit:
            m_inOrbit = true;
            LOG("Entered orbit", xy::Logger::Type::Info);
            break;
        case LMGameEvent::TimerExpired:
            kill();
            break;
        }
    };
    addMessageHandler(mh);
}

//public
void PlayerController::entityUpdate(xy::Entity& entity, float dt)
{
    if (!m_inOrbit)
    {
        entity.move(m_velocity * dt);
        if (m_input & LMInputFlags::SteerRight)
        {
            m_velocity += (m_rightVector * dt);
            m_rightVector = xy::Util::Vector::normalise({ m_velocity.y, -m_velocity.x })* rcsStrength;
        }
        if (m_input & LMInputFlags::SteerLeft)
        {
            m_velocity += (-m_rightVector * dt);
            m_rightVector = xy::Util::Vector::normalise({ m_velocity.y, -m_velocity.x })* rcsStrength;
        }
    }
    m_input = 0;
}

void PlayerController::onStart(xy::Entity& e)
{
    m_entity = &e;
    XY_ASSERT(m_entity, "Invalid entity instance");
}

void PlayerController::leaveOrbit(const sf::Vector2f& newVelocity)
{
    if (!m_inOrbit) return;
    m_velocity = newVelocity;
    m_rightVector = xy::Util::Vector::normalise({ newVelocity.y, -newVelocity.x }) * rcsStrength;
    m_inOrbit = false;

    auto msg = sendMessage<LMGameEvent>(GameEvent);
    msg->type = LMGameEvent::LeftOrbit;
    msg->posX = m_entity->getWorldPosition().x;
    msg->posY = m_entity->getWorldPosition().y;

    if (auto beam = m_entity->getComponent<BeamDrawable>())
    {
        beam->destroy();
    }
}

void PlayerController::collisionCallback(lm::CollisionComponent* cc)
{
    std::function<bool()> innerCollision = [this, cc]()->bool
    {
        auto collisionComponent = m_entity->getComponent<lm::CollisionComponent>();
        XY_ASSERT(collisionComponent, "missing player collision component");

        auto collisionDirection = collisionComponent->getCentre() - cc->getCentre();
        float distSqr = xy::Util::Vector::lengthSquared(collisionDirection);
        float minDistSqr = (collisionComponent->getInnerRadius() * collisionComponent->getInnerRadius()) + (cc->getInnerRadius() * cc->getInnerRadius());

        return (distSqr < minDistSqr);
    };
    
    switch (cc->getID())
    {
    default: break;
    case lm::CollisionComponent::ID::Bounds:   
    {
        kill();
    }
        break;
    case lm::CollisionComponent::ID::Body:
    case lm::CollisionComponent::ID::Alien:
        //use inner radius check
        if (innerCollision()) kill();
        break;
    }
    
}

//private
sf::Vector3f PlayerController::getManifold(const sf::FloatRect& worldRect)
{
    sf::FloatRect overlap;
    sf::FloatRect playerBounds = m_entity->getComponent<lm::CollisionComponent>()->globalBounds();

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

void PlayerController::kill()
{
    m_entity->destroy();
    auto msg = sendMessage<LMGameEvent>(GameEvent);
    msg->type = LMGameEvent::PlayerDied;
    msg->posX = m_entity->getWorldPosition().x;
    msg->posY = m_entity->getWorldPosition().y;

    LOG("Player Died!", xy::Logger::Type::Info);
}