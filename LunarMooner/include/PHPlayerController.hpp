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

#ifndef PH_PLAYER_CONTROLLER_HPP_
#define PH_PLAYER_CONTROLLER_HPP_

#include <CommandIds.hpp>

#include <xygine/components/Component.hpp>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

namespace lm
{
    class CollisionComponent;
}

namespace ph
{
    class PlayerController final : public xy::Component
    {
    public:
        explicit PlayerController(xy::MessageBus&);
        ~PlayerController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        void leaveOrbit(const sf::Vector2f&);
        void moveLeft() { m_input |= LMInputFlags::SteerLeft; }
        void moveRight() { m_input |= LMInputFlags::SteerRight; }
        void setInput(const sf::Uint8 ip) { m_input = ip; }
        const sf::Vector2f& getVelocity() const { return m_velocity; }

        void collisionCallback(lm::CollisionComponent*);

    private:
        sf::Vector2f m_velocity;
        sf::Vector2f m_rightVector;
        xy::Entity* m_entity;

        bool m_inOrbit;
        sf::Uint8 m_input;

        sf::Vector3f getManifold(const sf::FloatRect&);
        void kill();
    };
}

#endif //PH_PLAYER_CONTROLLER_HPP_