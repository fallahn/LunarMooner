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

#ifndef LM_PLAYER_INF_DISP_HPP_
#define LM_PLAYER_INF_DISP_HPP_

#include <LMPlayerState.hpp>
#include <LMLevelMeter.hpp>
#include <LMClockDisplay.hpp>
#include <LMCounterDisplay.hpp>

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace xy
{
    class FontResource;
}

struct ResourceCollection;

namespace lm
{
    class ScoreDisplay final : public sf::Drawable, public xy::Component
    {
    public:
        ScoreDisplay(xy::MessageBus&, ResourceCollection&, std::vector<PlayerState>&);
        ~ScoreDisplay() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }        
        void entityUpdate(xy::Entity&, float) override;
        sf::FloatRect globalBounds() const override { return m_bounds; }

        void setPlayerActive(std::size_t);
        void showMessage(const std::string&);
        void showScore(sf::Uint16, const sf::Vector2f&, sf::Color = sf::Color::Yellow);

    private:
        sf::FloatRect m_bounds;

        xy::FontResource& m_fontResource;
        std::vector<PlayerState>& m_playerStates;

        sf::Text m_messageText;
        bool m_showMessage;
        float m_messageDisplayTime;

        struct UIElements final : public sf::Drawable
        {
            UIElements(ResourceCollection&, sf::Uint8);
            void update(float);

            ClockDisplay clockDisplay;
            CounterDisplay ammo;
            CounterDisplay lives;
            CounterDisplay score;
            LevelMeter level;

        private:
            void draw(sf::RenderTarget&, sf::RenderStates) const override;
        };
        std::vector<UIElements> m_uiElements;
        sf::Sprite m_activePlayerSprite;

        struct ScoreTag final
        {
            sf::Text text;
            float alpha = 1.f;
            void update(float);
        };
        std::list<ScoreTag> m_scoreTags;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_PLAYER_INF_DISP_HPP_