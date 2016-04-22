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

#include <LMWeaponEMP.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Reports.hpp>
#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const float startIncrease = 470.f;
    const float decreasePerSecond = 540.f;
}

WeaponEMP::WeaponEMP(xy::MessageBus& mb, const xy::Scene& scene)
    : xy::Component (mb, this),
    m_scene         (scene),
    m_radiusIncrease(startIncrease)
{
    m_shape.setRadius(0.5f);
    m_shape.setOrigin(0.25f, 0.25f);
    m_shape.setFillColor(sf::Color::Transparent);
    m_shape.setOutlineColor(sf::Color::Magenta);
    m_shape.setOutlineThickness(2.f);
}

//public
void WeaponEMP::entityUpdate(xy::Entity& entity, float dt)
{
    m_radiusIncrease = std::max(0.f, m_radiusIncrease - (decreasePerSecond * dt));
    
    auto radius = m_shape.getRadius();
    radius += m_radiusIncrease * dt;
    m_shape.setRadius(radius);
    m_shape.setOrigin(radius, radius);

    if (m_radiusIncrease == 0)
    {
        entity.destroy();
    }

    const float alpha = std::max(0.f, m_radiusIncrease / startIncrease);
    sf::Color c(255u, 0u, 255u, static_cast<sf::Uint8>(alpha * 255.f));
    m_shape.setOutlineColor(c);

    //collision detection
    auto position = entity.getPosition();
    const auto objects = m_scene.queryQuadTree({ position.x - radius, position.y - radius, radius * 2.f, radius * 2.f });
    for (const auto& o : objects)
    {
        REPORT("Emp Collision", std::to_string(objects.size()));
        CollisionComponent* cc = nullptr;
        if (!o->destroyed() && (cc = o->getEntity()->getComponent<CollisionComponent>()))
        {
            //don't accidentally kill powerups!
            switch(auto id = cc->getID())
            {
            default:break;
            case CollisionComponent::ID::Alien:
            case CollisionComponent::ID::Ammo:
            case CollisionComponent::ID::Shield:
            {
                sf::FloatRect ccBounds = cc->globalBounds();
                sf::Vector2f ccPosition(ccBounds.left + (ccBounds.width / 2.f), ccBounds.top + (ccBounds.height / 2.f));
                sf::Vector2f distance = position = ccPosition;
                float distSquared = xy::Util::Vector::lengthSquared(distance);
                float radiusSquared = radius * radius;

                //test outer radius
                if ((cc->getOuterRadius() * cc->getOuterRadius()) + radiusSquared > distSquared)
                {
                    //we can't be colliding
                    break;
                }

                //test inner radius
                if ((cc->getInnerRadius() *  cc->getInnerRadius()) + radiusSquared < distSquared)
                {
                    //we're definitely colliding
                    //kill the colliding thing
                    killThing(cc, id);
                    break;
                }

                //test point in rect
                sf::Vector2f point = xy::Util::Vector::normalise(distance) * radius;
                if (ccBounds.contains(position + point))
                {
                    //we're inside the box
                    //kill thing
                    killThing(cc, id);
                }

            }
                break;
            }
        }
    }
}


//private
void WeaponEMP::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.blendMode = sf::BlendAdd;
    rt.draw(m_shape, states);
}

void WeaponEMP::killThing(CollisionComponent* cc, CollisionComponent::ID id)
{
    cc->getParentEntity().destroy();

    auto position = cc->getParentEntity().getPosition();
    auto msg = getMessageBus().post<LMGameEvent>(LMMessageId::GameEvent);
    msg->posX = position.x;
    msg->posY = position.y;
    msg->type = (id == CollisionComponent::ID::Alien) ? LMGameEvent::AlienDied : LMGameEvent::CollectibleDied;
    msg->value = (id == CollisionComponent::ID::Alien) ? 2 : 0; //let's not be too generous for being over powered :)
}