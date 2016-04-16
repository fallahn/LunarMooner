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

#include <LMShieldDrawable.hpp>
#include <CommandIds.hpp>

#include <xygine/util/Const.hpp>
#include <xygine/util/Wavetable.hpp>
#include <xygine/Entity.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Texture.hpp>

using namespace lm;

namespace
{
    const sf::Color shieldColour = sf::Color(0u, 255u, 255u, 220u);
    const float halfWidth = 960.f;

    const float thickness = 100.f;

    //shader - applies animated noise texture
    //and pulses intensity slightly. Can have it here
    //as unlikely to be more than one instance
    const std::string fragShader =
        "#version 120\n"

        "uniform sampler2D u_texture;\n"
        "uniform vec2 u_offset = vec2(0.0);\n"
        "uniform float u_intensity = 1.0;\n"

        "uniform vec2 u_impactPoint = vec2(0.5);\n"
        "uniform float u_impactRadius = 0.01;\n"
        "uniform float u_impactIntensity = 1.0;\n"

        "void main()\n"
        "{\n"

        /*"    else\n"*/
        "    {\n"
        "        vec2 stretch = vec2(0.9);\n"
        "        stretch.y *= gl_TexCoord[0].y;\n"
        "        gl_FragColor = gl_Color * texture2D(u_texture, gl_TexCoord[0].xy + (u_offset - stretch)) * u_intensity;\n"
        "    }\n"

        "    vec2 impactVec = gl_TexCoord[0].xy - u_impactPoint;\n"
        "    float distance = dot(impactVec, impactVec);\n"
        "    if(distance < u_impactRadius)\n"
        "    {\n"
        "        gl_FragColor += gl_Color * u_impactIntensity * (distance / u_impactRadius);\n"
        "    }\n"
        "}\n";

    const float offsetSpeed = 0.125f;
    const float maxImpactRadius = 0.15f;
}

ShieldDrawable::ShieldDrawable(xy::MessageBus& mb, float radius)
    : xy::Component     (mb, this),
    m_entity            (nullptr),
    m_impactRadius      (0.f),
    m_impactIntensity   (1.f),
    m_texture           (nullptr)
{
    XY_ASSERT(radius > halfWidth, "Needs a bigger radius");
    const float startAngle = std::asin(halfWidth / radius);
    const float endAngle = -startAngle;
    const float step = (startAngle - endAngle) / ((m_vertices.size() / 2) - 1);

    for (auto i = 0u, j = 0u; i < m_vertices.size() - 1; i += 2, ++j)
    {
        const float theta = startAngle - ((i / 2) * step);
        sf::Vector2f position(sin(theta), cos(theta));

        m_vertices[i].position = position * (radius - thickness);
        m_vertices[i].texCoords = m_vertices[i].position;
        m_vertices[i].color = sf::Color::Transparent;

        m_vertices[i+1].position = position * radius;
        m_vertices[i+1].texCoords = m_vertices[i+1].position;
        m_vertices[i+1].color = shieldColour;

        m_outline[j] = m_vertices[i+1];

        position *= radius;
        if (position.x < m_bounds.left)
        {
            m_bounds.left = position.x;
        }
        else if (position.x - m_bounds.left > m_bounds.width)
        {
            m_bounds.width = position.x - m_bounds.left;
        }

        if (position.y < m_bounds.top)
        {
            m_bounds.top = position.y;
        }
        else if (position.y - m_bounds.top > m_bounds.height)
        {
            m_bounds.height = position.y - m_bounds.top;
        }
    }

    m_shader.loadFromMemory(fragShader, sf::Shader::Fragment);

    m_wavetable = xy::Util::Wavetable::sine(0.5f);
    for (auto& w : m_wavetable)
    {
        w = (w + 1.f) * 0.5f; //normalise
        w = (w + 1.f) * 0.5f; //to range 0.5 - 1.0
        w = (w + 1.f) * 0.5f; //to range 0.75 - 1.0
    }

    //callback for handling impact effects
    xy::Component::MessageHandler mh;
    mh.id = LMMessageId::GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMGameEvent>();
        if (msgData.type == LMGameEvent::MeteorExploded && msgData.value == 0)
        {
            sf::Vector2f position(msgData.posX, msgData.posY);
            position = m_entity->getInverseTransform().transformPoint(position);

            auto texSize = m_texture->getSize();
            position.x /= texSize.x;
            position.y /= texSize.y;         

            m_shader.setUniform("u_impactPoint", position);
            m_impactRadius = 0.f;
            m_impactIntensity = 1.f;
        }
    };
    addMessageHandler(mh);
}

//public
void ShieldDrawable::entityUpdate(xy::Entity&, float dt)
{
    //update the shader params
    m_textureOffset.y += dt * offsetSpeed;
    m_waveTableIndex = (m_waveTableIndex + 1) % m_wavetable.size();

    m_shader.setUniform("u_offset", m_textureOffset);
    m_shader.setUniform("u_intensity", m_wavetable[m_waveTableIndex]);

    m_impactIntensity = std::max(0.f, m_impactIntensity - dt);
    m_impactRadius = std::min(maxImpactRadius, m_impactRadius + dt * maxImpactRadius);

    m_shader.setUniform("u_impactRadius", m_impactRadius * m_impactRadius);
    m_shader.setUniform("u_impactIntensity", m_impactIntensity);
}

void ShieldDrawable::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

sf::FloatRect ShieldDrawable::globalBounds() const
{
    return m_bounds;
}

void ShieldDrawable::setTexture(sf::Texture& t)
{
    t.setRepeated(true);
    m_texture = &t;
    m_shader.setUniform("u_texture", t);
}

//private
void ShieldDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.blendMode = sf::BlendAdd;
    states.texture = m_texture;
    states.shader = &m_shader;

    rt.draw(m_vertices.data(), m_vertices.size(), sf::TrianglesStrip, states);
    rt.draw(m_outline.data(), m_outline.size(), sf::LinesStrip, states);
}