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

#include <OLAchievementTag.hpp>

#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace lm;

namespace
{
    const sf::Color fillColour(85, 97, 74);
    const sf::Color borderColour(61, 67, 84);
    const sf::Color textColour(198, 198, 198);

    const sf::Vector2f tagSize(260.f, 90.f);

    const float moveTime = 0.45f;
    const float holdTime = 2.5f;
    const float moveSpeed = tagSize.y / moveTime;
}

AchievementTag::AchievementTag(xy::MessageBus& mb, xy::FontResource& fr, const std::string& text)
    : xy::Component(mb, this),
    m_inTime    (moveTime),
    m_holdTime  (holdTime),
    m_alpha     (1.f)
{
    m_text.setFont(fr.get("achievement_tag"));
    m_text.setCharacterSize(20u);
    m_text.setString(text);
    m_text.setFillColor(textColour);
    xy::Util::Position::centreOrigin(m_text);
    m_text.setPosition(-tagSize.x / 2.f, tagSize.y / 2.f);

    m_shape.setSize(tagSize);
    m_shape.setFillColor(fillColour);
    m_shape.setOutlineThickness(2.f);
    m_shape.setOutlineColor(borderColour);
    m_shape.setOrigin(tagSize.x, 0.f);
}

//public
void AchievementTag::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_holdTime > 0)
    {
        //move in if move time
        if (m_inTime > 0)
        {
            m_inTime -= dt;
            entity.move(0.f, -moveSpeed * dt);
        }
        //hold
        else
        {
            m_holdTime -= dt;
        }
    }
    else
    {
        //fade out
        m_alpha = std::max(0.f, m_alpha - dt);
        sf::Uint8 alpha = static_cast<sf::Uint8>(m_alpha * 255.f);

        auto colour = fillColour;
        colour.a = alpha;
        m_shape.setFillColor(colour);

        colour = borderColour;
        colour.a = alpha;
        m_shape.setOutlineColor(colour);

        colour = textColour;
        colour.a = alpha;
        m_text.setFillColor(colour);

        if (m_alpha == 0) entity.destroy();
    }
}

//private
void AchievementTag::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_shape, states);
    rt.draw(m_text, states);
}