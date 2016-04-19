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
    SkinOfYourTeeth,
    DeadEye,
    Stamper,
    Hero,
    SuperHero,
    Champion,
    AcrossTheBoard,
    Pacifist,
    MoreHasteLessSpeed,
    BackWithABump,
    Survivor,
    Hoarder,
    Rank10,
    Rank20,
    Rank30,
    Rank40,
    Rank50,
    Count
};

static const std::array<std::string, AchievementID::Count> achievementNames = 
{
    "Long Haul - Get to Level 10",
    "Skin Of Your Teeth - Win With 1 Second Remaining",
    "Dead Eye - Shoot An Asteroid",
    "Stamper - Shoot 10 Asteroids",
    "Hero - Rescue 50 People",
    "Super Hero - Rescue 100 People",
    "Champion - Rescue 250 People",
    "Across The Board - Complete At Least One Level On Each Difficulty",
    "Pacifist - Complete A Level Without Firing A Bullet",
    "More Haste, Less Speed - Complete A Level Before The 30 Second Warning",
    "Bumpy Landing - Survive A Hard Landing With A Shield",
    "Survivor - Complete At Least One Level On Each Difficulty Without Dying",
    "Hoarder - Collect 50 power ups",
    "Rank 10 - Get 3,350 XP",
    "Rank 20 - Get 12,100 XP",
    "Rank 30 - Get 25,950 XP",
    "Rank 40 - Get 44,700 XP",
    "Rank 50 - Get 68,450 XP"
};

//to get to next level the increase is current level * 50 + 100
static const std::array<int, 50u> rankXP = 
{
    0,     150,   350,   600,   900,   1250,  1700,  2200,  2750,  3350,
    4000,  4700,  5450,  6250,  7100,  8000,  8950,  9950,  11000, 12100,
    13350, 14550, 15800, 17100, 18450, 19850, 21300, 22800, 24350, 25950,
    27600, 29300, 31050, 32850, 34700, 36600, 38550, 40550, 42600, 44700,
    46850, 49050, 51300, 53600, 55950, 58350, 60800, 63300, 65850, 68450
};
#endif //PLAYER_ACHIEVEMENTS_HPP_