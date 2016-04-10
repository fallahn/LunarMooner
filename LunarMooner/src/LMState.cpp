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

#include <BGScoreboardMask.hpp>
#include <BGPlanetDrawable.hpp>
#include <BGNormalBlendShader.hpp>
#include <BGStarfield.hpp>

#include <xygine/App.hpp>
#include <xygine/Assert.hpp>
#include <xygine/Reports.hpp>
#include <xygine/PostChromeAb.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/ParticleController.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/AudioSource.hpp>
#include <xygine/shaders/NormalMapped.hpp>

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

    m_loadingSprite.setTexture(m_textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

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

    buildBackground();

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

    //ewwww we've copied this from game controller
    const sf::FloatRect alienArea(280.f, 200.f, 1360.f, 480.f);

    const float moonWidth = 570.f;
    const float gameMusicVolume = 30.f;
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
    auto gameController =
        xy::Component::create<lm::GameController>(m_messageBus, m_scene, m_collisionWorld,
            getContext().appInstance.getAudioSettings(), m_soundResource, m_textureResource, m_fontResource);

    for (auto i = 0; i < playerCount; ++i)
    {
        gameController->addPlayer();
    }
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
    soundPlayer->preCache(LMSoundID::StrikeWarning, "assets/sound/speech/incoming.wav", 1);
    soundPlayer->preCache(LMSoundID::NukeWarning, "assets/sound/speech/meltdown.wav", 1);
    soundPlayer->preCache(LMSoundID::NukeWarning30, "assets/sound/speech/meltdown30.wav", 1);
    soundPlayer->preCache(LMSoundID::NukeWarning5, "assets/sound/speech/meltdown5.wav", 1);
    soundPlayer->preCache(LMSoundID::ShieldCollected, "assets/sound/fx/shield_collected.wav");
    soundPlayer->preCache(LMSoundID::AmmoCollected, "assets/sound/fx/ammo_collected.wav");
    soundPlayer->preCache(LMSoundID::ShipLanded, "assets/sound/fx/ship_landed.wav");
    soundPlayer->preCache(LMSoundID::ShipLaunched, "assets/sound/fx/ship_launched.wav");
    soundPlayer->preCache(LMSoundID::LifeBonus, "assets/sound/fx/extra_life.wav");
    soundPlayer->preCache(LMSoundID::ShieldLost, "assets/sound/fx/shield_lost.wav");
    soundPlayer->preCache(LMSoundID::RoundEnded, "assets/sound/fx/end_of_round.wav");

    const auto& audioSettings = getContext().appInstance.getAudioSettings();
    soundPlayer->setMasterVolume((audioSettings.muted) ? 0.f : audioSettings.volume);

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
        case LMGameEvent::PlayerLanded:
            player->playSound(LMSoundID::ShipLanded, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::PlayerSpawned:
            player->playSound(LMSoundID::ShipLaunched, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::PlayerGotAmmo:
            player->playSound(LMSoundID::AmmoCollected, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::PlayerGotShield:
            player->playSound(LMSoundID::ShieldCollected, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::PlayerLostShield:
            player->playSound(LMSoundID::ShieldLost, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::ExtraLife:
            player->playSound(LMSoundID::LifeBonus, 960.f, 540.f);
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
            //lots of magic numbers here...
        case LMStateEvent::CountDownStarted:
            player->playSound(LMSoundID::NukeWarning, 960.f, 540.f);
            break;
        case LMStateEvent::CountDownWarning:
            player->playSound(LMSoundID::NukeWarning30, 960.f, 540.f);
            break;
        case LMStateEvent::CountDownInProgress:
            player->playSound(LMSoundID::NukeWarning5, 960.f, 540.f);
            break;
        case LMStateEvent::RoundEnd:
            player->setChannelVolume(1, 0.f); //mutes voice over
            player->playSound(LMSoundID::RoundEnded, 960.f, 540.f);
            break;
        case LMStateEvent::RoundBegin:
        {
            const auto& audioSettings = getContext().appInstance.getAudioSettings();
            const float volume = (audioSettings.muted) ? 0.f : audioSettings.volume;

            player->setChannelVolume(1, volume);
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
        case xy::Message::UIEvent::MenuOpened:
            player->setMasterVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
        case xy::Message::UIEvent::RequestVolumeChange:
        case xy::Message::UIEvent::MenuClosed:
        {
            const auto& audioSettings = getContext().appInstance.getAudioSettings();
            player->setMasterVolume(audioSettings.volume);
        }
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);


    //game music
    auto gameMusic = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
    gameMusic->setSound("assets/sound/music/game.ogg", xy::AudioSource::Mode::Stream);
    gameMusic->setFadeOutTime(1.f);
    gameMusic->setFadeInTime(1.f);
    //gameMusic->setVolume(20.f);
    
    const auto& settings = getContext().appInstance.getAudioSettings();

    mh.id = LMMessageId::StateEvent;
    mh.action = [&settings](xy::Component* c, const xy::Message& msg)
    {
        xy::AudioSource* as = dynamic_cast<xy::AudioSource*>(c);
        auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMStateEvent::RoundBegin:
            as->setVolume(settings.muted ? 0.f : settings.volume * gameMusicVolume);
            as->play(true);
            break;
        case LMStateEvent::RoundEnd:
            as->stop();
            break;
        }
    };
    gameMusic->addMessageHandler(mh);

    mh.id = xy::Message::UIMessage;
    mh.action = [&settings](xy::Component* c, const xy::Message& msg)
    {
        xy::AudioSource* as = dynamic_cast<xy::AudioSource*>(c);
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::MenuOpened:
            as->pause();
            break;
        case xy::Message::UIEvent::MenuClosed:
            as->play(true);
            break;
        case xy::Message::UIEvent::RequestAudioMute:
            as->setVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
        case xy::Message::UIEvent::RequestVolumeChange:
            as->setVolume(settings.volume * gameMusicVolume);
            break;
        }
    };
    gameMusic->addMessageHandler(mh);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(soundPlayer);
    entity->addComponent(gameMusic);

    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void LunarMoonerState::initParticles()
{
    //particle spawner
    auto particleManager = xy::Component::create<xy::ParticleController>(m_messageBus);
    xy::Component::MessageHandler handler;
    handler.id = LMMessageId::GameEvent;
    handler.action = [](xy::Component* c, const xy::Message& msg)
    {
        auto component = dynamic_cast<xy::ParticleController*>(c);
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::AlienDied:
        case LMGameEvent::PlayerDied:
        case LMGameEvent::MeteorExploded:
            component->fire(LMParticleID::SmallExplosion, { msgData.posX, msgData.posY });
            break;
        }
    };
    particleManager->addMessageHandler(handler);

    auto entity = xy::Entity::create(m_messageBus);
    auto pc = entity->addComponent(particleManager);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    xy::ParticleSystem::Definition pd;
    pd.loadFromFile("assets/particles/small_explosion.xyp", m_textureResource);
    pc->addDefinition(LMParticleID::SmallExplosion, pd);
}

void LunarMoonerState::buildBackground()
{
    m_shaderResource.preload(LMShaderID::NormalMapColoured, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED);
    m_shaderResource.preload(LMShaderID::Prepass, xy::Shader::Default::vertex, lm::materialPrepassFrag);
    



    //background
    auto background = xy::Component::create<lm::Starfield>(m_messageBus, m_textureResource);
    background->setVelocity({ 0.f, 1.f });
    auto scoreMask = xy::Component::create<lm::ScoreMask>(m_messageBus, alienArea);

    m_scene.getLayer(xy::Scene::Layer::BackRear).addComponent(background);
    m_scene.getLayer(xy::Scene::Layer::BackRear).addComponent(scoreMask);

    auto moon = xy::Component::create<lm::PlanetDrawable>(m_messageBus, moonWidth);
    moon->setBaseNormal(m_textureResource.get("assets/images/background/sphere_normal.png"));
    moon->setDetailNormal(m_textureResource.get("assets/images/background/moon_normal.png"));
    moon->setDiffuseTexture(m_textureResource.get("assets/images/background/moon_diffuse.png"));
    moon->setMaskTexture(m_textureResource.get("assets/images/background/moon_mask.png"));
    moon->setPrepassShader(m_shaderResource.get(LMShaderID::Prepass));
    moon->setNormalShader(m_shaderResource.get(LMShaderID::NormalMapColoured));
    moon->setRotationVelocity({ 0.f, 0.009f });

    auto entity = xy::Entity::create(m_messageBus);
    entity->setPosition(960.f - (moonWidth), 540.f);
    entity->addComponent(moon);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void LunarMoonerState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}
