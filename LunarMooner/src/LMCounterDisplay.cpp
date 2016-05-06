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

#include <LMCounterDisplay.hpp>
#include <CommandIds.hpp>

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <xygine/Assert.hpp>

#include <functional>

using namespace lm;

namespace
{
    const float textSpacing = 34.f;
}

CounterDisplay::CounterDisplay(sf::Texture& texture, sf::Uint8 digitCount, sf::Time updateTime)
    : m_currentValue(0),
    m_subRects      (digitCount),
    m_texture       (texture),
    m_vertices      (digitCount * 12) //12 verts per digit
{
    texture.setRepeated(true);

    sf::Vector2f size(texture.getSize());
    size.x /= 2.f;
    size.y /= 10.f;

    auto v = 0u;
    auto factor = 0;
    for (auto i = 0u; i < m_subRects.size(); ++i)
    {
        m_subRects[i].vertices[1].texCoords.x = size.x;
        m_subRects[i].vertices[2].texCoords = size;
        m_subRects[i].vertices[3].texCoords.y = size.y;

        m_subRects[i].vertices[0].position.x = size.x * i;
        m_subRects[i].vertices[1].position.x = (size.x * i) + size.x;
        m_subRects[i].vertices[2].position = { (size.x * i) + size.x , size.y };
        m_subRects[i].vertices[3].position = { size.x * i, size.y };

        //use the same verts for the motion blur to start off with
        m_subRects[i].blurVertices[0] = m_subRects[i].vertices[0];
        m_subRects[i].blurVertices[1] = m_subRects[i].vertices[1];
        m_subRects[i].blurVertices[2] = m_subRects[i].vertices[2];
        m_subRects[i].blurVertices[3] = m_subRects[i].vertices[3];

        m_subRects[i].size = size;

        for (auto& vert : m_subRects[i].vertices)
        {
        	m_vertices[v++] = vert;
        }
        
        //add blur verts and make them 50% alpha
        for (auto& vert : m_subRects[i].blurVertices)
        {
        	vert.color = sf::Color(255,255,255,128);
        	m_vertices[v++] = vert;
        }

        m_subRects[i].updateTime = updateTime;

        //tell it which factor this digit is displaying
        m_subRects[i].factor = digitCount - 1 - factor++; //abrakadabra
    }

    for (const auto& sr : m_subRects)
    {
        for (const auto vert : sr.vertices)
        {
            m_vertices[v].texCoords = vert.texCoords;
            m_vertices[v].texCoords.x += size.x;
            m_vertices[v++].position = vert.position;
        }
    }
}

//public
void CounterDisplay::update(float dt)
{
    auto i = 0u;
    for (auto& sr : m_subRects)
    {
        const auto& tx = sr.getTransform();
        for (const auto v : sr.vertices)
        {
            m_vertices[i++].texCoords = tx.transformPoint(v.texCoords);
        }
        for (const auto v : sr.blurVertices)
        {
            m_vertices[i++].texCoords = tx.transformPoint(v.texCoords);
        }

        //update digit
        sr.update(dt);
    }
}

void CounterDisplay::setValue(int value)
{
    XY_ASSERT(value < (std::pow(10.f, m_subRects.size()) - 1), "Value too large!");

    bool valueChanged = value != m_currentValue;

    LMDirection direction = (value > m_currentValue) ? LMDirection::Up : LMDirection::Down;

    for (auto& digit : m_subRects)
    {
        digit.targetValue = value;
        if (valueChanged)
        {
            digit.timeSinceValueChange = sf::seconds(0);
            digit.lastValue = m_currentValue;
        }
    } 
    m_currentValue = value;
}

//private
void CounterDisplay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}

//digit update
void CounterDisplay::SubRect::update(float dt)
{
    timeSinceValueChange += sf::seconds(dt);

	//these are ints so the value is floored to an integer,
	//make them floats to have digits partially in view
    int factoredValue(targetValue / static_cast<int>(std::pow(10, factor)));
    int factoredLastValue(lastValue / static_cast<int>(std::pow(10, factor)));

    //update the motion blur
    blurVertices[0].texCoords = vertices[0].texCoords;
    blurVertices[1].texCoords = vertices[1].texCoords;
    blurVertices[2].texCoords = vertices[2].texCoords;
    blurVertices[3].texCoords = vertices[3].texCoords;

    float newVal;
    if (timeSinceValueChange > updateTime)
    {
        //update finished - set to final value
        newVal = static_cast<float>(factoredValue);
    }
    else
    {
        //update in progress, update values accordingly
        auto timeFactor = timeSinceValueChange / updateTime;
        newVal = factoredLastValue + (factoredValue - factoredLastValue)*static_cast<float>(std::sin((timeFactor-0.5f)*(std::atan(1)*4))+1)/2;
    }
    vertices[0].texCoords = sf::Vector2f(0, size.y * newVal);
    vertices[1].texCoords = sf::Vector2f(size.x, size.y * newVal);
    vertices[2].texCoords = sf::Vector2f(size.x, size.y * newVal + size.y);
    vertices[3].texCoords = sf::Vector2f(0, size.y * newVal + size.y);
}