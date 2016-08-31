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

const sf::Keyboard::Key keyStart = sf::Keyboard::Return;
const sf::Keyboard::Key keyLeft = sf::Keyboard::A;
const sf::Keyboard::Key keyRight = sf::Keyboard::D;
const sf::Keyboard::Key keyThrust = sf::Keyboard::W;
const sf::Keyboard::Key keyFire = sf::Keyboard::Space;
const sf::Keyboard::Key keySpecial = sf::Keyboard::LControl;

const sf::Keyboard::Key altKeyLeft = sf::Keyboard::Left;
const sf::Keyboard::Key altKeyRight = sf::Keyboard::Right;
const sf::Keyboard::Key altKeyThrust = sf::Keyboard::Up;
const sf::Keyboard::Key altKeySpecial = sf::Keyboard::RControl;

//x360 controller mapping
const sf::Uint32 buttonA = 0u;
const sf::Uint32 buttonB = 1u;
const sf::Uint32 buttonX = 2u;
const sf::Uint32 buttonY = 3u;
const sf::Uint32 buttonLB = 4u;
const sf::Uint32 buttonRB = 5u;
const sf::Uint32 buttonBack = 6u;
const sf::Uint32 buttonStart = 7u;
const float joyDeadZone = 25.f;

#endif //KEYMAP_INL_