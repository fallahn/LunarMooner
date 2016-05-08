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

#include <LMPlayerDrawable.hpp>

#include <xygine/Resource.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Shader.hpp>

using namespace lm;

PlayerDrawable::PlayerDrawable(xy::MessageBus& mb, xy::TextureResource& tr, const sf::Vector2f& size)
    : xy::Component (mb, this),
    m_hasShield     (false),
    m_diffuseTexture(tr.get("assets/images/game/ship_diffuse.png")),
    m_normalMap     (tr.get("assets/images/game/ship_normal.png")),
    m_shieldTexture (tr.get("assets/images/game/ship_shield.png"))
{
    m_bounds.width = size.x;
    m_bounds.height = size.y;

    sf::Vector2f texSize(m_diffuseTexture.getSize());
    const float offset = (texSize.x - size.x) / 2.f;

    m_vertices[0].texCoords.x = offset;
    m_vertices[1].texCoords.x = offset + size.x;
    m_vertices[1].position.x = size.x;
    m_vertices[2].texCoords = { offset + size.x, size.y };
    m_vertices[2].position = size;
    m_vertices[3].texCoords = { offset, size.y };
    m_vertices[3].position.y = size.y;

    //left leg
    m_legs[0].vertices[1].position.x = offset;
    m_legs[0].vertices[1].texCoords.x = offset;
    m_legs[0].vertices[2].position = { offset, size.y };
    m_legs[0].vertices[2].texCoords = { offset, size.y };
    m_legs[0].vertices[3].position.y = size.y;
    m_legs[0].vertices[3].texCoords.y = size.y;
    m_legs[0].setOrigin(offset, 0.f);

    //right leg
    m_legs[1].vertices[0].texCoords.x = offset + size.x;
    m_legs[1].vertices[1].position.x = offset;
    m_legs[1].vertices[1].texCoords.x = texSize.x;
    m_legs[1].vertices[2].position = { offset, size.y };
    m_legs[1].vertices[2].texCoords = texSize;
    m_legs[1].vertices[3].position.y = size.y;
    m_legs[1].vertices[3].texCoords = { offset + size.x, size.y };
    m_legs[1].setPosition(size.x, 0.f);

    auto i = 4u;
    for (auto& l : m_legs)
    {
        for (const auto& v : l.vertices)
        {
            m_vertices[i++] = v;
        }
        l.setScale(0.2f, 1.f);
    }
}

//public
void PlayerDrawable::entityUpdate(xy::Entity&, float)
{
    //update vertex array
    auto i = 4u;
    for (const auto& l : m_legs)
    {
        const auto& tx = l.getTransform();
        for (const auto& v : l.vertices)
        {
            m_vertices[i++].position = tx.transformPoint(v.position);
        }
    }
}

void PlayerDrawable::setSpeed(float scale)
{
    for (auto& l : m_legs)
    {
        l.setScale(1.f - scale, 1.f);
    }
}

//private
void PlayerDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (auto shader = getShader())
    {
        shader->setUniform("u_diffuseMap", m_diffuseTexture);
        shader->setUniform("u_normalMap", m_normalMap);

        const auto& tx = states.transform.getInverse();
        shader->setUniform("u_inversWorldViewMatrix", sf::Glsl::Mat4(tx));
    }

    states.shader = getActiveShader();
    states.texture = &m_diffuseTexture;
    states.transform *= getTransform();
    
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);

    if (m_hasShield)
    {
        states.blendMode = sf::BlendAdd;
        states.texture = &m_shieldTexture;
        states.shader = nullptr;
        rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
    }
}