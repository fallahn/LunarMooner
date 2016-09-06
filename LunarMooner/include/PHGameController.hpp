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

#ifndef PH_GAME_CONTROLLER_HPP_
#define PH_GAME_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

namespace xy
{
    class Scene;
    class MeshRenderer;
}

namespace lm
{
    class CollisionWorld;
}

struct ResourceCollection;
namespace ph
{
    class GameController final : public xy::Component
    {
    public:
        GameController(xy::MessageBus&, ResourceCollection&, xy::Scene&, lm::CollisionWorld&, xy::MeshRenderer&);
        ~GameController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void spawnPlayer();
        bool gameEnded() const;

    private:
        ResourceCollection& m_resources;
        xy::Scene& m_scene;
        lm::CollisionWorld& m_collisionWorld;
        xy::MeshRenderer& m_meshRenderer;

        sf::Vector2f m_spawnPosition;
        bool m_playerSpawned;

        sf::Uint64 m_targetUID;
        sf::Clock m_targetClock;
        sf::Uint64 m_currentParent;
        float m_lastOrbitTime;

        bool m_gameEnded;

        void buildScene();
        xy::Entity* addBody(const sf::Vector2f&, float);
        void addMessageHandlers();
        void spawnDebris();
        void buildUI();
        void showMessage(const std::string&, const std::string&);
    };
}

#endif //PH_GAME_CONTROLLER_HPP_