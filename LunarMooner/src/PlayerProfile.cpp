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

#include <PlayerProfile.hpp>
#include <CommandIds.hpp>

#include <xygine/MessageBus.hpp>
#include <xygine/Log.hpp>

#include <fstream>
#include <cstring>

namespace
{
    const int IDENT = 0xCEFA1DBA;
    const int VERSION = 1;
    const std::string FILE_NAME = "player.pfl";

    const int heroLevel = 50;
    const int superHeroLevel = 100;
    const int championLevel = 250;

    int bulletsFired = 0;


}

/*
XP Calc:
Event value is multiplied by ((rank + 10) / 10);
XP is summed but not added to the player unless the round ends
to prevent rage quitting
*/

PlayerProfile::PlayerProfile(xy::MessageBus& mb)
    : m_messageBus  (mb),
    m_XP            (0),
    m_enabled       (false)
{

}

//public
void PlayerProfile::load()
{
    std::fstream file(FILE_NAME, std::ios::in | std::ios::binary);
    if (!file.good() || !file.is_open() || file.fail())
    {
        LOG("Unable to open player profile", xy::Logger::Type::Warning);
        file.close();
        return;
    }

    file.seekg(0, std::ios::end);
    int fileSize = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (fileSize < sizeof(Header))
    {
        LOG("Incorrect file size detected", xy::Logger::Type::Error);
        file.close();
        return;
    }

    std::vector<char> fileData(fileSize);
    file.read(fileData.data(), fileSize);
    file.close();

    Header header;
    std::memcpy(&header, fileData.data(), sizeof(Header));

    if (header.IDENT != IDENT)
    {
        LOG("Incorrect profile IDENT found", xy::Logger::Type::Error);
        return;
    }

    if (header.VERSION != VERSION || header.COUNT != AchievementID::Count)
    {
        LOG("Profile version differs, results may not be as expected", xy::Logger::Type::Warning);
    }
    
    if (header.COUNT <= AchievementID::Count)
    {
        std::memcpy(m_achievements.data(), fileData.data() + sizeof(Header), sizeof(Achievement) * header.COUNT);
    }
    else
    {
        std::memcpy(m_achievements.data(), fileData.data() + sizeof(Header), sizeof(Achievement) * AchievementID::Count);
    }

    m_XP = header.XP;
}

void PlayerProfile::save()
{
    std::fstream file(FILE_NAME, std::ios::binary | std::ios::out);
    if (!file.good() || !file.is_open() || file.fail())
    {
        LOG("Failed opening profile data for writing", xy::Logger::Type::Error);
        file.close();
        return;
    }

    Header header;
    header.VERSION = VERSION;
    header.COUNT = AchievementID::Count;
    header.XP = m_XP;

    file.write((char*)&header, sizeof(header));
    file.write((char*)m_achievements.data(), sizeof(Achievement) * m_achievements.size());
    file.close();
}

void PlayerProfile::handleMessage(const xy::Message& msg)
{
    if (!m_enabled) return;

    switch (msg.id)
    {
    default: break;
    case LMMessageId::GameEvent:
    {
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::HumanRescued:
            //Hero
            if (!m_achievements[AchievementID::Hero].unlocked)
            {
                m_achievements[AchievementID::Hero].value++;
                if (m_achievements[AchievementID::Hero].value == heroLevel)
                {
                    m_achievements[AchievementID::Hero].unlocked = true;
                    m_achievements[AchievementID::SuperHero].value = heroLevel;
                    raiseAchievementMessage(AchievementID::Hero);
                }
            }
            //SuperHero
            else if(!m_achievements[AchievementID::SuperHero].unlocked)
            {
                m_achievements[AchievementID::SuperHero].value++;
                if (m_achievements[AchievementID::SuperHero].value == superHeroLevel)
                {
                    m_achievements[AchievementID::SuperHero].unlocked = true;
                    m_achievements[AchievementID::Champion].value = superHeroLevel;
                    raiseAchievementMessage(AchievementID::SuperHero);
                }
            }
            //Champion
            else if (!m_achievements[AchievementID::Champion].unlocked)
            {
                m_achievements[AchievementID::Champion].value++;
                if (m_achievements[AchievementID::Champion].value == championLevel)
                {
                    m_achievements[AchievementID::Champion].unlocked = true;
                    raiseAchievementMessage(AchievementID::Champion);
                }
            }
            break;
        case LMGameEvent::LevelChanged:
            //Long Haul
            if (msgData.value == 10 && !m_achievements[AchievementID::LongHaul].unlocked)
            {
                m_achievements[AchievementID::LongHaul].unlocked = true;
                raiseAchievementMessage(AchievementID::LongHaul);
            }
            //Skin of your teeth
            if (msgData.posX < 2 && !m_achievements[AchievementID::SkinOfYourTeeth].unlocked)
            {
                m_achievements[AchievementID::SkinOfYourTeeth].unlocked = true;
                raiseAchievementMessage(AchievementID::SkinOfYourTeeth);
            }
            //more haste less speed
            if (msgData.posX >= 31 && !m_achievements[AchievementID::MoreHasteLessSpeed].unlocked)
            {
                m_achievements[AchievementID::MoreHasteLessSpeed].unlocked = true;
                raiseAchievementMessage(AchievementID::MoreHasteLessSpeed);
            }
            //across the board
            if (!m_achievements[AchievementID::AcrossTheBoard].unlocked)
            {
                int difficulty = static_cast<int>(msgData.posY);
                m_achievements[AchievementID::AcrossTheBoard].value |= (1 << difficulty);
                if ((m_achievements[AchievementID::AcrossTheBoard].value & ((1 << 0) | (1 << 1) | (1 << 2))))
                {
                    m_achievements[AchievementID::AcrossTheBoard].unlocked = true;
                    raiseAchievementMessage(AchievementID::AcrossTheBoard);

                    std::cout << "ATB val: " << m_achievements[AchievementID::AcrossTheBoard].value;
                }
            }
            //pacifist
            if (bulletsFired == 0 && !m_achievements[AchievementID::Pacifist].unlocked)
            {
                m_achievements[AchievementID::Pacifist].unlocked = true;
                raiseAchievementMessage(AchievementID::Pacifist);
            }
            bulletsFired = 0; //reset every round
            break;
        case LMGameEvent::MeteorExploded:
            if (msgData.value > 0) //shot by player
            {
                //dead eye
                if (!m_achievements[AchievementID::DeadEye].unlocked)
                {
                    m_achievements[AchievementID::DeadEye].unlocked = true;
                    raiseAchievementMessage(AchievementID::DeadEye);
                }
                //stamper
                if (!m_achievements[AchievementID::Stamper].unlocked)
                {
                    m_achievements[AchievementID::Stamper].value++;
                    if (m_achievements[AchievementID::Stamper].value == 10)
                    {
                        m_achievements[AchievementID::Stamper].unlocked = true;
                        raiseAchievementMessage(AchievementID::Stamper);
                    }
                }
            }
            break;
        case LMGameEvent::PlayerLostShield:
            //back with a bump
            if (msgData.value == LMGameEvent::HitGround && !m_achievements[AchievementID::BackWithABump].unlocked)
            {
                m_achievements[AchievementID::BackWithABump].unlocked = true;
                raiseAchievementMessage(AchievementID::BackWithABump);
            }
            break;
        case LMGameEvent::LaserFired:
            //count bullets for pacifist achievement
            bulletsFired++;
            break;
        }
    }
        break;
        case LMMessageId::StateEvent:
        {
            auto& msgData = msg.getData<LMStateEvent>();
            switch (msgData.type)
            {
            default:break;
            case LMStateEvent::RoundBegin:
                //count bullets for pacifist
                bulletsFired = 0;
                break;
            }
        }
        break;
    }
}

bool PlayerProfile::hasAchievement(AchievementID id) const
{
    return m_achievements[id].unlocked;
}

int PlayerProfile::getRank() const
{
    int i = 0;
    while (i < rankXP.size() && rankXP[i++] < m_XP) {}
    return i;
}

//private
void PlayerProfile::raiseAchievementMessage(AchievementID id)
{
    auto msg = m_messageBus.post<LMAchievementEvent>(LMMessageId::AchievementEvent);
    msg->ID = id;

    //add XP
    m_XP += 100;
}