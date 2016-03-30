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

#include <LMRoundSummary.hpp>
#include <CommandIds.hpp>

#include <xygine/Resource.hpp>
#include <xygine/Entity.hpp>
#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const sf::Uint16 lifeBonus = 200;
    const sf::Uint16 humanBonus = 100;
    const sf::Uint16 timeBonus = 12; //TODO adjust this it might be a bit conservative with current times (also see below)

    //all the bonus values must be divisible by this! :D
    const sf::Uint16 updateStep = 4; 

    const sf::Uint32 extraLifeValue = 7500;
}

RoundSummary::RoundSummary(xy::MessageBus& mb, PlayerState& ps, xy::TextureResource& tr, xy::FontResource& fr, bool doScores)
    : xy::Component     (mb, this),
    m_entity            (nullptr),
    m_playerState       (ps),
    m_summaryComplete   (!doScores),
    m_livesBonus        (ps.lives * lifeBonus),
    m_humanBonus        (ps.humansSaved * humanBonus),
    m_timeBonus         (sf::Uint32(ps.timeRemaining) * timeBonus),
    m_livesDisplayBonus (m_livesBonus),
    m_humanDisplayBonus (m_humanBonus),
    m_timeDisplayBonus  (m_timeBonus),
    m_initialScore      (ps.score)
{
    m_mainText.setFillColor(sf::Color::Yellow);
    m_mainText.setOutlineColor(sf::Color::Red);
    m_mainText.setOutlineThickness(2.f);
    m_mainText.setFont(fr.get("round_summary_55"));
    
    if (!doScores)
    {
        m_mainText.setString("You Died!");
        m_mainText.setCharacterSize(80u);
        xy::Util::Position::centreOrigin(m_mainText);
        m_mainText.setPosition(960.f, 440.f);
    }
    else
    {
        m_mainText.setPosition(600.f, 200.f);
        m_mainText.setCharacterSize(50u);

        m_okText.setFillColor(sf::Color::Magenta);
        m_okText.setOutlineThickness(2.f);
        m_okText.setOutlineColor(sf::Color(255, 127, 0));
        m_okText.setFont(fr.get("round_summary_72"));
        m_okText.setCharacterSize(80u);
        m_okText.setPosition(960.f, 660.f);
        m_okText.setString("OK!");
        xy::Util::Position::centreOrigin(m_okText);
    }
}

//public
void RoundSummary::entityUpdate(xy::Entity&, float dt)
{
    //animate round summary
    //we get a bonus for each human saved, and an extra life every 5000 points
    if (!m_summaryComplete)
    {
        
        if (m_livesBonus > 0)
        {
            m_livesBonus -= updateStep;
            m_playerState.score += updateStep;
        }
        else if (m_humanBonus > 0)
        {
            m_humanBonus -= updateStep;
            m_playerState.score += updateStep;
        }
        else if (m_timeBonus > 0)
        {
            m_timeBonus -= updateStep;
            m_playerState.score += updateStep;
        }
        else
        {            
            m_summaryComplete = true;
        }
        updateMainString();
    }
}

void RoundSummary::onStart(xy::Entity& ent)
{
    m_entity = &ent;
}

void RoundSummary::completeSummary()
{
    if (m_summaryComplete)
    {
        //close and message
        m_entity->destroy();

        auto msg = getMessageBus().post<LMStateEvent>(LMMessageId::StateEvent);
        msg->type = LMStateEvent::SummaryFinished;
    }
    else
    {
        //skip to end of score adding
        m_playerState.score += m_humanBonus + m_livesBonus + m_timeBonus;
        m_humanBonus = m_livesBonus = m_timeBonus = 0;

        updateMainString();
        m_summaryComplete = true;
    }
}

//private
void RoundSummary::updateMainString()
{
    m_mainString =
        "Lives remaining:       " + std::to_string(m_livesDisplayBonus - m_livesBonus)
        + "\n\n"
        + "Colonists rescued:     " + std::to_string(m_humanDisplayBonus - m_humanBonus)
        + "\n\n"
        + "Time Remaining:        " + std::to_string(m_timeDisplayBonus - m_timeBonus);
    m_mainText.setString(m_mainString);
}

void RoundSummary::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_mainText, states);
    if (m_summaryComplete)
    {
        rt.draw(m_okText, states);
    }
}