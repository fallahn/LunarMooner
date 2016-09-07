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

#include <PHHeadsUp.hpp>
#include <CommandIds.hpp>

#include <xygine/util/Position.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace ph;

HeadsUpDisplay::HeadsUpDisplay(xy::MessageBus& mb, ResourceCollection& rc)
    : xy::Component (mb, this),
    m_started       (false),
    m_clock         (rc.textureResource.get("assets/images/game/console/nixie_sheet.png")),
    m_time          (60.f)
{
    m_clock.setTime(m_time);
    m_clock.setOrigin(m_clock.getLocalBounds().width / 2.f, 0.f);
    m_clock.setPosition(xy::DefaultSceneSize.x / 2.f, 20.f);

    xy::Component::MessageHandler mh;
    mh.id = GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto data = msg.getData<LMGameEvent>();
        switch (data.type)
        {
        default: break;
        case LMGameEvent::PlayerSpawned:
            m_started = true;
            break;
        case LMGameEvent::PlayerDied:
            m_started = false;
            break;
        }
    };
    addMessageHandler(mh);

    mh.id = StateEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto data = msg.getData<LMStateEvent>();
        switch (data.type)
        {
        default: break;
        case LMStateEvent::RoundEnd:
            m_started = false;
            break;
        }
    };
    addMessageHandler(mh);
}

//public
void HeadsUpDisplay::entityUpdate(xy::Entity&, float dt)
{
    if (m_started)
    {
        float lastTime = m_time;
        m_time = std::max(0.f, m_time - dt);

        //raise event if time expired
        if (m_time == 0 && lastTime > 0)
        {
            auto msg = sendMessage<LMGameEvent>(GameEvent);
            msg->type = LMGameEvent::TimerExpired;
        }

        m_clock.setTime(m_time);
    }
}

//private
void HeadsUpDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_clock);
}