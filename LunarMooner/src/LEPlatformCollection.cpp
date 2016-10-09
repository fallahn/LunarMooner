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

#include <LEPlatformCollection.hpp>

#include <xygine/Log.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace le;

namespace
{
    const std::size_t maxPlatforms = 5;
}

PlatformCollection::PlatformCollection()
    : m_nextPlatformSize    (100.f, 25.f),
    m_nextPlatformValue     (10)
{

}

//public
SelectableItem* PlatformCollection::getSelected(const sf::Vector2f& mousePosition)
{
    if (frozen()) return nullptr;
    
    for (auto& p : m_platforms)
    {
        if (p->globalBounds().contains(mousePosition))
        {
            p->select();
            return p.get();
        }
    }
    return nullptr;
}

void PlatformCollection::update()
{
    m_platforms.erase(std::remove_if(std::begin(m_platforms), std::end(m_platforms), 
        [](const std::unique_ptr<PlatformItem>& p)
    {
        return p->deleted();
    }), std::end(m_platforms));
}

SelectableItem* PlatformCollection::add(const sf::Vector2f& position)
{
    if (m_platforms.size() < maxPlatforms)
    {
        m_platforms.emplace_back(std::make_unique<PlatformItem>());
        m_platforms.back()->setPosition(position);
        m_platforms.back()->setSize(m_nextPlatformSize);
        m_platforms.back()->setValue(m_nextPlatformValue);
        return m_platforms.back().get();
    }
    else
    {
        xy::Logger::log("Maximum number of platforms is " + std::to_string(maxPlatforms), xy::Logger::Type::Info);
    }
    return nullptr;
}

void PlatformCollection::setFrozen(bool frozen)
{
    SelectableCollection::setFrozen(frozen);
    for (auto& p : m_platforms)
    {
        p->setFrozen(frozen);
    }
}

//private
void PlatformCollection::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    if (hidden()) return;
    for (const auto& p : m_platforms)
    {
        rt.draw(*p);
    }
}