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

#include <GameMessage.hpp>

#include <xygine/util/Position.hpp>
#include <xygine/Entity.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
    const float moveSpeed = 2500.f;
}

GameMessage::GameMessage(xy::MessageBus& mb, const sf::Font& font, const std::string& message, sf::Uint32 charSize)
    : xy::Component(mb, this),
    m_clear(false)
{
    m_text.setFont(font);
    m_text.setString(message);
    m_text.setFillColor(sf::Color::Yellow);
    m_text.setOutlineColor(sf::Color::Red);
    m_text.setOutlineThickness(2.f);
    m_text.setCharacterSize(charSize);
    xy::Util::Position::centreOrigin(m_text);
}

//public
void GameMessage::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_clear)
    {
        entity.move(-moveSpeed * dt, 0.f);
        if (entity.getWorldPosition().x < -(xy::DefaultSceneSize.x / 2.f))
        {
            entity.destroy();
        }
    }
}

//private
void GameMessage::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_text, states);
}