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

#include <SFML/Graphics/Sprite.hpp>

using namespace lm;

PlanetDrawable::PlanetDrawable(xy::MessageBus& mb)
    : xy::Component (mb, this),
    m_baseNormal    (nullptr),
    m_detailNormal  (nullptr),
    m_blendShader   (nullptr),
    m_normalShader  (nullptr)
{
    m_renderTexture.create(1024, 1024);

    m_shape.setRadius(200.f);
    m_shape.setTexture(&m_renderTexture.getTexture());
}

//public
void PlanetDrawable::entityUpdate(xy::Entity&, float)
{
    //update texture position

    //update blend shader parameter

    //update render texture
    XY_ASSERT(m_baseNormal, "Base texture missing from planet drawable!");
    XY_ASSERT(m_detailNormal, "Detail texture missing from planet drawable!");
    XY_ASSERT(m_blendShader, "Blend shader missing from planet drawable");

    m_renderTexture.clear();
    m_renderTexture.draw(sf::Sprite(*m_baseNormal), m_blendShader);
    m_renderTexture.display();


}

void PlanetDrawable::setBaseNormal(sf::Texture& t)
{
    m_baseNormal = &t;
}

void PlanetDrawable::setDetailNormal(sf::Texture& t)
{
    m_detailNormal = &t;
    m_detailNormal->setRepeated(true);
}

void PlanetDrawable::setBlendShader(sf::Shader& s)
{
    m_blendShader = &s;
}

void PlanetDrawable::setNormalShader(sf::Shader& s)
{
    m_normalShader = &s;
}

//private
void PlanetDrawable::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    //XY_ASSERT(m_normalShader, "Shader not set in planet drawable!");
    states.shader = m_normalShader;
    rt.draw(m_shape, states);
}