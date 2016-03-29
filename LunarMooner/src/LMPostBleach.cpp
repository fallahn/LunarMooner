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

#include <xygine/util/Wavetable.hpp>
#include <xygine/util/Random.hpp>

#include <SFML/Graphics/RenderTexture.hpp>

using namespace lm;

namespace
{
    const auto wavetable = xy::Util::Wavetable::sine(0.025f);
    std::vector<sf::Vector2f> offsets;

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
    : m_index(0),
    m_running(false)
{
    m_shader.loadFromMemory(fragShader, sf::Shader::Fragment);

    offsets.resize(wavetable.size());
    for (auto& o : offsets)
    {
        o.x = xy::Util::Random::value(-maxX, maxX);
        o.y = xy::Util::Random::value(-maxY, maxY);
    }
}

//public
void PostBleach::apply(const sf::RenderTexture& src, sf::RenderTarget& dst)
{
    m_shader.setUniform("u_srcTexture", src.getTexture());
    applyShader(m_shader, dst);
}

void PostBleach::update(float)
{
    if (!m_running) return;

    const float val = wavetable[m_index];
    m_shader.setUniform("u_amount", val);

    const auto offset = offsets[m_index];
    m_shader.setUniform("u_offset", offset);

    m_index = (m_index + 1) % (wavetable.size() / 4);
}

//private
void PostBleach::start()
{
    m_running = true;
}

void PostBleach::stop()
{
    m_running = false;
}

void PostBleach::reset()
{
    m_index = 0;
}