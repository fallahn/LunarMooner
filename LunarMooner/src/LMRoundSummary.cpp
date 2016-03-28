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
    std::string tempStr;
    float tempFloat = 0.f;
}

RoundSummary::RoundSummary(xy::MessageBus& mb, PlayerState& ps, xy::TextureResource& tr, xy::FontResource& fr, bool doScores)
    : xy::Component     (mb, this),
    m_entity            (nullptr),
    m_playerState       (ps),
    m_summaryComplete   (!doScores)
{
    m_text.setFillColor(sf::Color::Red);
    m_text.setOutlineColor(sf::Color::Black);
    m_text.setOutlineThickness(2.f);
    m_text.setFont(fr.get("round_summary_55"));
    m_text.setPosition(960.f, 540.f);

    if (!doScores)
    {
        m_text.setString("You Died!");
        m_text.setCharacterSize(80u);
        xy::Util::Position::centreOrigin(m_text); 
    }
}

//public
void RoundSummary::entityUpdate(xy::Entity&, float dt)
{
    //animate round summary
    if (!m_summaryComplete)
    {
        tempFloat += dt;
        if (tempFloat > 0.1)
        {
            tempFloat = 0.f;
            tempStr += 'a';

            m_text.setString(tempStr);
            xy::Util::Position::centreOrigin(m_text);
        }

        if (tempStr.size() == 26)
        {
            tempStr += "\n\nOK";

            m_text.setString(tempStr);
            xy::Util::Position::centreOrigin(m_text);

            m_summaryComplete = true;
        }
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

        auto msg = getMessageBus().post<LMEvent>(LMMessageId::LMMessage);
        msg->type = LMEvent::SummaryFinished;

        tempStr.clear();
    }
    else
    {
        //skip to end of score adding
        tempStr = "aaaaaaaaaaaaaaaaaaaaaaaaaa\n\nOK";

        m_text.setString(tempStr);
        xy::Util::Position::centreOrigin(m_text);

        m_summaryComplete = true;
    }
}

//private
void RoundSummary::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_text, states);
}