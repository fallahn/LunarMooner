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

#ifndef RD_GAME_CONTROLLER_HPP_
#define RD_GAME_CONTROLLER_HPP_

#include <ResourceCollection.hpp>

#include <xygine/components/Component.hpp>

namespace xy
{
    class Scene;
}

namespace lm
{
    class CollisionWorld;
}

namespace rd
{
    class GameController final : public xy::Component
    {
    public:
        GameController(xy::MessageBus&, xy::Scene&, ResourceCollection&, lm::CollisionWorld&);
        ~GameController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void fireLaser(const sf::Vector2f&);
        bool gameEnded() const { return m_remainingTime < 0 && !m_roundStarted; }

    private:

        xy::Scene& m_scene;
        ResourceCollection& m_resources;
        lm::CollisionWorld& m_collisionWorld;

        float m_remainingTime;
        bool m_roundStarted;

        float m_spawnTime;

        void buildBackground();

        void spawnPlayer();
        void spawnRock();
        void spawnDoofer();
    };
}

#endif //RD_GAME_CONTROLLER_HPP_