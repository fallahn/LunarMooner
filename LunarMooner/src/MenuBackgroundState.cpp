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

#include <MenuBackgroundState.hpp>
#include <LMPostBleach.hpp>

#include <xygine/App.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

#include <SFML/Graphics/CircleShape.hpp>

MenuBackgroundState::MenuBackgroundState(xy::StateStack& ss, Context context)
    : xy::State (ss, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene (m_messageBus)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    setup();

    quitLoadingScreen();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::RequestState;
    msg->stateId = States::ID::MenuMain;
}

//public
bool MenuBackgroundState::update(float dt)
{
    m_scene.update(dt);
    return true;
}

void MenuBackgroundState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    //rw.setView(getContext().defaultView);
}

bool MenuBackgroundState::handleEvent(const sf::Event&)
{
    return false;
}

void MenuBackgroundState::handleMessage(const xy::Message&)
{

}

//private
void MenuBackgroundState::setup()
{
    auto circle = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(m_messageBus);
    circle->getDrawable().setRadius(200.f);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(circle);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);
}