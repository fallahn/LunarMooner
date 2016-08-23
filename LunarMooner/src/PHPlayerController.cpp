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
#include <CommandIDs.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Assert.hpp>

using namespace ph;

namespace
{
    const float speed = 500.f;
}

PlayerController::PlayerController(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_entity        (nullptr),
    m_inOrbit       (true)
{
    m_velocity = xy::Util::Vector::normalise({ 1.f, 0.87f });
}

//public
void PlayerController::entityUpdate(xy::Entity& ent, float dt)
{
    if(!m_inOrbit) ent.move(m_velocity * speed * dt);
}

void PlayerController::onStart(xy::Entity& e)
{
    m_entity = &e;
    XY_ASSERT(m_entity, "Invalid entity instance");
}

void PlayerController::collisionCallback(lm::CollisionComponent* cc)
{
    switch (cc->getID())
    {
    case lm::CollisionComponent::ID::Bounds:
    {
        //bool die = false;

        //if (!m_inOrbit)
        //{
        //    die = true;
        //}
        //else
        //{
        //    //use inner radius check
        //    auto collisionComponent = m_entity->getComponent<lm::CollisionComponent>();
        //    XY_ASSERT(collisionComponent, "missing player collision component");

        //    auto collisionDirection = collisionComponent->getCentre() - cc->getCentre();
        //    float distSqr = xy::Util::Vector::lengthSquared(collisionDirection);
        //    float minDistSqr = (collisionComponent->getInnerRadius() * collisionComponent->getInnerRadius()) + (cc->getInnerRadius() * cc->getInnerRadius());

        //    die = (distSqr < minDistSqr);
        //}

        //if (die)
        {
            kill();
        }
    }
        break;
    case lm::CollisionComponent::ID::Body:
        //use inner radius check
    {
        auto collisionComponent = m_entity->getComponent<lm::CollisionComponent>();
        XY_ASSERT(collisionComponent, "missing player collision component");

        auto collisionDirection = collisionComponent->getCentre() - cc->getCentre();
        float distSqr = xy::Util::Vector::lengthSquared(collisionDirection);
        float minDistSqr = (collisionComponent->getInnerRadius() * collisionComponent->getInnerRadius()) + (cc->getInnerRadius() * cc->getInnerRadius());

        if (distSqr < minDistSqr)
        {
            //collision
            kill();
        }
    }
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
}