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

#include <PHHumanController.hpp>
#include <CommandIds.hpp>

#include <xygine/util/Wavetable.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/AnimatedDrawable.hpp>

using namespace ph;

namespace
{
    const auto waveTable = xy::Util::Wavetable::sine(5.f, 3.f);
    const float fadeSpeed = 0.5f;
}

HumanController::HumanController(xy::MessageBus& mb, xy::AnimatedDrawable& spr)
    : xy::Component (mb, this),
    m_sprite        (spr),
    m_transparency  (1.f),
    m_waveTableIndex(xy::Util::Random::value(0, waveTable.size() - 1)),
    m_entity        (nullptr),
    m_rescued       (false),
    m_hasOrbiter    (false)
{
    xy::Component::MessageHandler mh;
    mh.id = GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        const auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::EnteredOrbit:
            if (msgData.value == m_entity->getUID())
            {
                m_hasOrbiter = true;

                if (!m_rescued)
                {
                    auto msg = sendMessage<LMGameEvent>(GameEvent);
                    msg->type = LMGameEvent::HumanPickedUp;
                    msg->posX = m_entity->getWorldPosition().x;
                    msg->posY = m_entity->getWorldPosition().y;
                }
            }
            break;
        case LMGameEvent::LeftOrbit:
            m_hasOrbiter = false;
            break;
        }
    };
    addMessageHandler(mh);
}

//public
void HumanController::entityUpdate(xy::Entity& entity, float dt)
{
    //bob animation
    m_waveTableIndex = (m_waveTableIndex + 1) % waveTable.size();
    m_sprite.setPosition(0.f, waveTable[m_waveTableIndex]);

    //fade if being 'beamed up'
    if (m_hasOrbiter && !m_rescued)
    {
        float oldTransparency = m_transparency;
        m_transparency = std::max(0.f, m_transparency - (dt * fadeSpeed));

        //check if we're completely gone and mark as rescued
        if (m_transparency == 0 && oldTransparency > 0)
        {
            m_rescued = true;
            auto msg = sendMessage<LMGameEvent>(GameEvent);
            msg->type = LMGameEvent::HumanRescued;
            msg->posX = entity.getWorldPosition().x;
            msg->posY = entity.getWorldPosition().y;
        }
        m_sprite.setColour({ 255, 255, 255, static_cast<sf::Uint8>(m_transparency * 255.f) });
    }
    else if(!m_rescued)
    {
        //fade back in again if left orbit without picking up
        m_transparency = std::min(1.f, m_transparency + (dt * fadeSpeed));
        m_sprite.setColour({ 255, 255, 255, static_cast<sf::Uint8>(m_transparency * 255.f) });
    }
}

void HumanController::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

//private