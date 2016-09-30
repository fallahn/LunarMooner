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

#include <LEPointCollection.hpp>

#include <xygine/Log.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace le;

namespace
{
    const std::size_t maxPoints = 50;
}

PointCollection::PointCollection()
{

}

//public
SelectableItem* PointCollection::getSelected(const sf::Vector2f& mousePos)
{
    for (auto& p : m_points)
    {
        if (p->globalBounds().contains(mousePos))
        {
            p->select();
            return p.get();
        }
    }
    return nullptr;
}

void PointCollection::update()
{
    m_points.erase(std::remove_if(std::begin(m_points), std::end(m_points),
        [](const std::unique_ptr<PointItem>& item)
    {
        return item->deleted();
    }), std::end(m_points));

    //rebuild the vertex array
    auto i = 0;
    for (const auto& p : m_points)
    {
        m_vertices[i++] = sf::Vertex(p->getPosition(), sf::Color::Green);
    }
}

SelectableItem* PointCollection::add(const sf::Vector2f& position)
{
    if (m_points.size() < maxPoints)
    {
        m_points.emplace_back(std::make_unique<PointItem>());
        m_points.back()->setPosition(position);
        return m_points.back().get();
    }
    else
    {
        xy::Logger::log("Maximum number of points is " + std::to_string(maxPoints), xy::Logger::Type::Info);
    }
    return nullptr;
}

std::vector<sf::Vector2f> PointCollection::getPoints() const
{
    std::vector<sf::Vector2f> points;
    for (const auto& p : m_points)
    {
        points.push_back(p->getPosition());
    }
    return std::move(points);
}

//private
void PointCollection::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_vertices.data(), m_points.size(), sf::PrimitiveType::LineStrip);
    for (const auto& p : m_points) rt.draw(*p);
}