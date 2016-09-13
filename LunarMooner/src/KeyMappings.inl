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

#ifndef KEYMAP_INL_
#define KEYMAP_INL_

//const sf::Keyboard::Key keyStart = sf::Keyboard::Return;
//const sf::Keyboard::Key keyLeft = sf::Keyboard::A;
//const sf::Keyboard::Key keyRight = sf::Keyboard::D;
//const sf::Keyboard::Key keyThrust = sf::Keyboard::W;
//const sf::Keyboard::Key keyFire = sf::Keyboard::Space;
//const sf::Keyboard::Key keySpecial = sf::Keyboard::LControl;
//
//const sf::Keyboard::Key altKeyLeft = sf::Keyboard::Left;
//const sf::Keyboard::Key altKeyRight = sf::Keyboard::Right;
//const sf::Keyboard::Key altKeyThrust = sf::Keyboard::Up;
//const sf::Keyboard::Key altKeySpecial = sf::Keyboard::RControl;
//
////x360 controller mapping
//const sf::Uint32 buttonA = 0u;
//const sf::Uint32 buttonB = 1u;
//const sf::Uint32 buttonX = 2u;
//const sf::Uint32 buttonY = 3u;
//const sf::Uint32 buttonLB = 4u;
//const sf::Uint32 buttonRB = 5u;
//const sf::Uint32 buttonBack = 6u;
//const sf::Uint32 buttonStart = 7u;
const float joyDeadZone = 25.f;

static void parseControllerInput(sf::Uint8& inputFlags)
{
    //we need to convert the analogue input
    //to out own 'Pressed / Released' events
    static bool lastJoyLeft = false;
    static bool lastJoyRight = false;
    static bool lastPovLeft = false;
    static bool lastPovRight = false;
    static bool lastThrust = false;

    //joystick direction handling    
    std::function<void(bool&, bool&, float)> parse = [&inputFlags](bool& lastLeft, bool& lastRight, float xVal)
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
                inputFlags |= LMInputFlags::SteerLeft :
                inputFlags &= ~LMInputFlags::SteerLeft;
        }
        if (lastRight != right)
        {
            (right) ?
                inputFlags |= LMInputFlags::SteerRight :
                inputFlags &= ~LMInputFlags::SteerRight;
        }
        lastLeft = left;
        lastRight = right;
    };

    float xValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
    parse(lastJoyLeft, lastJoyRight, xValue);

    //dpad
    xValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
    parse(lastPovLeft, lastPovRight, xValue);

    //xbox triggers
    float zValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Z);
    bool thrust = (zValue < -joyDeadZone || zValue > joyDeadZone);
    if (lastThrust != thrust)
    {
        (thrust) ?
            inputFlags |= LMInputFlags::Thrust :
            inputFlags &= ~LMInputFlags::Thrust;

    }
    lastThrust = thrust;
}

#endif //KEYMAP_INL_