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

#include <xygine/util/Vector.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

#include <SFML/Graphics/RectangleShape.hpp>

using namespace lm;

namespace
{
    const float speed = 1000.f;
}

AsteroidController::AsteroidController(xy::MessageBus& mb, const sf::FloatRect& bounds)
    : xy::Component (mb, this),
    m_bounds        (bounds),
    m_trail         (nullptr),
    m_entity        (nullptr),
    m_velocity      (-1.f, 1.f)
{
    m_velocity = xy::Util::Vector::normalise(m_velocity);
}

//public
void AsteroidController::entityUpdate(xy::Entity& entity, float dt)
{
    entity.move(m_velocity * speed * dt);
    entity.rotate(25.f * dt);

    auto position = entity.getPosition();

    if (position.x < m_bounds.left || position.y > 1080.f
        && m_trail->started())
    {
        m_trail->stop();
        entity.getComponent<xy::SfDrawableComponent<sf::RectangleShape>>()->getDrawable().setFillColor(sf::Color::Transparent);
        m_velocity = sf::Vector2f();
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