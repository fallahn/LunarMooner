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

#include <LMPlayerInfoDisplay.hpp>

#include <xygine/Resource.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const sf::Vector2f messageSpeed(-1800.f, 0.f);
    const float messageDisplayTime = 2.f;
    const sf::Vector2f messagePosition(960.f, 540.f);

    const sf::Vector2f playerOnePosition(20.f, 20.f);
    const sf::Vector2f playerTwoPosition(1660, 20.f);
}

ScoreDisplay::ScoreDisplay(xy::MessageBus& mb, xy::FontResource& fr, std::vector<PlayerState>& ps)
    : xy::Component         (mb, this),
    m_playerStates          (ps),
    m_showMessage           (false),
    m_messageDisplayTime    (0.f)
{
    auto& font = fr.get("buns");

    m_messageText.setFont(font);
    m_messageText.setCharacterSize(80u);

    m_playerOneText.setFont(font);
    m_playerOneText.setPosition(playerOnePosition);
    m_playerOneText.setCharacterSize(24u);
    m_playerTwoText.setFont(font);
    m_playerTwoText.setPosition(playerTwoPosition);
    m_playerTwoText.setCharacterSize(24u);
}

//public
void ScoreDisplay::entityUpdate(xy::Entity&, float dt)
{
    //show any messages if we have any
    if (m_showMessage)
    {
        m_messageDisplayTime -= dt;
        if (m_messageDisplayTime < 0)
        {
            m_messageText.move(messageSpeed * dt);
            if (m_messageText.getPosition().x < -m_messageText.getGlobalBounds().width)
            {
                m_showMessage = false;
            }
        }
    }

    //update player info graphics - TODO we can better adapt to multiple players
    m_playerOneText.setString(
        "Player One\n"
        "Score: " + std::to_string(m_playerStates[0].score) + "\n"
        "Lives: " + std::to_string(m_playerStates[0].lives)); //TODO set this to 'Game Over' when -1

    if (m_playerStates.size() > 1)
    {
        m_playerTwoText.setString(
            "Player Two\n"
            "Score: " + std::to_string(m_playerStates[1].score) + "\n"
            "Lives: " + std::to_string(m_playerStates[1].lives)); //TODO set this to 'Game Over' when -1
    }
}

void ScoreDisplay::showMessage(const std::string& msg)
{
    m_messageText.setString(msg);
    xy::Util::Position::centreOrigin(m_messageText);
    m_messageText.setPosition(messagePosition);
    m_messageDisplayTime = messageDisplayTime;
    m_showMessage = true;
}

//private
void ScoreDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if(m_showMessage) rt.draw(m_messageText, states);
    rt.draw(m_playerOneText, states);
    rt.draw(m_playerTwoText, states);
}