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

#include <LMState.hpp>
#include <LMGameController.hpp>
#include <CommandIds.hpp>

#include <xygine/App.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
    const sf::Keyboard::Key keyStart = sf::Keyboard::Return;
    const sf::Keyboard::Key keyLeft = sf::Keyboard::A;
    const sf::Keyboard::Key keyRight = sf::Keyboard::D;
    const sf::Keyboard::Key keyThrust = sf::Keyboard::W;
    const sf::Keyboard::Key keyFire = sf::Keyboard::Space;

    const sf::Keyboard::Key altKeyLeft = sf::Keyboard::Left;
    const sf::Keyboard::Key altKeyRight = sf::Keyboard::Right;
    const sf::Keyboard::Key altKeyThrust = sf::Keyboard::Up;

    //x360 controller mapping
    const sf::Uint32 buttonA = 0u;
    const sf::Uint32 buttonB = 1u;
    const sf::Uint32 buttonStart = 7u;
    const float joyDeadZone = 25.f;
}

LunarMoonerState::LunarMoonerState(xy::StateStack& stack, Context context, sf::Uint8 playerCount)
    : xy::State         (stack, context),
    m_playerCount       (playerCount),
    m_scene             (context.appInstance.getMessageBus()),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_inputFlags        (0),
    m_prevInputFlags    (0)
{
    XY_ASSERT(playerCount > 0, "Need at least one player");
    launchLoadingScreen();
    
    m_scene.setView(context.defaultView);
    //m_scene.drawDebug(true);

    auto gameController = xy::Component::create<lm::GameController>(m_messageBus, m_scene, m_collisionWorld);
    for (auto i = 0; i < playerCount; ++i)  gameController->addPlayer();
    gameController->start();

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(gameController);
    entity->addCommandCategories(LMCommandID::GameController);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    xy::Stats::clear();
    m_reportText.setFont(m_fontResource.get("buns"));

    quitLoadingScreen();
}

//public
bool LunarMoonerState::handleEvent(const sf::Event& evt)
{
    //TODO handle controller connect / disconnect events
    switch(evt.type)
    {
    case sf::Event::KeyReleased:
        switch (evt.key.code)
        {
        case keyStart:
            m_inputFlags &= ~LMInputFlags::Start;
            break;
        case keyLeft:
        case altKeyLeft:
            m_inputFlags &= ~LMInputFlags::SteerLeft;
            break;
        case keyRight:
        case altKeyRight:
            m_inputFlags &= ~LMInputFlags::SteerRight;
            break;
        case keyThrust:
        case altKeyThrust:
            m_inputFlags &= ~LMInputFlags::Thrust;
            break;
        case keyFire:
            m_inputFlags &= ~LMInputFlags::Shoot;
            break;
        default:break;
        }
        break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case keyStart:
            m_inputFlags |= LMInputFlags::Start;
            break;
        case keyLeft:
        case altKeyLeft:
            m_inputFlags |= LMInputFlags::SteerLeft;
            break;
        case keyRight:
        case altKeyRight:
            m_inputFlags |= LMInputFlags::SteerRight;
            break;
        case keyThrust:
        case altKeyThrust:
            m_inputFlags |= LMInputFlags::Thrust;
            break;
        case keyFire:
            m_inputFlags |= LMInputFlags::Shoot;
            break;
        default:break;
        }
        break;


        //controller input (default for x360 layout)
    case sf::Event::JoystickButtonPressed:
        if (evt.joystickButton.joystickId != 0) break;
        switch (evt.joystickButton.button)
        {
        case buttonA:
            m_inputFlags |= LMInputFlags::Thrust;
            break;
        case buttonB:
            m_inputFlags |= LMInputFlags::Shoot;
            break;
        case buttonStart:
            m_inputFlags |= LMInputFlags::Start;
            break;
        }
        break;
    case sf::Event::JoystickButtonReleased:
        if (evt.joystickButton.joystickId != 0) break;
        switch (evt.joystickButton.button)
        {
        case buttonA:
            m_inputFlags &= ~LMInputFlags::Thrust;
            break;
        case buttonB:
            m_inputFlags &= ~LMInputFlags::Shoot;
            break;
        case buttonStart:
            m_inputFlags &= ~LMInputFlags::Start;
            break;
        }
        break;
    default: break;
    }
    return true;
}

void LunarMoonerState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

bool LunarMoonerState::update(float dt)
{
    parseControllerInput();
    
    if (m_inputFlags != m_prevInputFlags)
    {
        m_prevInputFlags = m_inputFlags;
        xy::Command cmd;
        cmd.category = LMCommandID::GameController;
        cmd.action = [this](xy::Entity& entity, float)
        {
            XY_ASSERT(entity.getComponent<lm::GameController>(), "");
            entity.getComponent<lm::GameController>()->setInput(m_inputFlags);
        };
        m_scene.sendCommand(cmd);
    }
    
    m_scene.update(dt);
    m_collisionWorld.update();

    //m_reportText.setString(xy::Stats::getString());

    return true;
}

void LunarMoonerState::draw()
{
    auto& rw = getContext().renderWindow;

    rw.draw(m_scene);

    rw.setView(getContext().defaultView);
    //rw.draw(m_reportText);
}

//private
namespace
{
    //we need to convert the analogue input
    //to out own 'Pressed / Released' events
    bool lastJoyLeft = false;
    bool lastJoyRight = false;
    bool lastPovLeft = false;
    bool lastPovRight = false;
}

void LunarMoonerState::parseControllerInput()
{
    //joystick direction handling    
    std::function<void(bool&, bool&, float)> parse = [this](bool& lastLeft, bool& lastRight, float xVal)
    {
        bool left = false;
        bool right = false;
        
        if (xVal < -joyDeadZone)
        {
            left = true;
        }
        else if (xVal > joyDeadZone)
        {
            right = true;
        }

        if (lastLeft != left)
        {
            (left) ?
                m_inputFlags |= LMInputFlags::SteerLeft :
                m_inputFlags &= ~LMInputFlags::SteerLeft;
        }
        if (lastRight != right)
        {
            (right) ?
                m_inputFlags |= LMInputFlags::SteerRight :
                m_inputFlags &= ~LMInputFlags::SteerRight;
        }
        lastLeft = left;
        lastRight = right;
    };

    float xValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
    parse(lastJoyLeft, lastJoyRight, xValue);

    //dpad
    xValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
    parse(lastPovLeft, lastPovRight, xValue);
}