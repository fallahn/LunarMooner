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

#ifndef COMMAND_IDS_HPP_
#define COMMAND_IDS_HPP_

#include <StateIds.hpp>

#include <xygine/MessageBus.hpp>

enum LMCommandID
{
    Mothership = 0x1,
    GameController = 0x2,
    Human = 0x4,
    Player = 0x8,
    Alien = 0x10,
    UI = 0x20,
    Item = 0x40,
    DropShip = 0x80,
    TerrainObject = 0x100,
    Background = 0x200,
    Prop = 0x400,
    InUse = 0x800
};

enum LMInputFlags
{
    SteerLeft = 0x1,
    SteerRight = 0x2,
    ThrustUp = 0x4,
    ThrustDown = 0x8,
    Shoot = 0x10,
    Start = 0x20,
    Special = 0x40
};

enum class LMDirection
{
    Up,
    Down,
    Left,
    Right
};

enum LMMessageId
{
    GameEvent = xy::Message::Count,
    StateEvent,
    MenuEvent,
    AchievementEvent,
    RankEvent,
    TutorialEvent
};

struct LMGameEvent
{
    enum ShieldReason
    {
        HitGround = 1,
        HitAlien,
        HitRoid
    };

    enum
    {
        PlayerDied,
        PlayerLanded,
        PlayerSpawned,
        PlayerGotAmmo,
        PlayerGotShield,
        PlayerLostShield,
        HumanRescued,
        HumanPickedUp,
        AlienDied,
        ExtraLife,
        ItemCollected,
        MeteorExploded,
        LaserFired,
        EarlyWarning,
        LevelChanged,
        CollectibleDied,
        EmpFired,
        WeaponCharged,
        EnteredOrbit,
        LeftOrbit,
        TimerExpired
    }type;
    float posX = 0.f; //<holds time remaining on level change
    float posY = 0.f; //<holds difficulty on level change
    sf::Int32 value = 0;
};

struct LMStateEvent
{
    enum
    {
        GameStart,
        GameOver,
        SummaryFinished,
        RoundBegin,
        RoundEnd,
        CountDownStarted,
        CountDownInProgress,
        CountDownWarning,
        SwitchedPlayer,
        Tutorial
    }type;
    sf::Int32 value = -1;
    xy::StateID stateID = States::ID::SinglePlayer;
};

struct LMAchievementEvent
{
    sf::Int32 ID = -1;
};

struct LMRankEvent
{
    enum
    {
        RankUp,
        XPAwarded
    }type;
    sf::Int32 value = 0;
};

struct LMMenuEvent
{
    sf::Uint32 playerId = 0u;
    sf::Uint32 score = 0u;
};

struct LMTutorialEvent
{
    enum
    {
        Opened = -2,
        Closed = -1,
        FirstLaunch = 0,
        LandingPad,
        RescueSurvivors,
        FireLaser,
        CollectShield,
        CollectAmmo,
        Docking,
        UnlockXP,
        Count
    }action;
    float posX = 0.f;
    float posY = 0.f;
};

enum LMParticleID
{
    Thruster = 0,
    RcsLeft,
    RcsRight,
    RcsDown,
    RoidTrail,
    JetPack,
    SmallExplosion,
    LargeExplosion
};

enum LMSoundID
{
    Engine,
    RCS,
    Laser,
    Explosion01,
    Explosion02,
    Explosion03,
    Explosion04,
    Nuke,
    StrikeWarning,
    NukeWarning,
    NukeWarning30,
    NukeWarning5,
    ShipLanded,
    ShipLaunched,
    ShieldCollected,
    AmmoCollected,
    ShieldLost,
    LifeBonus,
    RoundEnded,
    RoundCountLoop,
    RoundCountEnd,
    MissionTerminated,
    PlayerDied,
    HumanRescued,
    ShieldHit,
    EmpExplosion,
    CollectibleDied,
    AnnouncePlayerOne,
    AnnouncePlayerTwo,
    ChargeProgress,
    ChargeComplete,
    TutTip
};

#endif //COMMAND_IDS_HPP_
