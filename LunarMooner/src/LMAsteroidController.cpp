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

#include <LMAsteroidController.hpp>
#include <LMCollisionComponent.hpp>
#include <CommandIds.hpp>

#include <xygine/util/Vector.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/AudioSource.hpp>

#include <SFML/Graphics/RectangleShape.hpp>

using namespace lm;

namespace
{
    const float speed = 1000.f;

    const float shieldRadius = 3000.f * 3000.f;
    const sf::Vector2f shieldCentre(960.f, 3700.f);

    bool collides(const sf::Vector2f& position)
    {
        return (xy::Util::Vector::lengthSquared(position - shieldCentre) < shieldRadius);
    }
}

AsteroidController::AsteroidController(xy::MessageBus& mb, const sf::FloatRect& bounds, const sf::Vector2f& vel)
    : xy::Component (mb, this),
    m_bounds        (bounds),
    m_trail         (nullptr),
    m_entity        (nullptr),
    m_velocity      (vel)
{
    m_velocity = xy::Util::Vector::normalise(m_velocity);
}

//public
void AsteroidController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(m_velocity * speed * dt);
    entity.rotate(25.f * dt);

    auto position = entity.getPosition();

    if ((position.x < m_bounds.left || position.x > m_bounds.left + m_bounds.width || /*position.y > 1080.f*/collides(position))
        && m_trail->started())
    {
        //raise message
        auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
        msg->type = LMGameEvent::MeteorExploded;
        msg->posX = position.x;
        msg->posY = position.y;
        msg->value = 0;
        
        m_trail->stop();
        entity.getComponent<xy::AudioSource>()->stop();

        entity.getComponent<xy::SfDrawableComponent<sf::RectangleShape>>()->getDrawable().setFillColor(sf::Color::Transparent);
        m_velocity = sf::Vector2f();
        //keep just about in bounds to prevent accidental culling of trail by renderer
        //but preventing accidental collision 
        entity.setPosition(position.x, 1078.f); 
    }
    if (!m_trail->started() && !m_trail->active())
    {
        entity.destroy();
    }
}

void AsteroidController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
    m_trail = m_entity->getComponent<xy::ParticleSystem>();
    XY_ASSERT(m_trail, "Asteroid trail nullptr");
    m_trail->start();
}

sf::Vector2f AsteroidController::getVelocity() const
{
    return m_velocity * speed;
}

void  AsteroidController::collisionCallback(CollisionComponent* cc)
{
    if(cc->getID() == CollisionComponent::ID::Bullet)
    {
        auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
        msg->type = LMGameEvent::MeteorExploded;
        msg->posX = m_entity->getPosition().x;
        msg->posY = m_entity->getPosition().y;
        msg->value = m_entity->getComponent<CollisionComponent>()->getScoreValue();
        m_entity->destroy();
    }
}