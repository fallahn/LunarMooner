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

#include <LMCoolDownMeter.hpp>
#include <CommandIds.hpp>

#include <xygine/Assert.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

using namespace lm;

namespace
{
    const float frameCount = 6.f;
}

CooldownMeter::CooldownMeter(xy::MessageBus& mb, const sf::Texture& t)
    : xy::Component (mb, this),
    m_texture       (t),
    m_size          (t.getSize()),
    m_level         (0.f)
{
    m_size.y /= frameCount;

    m_vertices[1].position.x = m_size.x;
    m_vertices[2].position = m_size;
    m_vertices[3].position.y = m_size.y;

    for (auto& v : m_vertices)v.texCoords = v.position;
}

//public
void CooldownMeter::entityUpdate(xy::Entity&, float dt)
{

}

void CooldownMeter::setValue(float value)
{
    XY_ASSERT(value > 0 && value <= 1, "Need normalised value");
    const float level = std::min(frameCount - 1.f, std::floor((frameCount - 1.f) * value));

    if (level != m_level)
    {
        //update frame
        const float offset = level * m_size.y;
        m_vertices[0].texCoords.y = offset;
        m_vertices[1].texCoords.y = offset;
        m_vertices[2].texCoords.y = offset + m_size.y;
        m_vertices[3].texCoords.y = offset + m_size.y;

        m_level = level;

        //raise an event for sfx
        auto msg = sendMessage<LMGameEvent>(LMMessageId::GameEvent);
        msg->type = LMGameEvent::WeaponCharged;
        msg->value = static_cast<sf::Int32>(level);
    }
}

//private
void CooldownMeter::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}