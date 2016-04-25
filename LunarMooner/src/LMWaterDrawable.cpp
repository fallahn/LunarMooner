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

#include <LMWaterDrawable.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

using namespace lm;

namespace
{
    float timeElapsed = 0.f;
    const float timeMultiplier = 0.1f;
    const sf::Color colour(20u, 50u, 200u, 120u);
}

WaterDrawable::WaterDrawable(xy::MessageBus& mb, const sf::Texture& t, float level)
    : xy::Component (mb, this),
    m_level         (level),
    m_reflectOffset (0.f),
    m_texture       (t),
    m_shader        (nullptr)
{
    auto size = sf::Vector2f(t.getSize());

    m_vertices[1].position.x = size.x;
    m_vertices[2].position = { size.x, level };
    m_vertices[3].position.y = level;
    
    const float offset = size.y - level;
    m_vertices[0].texCoords.y = offset;
    m_vertices[1].texCoords = { size.x, offset };
    m_vertices[2].texCoords =size;
    m_vertices[3].texCoords.y = size.y;

    m_reflectOffset = offset / size.y;

    for (auto& v : m_vertices) v.color = colour;

    m_renderTexture.create(static_cast<sf::Uint32>(size.x) / 2, static_cast<sf::Uint32>(level) / 2);
    m_renderTexture.setView(sf::View({ 0.f, 0.f, size.x, level }));
    m_renderTexture.setSmooth(true);
    m_sprite.setTexture(m_renderTexture.getTexture());
    m_sprite.setScale(2.f, 2.f);
}

//public
void WaterDrawable::entityUpdate(xy::Entity& entity, float dt)
{
    timeElapsed += dt;

    sf::RenderStates states;
    states.texture = &m_texture;
    states.shader = m_shader;

    m_shader->setUniform("u_texture", m_texture);
    m_shader->setUniform("u_time", timeElapsed * timeMultiplier);
    m_shader->setUniform("u_texOffset", m_reflectOffset);

    m_renderTexture.clear(sf::Color::Transparent);
    m_renderTexture.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
    m_renderTexture.display();
}

//private
void WaterDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const 
{
    rt.draw(m_sprite, states);
}