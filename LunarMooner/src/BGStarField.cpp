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

#include <BGStarfield.hpp>

#include <xygine/util/Math.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/Resource.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const std::string fragShader =
        "#version 120\n"
        "uniform sampler2D u_texture;\n"
        "uniform vec2 u_offset;\n"
        
        "void main()\n"
        "{\n"
        "    gl_FragColor = texture2D(u_texture, gl_TexCoord[0].xy + u_offset);\n"
        "}\n";

    const float backgroundSpeed = 0.006f;
    const float starSpeed = -6.5f;
    const sf::Uint8 maxStars = 40u;
}

Starfield::Starfield(xy::MessageBus& mb, xy::TextureResource& tr)
    : xy::Component     (mb, this),
    m_speedRatio        (1.f),
    m_velocity          (1.f, 0.f),
    m_starTexture       (&tr.get("assets/images/background/star.png"))
{
    auto& texture = tr.get("assets/images/background/background.png");
    texture.setRepeated(true);
    m_backgroundSprite.setTexture(texture);
    
    m_shader.loadFromMemory(fragShader, sf::Shader::Fragment);

    m_bounds.width = xy::DefaultSceneSize.x;
    m_bounds.height = xy::DefaultSceneSize.y;

    //create stars
    for (auto i = 0u; i < maxStars; ++i)
    {
        m_stars.emplace_back();

        auto& star = m_stars.back();
        star.setPosition(xy::Util::Random::value(0.f, m_bounds.width), xy::Util::Random::value(0.f, m_bounds.height));
        star.depth = xy::Util::Random::value(1.f, 6.f);
        star.setScale(star.depth, star.depth);
    }
}

//public
void Starfield::entityUpdate(xy::Entity&, float dt)
{
    m_position += m_velocity * dt * backgroundSpeed * m_speedRatio;    
    
    m_vertices.clear();
    for (auto& s : m_stars)
    {
        s.move(m_velocity * starSpeed * m_speedRatio * dt * s.depth);

        auto pos = s.getPosition();
        if (pos.x < -10.f)
        {
            s.move(1940.f, 0.f);
        }
        else if (pos.x > 1930.f)
        {
            s.move(-1940.f, 0.f);
        }

        if (pos.y < -10.f)
        {
            s.move(0.f, 1100.f);
        }
        else if (pos.y > 1090.f)
        {
            s.move(0.f, -1000.f);
        }

        const auto& t = s.getTransform();
        for (const auto& p : s.positions)
        {
            //magic 8. This is the size of the texture
            m_vertices.push_back({ t.transformPoint(p), p * 8.f });
        }
    }
}

void Starfield::setVelocity(const sf::Vector2f& vel)
{
    m_velocity = xy::Util::Vector::normalise(vel);
}

//private
void Starfield::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    m_shader.setUniform("u_offset", m_position);
    m_shader.setUniform("u_texture", *m_backgroundSprite.getTexture());
    states.shader = &m_shader;
    rt.draw(m_backgroundSprite, states);

    states.shader = nullptr;
    states.blendMode = sf::BlendAdd;
    states.texture = m_starTexture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}