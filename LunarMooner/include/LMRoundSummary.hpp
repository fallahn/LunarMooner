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

#ifndef LM_ROUND_SUMMARY_HPP_
#define LM_ROUND_SUMMARY_HPP_

#include <LMPlayerState.hpp>

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/Audio/Sound.hpp>

namespace xy
{
    class TextureResource;
    class FontResource;
}

namespace lm
{
    class RoundSummary final : public xy::Component, public sf::Drawable
    {
    public:
        RoundSummary(xy::MessageBus&, PlayerState&, xy::TextureResource&, xy::FontResource&, bool = true);
        ~RoundSummary() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        void completeSummary();

        void setSoundBuffer(sf::Int32, sf::SoundBuffer&, float);

    private:
        
        xy::Entity* m_entity;
        PlayerState& m_playerState;
        bool m_summaryComplete;

        std::string m_mainString;

        sf::Uint16 m_livesBonus;
        sf::Uint16 m_humanBonus;
        sf::Uint16 m_timeBonus;

        sf::Uint16 m_livesDisplayBonus;
        sf::Uint16 m_humanDisplayBonus;
        sf::Uint16 m_timeDisplayBonus;

        sf::Uint32 m_initialScore;

        sf::Text m_mainText;
        sf::Text m_okText;

        sf::Sound m_countLoop;
        sf::Sound m_countEnd;

        void updateMainString();
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}


#endif // LM_ROUND_SUMMARY_HPP_