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

#include <BGPlanetDrawable.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Vector.hpp>

#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Sprite.hpp>

using namespace lm;

PlanetDrawable::PlanetDrawable(xy::MessageBus& mb, float radius)
    : xy::Component (mb, this),
    m_baseNormal    (nullptr),
    m_detailNormal  (nullptr),
    m_diffuseTexture(nullptr),
    m_maskTexture   (nullptr),
    m_prepassShader (nullptr),
    m_normalShader  (nullptr),
    m_radius        (radius)
{
    m_renderTexture.create(2048, 2048, 3);
    
    //sf::Vector2f centre(radius, radius);
    //m_vertices.emplace_back(centre);
    //const float pointCount = 40.f;
    //const float step = xy::Util::Const::TAU / pointCount;
    //for (auto i = 0.f; i <= pointCount; ++i)
    //{
    //    const float theta = i * step;
    //    m_vertices.emplace_back((sf::Vector2f(std::cos(theta), std::sin(theta)) * radius) + centre);
    //}

    m_vertices =
    {
        sf::Vector2f(),
        sf::Vector2f(radius * 2.f, 0.f),
        sf::Vector2f(radius * 2.f, radius * 2.f),
        sf::Vector2f(0.f, radius * 2.f)
    };

    m_localBounds.width = radius * 2.f;
    m_localBounds.height = m_localBounds.width;

    m_textureVelocity.x = 0.01f;
    m_textureVelocity.y = 0.005f;
}

//public
void PlanetDrawable::entityUpdate(xy::Entity& entity, float dt)
{
    //entity.move(m_textureVelocity);
    m_globalBounds = entity.getWorldTransform().transformRect(m_localBounds);

    XY_ASSERT(m_baseNormal, "Base texture missing from planet drawable!");
    XY_ASSERT(m_detailNormal, "Detail texture missing from planet drawable!");
    XY_ASSERT(m_diffuseTexture, "Diffuse texture missing from planet drawable!");
    XY_ASSERT(m_maskTexture, "Mask texture missing from planet drawable!");
    XY_ASSERT(m_prepassShader, "Prepass shader missing from planet drawable");

    //update texture position
    m_textureOffset += m_textureVelocity * dt;
    m_prepassShader->setUniform("u_detailOffset", sf::Glsl::Vec2(m_textureOffset));

    //update blend shader parameter
    m_prepassShader->setUniform("u_distortionTexture", *m_baseNormal);
    m_prepassShader->setUniform("u_normalTexture", *m_detailNormal);
    m_prepassShader->setUniform("u_diffuseTexture", *m_diffuseTexture);
    m_prepassShader->setUniform("u_maskTexture", *m_maskTexture);

    //update render texture
    m_renderTexture.clear(sf::Color::Transparent);
    //TODO use the sprite to make sure texture is scaled to 1024?
    //texture size should be irrelevant when using uniform samplers
    sf::RenderStates states;
    m_renderTexture.draw(sf::Sprite(*m_detailNormal), m_prepassShader);
    m_renderTexture.display();
}

void PlanetDrawable::setBaseNormal(sf::Texture& t)
{
    m_baseNormal = &t;
    m_baseNormal->setRepeated(true);
}

void PlanetDrawable::setDetailNormal(sf::Texture& t)
{
    m_detailNormal = &t;
    m_detailNormal->setRepeated(true);
}

void PlanetDrawable::setDiffuseTexture(sf::Texture& t)
{
    sf::Vector2f size(t.getSize());

    //const float diameter = m_radius * 2.f;
    //sf::Vector2f vertSize(diameter, diameter);

    //for (auto& v : m_vertices)
    //{
    //    const float x = v.position.x / vertSize.x;
    //    const float y = v.position.y / vertSize.y;
    //    v.texCoords.x = size.x * x;
    //    v.texCoords.y = size.y * y;
    //}

    m_vertices[1].texCoords.x = size.x;
    m_vertices[2].texCoords = size;
    m_vertices[3].texCoords.y = size.y;

    m_diffuseTexture = &t;
    m_diffuseTexture->setRepeated(true);
}

void PlanetDrawable::setMaskTexture(sf::Texture& t)
{
    m_maskTexture = &t;
    m_maskTexture->setRepeated(true);
}

void PlanetDrawable::setPrepassShader(sf::Shader& s)
{
    m_prepassShader = &s;
}

void PlanetDrawable::setNormalShader(sf::Shader& s)
{
    m_normalShader = &s;
}

void PlanetDrawable::setColour(const sf::Color& c)
{
    for (auto& v : m_vertices)
    {
        v.color = c;
    }
}

void PlanetDrawable::setRotationVelocity(const sf::Vector2f& vel)
{
    m_textureVelocity = vel;
}

void PlanetDrawable::setTextureOffset(const sf::Vector2f& offset)
{
    m_textureOffset = offset;
}

//private
void PlanetDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    XY_ASSERT(m_normalShader, "Shader not set in planet drawable!");
    //update normal map shader
    auto wvm = xy::Scene::getViewMatrix() * states.transform;
    m_normalShader->setUniform("u_inverseWorldViewMatrix", sf::Glsl::Mat4(wvm.getInverse()));
    m_normalShader->setUniform("u_normalMap", m_renderTexture.getTexture(0)/**m_baseNormal*/);
    m_normalShader->setUniform("u_diffuseMap", m_renderTexture.getTexture(1));
    m_normalShader->setUniform("u_maskMap", m_renderTexture.getTexture(2));
    
    states.texture = &m_renderTexture.getTexture(1);
    states.shader = m_normalShader;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}