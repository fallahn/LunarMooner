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

#include <LMPostBleach.hpp>
#include <CommandIds.hpp>

#include <xygine/util/Wavetable.hpp>
#include <xygine/util/Random.hpp>

#include <SFML/Graphics/RenderTexture.hpp>

using namespace lm;

namespace
{
    const std::string fragShader =
        "#version 120\n"
        "\n"
        "uniform float u_amount;\n"
        "uniform vec2 u_offset;\n"
        "uniform sampler2D u_srcTexture;\n"

        "void main()\n"
        "{\n"
        "    vec3 sat = vec3(1.0) * u_amount;\n"
        "    vec4 colour = texture2D(u_srcTexture, gl_TexCoord[0].xy + (u_offset * u_amount));\n"
        "    colour.rgb += sat;\n"
        "    gl_FragColor = colour;\n"
        "}\n";

    const float maxX = 0.01f;
    const float maxY = 0.02f;
}

PostBleach::PostBleach()
    : m_index   (0),
    m_fadeIndex (0),
    m_running   (false),
    m_speed     (1),
    m_wavetable (xy::Util::Wavetable::sine(0.025f))
{
    m_shader.loadFromMemory(fragShader, sf::Shader::Fragment);

    //adds a quick fade out to end of table
    auto quarter = m_wavetable.size() / 4;
    auto half = quarter * 2;
    std::size_t size = 0;
    for (auto i = quarter, j = quarter; j < half; ++i, j += 10)
    {
        m_wavetable[i] = m_wavetable[j];
        size = i + 1;
    }
    m_wavetable.resize(size);
    m_fadeIndex = quarter; //so we can jump to fade out

    //screen shake offsets
    m_offsets.resize(m_wavetable.size());
    for (auto& o : m_offsets)
    {
        o.x = xy::Util::Random::value(-maxX, maxX);
        o.y = xy::Util::Random::value(-maxY, maxY);
    }

    MessageHandler mh;
    mh.id = LMMessageId::StateEvent;
    mh.action = [this](const xy::Message& msg)
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
    mh.action = [this](const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::PlayerSpawned:
            if (msgData.value > 0)
            {
                start(msgData.value);
            }
            break;
        case LMGameEvent::PlayerDied:
            stop();
            break;
        }
    };
    addMessageHandler(mh);
}

//public
void PostBleach::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    m_shader.setUniform("u_srcTexture", src.getTexture());
    applyShader(m_shader, dst);
}

void PostBleach::update(float)
{
    if (m_running)
    {
        const float val = m_wavetable[m_index];
        m_shader.setUniform("u_amount", val);

        const auto offset = m_offsets[m_index];
        m_shader.setUniform("u_offset", offset);

        auto index = (m_index + m_speed) % m_wavetable.size();

        if (index < m_index)
        {
            m_running = false;
            m_shader.setUniform("u_amount", 0.f);
            m_index = 0;
            m_speed = 1u;
        }
        else
        {
            m_index = index;
        }
    }
}

//private
void PostBleach::start(std::size_t speed)
{
    reset();
    m_running = true;
    m_speed = speed;
}

void PostBleach::stop()
{
    //skip forward to fade
    if (m_running && m_index < m_fadeIndex && m_index < m_wavetable.size())
    {
        float current = m_wavetable[m_index];
        while (m_wavetable[++m_index] > current) {}
    }
}

void PostBleach::reset()
{
    m_index = 0;
    m_shader.setUniform("u_amount", 0.f);
    m_speed = 1;
}