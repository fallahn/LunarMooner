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
#include <CommandIds.hpp>

#include <xygine/components/Component.hpp>
#include <xygine/components/ParticleSystem.hpp>
#include <xygine/Scene.hpp>
#include <xygine/App.hpp>

#include <SFML/Audio/SoundBuffer.hpp>

#include <list>
#include <array>
#include <map>

namespace xy
{
    class SoundResource;
    class TextureResource;
    class FontResource;
}

struct ResourceCollection;

namespace lm
{
    class PlayerController;
    class CollisionWorld;
    class SpeedMeter;
    class ScoreDisplay;
    class Terrain;
    class SpriteBatch;
    class GameController final : public xy::Component
    {
    public:
        GameController(xy::MessageBus&, xy::Scene&, CollisionWorld&, const xy::App::AudioSettings&, ResourceCollection&);
        ~GameController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void setInput(sf::Uint8);
        
        void addPlayer();
        void start();

        void setDifficulty(xy::Difficulty d) { m_difficulty = d; }

    private:
        xy::Difficulty m_difficulty;

        xy::Scene& m_scene;
        CollisionWorld& m_collisionWorld;
        const xy::App::AudioSettings& m_audioSettings;
        ResourceCollection& m_resources;

        sf::Uint8 m_inputFlags;

        std::map<sf::Int32, sf::SoundBuffer> m_soundCache;

        std::array<xy::ParticleSystem::Definition, 4u> m_particleDefs;
        bool m_spawnReady;
        PlayerController* m_player;
        void spawnPlayer();
        bool m_timeRound;

        xy::Entity* m_mothership;
        void createMothership();

        Terrain* m_terrain;

        std::vector<xy::Entity*> m_humans;
        void spawnHuman(const sf::Vector2f&);
        void spawnHumans();

        enum EventID
        {
            SpawnRoid = 1,
            SpawnPlayer = 2
        };
        struct DelayedEvent
        {
            float time = 0.f;
            std::function<void()> action;
            sf::Int32 id = 0;
        };
        std::list<DelayedEvent> m_delayedEvents;
        std::list<DelayedEvent> m_pendingDelayedEvents;

        std::vector<xy::Entity*> m_aliens;
        SpriteBatch* m_alienBatch;
        void spawnAlien(const sf::Vector2f&);
        void spawnAliens();

        void createTerrain();
        void updatePlatforms();

        void addRescuedHuman();
        void spawnBullet(const sf::Vector2f&, LMDirection = LMDirection::Up);
        void fireSpecial();

        SpeedMeter* m_speedMeter;
        ScoreDisplay* m_scoreDisplay;
        void createUI();

        std::vector<PlayerState> m_playerStates;
        std::size_t m_currentPlayer;
        void storePlayerState();
        void swapPlayerState();
        void restorePlayerState();

        void moveToNextRound();
        void restartRound();
        void addDelayedRespawn();

        void spawnEarlyWarning(const sf::Vector2f&);
        void spawnAsteroid(const sf::Vector2f&);
        void addDelayedAsteroid();
        bool m_flushRoidEvents;

        sf::Uint8 m_itemCount;
        void spawnCollectable(const sf::Vector2f&);

        void showRoundSummary(bool);

        void spawnDeadGuy(float, float);
    };
}

#endif //LM_GAME_CONTROLLER_HPP_