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

#include <LMShipLight.hpp>
#include <CommandIds.hpp>

#include <xygine/Entity.hpp>
#include <xygine/components/PointLight.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

using namespace lm;

namespace
{
    const float flashTime = 0.5f;
    const float lightIntensity = 1.2f;
}

ShipLight::ShipLight(xy::MessageBus& mb, sf::Texture& t)
    : xy::Component (mb, this),
    m_texture       (t),
    m_currentColour (sf::Color::Red),
    m_flash         (true),
    m_flashTime     (0.f)
{
    //set up drawable
    sf::Vector2f size(t.getSize());

    m_vertices[1].position.x = size.x;
    m_vertices[1].texCoords.x = size.x;

    m_vertices[2].position = size;
    m_vertices[2].texCoords = size;

    m_vertices[3].position.y = size.y;
    m_vertices[3].texCoords.y = size.y;

    for (auto& v : m_vertices)
    {
        v.color = m_currentColour;
    }
    setOrigin(size / 2.f);

    //add callbacks for messages
    xy::Component::MessageHandler mh;
    mh.id = GameEvent;
    mh.action = [this](xy::Component*, const xy::Message& msg)
    {
        auto msgData = msg.getData<LMGameEvent>();
        if (msgData.type == LMGameEvent::HumanPickedUp)
        {
            m_currentColour = sf::Color::Green;
            m_flash = false;

            for (auto& v : m_vertices) v.color = m_currentColour;
            auto light = m_entity->getComponent<xy::PointLight>();
            if (light)
            {
                light->setDiffuseColour(sf::Color::Green);
                light->setIntensity(lightIntensity);
            }
        }
        else if (msgData.type == LMGameEvent::HumanRescued)
        {
            m_currentColour = sf::Color::Red;
            m_flash = true;
            m_flashTime = 0.f;

            for (auto& v : m_vertices) v.color = m_currentColour;
            auto light = m_entity->getComponent<xy::PointLight>();
            if (light)
            {
                light->setDiffuseColour(sf::Color::Red);
                light->setIntensity(lightIntensity);
            }
        }
    };
    addMessageHandler(mh);
}

//public
void ShipLight::entityUpdate(xy::Entity&, float dt)
{
    if (m_flash)
    {
        m_flashTime += dt;
        if (m_flashTime > flashTime)
        {
            m_flashTime = 0.f;
            m_currentColour.a = (m_currentColour.a == 0) ? 255 : 0;

            for (auto& v : m_vertices) v.color = m_currentColour;

            auto light = m_entity->getComponent<xy::PointLight>();
            if (light)
            {
                light->setIntensity((m_currentColour.a == 0) ? 0.f : lightIntensity);
            }
        }
    }
}

//private
void ShipLight::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.transform *= getTransform();
    states.texture = &m_texture;
    states.blendMode = sf::BlendAdd;

    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}