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

#include <LMNukeDrawable.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace lm;

namespace
{
    const float maxX = 12.f;
    const float maxY = 10.f;
    const std::size_t offsetCount = 30u;
    const sf::Vector2f basePosition(xy::DefaultSceneSize / 2.f);

    const float maxAlpha = 0.96f;
    const float maxTime = 10.f;
    const float fadeOutMultiplier = 12.f;
    const sf::Color baseColour(255u, 255u, 240u, 0u);
}

NukeEffect::NukeEffect(xy::MessageBus& mb, const sf::Vector2f& size)
    : xy::Component (mb, this),
    m_fade          (Fade::In),
    m_currentAlpha  (0.f),
    m_currentStep   (0.f),
    m_offsetIndex   (0u)
{
    auto halfsize = size / 2.f;
    m_vertices[0].position = -halfsize;
    m_vertices[1].position = { halfsize.x, -halfsize.y };
    m_vertices[2].position = halfsize;
    m_vertices[3].position = { -halfsize.x, halfsize.y };

    for (auto& v : m_vertices)
    {
        v.color = baseColour;
    }

    //message handler
    MessageHandler mh;
    mh.id = LMMessageId::StateEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMStateEvent::CountDownStarted:
            start();
            break;
        case LMStateEvent::RoundBegin:
            reset();
            break;
        case LMStateEvent::RoundEnd:
            stop();
            break;
        }
    };
    addMessageHandler(mh);

    mh.id = LMMessageId::GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::PlayerSpawned:
            if (msgData.value > 0)
            {
                //value is multiplier of default speed
                start(msgData.value);
            }
            break;
        case LMGameEvent::PlayerDied:
            stop();
            break;
        }
    };
    addMessageHandler(mh);


    for (auto i = 0u; i < offsetCount; ++i)
    {
        m_offsets.emplace_back(sf::Vector2f(
            xy::Util::Random::value(-maxX, maxX),
            xy::Util::Random::value(-maxY, maxY)));
    }
}

//public
void NukeEffect::entityUpdate(xy::Entity& entity, float dt)
{
    if (m_running)
    {
        //update alpha    
        if (m_fade == Fade::In)
        {
            m_currentAlpha += m_currentStep * dt;
            if (m_currentAlpha > maxAlpha)
            {
                m_fade = Fade::Out;
            }
        }
        else
        {
            m_currentAlpha = std::max(0.f, m_currentAlpha - ((m_currentStep * fadeOutMultiplier) * dt));
            if (m_currentAlpha == 0) m_running = false;
        }
        sf::Color colour = baseColour;
        colour.a = static_cast<sf::Uint8>(m_currentAlpha * 255.f);
        for (auto& v : m_vertices)
        {
            v.color = colour;
        }


        //shake entity
        m_offsetIndex = (m_offsetIndex + 1) % offsetCount;
        entity.setPosition(basePosition + (m_offsets[m_offsetIndex] * m_currentAlpha));
    }
}

//private
void NukeEffect::start(std::size_t multiplier)
{
    reset();
    m_currentStep = (maxAlpha / maxTime) * multiplier;
    m_running = true;
}

void NukeEffect::stop()
{
    m_fade = Fade::Out;
}

void NukeEffect::reset()
{
    m_currentStep = 0.f;
    m_currentAlpha = 0.f;
    m_fade = Fade::In;

    for (auto& v : m_vertices)
    {
        v.color = baseColour;
    }
}

void NukeEffect::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (!m_running) return;
    states.blendMode = sf::BlendAdd;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}