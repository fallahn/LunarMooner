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
#include <LMPostBlur.hpp>
#include <LMNukeDrawable.hpp>
#include <CommandIds.hpp>
#include <Game.hpp>

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
#include <xygine/components/PointLight.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/components/Camera.hpp>
#include <xygine/components/SoundPlayer.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/shaders/NormalMapped.hpp>

#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>

#include <SFML/Window/Event.hpp>

namespace
{
#include "KeyMappings.inl"

    lm::GameController* currentController = nullptr;
    bool resumeState = false;
    lm::PlayerState playerState;

    const std::size_t levelsBeforeBonus = 2;
}

LunarMoonerState::LunarMoonerState(xy::StateStack& stack, Context context, sf::Uint8 playerCount, PlayerProfile& profile)
    : xy::State         (stack, context),
    m_playerCount       (playerCount),
    m_scene             (context.appInstance.getMessageBus()),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_meshRenderer      ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene),
    m_inputFlags        (0),
    m_prevInputFlags    (0),
    m_collisionWorld    (m_scene),
    m_overlay           (m_messageBus, m_resources, m_scene),
    m_useController     (false),
    m_levelChangeCount  (0)
{
    XY_ASSERT(playerCount > 0, "Need at least one player");

    m_loadingSprite.setTexture(m_resources.textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();
    
    m_scene.setView(context.defaultView);
    m_scene.setSize({ -100.f, -100.f, 2120.f, 1280.f });
    //m_scene.drawDebug(true);

    auto pp = xy::PostProcess::create<lm::PostBlur>();
    m_scene.addPostProcess(pp);
    pp = xy::PostProcess::create<xy::PostChromeAb>(false);
    m_scene.addPostProcess(pp);

    m_overlay.setView(context.defaultView);

    //m_meshRenderer.setView(context.defaultView);

    sf::Uint8 level = 0;
    lm::SpecialWeapon weapon = lm::SpecialWeapon::None;
    if (playerCount == 1)
    {
        level = profile.getRank() / 10;
        weapon = profile.getSpecialWeapon();
    }

    //precache ALL shaders first
    m_resources.shaderResource.preload(Shader::NormalMapGame, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED_ILLUM);
    m_resources.shaderResource.get(Shader::NormalMapGame).setUniform("u_directionalLight.intensity", 0.4f);
    m_resources.shaderResource.get(Shader::NormalMapGame).setUniform("u_directionalLightDirection", sf::Glsl::Vec3(0.25f, 0.5f, -1.f));
    m_resources.shaderResource.get(Shader::NormalMapGame).setUniform("u_ambientColour", sf::Glsl::Vec3(0.06f, 0.06f, 0.02f));
    m_resources.shaderResource.get(Shader::NormalMapGame).setUniform("u_directionalLight.diffuseColour", sf::Glsl::Vec4(1.f, 0.8f, 0.7f, 1.f));
    m_resources.shaderResource.preload(Shader::NormalMapPlanet, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED);
    m_resources.shaderResource.get(Shader::NormalMapPlanet).setUniform("u_ambientColour", sf::Glsl::Vec3(0.03f, 0.03f, 0.01f));
    m_resources.shaderResource.preload(Shader::Prepass, xy::Shader::Default::vertex, lm::materialPrepassFrag);    

    m_resources.shaderResource.preload(Shader::MeshTextured, DEFERRED_TEXTURED_VERTEX, DEFERRED_TEXTURED_FRAGMENT);
    m_resources.shaderResource.preload(Shader::MeshVertexColoured, DEFERRED_VERTCOLOURED_VERTEX, DEFERRED_VERTCOLOURED_FRAGMENT);
    m_resources.shaderResource.preload(Shader::Shadow, SHADOW_VERTEX, SHADOW_FRAGMENT);

    initSounds();
    initParticles();
    initMeshes();
    initGameController(playerCount, level, weapon);

    buildBackground();

    profile.enable(playerCount == 1);

    xy::Stats::clear();

    m_useController = sf::Joystick::isConnected(0) && context.appInstance.getGameSettings().controllerEnabled;

    auto msg = m_messageBus.post<LMStateEvent>(LMMessageId::StateEvent);
    msg->type = LMStateEvent::GameStart;
    msg->stateID = (playerCount == 1) ? States::ID::SinglePlayer : States::ID::MultiPlayer;

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
        case keySpecial:
        case altKeySpecial:
            m_inputFlags &= ~LMInputFlags::Special;
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
        case keySpecial:
        case altKeySpecial:
            m_inputFlags |= LMInputFlags::Special;
            break;
        case sf::Keyboard::P:
        case sf::Keyboard::Pause:
        case sf::Keyboard::Escape:
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
        case buttonRB:
            m_inputFlags |= LMInputFlags::Shoot;
            break;
        case buttonB:
            m_inputFlags |= LMInputFlags::Thrust;
            break;
        case buttonX:
        case buttonLB:
            m_inputFlags |= LMInputFlags::Special;
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
        case buttonRB:
            m_inputFlags &= ~LMInputFlags::Shoot;
            break;
        case buttonB:
            m_inputFlags &= ~LMInputFlags::Thrust;
            break;
        case buttonX:
        case buttonLB:
            m_inputFlags &= ~LMInputFlags::Special;
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
    m_meshRenderer.handleMessage(msg);

    if (msg.id == LMMessageId::StateEvent)
    {
        const auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        case LMStateEvent::GameOver:
            requestStackPush(States::ID::GameOver);
            break;
        case LMStateEvent::SummaryFinished:
            if (m_playerCount == 1 && m_levelChangeCount == levelsBeforeBonus)
            {
                //in single player launch mini game
                //TODO alternate types
                requestStackPop();
                requestStackPush(States::ID::PlanetHopping);
                m_levelChangeCount = 0;

                //save the state for resumption
                playerState = currentController->getPlayerState(0);
                resumeState = true;
            }
            break;
        default:break;
        }
    }
    else if (msg.id == LMMessageId::GameEvent)
    {
        const auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::LevelChanged:
            m_levelChangeCount++;
            break;
        }
    }
    else if (msg.id == xy::Message::UIMessage)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            m_scene.setView(getContext().defaultView);
            m_overlay.setView(getContext().defaultView);
            //m_meshRenderer.setView(getContext().defaultView);
            break;
        case xy::Message::UIEvent::RequestControllerDisable:
            m_useController = false;
            break;
        case xy::Message::UIEvent::RequestControllerEnable:
            m_useController = true;
            break;
        }
    }
    m_overlay.handleMessage(msg);
}

bool LunarMoonerState::update(float dt)
{
    //get input
    if(m_useController) parseControllerInput(m_inputFlags);
    
    //if (m_inputFlags != m_prevInputFlags)
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
    
    //update lights
    auto& normalMapPlanetShader = m_resources.shaderResource.get(Shader::NormalMapPlanet);
    auto& normalMapGameShader = m_resources.shaderResource.get(Shader::NormalMapGame);
    auto lights = m_scene.getVisibleLights(m_scene.getVisibleArea());
    auto i = 0u;
    for (; i < lights.size() && i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        auto light = lights[i];
        //if (light)
        {
            const std::string idx = std::to_string(i);
            auto pos = light->getWorldPosition();

            normalMapPlanetShader.setUniform("u_pointLightPositions[" + idx + "]", pos);
            normalMapPlanetShader.setUniform("u_pointLights[" + idx + "].intensity", light->getIntensity());
            normalMapPlanetShader.setUniform("u_pointLights[" + idx + "].diffuseColour", sf::Glsl::Vec4(light->getDiffuseColour()));
            //normalMapShader.setUniform("u_pointLights[" + idx + "].specularColour", sf::Glsl::Vec4(light->getSpecularColour()));
            normalMapPlanetShader.setUniform("u_pointLights[" + idx + "].inverseRange", light->getInverseRange());

            normalMapGameShader.setUniform("u_pointLightPositions[" + idx + "]", pos);
            normalMapGameShader.setUniform("u_pointLights[" + idx + "].intensity", light->getIntensity());
            normalMapGameShader.setUniform("u_pointLights[" + idx + "].diffuseColour", sf::Glsl::Vec4(light->getDiffuseColour()));
            //normalMapShader.setUniform("u_pointLights[" + idx + "].specularColour", sf::Glsl::Vec4(light->getSpecularColour()));
            normalMapGameShader.setUniform("u_pointLights[" + idx + "].inverseRange", light->getInverseRange());
        }
    }

    //switch off inactive lights
    for (; i < xy::Shader::NormalMapped::MaxPointLights; ++i)
    {
        const auto idx = std::to_string(i);
        normalMapPlanetShader.setUniform("u_pointLights[" + idx + "].intensity", 0.f);
        normalMapGameShader.setUniform("u_pointLights[" + idx + "].intensity", 0.f);
    }
    REPORT("Light Count", std::to_string(lights.size()));

    m_scene.update(dt);
    m_collisionWorld.update();
    m_meshRenderer.update();
    m_overlay.update(dt);

    return true;
}

void LunarMoonerState::draw()
{
    auto& rw = getContext().renderWindow;

    rw.draw(m_scene);
    rw.draw(m_overlay);

    //rw.setView(getContext().defaultView);
    //rw.draw(m_meshRenderer);
}

//private
namespace
{
    //ewwww we've copied this from game controller
    const sf::FloatRect alienArea(280.f, 200.f, 1360.f, 480.f);

    const float moonWidth = 570.f;
    const float gameMusicVolume = 30.f;
}

void LunarMoonerState::initGameController(sf::Uint8 playerCount, sf::Uint8 level, lm::SpecialWeapon weapon)
{
    auto gameController =
        xy::Component::create<lm::GameController>(m_messageBus, m_scene, m_collisionWorld,
            getContext().appInstance.getAudioSettings(), m_resources, m_meshRenderer);

    gameController->setDifficulty(getContext().appInstance.getGameSettings().difficulty);

    for (auto i = 0; i < playerCount; ++i)
    {
        gameController->addPlayer(resumeState ? playerState.level - 1 : level, weapon);
    }

    //if we're resuming a game get the current profile
    if (resumeState)
    {
        auto state = gameController->getPlayerState(0);
        state.ammo = playerState.ammo;
        state.level = playerState.level;
        state.lives = playerState.lives;
        state.score = playerState.score;
        state.previousScore = state.score;
        
        gameController->setPlayerState(state, 0);
        resumeState = false;
    }

    gameController->start();

    auto entity = xy::Entity::create(m_messageBus);
    currentController = entity->addComponent(gameController);
    entity->addCommandCategories(LMCommandID::GameController);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void LunarMoonerState::initSounds()
{
    auto soundPlayer = xy::Component::create<xy::SoundPlayer>(m_messageBus, m_resources.soundResource);
    soundPlayer->preCache(LMSoundID::Laser, "assets/sound/fx/laser.wav");
    soundPlayer->preCache(LMSoundID::Explosion01, "assets/sound/fx/explode01.wav");
    soundPlayer->preCache(LMSoundID::Explosion02, "assets/sound/fx/explode02.wav");
    soundPlayer->preCache(LMSoundID::Explosion03, "assets/sound/fx/explode03.wav");
    soundPlayer->preCache(LMSoundID::Explosion04, "assets/sound/fx/explode04.wav");
    soundPlayer->preCache(LMSoundID::StrikeWarning, "assets/sound/speech/incoming.wav", 1);
    soundPlayer->preCache(LMSoundID::NukeWarning, "assets/sound/speech/meltdown.wav", 1);
    soundPlayer->preCache(LMSoundID::NukeWarning30, "assets/sound/speech/meltdown30.wav", 1);
    soundPlayer->preCache(LMSoundID::ShieldCollected, "assets/sound/fx/shield_collected.wav");
    soundPlayer->preCache(LMSoundID::AmmoCollected, "assets/sound/fx/ammo_collected.wav");
    soundPlayer->preCache(LMSoundID::ShipLanded, "assets/sound/fx/ship_landed.wav");
    soundPlayer->preCache(LMSoundID::ShipLaunched, "assets/sound/fx/ship_launched.wav");
    soundPlayer->preCache(LMSoundID::LifeBonus, "assets/sound/fx/extra_life.wav");
    soundPlayer->preCache(LMSoundID::ShieldLost, "assets/sound/fx/shield_lost.wav");
    soundPlayer->preCache(LMSoundID::RoundEnded, "assets/sound/fx/end_of_round.wav");
    soundPlayer->preCache(LMSoundID::PlayerDied, "assets/sound/speech/player_die.wav");
    soundPlayer->preCache(LMSoundID::MissionTerminated, "assets/sound/speech/game_over.wav");
    soundPlayer->preCache(LMSoundID::HumanRescued, "assets/sound/speech/alright.wav");
    soundPlayer->preCache(LMSoundID::ShieldHit, "assets/sound/fx/shield_hit.wav");
    soundPlayer->preCache(LMSoundID::EmpExplosion, "assets/sound/fx/emp_blast.wav");
    soundPlayer->preCache(LMSoundID::CollectibleDied, "assets/sound/fx/collectible_died.wav");
    soundPlayer->preCache(LMSoundID::AnnouncePlayerOne, "assets/sound/speech/player_one.wav");
    soundPlayer->preCache(LMSoundID::AnnouncePlayerTwo, "assets/sound/speech/player_two.wav");
    soundPlayer->preCache(LMSoundID::ChargeComplete, "assets/sound/fx/charge_complete.wav", 2);
    soundPlayer->preCache(LMSoundID::ChargeProgress, "assets/sound/fx/charge_progress.wav", 2);

    const auto& audioSettings = getContext().appInstance.getAudioSettings();
    soundPlayer->setMasterVolume((audioSettings.muted) ? 0.f : audioSettings.volume);

    soundPlayer->setChannelVolume(2, 0.25f);

    xy::Component::MessageHandler mh;
    mh.id = LMMessageId::GameEvent;
    mh.action = [](xy::Component* c, const xy::Message& msg)
    {
        xy::SoundPlayer* player = dynamic_cast<xy::SoundPlayer*>(c);
        auto& msgData = msg.getData<LMGameEvent>();

        const auto screenCentre = xy::DefaultSceneSize / 2.f;

        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::LaserFired:
            player->playSound(LMSoundID::Laser, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::MeteorExploded:
            player->playSound(LMSoundID::ShieldHit, msgData.posX, msgData.posY);
        case LMGameEvent::PlayerDied:
            //player->playSound(LMSoundID::PlayerDied, screenCentre);
            //break; //makes explodey below
        case LMGameEvent::AlienDied:
            player->playSound(xy::Util::Random::value(LMSoundID::Explosion01, LMSoundID::Explosion04), msgData.posX, msgData.posY);
            break;
        case LMGameEvent::EarlyWarning:
            player->playSound(LMSoundID::StrikeWarning, screenCentre.x, screenCentre.y);
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
            player->playSound(LMSoundID::LifeBonus, screenCentre.x, screenCentre.y);
            break;
        case LMGameEvent::HumanRescued:
            player->playSound(LMSoundID::HumanRescued, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::EmpFired:
            player->playSound(LMSoundID::EmpExplosion, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::CollectibleDied:
            player->playSound(LMSoundID::CollectibleDied, msgData.posX, msgData.posY);
            break;
        case LMGameEvent::WeaponCharged:
            switch (msgData.value)
            {
            default: break;
            case 1:
            case 2:
            case 3:
            case 4:
                player->playSound(LMSoundID::ChargeProgress, screenCentre.x, screenCentre.y);
                break;
            case 5:
                player->playSound(LMSoundID::ChargeComplete, screenCentre.x, screenCentre.y);
                break;
            }
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    mh.id = LMMessageId::StateEvent;
    mh.action = [this](xy::Component* c, const xy::Message& msg)
    {
        xy::SoundPlayer* player = dynamic_cast<xy::SoundPlayer*>(c);
        auto& msgData = msg.getData<LMStateEvent>();

        const auto screenCentre = xy::DefaultSceneSize / 2.f;

        switch (msgData.type)
        {
        default: break;
        case LMStateEvent::CountDownStarted:
            player->playSound(LMSoundID::NukeWarning, screenCentre.x, screenCentre.y);
            break;
        case LMStateEvent::CountDownWarning:
            player->playSound(LMSoundID::NukeWarning30, screenCentre.x, screenCentre.y);
            break;
        case LMStateEvent::RoundEnd:
            player->setChannelVolume(1, 0.f); //mutes voice over
            player->playSound(LMSoundID::RoundEnded, screenCentre.x, screenCentre.y);
            break;
        case LMStateEvent::RoundBegin:
        {
            const auto& audioSettings = getContext().appInstance.getAudioSettings();
            const float volume = (audioSettings.muted) ? 0.f : audioSettings.volume;

            player->setChannelVolume(1, volume);
        }
            break;
        case LMStateEvent::GameOver:
            player->playSound(LMSoundID::MissionTerminated, screenCentre.x, screenCentre.y);
            break;
        case LMStateEvent::SwitchedPlayer:
            (msgData.value == 1)
                ? player->playSound(LMSoundID::AnnouncePlayerTwo, screenCentre.x, screenCentre.y)
                : player->playSound(LMSoundID::AnnouncePlayerOne, screenCentre.x, screenCentre.y);
            break;
        }
    };
    soundPlayer->addMessageHandler(mh);

    //need a handler to react to UI events
    mh.id = xy::Message::UIMessage;
    mh.action = [this](xy::Component* c, const xy::Message& msg)
    {
        xy::SoundPlayer* player = dynamic_cast<xy::SoundPlayer*>(c);
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
    auto gameMusic = xy::Component::create<xy::AudioSource>(m_messageBus, m_resources.soundResource);
    gameMusic->setSound("assets/sound/music/game.ogg", xy::AudioSource::Mode::Stream);
    gameMusic->setFadeOutTime(1.f);
    gameMusic->setFadeInTime(1.f);
    
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
            as->setVolume(0.f);
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
        case LMGameEvent::PlayerDied:
            component->fire(LMParticleID::LargeExplosion, { msgData.posX, msgData.posY });
        case LMGameEvent::AlienDied:
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
    pd.loadFromFile("assets/particles/small_explosion.xyp", m_resources.textureResource);
    pc->addDefinition(LMParticleID::SmallExplosion, pd);

    pd.loadFromFile("assets/particles/large_explosion.xyp", m_resources.textureResource);
    pc->addDefinition(LMParticleID::LargeExplosion, pd);
}

void LunarMoonerState::initMeshes()
{
    m_scene.setAmbientColour({ 76, 70, 72 });
    m_scene.getSkyLight().setIntensity(0.4f);
    m_scene.getSkyLight().setDiffuseColour({ 255, 255, 100 });
    m_scene.getSkyLight().setSpecularColour({ 120, 255, 58 });
    m_scene.getSkyLight().setDirection({ 0.25f, 0.5f, -1.f });

    xy::IQMBuilder ib("assets/models/player_ship.iqm");
    m_meshRenderer.loadModel(Mesh::Player, ib);

    xy::IQMBuilder ib2("assets/models/mothership.iqm");
    m_meshRenderer.loadModel(Mesh::MotherShip, ib2);

    xy::IQMBuilder ib3("assets/models/corpse.iqm");
    m_meshRenderer.loadModel(Mesh::DeadDoofer, ib3);

    auto& playerMat = m_resources.materialResource.add(Material::Player, m_resources.shaderResource.get(Shader::MeshTextured));
    playerMat.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/ship_diffuse.png") });
    playerMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    playerMat.addRenderPass(xy::RenderPass::ShadowMap, m_resources.shaderResource.get(Shader::Shadow));
    //playerMat.getRenderPass(xy::RenderPass::ShadowMap)->setCullFace(xy::CullFace::Front); //the model has no back faces!

    auto& shipMat = m_resources.materialResource.add(Material::MotherShip, m_resources.shaderResource.get(Shader::MeshTextured));
    shipMat.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/mothership_diffuse.png") });
    shipMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());

    auto& dooferMat = m_resources.materialResource.add(Material::DeadDoofer, m_resources.shaderResource.get(Shader::MeshVertexColoured));
    dooferMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());

    //add drawable to scene
    auto md = m_meshRenderer.createDrawable(m_messageBus);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(md);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);
}

void LunarMoonerState::buildBackground()
{    
    //nuke effect
    auto ne = xy::Component::create<lm::NukeEffect>(m_messageBus, sf::Vector2f(alienArea.width, xy::DefaultSceneSize.y));
    auto camera = xy::Component::create<xy::Camera>(m_messageBus, m_scene.getView());
    
    const auto& audioSettings = getContext().appInstance.getAudioSettings();
    auto nukeAudio = xy::Component::create<xy::AudioSource>(m_messageBus, m_resources.soundResource);
    nukeAudio->setSound("assets/sound/fx/nuke.wav");
    nukeAudio->setFadeInTime(5.f);
    nukeAudio->setFadeOutTime(1.f);
    nukeAudio->setVolume(audioSettings.muted ? 0.f : audioSettings.volume * Game::MaxVolume);
    xy::Component::MessageHandler mh;
    mh.id = LMMessageId::GameEvent;
    mh.action = [](xy::Component* c, const xy::Message& msg)
    {
        xy::AudioSource* audio = dynamic_cast<xy::AudioSource*>(c);
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::PlayerDied:
            audio->stop();
            break;
        case LMGameEvent::PlayerSpawned:
            if (msgData.value > 0)
            {
                audio->play(true);
            }
            break;
        }
    };
    nukeAudio->addMessageHandler(mh);

    mh.id = LMMessageId::StateEvent;
    mh.action = [](xy::Component* c, const xy::Message& msg)
    {
        xy::AudioSource* audio = dynamic_cast<xy::AudioSource*>(c);
        auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMStateEvent::CountDownStarted:
            audio->play(true);
            break;
        case LMStateEvent::RoundEnd:
            audio->stop();
            break;
        }
    };
    nukeAudio->addMessageHandler(mh);
    
    mh.id = xy::Message::UIMessage;
    mh.action = [&audioSettings](xy::Component* c, const xy::Message& msg)
    {
        xy::AudioSource* as = dynamic_cast<xy::AudioSource*>(c);
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::MenuOpened:
            if (as->getStatus() == xy::AudioSource::Status::Playing)
            {
                as->pause();
            }
            break;
        case xy::Message::UIEvent::MenuClosed:
            if (as->getStatus() == xy::AudioSource::Status::Paused)
            {
                as->play(true);
            }
            break;
        case xy::Message::UIEvent::RequestAudioMute:
            as->setVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
        case xy::Message::UIEvent::RequestVolumeChange:
            as->setVolume(audioSettings.volume * Game::MaxVolume);
            break;
        }
    };
    nukeAudio->addMessageHandler(mh);

    auto finalWarning = xy::Component::create<xy::AudioSource>(m_messageBus, m_resources.soundResource);
    finalWarning->setSound("assets/sound/speech/meltdown5.wav");
    finalWarning->setFadeOutTime(1.f);
    finalWarning->setVolume(audioSettings.muted ? 0.f : audioSettings.volume * Game::MaxVolume);
    auto fw = finalWarning.get();
    mh.id = LMMessageId::GameEvent;
    mh.action = [fw](xy::Component*, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMGameEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMGameEvent::PlayerDied:
            if (fw->getStatus() == xy::AudioSource::Status::Playing)
            {
                fw->pause();
            }
            break;
        case LMGameEvent::PlayerSpawned:
            if (msgData.value > 0)
            {
                fw->play();
            }
            break;
        }
    };
    fw->addMessageHandler(mh);

    mh.id = LMMessageId::StateEvent;
    mh.action = [fw](xy::Component*, const xy::Message& msg)
    {
        auto& msgData = msg.getData<LMStateEvent>();
        switch (msgData.type)
        {
        default: break;
        case LMStateEvent::CountDownStarted:
            fw->stop(); //rewinds the sound if needs be
            break;
        case LMStateEvent::CountDownInProgress:
            fw->play();
            break;
        case LMStateEvent::RoundEnd:
            if (fw->getStatus() == xy::AudioSource::Status::Playing)
            {
                fw->stop();
            }
            break;
        }
    };
    fw->addMessageHandler(mh);

    mh.id = xy::Message::UIMessage;
    mh.action = [&audioSettings](xy::Component* c, const xy::Message& msg)
    {
        xy::AudioSource* as = dynamic_cast<xy::AudioSource*>(c);
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::MenuOpened:
            if (as->getStatus() == xy::AudioSource::Status::Playing)
            {
                as->pause();
            }
            break;
        case xy::Message::UIEvent::MenuClosed:
            if (as->getStatus() == xy::AudioSource::Status::Paused)
            {
                as->play();
            }
            break;
        case xy::Message::UIEvent::RequestAudioMute:
            as->setVolume(0.f);
            break;
        case xy::Message::UIEvent::RequestAudioUnmute:
        case xy::Message::UIEvent::RequestVolumeChange:
            as->setVolume(audioSettings.volume * Game::MaxVolume);
            break;
        }
    };
    fw->addMessageHandler(mh);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(ne);
    entity->addComponent(nukeAudio);
    entity->addComponent(finalWarning);
    entity->setPosition(xy::DefaultSceneSize / 2.f);
    m_scene.setActiveCamera(entity->addComponent(camera));
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
  
    //background
    auto background = xy::Component::create<lm::Starfield>(m_messageBus, m_resources.textureResource);
    background->setVelocity({ 0.f, 1.f });
    auto scoreMask = xy::Component::create<lm::ScoreMask>(m_messageBus, alienArea, m_resources.textureResource.get("assets/images/game/console/panel.png"));

    m_scene.getLayer(xy::Scene::Layer::BackRear).addComponent(background);
    m_scene.getLayer(xy::Scene::Layer::UI).addComponent(scoreMask);

    auto moon = xy::Component::create<lm::PlanetDrawable>(m_messageBus, moonWidth);
    moon->setBaseNormal(m_resources.textureResource.get("assets/images/background/sphere_normal.png"));
    moon->setDetailNormal(m_resources.textureResource.get("assets/images/background/moon_normal_02.png"));
    moon->setDiffuseTexture(m_resources.textureResource.get("assets/images/background/moon_diffuse_02.png"));
    moon->setMaskTexture(m_resources.textureResource.get("assets/images/background/moon_mask.png"));
    moon->setPrepassShader(m_resources.shaderResource.get(Shader::Prepass));
    moon->setNormalShader(m_resources.shaderResource.get(Shader::NormalMapPlanet));
    moon->setRotationVelocity({ 0.f, 0.009f });

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition((xy::DefaultSceneSize.x / 2.f) - (moonWidth), xy::DefaultSceneSize.y / 2.f);
    entity->addComponent(moon);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    //background lighting
    auto lc = xy::Component::create<xy::PointLight>(m_messageBus, 1200.f, 250.f);
    lc->setDepth(400.f);
    lc->setDiffuseColour({ 255u, 245u, 235u });
    lc->setIntensity(1.2f);

    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(alienArea.left + alienArea.width, xy::DefaultSceneSize.y / 2.f);
    entity->addComponent(lc);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void LunarMoonerState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}
