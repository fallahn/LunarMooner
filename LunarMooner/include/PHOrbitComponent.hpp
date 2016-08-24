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

#ifndef PH_ORBIT_COMPONENT_HPP_
#define PH_ORBIT_COMPONENT_HPP_

#include <CommandIds.hpp>

#include <xygine/components/Component.hpp>

#include <SFML/System/Vector2.hpp>

namespace lm
{
    class CollisionComponent;
}

namespace ph
{
    class OrbitComponent final : public xy::Component
    {
    public:
        OrbitComponent(xy::MessageBus&, float);
        ~OrbitComponent() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        float getRadius() const { return m_radius; }
        float getInfluenceRadius() const { return m_influenceRadius; }

        void collisionCallback(lm::CollisionComponent*);

        sf::Vector2f removeParent();

    private:
        float m_radius;
        float m_influenceRadius;

        sf::Uint64 m_parentID; //currently orbiting...
        sf::Uint64 m_lastParentID; //previously orbiting

        xy::Entity* m_entity;
        float m_rotationSpeed;
        sf::Vector2f m_parentPosition;
        LMDirection m_rotation; //right is clockwise, because that's the side of the cockpit it would appear on

        void setParent(const xy::Entity&);
    };
}

#endif //PH_ORBIT_COMPONENT_HPP_