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

#ifndef LM_GAME_CONTROLLER_HPP_
#define LM_GAME_CONTROLLER_HPP_

#include <LMPlayerState.hpp>

#include <xygine/components/Component.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/Scene.hpp>
#include <xygine/Resource.hpp>

#include <list>
#include <array>

namespace lm
{
    class PlayerController;
    class CollisionWorld;
    class SpeedMeter;
    class ScoreDisplay;
    class GameController final : public xy::Component
    {
    public:
        GameController(xy::MessageBus&, xy::Scene&, CollisionWorld&);
        ~GameController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void setInput(sf::Uint8);
        
        void addPlayer();
        void start();

    private:
        xy::Scene& m_scene;
        CollisionWorld& m_collisionWorld;
        xy::TextureResource m_textureResource;
        xy::FontResource m_fontResource;

        sf::Uint8 m_inputFlags;

        std::array<xy::ParticleSystem::Definition, 3u> m_playerParticles;
        bool m_spawnReady;
        PlayerController* m_player;
        void spawnPlayer();

        xy::Entity* m_mothership;
        void createMothership();

        std::vector<xy::Entity*> m_humans;
        void spawnHuman(const sf::Vector2f&);
        void spawnHumans();

        struct DelayedEvent
        {
            float time = 0.f;
            std::function<void()> action;
        };
        std::list<DelayedEvent> m_delayedEvents;

        std::vector<xy::Entity*> m_aliens;
        void spawnAlien(const sf::Vector2f&);
        void spawnAliens();

        void createTerrain();

        void addRescuedHuman();
        void spawnBullet();

        SpeedMeter* m_speedMeter;
        ScoreDisplay* m_scoreDisplay;
        void createUI();

        std::vector<PlayerState> m_playerStates;
        std::size_t m_currentPlayer;
        void storePlayerState();
        void swapPlayerState();
        void restorePlayerState();

        void moveToNextRound();
        void addDelayedRespawn();
    };
}

#endif //LM_GAME_CONTROLLER_HPP_