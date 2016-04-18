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

#ifndef PLAYER_PROFILE_HPP_
#define PLAYER_PROFILE_HPP_

#include <Achievements.hpp>

#include <array>

namespace xy
{
    class Message;
    class MessageBus;
}

class PlayerProfile final
{
public:
    explicit PlayerProfile(xy::MessageBus&);
    ~PlayerProfile() = default;
    PlayerProfile(const PlayerProfile&) = delete;
    PlayerProfile& operator = (const PlayerProfile&) = delete;

    void load();
    void save();

    void handleMessage(const xy::Message&);
    bool hasAchievement(AchievementID) const;

    void enable(bool b) { m_enabled = b; }

private:
    /*
    Format:
    Header
    Array of Header::COUNT Achievements
    */
    struct Header final
    {
        int IDENT = 0xCEFA1DBA;
        int VERSION = -1;
        int XP = 0;
        int COUNT = -1;
    };

    struct Achievement final
    {
        int ID = -1;
        bool unlocked = false;
        int value = 0;
    };

    xy::MessageBus& m_messageBus;
    int m_XP;
    std::array<Achievement, AchievementID::Count> m_achievements;

    bool m_enabled;

    void raiseAchievementMessage(AchievementID);
};

#endif //PLAYER_PROFILE_HPP_