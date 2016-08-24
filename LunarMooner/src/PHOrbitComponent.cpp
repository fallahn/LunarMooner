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

#include <PHOrbitComponent.hpp>
#include <PHPlayerController.hpp>
#include <LMCollisionComponent.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Vector.hpp>

using namespace ph;

namespace
{
    const float maxRotation = 256.f;
    const float rotationMultiplier = 4.f;
}

OrbitComponent::OrbitComponent(xy::MessageBus& mb, float radius)
    :xy::Component      (mb, this),
    m_radius            (radius),
    m_influenceRadius   (radius * 2.1f),
    m_parentID          (0),
    m_lastParentID      (0),
    m_entity            (nullptr),
    m_rotationSpeed     (0.f),
    m_rotation          (LMDirection::Right)
{

}

//public
void OrbitComponent::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_parentID)
    {
        entity.rotate(m_rotationSpeed * dt);
    }
}

void OrbitComponent::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

void OrbitComponent::collisionCallback(lm::CollisionComponent* cc)
{
    switch (cc->getID())
    {
    default: break;
    case lm::CollisionComponent::ID::Gravity:
    {       
        //if our inner radius intersects the other outer radius
        if (auto otherOrbit = cc->getParentEntity().getComponent<OrbitComponent>())
        {
            auto distSqr = (otherOrbit->getInfluenceRadius() * otherOrbit->getInfluenceRadius()) + (m_radius * m_radius);
            auto otherPosition = cc->getParentEntity().getWorldPosition();
            auto position = m_entity->getWorldPosition();
            auto direction = position - otherPosition;
            auto lenSqr = xy::Util::Vector::lengthSquared(direction);
            if (lenSqr < distSqr)
            {
                auto otherID = cc->getParentUID();
                if (otherID != m_entity->getUID())
                {
                    //this isn't us, is it new?
                    if (m_parentID == 0
                        && otherID != m_lastParentID)
                    {
                        //wait until tangetial before entering orbit
                        if (auto player = m_entity->getComponent<PlayerController>())
                        {
                            if (xy::Util::Vector::dot(-direction, player->getVelocity()) > 0)
                            {
                                break;
                            }

                            //see which direction we're coming from
                            if
                            (
                                ((direction.x < 0) && (otherPosition.y > position.y)) ||
                                ((direction.y > 0) && (otherPosition.x > position.x)) ||
                                ((direction.x > 0) && (otherPosition.y < position.y)) ||
                                ((direction.y < 0) && (otherPosition.x < position.x))
                            )m_rotation = LMDirection::Left;
                        }
                        //switch to new parent
                        setParent(cc->getParentEntity());
                    }
                }
            }
        }
    }
        break;
    }
}

sf::Vector2f OrbitComponent::removeParent()
{
    //look at which way we're rotating
    auto position = m_entity->getWorldPosition();
    auto parentDir = position - m_parentPosition;
    sf::Vector2f retVal = (m_rotation == LMDirection::Right) 
        ? sf::Vector2f(-parentDir.y, parentDir.x) : sf::Vector2f(parentDir.y, -parentDir.x);
    
    m_lastParentID = m_parentID;
    m_parentID = 0;

    m_entity->setOrigin(0.f, 0.f);
    m_entity->setPosition(position);

    LOG("Left orbit", xy::Logger::Type::Info);
    return retVal;
}

//private
void OrbitComponent::setParent(const xy::Entity& entity)
{
    //only orbit bigger objects
    if (entity.getComponent<OrbitComponent>()->getInfluenceRadius() < m_influenceRadius) return;
    LOG("Setting orbit parent to: " + std::to_string(entity.getUID()), xy::Logger::Type::Info);
    
    if (m_parentID) m_lastParentID = m_parentID;
    m_parentID = entity.getUID();

    //set up properties for motion
    m_parentPosition = entity.getWorldPosition();
    m_entity->setOrigin(m_entity->getWorldPosition() - m_parentPosition);
    m_entity->move(-m_entity->getOrigin());
    //m_entity->setRotation(0.f);
    
    //TODO calculate which direction this should be
    m_rotationSpeed = (maxRotation - xy::Util::Vector::length(m_entity->getOrigin())) * rotationMultiplier;
    if (m_rotation == LMDirection::Left) m_rotationSpeed = -m_rotationSpeed;

    auto msg = sendMessage<LMGameEvent>(GameEvent);
    msg->type = LMGameEvent::EnteredOrbit;
}