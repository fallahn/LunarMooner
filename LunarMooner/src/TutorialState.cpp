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

#include <TutorialState.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

TutorialState::TutorialState(xy::StateStack& stack, Context context, xy::StateID parentID)
    : xy::State (stack, context),
    m_active    (false)
{
    switch (parentID)
    {
    default:
        messageHandler = [](const xy::Message&) {};
        break;
    case States::ID::SinglePlayer:
        messageHandler = std::bind(&TutorialState::handleGameMessage, this, std::placeholders::_1);
        break;
    case States::ID::PlanetHopping:
        messageHandler = std::bind(&TutorialState::handleHopMesage, this, std::placeholders::_1);
        break;
    }

    m_testShape.setRadius(500.f);
    m_testShape.setFillColor(sf::Color::Red);
    m_testShape.setPosition(xy::DefaultSceneSize / 2.f);
}

//public
bool TutorialState::handleEvent(const sf::Event& evt)
{
    return !m_active;
}

void TutorialState::handleMessage(const xy::Message& msg)
{
    messageHandler(msg);
}

bool TutorialState::update(float dt)
{
    //for(auto& t : things) t.update(dt);
    return !m_active;
}

void TutorialState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.setView(getContext().defaultView);
    
    rw.draw(m_testShape);


    //for(auto& t : things) rw.draw(t);
}

//private
void TutorialState::handleGameMessage(const xy::Message& msg)
{

}

void TutorialState::handleHopMesage(const xy::Message& msg)
{

}