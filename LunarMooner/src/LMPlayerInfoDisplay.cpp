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
#include <ResourceCollection.hpp>

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
    const sf::Vector2f messagePosition(xy::DefaultSceneSize / 2.f);

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

    const std::array<sf::Vector2f, 2u> playerActivePositions
    {
        sf::Vector2f(54.f, 608.f),
        sf::Vector2f(1694.f, 608.f)
    };
}

ScoreDisplay::ScoreDisplay(xy::MessageBus& mb, ResourceCollection& rc, std::vector<PlayerState>& ps)
    : xy::Component         (mb, this),
    m_fontResource          (rc.fontResource),
    m_playerStates          (ps),
    m_showMessage           (false),
    m_messageDisplayTime    (0.f)
{
    auto& font = rc.fontResource.get("player_info_display_73");

    m_messageText.setFont(font);
    m_messageText.setCharacterSize(80u);
    m_messageText.setFillColor(sf::Color::Yellow);
    m_messageText.setOutlineColor(sf::Color::Red);
    m_messageText.setOutlineThickness(2.f);

    XY_ASSERT(ps.size() <= 2, "Currently only supporting 2 player display");
    for (auto i = 0u; i < ps.size(); ++i)
    {
        m_uiElements.emplace_back(rc, i);
    }

    m_activePlayerSprite.setTexture(rc.textureResource.get("assets/images/game/console/active_button.png"));
    m_activePlayerSprite.setPosition(playerActivePositions[0]);

    m_bounds.width = xy::DefaultSceneSize.x;
    m_bounds.height = xy::DefaultSceneSize.y;
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
        m_uiElements[i].ammo.setValue(m_playerStates[i].ammo);
        m_uiElements[i].clockDisplay.setTime(m_playerStates[i].timeRemaining);
        m_uiElements[i].level.setLevel(m_playerStates[i].level);
        m_uiElements[i].lives.setValue(m_playerStates[i].lives);
        m_uiElements[i].score.setValue(m_playerStates[i].score);
        m_uiElements[i].update(dt);
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

void ScoreDisplay::setPlayerActive(std::size_t idx)
{
    //XY_ASSERT(idx < m_playerTexts.size(), "Index out of range");
    m_activePlayerSprite.setPosition(playerActivePositions[idx]);
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
    for (const auto& s : m_uiElements)
    {
        rt.draw(s, states);
    }
    rt.draw(m_activePlayerSprite, sf::BlendAdd);

    if(m_showMessage) rt.draw(m_messageText, states);

    for (const auto& st : m_scoreTags)
    {
        rt.draw(st.text, states);
    }
}


//score displayer thinger
void ScoreDisplay::ScoreTag::update(float dt)
{
    text.move(0.f, textSpeed * dt);
    alpha -= dt;

    auto colour = text.getFillColor();
    colour.a = static_cast<sf::Uint8>(255.f * std::max(0.f, alpha));
    text.setFillColor(colour);
}


//ui elements struct
ScoreDisplay::UIElements::UIElements(ResourceCollection& rc, sf::Uint8 player)
    : clockDisplay  (rc.textureResource.get("assets/images/game/console/nixie_sheet.png")),
    ammo            (rc.textureResource.get("assets/images/game/console/counter.png"), 2, sf::seconds(0.5f)),
    lives           (rc.textureResource.get("assets/images/game/console/counter.png"), 2, sf::seconds(0.5f)),
    score           (rc.textureResource.get("assets/images/game/console/counter.png"), 6, sf::seconds(2.0f)),
    level           (rc.textureResource.get("assets/images/game/console/level_meter.png"))
{
    //hackety blunge to layout elements
    if (player == 0)
    {
        clockDisplay.setPosition(40.f, 34.f);
        ammo.setPosition(191.f, 142.f);
        lives.setPosition(191.f, 196.f);
        score.setPosition(66.f, 270.f);
        level.setPosition(42.f, 536.f);
    }
    else
    {
        clockDisplay.setPosition(1680.f, 34.f);
        ammo.setPosition(1831.f, 142.f);
        lives.setPosition(1831.f, 196.f);
        score.setPosition(1706.f, 270.f);
        level.setPosition(1682.f, 536.f);
    }
}

void ScoreDisplay::UIElements::update(float dt)
{
    ammo.update(dt);
    lives.update(dt);
    score.update(dt);
    level.update(dt);
}

void ScoreDisplay::UIElements::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(clockDisplay, states);
    rt.draw(ammo, states);
    rt.draw(score, states);
    rt.draw(lives, states);
    rt.draw(level, states);
}
