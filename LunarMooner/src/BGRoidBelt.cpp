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

#include <BGRoidBelt.hpp>

#include <xygine/Entity.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/Scene.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Shader.hpp>

using namespace lm;

namespace
{
    const float height = 70.f;
    const std::size_t roidCount = 40u;
    const float baseSpeed = 20.f;
    const sf::Vector2f size(40.f, 40.f);
    const float fadeDistance = 30.f;
}

RoidBelt::RoidBelt(xy::MessageBus& mb, float width, sf::Texture& texture, sf::Texture& normalTexture)
    : xy::Component (mb, this),
    m_roids         (roidCount),
    m_texture       (texture),
    m_normalTexture (normalTexture),
    m_shader        (nullptr),
    m_vertices      (roidCount * 4)
{
    XY_ASSERT(width > fadeDistance, "Roid belt width too small!");

    m_bounds.width = width;
    m_bounds.height = height;

    sf::Vector2f texSize(texture.getSize());
    XY_WARNING(texSize.x == 0 || texSize.y == 0, "Invalid Roid texture Size");

    for (auto& roid : m_roids)
    {
        roid.rotation = xy::Util::Random::value(30.f, 80.f);
        const float vertPosition = xy::Util::Random::value(0.f, height);
        roid.setPosition(xy::Util::Random::value(0.f, width), vertPosition);
        const float scale = vertPosition / height;
        roid.speed = baseSpeed * scale;
        roid.setScale(scale, scale);
        roid.velocity.x = 1.f;
        roid.setOrigin(size / 2.f);

        roid.vertices[1].position.x = size.x;
        roid.vertices[2].position = size;
        roid.vertices[3].position.y = size.y;

        roid.vertices[1].texCoords.x = texSize.x;
        roid.vertices[2].texCoords = texSize;
        roid.vertices[3].texCoords.y = texSize.y;
    }

    //put the largest at the front
    std::sort(m_roids.begin(), m_roids.end(), [](const Roid& r1, const Roid& r2)
    {
        return (r1.getPosition().y < r2.getPosition().y);
    });
}

//public 
void RoidBelt::entityUpdate(xy::Entity&, float dt)
{
    std::size_t idx = 0u;
    for (auto& r : m_roids)
    {
        r.update(dt, m_bounds);

        //add to vertex array
        const auto& tx = r.getTransform();
        for (const auto& v : r.vertices)
        {
            sf::Vertex vert = v;
            vert.position = tx.transformPoint(v.position);
            vert.color.a = static_cast<sf::Uint8>(r.alpha * 255.f);
            m_vertices[idx++] = vert;
        }
    }
}

void RoidBelt::flipDirection()
{
    for (auto& r : m_roids)
    {
        r.velocity.x = -r.velocity.x;
        r.rotation = -r.rotation;
    }
}

//private
void RoidBelt::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (m_shader)
    {
        states.shader = m_shader;
        auto wvm = xy::Scene::getViewMatrix() * states.transform;
        m_shader->setUniform("u_inverseWorldViewMatrix", sf::Glsl::Mat4(wvm.getInverse()));
        m_shader->setUniform("u_normalMap", m_normalTexture);
        m_shader->setUniform("u_diffuseMap", m_texture);
    }

    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}


//------roid-----//
void RoidBelt::Roid::update(float dt, const sf::FloatRect& bounds)
{
    //rotate(rotation * dt);
    move(velocity * speed * dt);

    //check bounds
    auto position = getPosition();
    if (position.x < 0.f)
    {
        move(bounds.width, 0.f);
        position.x += bounds.width;
    }
    else if (position.x > bounds.width)
    {
        move(-bounds.width, 0.f);
        position.x -= bounds.width;
    }

    //set transparency based on position
    if (position.x < fadeDistance)
    {
        alpha = std::max(0.f, position.x / fadeDistance);
    }
    else if (position.x > bounds.width - fadeDistance)
    {
        const float dist = bounds.width - position.x;
        alpha = dist / fadeDistance;
    }
    else
    {
        alpha = 1.f;
    }
}