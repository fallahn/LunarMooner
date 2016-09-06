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

#include <PlanetHoppingState.hpp>
#include <PHGameController.hpp>
#include <PHPlayerController.hpp>
#include <PHOrbitComponent.hpp>
#include <BGStarfield.hpp>
#include <CommandIds.hpp>
#include <LMShaderIds.hpp>
#include <LMPostBlur.hpp>

#include <xygine/App.hpp>
#include <xygine/components/SoundPlayer.hpp>
#include <xygine/components/ParticleController.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/PostChromeAb.hpp>
#include <xygine/mesh/SphereBuilder.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
#include "KeyMappings.inl"
}

PlanetHoppingState::PlanetHoppingState(xy::StateStack& stack, Context context)
    : xy::State     (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_collisionWorld(m_scene),
    m_meshRenderer  ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene),
    m_useController (false),
    m_input         (0)
{
    m_loadingSprite.setTexture(m_resources.textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();

    xy::Stats::clear();

    m_scene.setView(context.defaultView);
    //m_scene.drawDebug(true);
    auto pp = xy::PostProcess::create<lm::PostBlur>();
    m_scene.addPostProcess(pp); 
    pp = xy::PostProcess::create<xy::PostChromeAb>(false);
    m_scene.addPostProcess(pp);

    loadMeshes();
    loadParticles();
    buildScene();

    m_useController = sf::Joystick::isConnected(0) && context.appInstance.getGameSettings().controllerEnabled;

    quitLoadingScreen();
    LOG("State Loaded", xy::Logger::Type::Info);
}

//public
bool PlanetHoppingState::update(float dt)
{
    //LOG("Start update", xy::Logger::Type::Info);
    if (m_useController) parseControllerInput(m_input);
    
    xy::Command cmd;
    cmd.category = LMCommandID::Player;
    cmd.action = [this](xy::Entity& entity, float)
    {
        if (entity.destroyed()) return;
        entity.getComponent<ph::PlayerController>()->setInput(m_input);
    };
    m_scene.sendCommand(cmd);

    m_scene.update(dt);
    m_collisionWorld.update();
    m_meshRenderer.update();
    
    //LOG("End update", xy::Logger::Type::Info);

    return false;
}

bool PlanetHoppingState::handleEvent(const sf::Event& evt)
{
    //in its own function so both keyb/joy events may call it
    std::function<void()> fire = [this]()
    {
        xy::Command cmd;
        cmd.category = LMCommandID::Player;
        cmd.action = [](xy::Entity& entity, float)
        {
            entity.getComponent<ph::PlayerController>()->leaveOrbit(entity.getComponent<ph::OrbitComponent>()->removeParent());
        };
        m_scene.sendCommand(cmd);

        //do this second so we don't launch the player on spawn
        cmd.category = LMCommandID::GameController;
        cmd.action = [this](xy::Entity& entity, float)
        {
            auto gc = entity.getComponent<ph::GameController>();

            if (gc->gameEnded())
            {
                requestStackPop();
                requestStackPush(States::ID::SinglePlayer);
            }
            else
            {
                gc->spawnPlayer();
            }
        };
        m_scene.sendCommand(cmd);
    };
    
    switch (evt.type)
    {
    default: break;
    case sf::Event::JoystickConnected:
        m_useController = (evt.joystickConnect.joystickId == 0 &&
            getContext().appInstance.getGameSettings().controllerEnabled);
        break;
    case sf::Event::JoystickDisconnected:
        if (evt.joystickConnect.joystickId == 0) m_useController = false;
        break;
    case sf::Event::JoystickButtonReleased:
        if (!m_useController || evt.joystickButton.joystickId != 0) break;
        switch (evt.joystickButton.button)
        {
        default:break;
        case buttonA:
            fire();
            break;
        case buttonStart:
            requestStackPush(States::ID::Pause);
            break;
        }
        break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        default: break;
        case keyLeft:
        case altKeyLeft:
            m_input &= ~LMInputFlags::SteerLeft;
            break;
        case keyRight:
        case altKeyRight:
            m_input &= ~LMInputFlags::SteerRight;
            break;
        case sf::Keyboard::P:
            requestStackPush(States::ID::Pause);
            break;
        }
        break;
    case sf::Event::KeyReleased:
    {
        switch (evt.key.code)
        {
        default:break;
        case keyLeft:
        case altKeyLeft:
            m_input |= LMInputFlags::SteerLeft;
            break;
        case keyRight:
        case altKeyRight:
            m_input |= LMInputFlags::SteerRight;
            break;
        case keyFire:
            fire();
        break;
#ifdef _DEBUG_
        case sf::Keyboard::BackSpace:
            requestStackPop();
            break;
#endif //_DEBUG_
        }
    }
    }
    return false;
}

void PlanetHoppingState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& data = msg.getData<xy::Message::UIEvent>();
        switch (data.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            //m_scene.setView(getContext().defaultView);
            m_meshRenderer.setView(getContext().defaultView);
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

void PlanetHoppingState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    //rw.draw(m_meshRenderer);

    rw.setView(rw.getDefaultView());
}

//private
void PlanetHoppingState::loadMeshes()
{
    m_scene.setAmbientColour({ 26, 20, 22 });
    m_scene.getSkyLight().setIntensity(0.6f);
    m_scene.getSkyLight().setDiffuseColour({ 255, 255, 100 });
    m_scene.getSkyLight().setSpecularColour({ 250, 255, 158 });
    m_scene.getSkyLight().setDirection({ 0.2f, 0.4f, -0.4f });

    //m_meshRenderer.setView(context.defaultView);
    auto meshDrawable = m_meshRenderer.createDrawable(m_messageBus);
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(meshDrawable);
    m_scene.addEntity(entity, xy::Scene::Layer::BackFront);

    //preload meshes
    xy::SphereBuilder sb(1.f, 10, true); //we'll scale per entity
    m_meshRenderer.loadModel(Mesh::Planet, sb);

    //preload shaders
    m_resources.shaderResource.preload(Shader::MeshTextured, DEFERRED_TEXTURED_VERTEX, DEFERRED_TEXTURED_FRAGMENT);

    //preload materials
    auto& desertPlanet = m_resources.materialResource.add(Material::DesertPlanet, m_resources.shaderResource.get(Shader::MeshTextured));
    desertPlanet.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    desertPlanet.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/dust_planet.png") });
    m_resources.textureResource.setFallbackColour(sf::Color::Black);
    desertPlanet.addProperty({ "u_maskMap", m_resources.textureResource.get("no_mask") });

    auto& lavaPlanet = m_resources.materialResource.add(Material::LavaPlanet, m_resources.shaderResource.get(Shader::MeshTextured));
    lavaPlanet.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    lavaPlanet.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/lava_planet_diffuse.png") });
    lavaPlanet.addProperty({ "u_maskMap", m_resources.textureResource.get("assets/images/game/textures/lava_planet_mask.png") });
}

void PlanetHoppingState::loadParticles()
{
    auto particleController = xy::Component::create<xy::ParticleController>(m_messageBus);
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
            component->fire(LMParticleID::SmallExplosion, { msgData.posX, msgData.posY });
            break;
        }
    };
    particleController->addMessageHandler(handler);

    auto entity = xy::Entity::create(m_messageBus);
    auto pc = entity->addComponent(particleController);
    xy::ParticleSystem::Definition pd;
    pd.loadFromFile("assets/particles/small_explosion.xyp", m_resources.textureResource);
    pc->addDefinition(LMParticleID::SmallExplosion, pd);

    pd.loadFromFile("assets/particles/large_explosion.xyp", m_resources.textureResource);
    pc->addDefinition(LMParticleID::LargeExplosion, pd);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}

void PlanetHoppingState::buildScene()
{
    auto gameController = xy::Component::create<ph::GameController>(m_messageBus, m_resources, m_scene, m_collisionWorld, m_meshRenderer);   
    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(gameController);
    entity->addCommandCategories(LMCommandID::GameController);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);

    auto background = xy::Component::create<lm::Starfield>(m_messageBus, m_resources.textureResource);
    background->setVelocity({ 1.f, 0.f });
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(background);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);
}

void PlanetHoppingState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}