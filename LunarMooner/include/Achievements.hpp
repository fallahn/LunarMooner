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

#ifndef PLAYER_ACHIEVEMENTS_HPP_
#define PLAYER_ACHIEVEMENTS_HPP_

#include <array>
#include <string>

enum AchievementID
{
    LongHaul = 0,
    BulletDodger,
    SkinOfYourTeeth,
    DeadEye,
    Stamper,
    Hero,
    SuperHero,
    Champion,
    AcrossTheBoard,
    Pacifist,
    MoreHasteLessSpeed,
    Count
};

static const std::array<std::string, AchievementID::Count> AchievementNames = 
{
    "Long Haul - Get to Level 10",
    "Bullet Dodger - Survive An Asteroid Strike With A Shield",
    "Skin Of Your Teeth - Win With 1 Second Remaining",
    "Dead Eye - Shoot An Asteroid",
    "Stamper - Shoot 10 Asteroids",
    "Hero - Rescue 50 People",
    "Super Hero - Rescue 100 People",
    "Champion - Rescue 250 People",
    "Across The Board - Complete At Least One Level On Each Difficulty",
    "Pacifist - Complete A Level Without Firing A Bullet",
    "More Haste, Less Speed - Complete A Level Before The 30 Second Warning"
};

#endif //PLAYER_ACHIEVEMENTS_HPP_