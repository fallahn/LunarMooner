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
    const int VERSION = 2;
    const std::string FILE_NAME = "player.pfl";

    const int heroLevel = 50;
    const int superHeroLevel = 100;
    const int championLevel = 250;

    int bulletsFired = 0;
    int livesLost = 0;

    const int alienXP = 1;
    const int collectXP = alienXP;
    const int rescueXP = 2;
    const int levelXP = 5;
    const int extraLifeXP = levelXP;
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
    m_potentialXP   (0),
    m_enabled       (false),
    m_specialWeapon (lm::SpecialWeapon::None)
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
    std::size_t fileSize = static_cast<std::size_t>(file.tellg());
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
        case LMGameEvent::PlayerDied:
            //count lives lost for Survivor
            livesLost++;
            break;
        case LMGameEvent::AlienDied:
            m_potentialXP += alienXP;
            break;
        case LMGameEvent::ExtraLife:
            m_potentialXP += extraLifeXP;
            break;
        case LMGameEvent::PlayerGotAmmo:
        case LMGameEvent::PlayerGotShield:
            if (!m_achievements[AchievementID::Hoarder].unlocked)
            {
                m_achievements[AchievementID::Hoarder].value++;
                if (m_achievements[AchievementID::Hoarder].value == 50)
                {
                    m_achievements[AchievementID::Hoarder].unlocked = true;
                    raiseAchievementMessage(AchievementID::Hoarder);
                }
            }

            m_potentialXP += collectXP;
            break;
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

            m_potentialXP += rescueXP;
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
                if (m_achievements[AchievementID::AcrossTheBoard].value == ((1 << 0) | (1 << 1) | (1 << 2)))
                {
                    m_achievements[AchievementID::AcrossTheBoard].unlocked = true;
                    raiseAchievementMessage(AchievementID::AcrossTheBoard);
                }
            }
            //survivor
            if (livesLost == 0 && !m_achievements[AchievementID::Survivor].unlocked)
            {
                int difficulty = static_cast<int>(msgData.posY);
                m_achievements[AchievementID::Survivor].value |= (1 << difficulty);
                if (m_achievements[AchievementID::Survivor].value == ((1 << 0) | (1 << 1) | (1 << 2)))
                {
                    m_achievements[AchievementID::Survivor].unlocked = true;
                    raiseAchievementMessage(AchievementID::Survivor);
                }
            }
            livesLost = 0;
            //pacifist
            if (bulletsFired == 0 && !m_achievements[AchievementID::Pacifist].unlocked)
            {
                m_achievements[AchievementID::Pacifist].unlocked = true;
                raiseAchievementMessage(AchievementID::Pacifist);
            }
            bulletsFired = 0; //reset every round

            m_potentialXP += levelXP;
            m_potentialXP += static_cast<int>(msgData.posX); //XP for time remaining

            //award any XP (hey at least we completed a level!)
            awardXP();
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
                m_potentialXP += levelXP;
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
            case LMStateEvent::GameStart:
                //count bullets for pacifist
                bulletsFired = 0;
                //count lives for survivor
                livesLost = 0;
                //reset game XP
                m_potentialXP = 0;
                break;
            case LMStateEvent::GameOver:
                awardXP(); //because we won't get a level change event when we're all dead
                break;                
            }
        }
        break;
        case LMMessageId::RankEvent:
        {
            auto& msgData = msg.getData<LMRankEvent>();
            switch (msgData.value)
            {
            default: break;
            case 10:
                m_achievements[AchievementID::Rank10].unlocked = true;
                raiseAchievementMessage(AchievementID::Rank10);
                break;
            case 20:
                m_achievements[AchievementID::Rank20].unlocked = true;
                raiseAchievementMessage(AchievementID::Rank20);
                break;
            case 30:
                m_achievements[AchievementID::Rank30].unlocked = true;
                raiseAchievementMessage(AchievementID::Rank30);
                break;
            case 40:
                m_achievements[AchievementID::Rank40].unlocked = true;
                raiseAchievementMessage(AchievementID::Rank40);
                break;
            case 50:
                m_achievements[AchievementID::Rank50].unlocked = true;
                raiseAchievementMessage(AchievementID::Rank50);
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
    return std::max(1, i - 1);
}

//private
void PlayerProfile::raiseAchievementMessage(AchievementID id)
{
    auto msg = m_messageBus.post<LMAchievementEvent>(LMMessageId::AchievementEvent);
    msg->ID = id;

    //add XP
    auto currentRank = getRank();
    m_XP += 100;
    auto rank = getRank();
    if (rank > currentRank)
    {
        //raise an event
        auto xpMsg = m_messageBus.post<LMRankEvent>(LMMessageId::RankEvent);
        xpMsg->type = LMRankEvent::RankUp;
        xpMsg->value = rank;
    }
}

void PlayerProfile::awardXP()
{
    int currentRank = getRank();

    const float multiplier = static_cast<float>(currentRank + 10.f) / 10.f;
    const float award = static_cast<float>(m_potentialXP) * multiplier;

    auto xp = static_cast<int>(award);
    auto msg = m_messageBus.post<LMRankEvent>(LMMessageId::RankEvent);
    msg->type = LMRankEvent::XPAwarded;
    msg->value = xp;

    m_XP += xp;

    int rank = getRank();
    if (rank > currentRank)
    {
        //raise an event
        msg = m_messageBus.post<LMRankEvent>(LMMessageId::RankEvent);
        msg->value = rank;
        msg->type = LMRankEvent::RankUp;
    }
    m_potentialXP = 0;
    save();
}