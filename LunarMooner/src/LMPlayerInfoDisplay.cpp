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

#include <sstream>
#include <iomanip>

using namespace lm;

namespace
{
    const sf::Vector2f messageSpeed(-1800.f, 0.f);
    const float messageDisplayTime = 2.f;
    const sf::Vector2f messagePosition(960.f, 540.f);

    const std::array<sf::Vector2f, 4u> textPositions = 
    {
        sf::Vector2f(20.f, 20.f),
        sf::Vector2f(1660, 20.f),
        sf::Vector2f(20.f, 560.f),
        sf::Vector2f(1660, 560.f)
    };

    const std::array<std::string, 4u> names =
    {
        "One",
        "Two",
        "Three",
        "Four"
    };

    const float textSpeed = -200.f;
}

ScoreDisplay::ScoreDisplay(xy::MessageBus& mb, xy::FontResource& fr, std::vector<PlayerState>& ps)
    : xy::Component         (mb, this),
    m_fontResource          (fr),
    m_playerStates          (ps),
    m_showMessage           (false),
    m_messageDisplayTime    (0.f)
{
    auto& font = fr.get("player_info_display_73");

    m_messageText.setFont(font);
    m_messageText.setCharacterSize(80u);
    m_messageText.setFillColor(sf::Color::Yellow);
    m_messageText.setOutlineColor(sf::Color::Red);
    m_messageText.setOutlineThickness(2.f);

    XY_ASSERT(ps.size() <= 4, "Currently only supporting 4 player display");
    for (auto i = 0u; i < ps.size(); ++i)
    {
        m_playerTexts.emplace_back("",font, 24u);
        m_playerTexts.back().setPosition(textPositions[i]);
    }
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

    //update player info graphics
    std::string lives;
    for (auto i = 0u; i < m_playerStates.size(); ++i)
    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') 
            << static_cast<std::uint32_t>(m_playerStates[i].timeRemaining / 60.f) << ":" 
            << std::setw(2) << std::setfill('0')
            << std::floor(std::fmod(m_playerStates[i].timeRemaining, 60.f));
        std::string remainingTime = ss.str();
        
        lives = (m_playerStates[i].lives > -1) ? "Lives: " + std::to_string(m_playerStates[i].lives) : "GAME OVER";
        m_playerTexts[i].setString(
            "Player " + names[i] + "\n"
            "Score: " + std::to_string(m_playerStates[i].score) + "\n"
            "Level: " + std::to_string(m_playerStates[i].level) + "\n"
            "Ammo: " + std::to_string(m_playerStates[i].ammo) + "\n"
            + lives + "\n"
            + remainingTime);
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

    text.setFont(m_fontResource.get("player_info_display_149"));
    text.setString(std::to_string(score));
    xy::Util::Position::centreOrigin(text);
    text.setFillColor(colour);
    text.setPosition(position);
}

//private
void ScoreDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if(m_showMessage) rt.draw(m_messageText, states);
    
    for (const auto& text : m_playerTexts)
    {
        rt.draw(text, states);
    }

    for (const auto& st : m_scoreTags)
    {
        rt.draw(st.text, states);
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