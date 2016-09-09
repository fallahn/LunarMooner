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
#include <Achievements.hpp>

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
    const std::function<void(const std::string&)> displayTag = 
        [this](const std::string& text)
    {
        auto tag = xy::Component::create<lm::AchievementTag>(m_messageBus, m_resources.fontResource, text);
        auto ent = xy::Entity::create(m_messageBus);
        ent->setPosition(xy::DefaultSceneSize.x - padding, xy::DefaultSceneSize.y);
        ent->addComponent(tag);

        //check for existing notifications and stack them
        if (!m_rootNode->getChildren().empty())
        {
            auto child = m_rootNode->removeChild(*m_rootNode->getChildren()[0]);
            child->move(-ent->getPosition());
            ent->addChild(child);
        }

        m_rootNode->addChild(ent);
    };
    
    if (msg.id == LMMessageId::AchievementEvent)
    {
        auto& msgData = msg.getData<LMAchievementEvent>();

        auto text = achievementNames[msgData.ID];
        text = text.substr(0, text.find_first_of('-') - 1);

        displayTag(text);
    }
    else if(msg.id == LMMessageId::RankEvent)
    {
        auto& msgData = msg.getData<LMRankEvent>();
        std::string text;
        if (msgData.type == LMRankEvent::RankUp)
        {
            text = "RANK UP!\nLevel: " + std::to_string(msgData.value);
            displayTag(text);
        }
        else if (msgData.type == LMRankEvent::XPAwarded && msgData.value > 0)
        {
            text = "+" + std::to_string(msgData.value) + " XP!";
            displayTag(text);
        }       
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