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

    const float textSpeed = -200.f;
}

ScoreDisplay::ScoreDisplay(xy::MessageBus& mb, xy::FontResource& fr, std::vector<PlayerState>& ps)
    : xy::Component         (mb, this),
    m_fontResource          (fr),
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
    std::string lives = (m_playerStates[0].lives > -1) ? "Lives: " + std::to_string(m_playerStates[0].lives) : "GAME OVER";
    m_playerOneText.setString(
        "Player One\n"
        "Score: " + std::to_string(m_playerStates[0].score) + "\n"
        + lives);

    if (m_playerStates.size() > 1)
    {
        lives = (m_playerStates[0].lives > -1) ? "Lives: " + std::to_string(m_playerStates[1].lives) : "GAME OVER";
        m_playerTwoText.setString(
            "Player Two\n"
            "Score: " + std::to_string(m_playerStates[1].score) + "\n"
            + lives);
    }

    //update any active scores and remove dead ones
    for (auto& st : m_scoreTags)
    {
        st.update(dt);
    }
    m_scoreTags.erase(std::remove_if(m_scoreTags.begin(), m_scoreTags.end(),
        [](const ScoreTag& st)
    {
        return (st.text.getFillColor().a == 0);
    }), m_scoreTags.end());
}

void ScoreDisplay::showMessage(const std::string& msg)
{
    m_messageText.setString(msg);
    xy::Util::Position::centreOrigin(m_messageText);
    m_messageText.setPosition(messagePosition);
    m_messageDisplayTime = messageDisplayTime;
    m_showMessage = true;
}

void ScoreDisplay::showScore(sf::Uint16 score, const sf::Vector2f& position, sf::Color colour)
{
    m_scoreTags.emplace_back();
    auto& text = m_scoreTags.back().text;

    text.setFont(m_fontResource.get("buns"));
    text.setString(std::to_string(score));
    xy::Util::Position::centreOrigin(text);
    text.setFillColor(colour);
    text.setPosition(position);
}

//private
void ScoreDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if(m_showMessage) rt.draw(m_messageText, states);
    rt.draw(m_playerOneText, states);
    rt.draw(m_playerTwoText, states);

    for (const auto& st : m_scoreTags)
    {
        rt.draw(st.text);
    }
}


//score diplayer thinger
void ScoreDisplay::ScoreTag::update(float dt)
{
    text.move(0.f, textSpeed * dt);
    alpha -= dt;

    auto colour = text.getFillColor();
    colour.a = static_cast<sf::Uint8>(255.f * std::max(0.f, alpha));
    text.setFillColor(colour);
}