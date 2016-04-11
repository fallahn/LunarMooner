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

#ifndef LM_PLAYER_CONTROLLER_HPP_
#define LM_PLAYER_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/System/Vector3.hpp>

namespace xy
{
    class ParticleSystem;
    class AudioSource;
}

namespace lm
{
    class CollisionComponent;
    class MothershipController;
    class PlayerController final : public xy::Component
    {
    public:
        explicit PlayerController(xy::MessageBus&, const MothershipController*, const std::vector<sf::Vector2f>&);
        ~PlayerController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;
        void onDelayedStart(xy::Entity&) override;
        void setInput(sf::Uint8);

        sf::Vector2f getPosition() const;
        float getSpeed() const;
        bool carryingHuman() const { return m_carrying; }

        void collisionCallback(CollisionComponent*);

        void setSize(const sf::Vector2f&);

    private:
        const MothershipController* m_mothership;
        sf::Uint8 m_inputFlags;
        sf::Vector2f m_velocity;
        xy::Entity* m_entity;

        bool m_carrying;
        bool m_shield;

        xy::ParticleSystem* m_thrust;
        xy::ParticleSystem* m_rcsLeft;
        xy::ParticleSystem* m_rcsRight;

        xy::AudioSource* m_rcsEffectLeft;
        xy::AudioSource* m_rcsEffectRight;
        xy::AudioSource* m_thrustEffect;

        float m_highestTerrainPoint;
        const std::vector<sf::Vector2f>& m_terrain;
        std::vector<sf::Vector2f> m_collisionSegments;
        bool collides(const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&, const sf::Vector2f&, sf::Vector3f&) const; 

        sf::Vector3f getManifold(const sf::FloatRect&);

        using UpdateState = std::function<void(xy::Entity&, float)>;
        UpdateState updateState;

        void flyingState(xy::Entity&, float);
        void landedState(xy::Entity&, float);

        void broadcastDeath();
    };
}

#endif //LM_PLAYER_CONTROLLER_HPP_
