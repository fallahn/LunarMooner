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

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Text.hpp>

namespace xy
{
    class FontResource;
}

namespace lm
{
    class ScoreDisplay final : public sf::Drawable, public xy::Component
    {
    public:
        ScoreDisplay(xy::MessageBus&, xy::FontResource&, std::vector<PlayerState>&);
        ~ScoreDisplay() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        //TODO measure this and only update when values change if needs be
        void entityUpdate(xy::Entity&, float) override; 

        void showMessage(const std::string&);
        void showScore(sf::Uint16, const sf::Vector2f&, sf::Color = sf::Color::Yellow);

    private:
        xy::FontResource& m_fontResource;
        std::vector<PlayerState>& m_playerStates;

        sf::Text m_messageText;
        bool m_showMessage;
        float m_messageDisplayTime;

        std::vector<sf::Text> m_playerTexts;

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