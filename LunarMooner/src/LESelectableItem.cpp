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

#include <LESelectableItem.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace le;

namespace
{
    const float maxPropHeight = 800.f;

    const sf::Vector2f pointSize(10.f, 10.f);
    const sf::Vector2f platSize(100.f, 25.f);
}

//----POINTS----//
PointItem::PointItem()
{
    m_shape.setSize(pointSize);
    m_shape.setOrigin(pointSize / 2.f);
    m_shape.setOutlineThickness(2.f);
    m_shape.setOutlineColor(sf::Color::Red);
    m_shape.setFillColor(sf::Color::Transparent);
}

//public
void PointItem::select()
{
    m_shape.setFillColor(sf::Color::Red);
}

void PointItem::deselect()
{
    m_shape.setFillColor(sf::Color::Transparent);
}

sf::FloatRect PointItem::globalBounds() const
{
    return getTransform().transformRect(m_shape.getGlobalBounds());
}

//private
void PointItem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_shape, states);
}

//------PLATFORM------//
PlatformItem::PlatformItem()
{
    m_shape.setSize(platSize);
    m_shape.setOutlineThickness(2.f);
    m_shape.setOutlineColor(sf::Color::Green);
    m_shape.setFillColor({ 0, 255, 0, 120 });
}

//public
void PlatformItem::select()
{
    m_shape.setOutlineColor(sf::Color::Red);
}

void PlatformItem::deselect()
{
    m_shape.setOutlineColor(sf::Color::Green);
}

sf::FloatRect PlatformItem::globalBounds() const
{
    return getTransform().transformRect(m_shape.getGlobalBounds());
}

void PlatformItem::setSize(const sf::Vector2f& size)
{
    m_shape.setSize(size);
}

const sf::Vector2f& PlatformItem::getSize() const
{
    return m_shape.getSize();
}

//private
void PlatformItem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    rt.draw(m_shape, states);
}

//----PROP----//
#include <xygine/Entity.hpp>
#include <xygine/components/Model.hpp>
PropItem::PropItem(xy::Entity& entity)
    : m_entity(entity)
{
    m_shape.setSize({ 200.f, 200.f });
    //m_shape.setOrigin(m_shape.getSize() / 2.f);
    m_shape.setOutlineThickness(2.f);
    m_shape.setOutlineColor(sf::Color::White);
    m_shape.setFillColor(sf::Color::Transparent);
}

PropItem::~PropItem()
{
    //delete entity
    m_entity.destroy();
}

//public
void PropItem::select()
{
    m_shape.setOutlineColor(sf::Color::Red);
}

void PropItem::deselect()
{
    m_shape.setOutlineColor(sf::Color::White);
}

sf::FloatRect PropItem::globalBounds() const 
{ 
    return m_shape.getGlobalBounds(); //entity global bounds
}

void PropItem::update()
{
    //clamp drawable Y position (see const.inl) 
    auto position = getPosition();
    position.y = std::max(maxPropHeight, position.y);
    setPosition(position);
    
    //set our entity's position
    m_entity.setPosition(getPosition());

    //update drawable with bounds
    auto bounds = m_entity.globalBounds();
    m_shape.setPosition(bounds.left, bounds.top);
    m_shape.setSize({ bounds.width, bounds.height });
}

void PropItem::setModel(std::unique_ptr<xy::Model>& model)
{
    m_entity.getComponent<xy::Model>()->destroy();
    m_entity.addComponent(std::move(model));
}

//private
void PropItem::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    rt.draw(m_shape, states);
}