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

#include <OLOverlay.hpp>
#include <OLAchievementTag.hpp>
#include <CommandIds.hpp>
#include <ResourceCollection.hpp>

#include <xygine/Scene.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

using namespace lm;

namespace
{
    const float padding = 20.f;
}

Overlay::Overlay(xy::MessageBus& mb, ResourceCollection& rc, xy::Scene& scene)
    : m_messageBus  (mb),
    m_resources     (rc)
{
    m_rootNode = xy::Entity::create(mb);
    m_rootNode->setScene(&scene);
}

//public
void Overlay::update(float dt)
{
    m_rootNode->update(dt);
}

void Overlay::handleMessage(const xy::Message& msg)
{
    if (msg.id == LMMessageId::AchievementEvent)
    {
        auto& msgData = msg.getData<LMAchievementEvent>();

        auto tag = xy::Component::create<lm::AchievementTag>(m_messageBus, m_resources.fontResource, msgData.ID);
        auto ent = xy::Entity::create(m_messageBus);
        ent->setPosition(1920.f - padding, 1080.f); //really, screen size should be a config variable somewhere
        ent->addComponent(tag);

        //check for existing notifications and stack them
        if (!m_rootNode->getChildren().empty())
        {
            auto child = m_rootNode->removeChild(*m_rootNode->getChildren()[0]);
            child->move(-ent->getPosition());
            ent->addChild(child);
        }

        m_rootNode->addChild(ent);
    }
}

void Overlay::setView(const sf::View& v)
{
    m_view = v;
}

//private
void Overlay::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    auto oldView = rt.getView();
    rt.setView(m_view);
    rt.draw(*m_rootNode, states);
    rt.setView(oldView);
}