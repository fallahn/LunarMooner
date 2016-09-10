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

#ifndef LM_TUTORIAL_STATE_HPP_
#define LM_TUTORIAL_STATE_HPP_

#include <StateIds.hpp>

#include <xygine/State.hpp>

#include <SFML/Graphics/CircleShape.hpp>

#include <functional>
#include <vector>

class TutorialState final : public xy::State
{
public:
    TutorialState(xy::StateStack&, Context, xy::StateID);
    ~TutorialState() = default;

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

    xy::StateID stateID() const override { return States::ID::Tutorial; }

private:

    sf::CircleShape m_testShape;

    bool m_active;

    //std::vector<IconThings> m_icons;

    std::function<void(const xy::Message&)> messageHandler;

    void handleGameMessage(const xy::Message&);
    void handleHopMesage(const xy::Message&);
};

#endif //LM_TUTORIAL_STATE_HPP_