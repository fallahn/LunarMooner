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
#include <LMPostBleach.hpp>
#include <LMSoundPlayer.hpp>
#include <CommandIds.hpp>

#include <xygine/App.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Reports.hpp>
#include <xygine/PostChromeAb.hpp>
#include <xygine/util/Random.hpp>

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
    m_prevInputFlags    (0),
    m_useController     (false)
{
    XY_ASSERT(playerCount > 0, "Need at least one player");
    launchLoadingScreen();
    
    m_scene.setView(context.defaultView);
    //m_scene.drawDebug(true);

    auto pp = xy::PostProcess::create<lm::PostBleach>();
    m_scene.addPostProcess(pp);
    pp = xy::PostProcess::create<xy::PostChromeAb>();
    m_scene.addPostProcess(pp);

    initGameController(playerCount);
    initSounds();
    initParticles();

    xy::Stats::clear();
    m_reportText.setFont(m_fontResource.get("game_state_81"));

    m_useController = sf::Joystick::isConnected(0) && context.appInstance.getGameSettings().controllerEnabled;

    quitLoadingScreen();
}

//public
bool LunarMoonerState::handleEvent(const sf::Event& evt)
{
    switch(evt.type)
    {
    case sf::Event::JoystickConnected:
        m_useController = (evt.joystickConnect.joystickId == 0 &&
            getContext().appInstance.getGameSettings().controllerEnabled);
        break;
    case sf::Event::JoystickDisconnected:
        if(evt.joystickConnect.joystickId == 0) m_useController = false;
        break;
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
        case sf::Keyboard::P:
            requestStackPush(States::ID::Pause);
            break;
        default:break;
        }
        break;


    //controller input (default for x360 layout)
    case sf::Event::JoystickButtonPressed:
        if (!m_useController || evt.joystickButton.joystickId != 0) break;
        switch (evt.joystickButton.button)
        {
        case buttonA:
            m_inputFlags |= LMInputFlags::Shoot;
            break;
        case buttonB:
            m_inputFlags |= LMInputFlags::Thrust;
            break;
        case buttonStart:
            //m_inputFlags |= LMInputFlags::Start;
            //requestStackPush(States::ID::Pause);
            break;
        default: break;
        }
        break;
    case sf::Event::JoystickButtonReleased:
        if (!m_useController || evt.joystickButton.joystickId != 0) break;
        switch (evt.joystickButton.button)
        {
        default: break;
        case buttonA:
            m_inputFlags &= ~LMInputFlags::Shoot;
            break;
        case buttonB:
            m_inputFlags &= ~LMInputFlags::Thrust;
            break;
        case buttonStart:
            requestStackPush(States::ID::Pause);
            //    m_inputFlags &= ~LMInputFlags::Start;
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

    if (msg.id == LMMessageId::StateEvent)
    {
        auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        case LMStateEvent::GameOver:
            requestStackPush(States::ID::GameOver);
            break;
        default:break;
        }
    }
    else if (msg.id == xy::Message::UIMessage)
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            m_scene.setView(getContext().defaultView);
            break;
        case xy::Message::UIEvent::RequestControllerDisable:
            m_useController = false;
            break;
        case xy::Message::UIEvent::RequestControllerEnable:
            m_useController = true;
            break;
        }
    }
}

bool LunarMoonerState::update(float dt)
{
    if(m_useController) parseControllerInput();
    
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

void LunarMoonerState::initGameController(sf::Uint8 playerCount)
{
    auto gameController = xy::Component::create<lm::GameController>(m_messageBus, m_scene, m_collisionWorld, m_soundResource);
    for (auto i = 0; i < playerCount; ++i)  gameController->addPlayer();
    gameController->start();

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(gameController);
    entity->addCommandCategories(LMCommandID::GameController);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void LunarMoonerState::initSounds()
{
    auto soundPlayer = xy::Component::create<lm::SoundPlayer>(m_messageBus, m_soundResource);
    soundPlayer->preCache(LMSoundID::Laser, "assets/sound/fx/laser.wav");
    soundPlayer->preCache(LMSoundID::Explosion01, "assets/sound/fx/explode01.wav");
    soundPlayer->preCache(LMSoundID::Explosion02, "assets/sound/fx/explode02.wav");
    soundPlayer->preCache(LMSoundID::Explosion03, "assets/sound/fx/explode03.wav");
    soundPlayer->preCache(LMSoundID::Explosion04, "assets/sound/fx/explode04.wav");
    soundPlayer->preCache(LMSoundID::StrikeWarning, "assets/sound/speech/incoming.wav");
    soundPlayer->preCache(LMSoundID::NukeWarning, "assets/sound/speech/meltdown.wav");
    soundPlayer->preCache(LMSoundID::NukeWarning30, "assets/sound/speech/meltdown30.wav");

    const auto& audioSettings = getContext().appInstance.getAudioSettings();
    soundPlayer->setVolume((audioSettings.muted) ? 0.f : audioSettings.volume);

    xy::Component::MessageHandler mh;
    mh.id = LMMessageId::GameEvent;
    mh.action = [](xy::Component* c, const xy::Message& msg)
    {
        lm::SoundPlayer* player = dynamic_cast<lm::SoundPlayer*>(c);
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::LaserFired:
            player->playSound(LMSoundID::Laser, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::AlienDied:
        case LMGameEvent::PlayerDied:
        case LMGameEvent::MeteorExploded:
            player->playSound(xy::Util::Random::value(LMSoundID::Explosion01, LMSoundID::Explosion04), msgData.posX, msgData.posY);
            break;
        case LMGameEvent::EarlyWarning:
            player->playSound(LMSoundID::StrikeWarning, 960.f, 540.f);
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    mh.id = LMMessageId::StateEvent;
    mh.action = [this](xy::Component* c, const xy::Message& msg)
    {
        lm::SoundPlayer* player = dynamic_cast<lm::SoundPlayer*>(c);
        auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMStateEvent::CountDownStarted:
            player->playSound(LMSoundID::NukeWarning, 960.f, 540.f);
            break;
        case LMStateEvent::CountDownWarning:
            player->playSound(LMSoundID::NukeWarning30, 960.f, 540.f);
            break;
        case LMStateEvent::RoundEnd:
            player->setVolume(0.f);
            break;
        case LMStateEvent::RoundBegin:
        {
            const auto& audioSettings = getContext().appInstance.getAudioSettings();
            const float volume = (audioSettings.muted) ? 0.f : audioSettings.volume;

            player->setVolume(volume);
        }
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    //need a handler to react to UI events
    mh.id = xy::Message::UIMessage;
    mh.action = [this](xy::Component* c, const xy::Message& msg)
    {
        lm::SoundPlayer* player = dynamic_cast<lm::SoundPlayer*>(c);
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::RequestAudioMute:
            player->setVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
        case xy::Message::UIEvent::RequestVolumeChange:
        {
            const auto& audioSettings = getContext().appInstance.getAudioSettings();
            player->setVolume(audioSettings.volume);
        }
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(soundPlayer);

    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void LunarMoonerState::initParticles()
{

}