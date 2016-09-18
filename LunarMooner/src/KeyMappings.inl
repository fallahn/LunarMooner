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
    std::function<void(bool&, bool&, float)> parseX = [&inputFlags](bool& lastLeft, bool& lastRight, float xVal)
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
    parseX(lastJoyLeft, lastJoyRight, xValue);

    //dpad
    xValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
    parseX(lastPovLeft, lastPovRight, xValue);



    //repeat for Y axis
    static bool lastJoyDown = false;
    static bool lastPovDown = false;

    //joystick direction handling    
    std::function<void(bool&, float)> parseY = [&inputFlags](bool& lastDown, float yVal)
    {
        bool down = false;

        if (yVal > joyDeadZone)
        {
            down = true;
        }

        if (lastDown != down)
        {
            (down) ?
                inputFlags |= LMInputFlags::ThrustDown :
                inputFlags &= ~LMInputFlags::ThrustDown;
        }

        lastDown = down;
    };
    float yValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
    parseY(lastJoyDown, yValue);

    yValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY);
    parseY(lastPovDown, -yValue);

    //xbox triggers
    float zValue = sf::Joystick::getAxisPosition(0, sf::Joystick::Z);
    bool thrust = (zValue < -joyDeadZone || zValue > joyDeadZone);
    if (lastThrust != thrust)
    {
        (thrust) ?
            inputFlags |= LMInputFlags::ThrustUp :
            inputFlags &= ~LMInputFlags::ThrustUp;

    }
    lastThrust = thrust;
}

#endif //KEYMAP_INL_