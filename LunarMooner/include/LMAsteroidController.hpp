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

#ifndef LM_ASTEROID_CONTROLLER_HPP_
#define LM_ASTEROID_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

namespace xy
{
    class ParticleSystem;
}

namespace lm
{
    class CollisionComponent;
    class AsteroidController final : public xy::Component
    {
    public:
        AsteroidController(xy::MessageBus&, const sf::FloatRect&, const sf::Vector2f&);
        ~AsteroidController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void onStart(xy::Entity&) override;

        sf::Vector2f getVelocity() const;
        void collisionCallback(CollisionComponent*);

    private:
        sf::FloatRect m_bounds;
        xy::ParticleSystem* m_trail;
        xy::Entity* m_entity;
        sf::Vector2f m_velocity;
    };
}

#endif //LM_ASTEROID_CONTROLLER_HPP_