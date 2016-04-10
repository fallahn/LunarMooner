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

#include <MenuBackgroundState.hpp>
#include <BGNormalBlendShader.hpp>
#include <BGPlanetDrawable.hpp>
#include <BGStarfield.hpp>
#include <Game.hpp>

#include <xygine/App.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/components/AudioSource.hpp>
#include <xygine/shaders/NormalMapped.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/PostChromeAb.hpp>

#include <SFML/Window/Mouse.hpp>

namespace
{
    const float maxMusicVol = 35.f;
}

MenuBackgroundState::MenuBackgroundState(xy::StateStack& ss, Context context)
    : xy::State         (ss, context),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_scene             (m_messageBus),
    m_normalMapShader   (nullptr),
    m_lightEntity       (nullptr)
{
    m_loadingSprite.setTexture(m_textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();
    m_scene.setView(context.defaultView);
    auto pp = xy::PostProcess::create<xy::PostChromeAb>();
    m_scene.addPostProcess(pp);

    m_shaderResource.preload(LMShaderID::Prepass, xy::Shader::Default::vertex, lm::materialPrepassFrag);
    m_shaderResource.preload(LMShaderID::NormalMapColoured, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED_SPECULAR_ILLUM);

    m_normalMapShader = &m_shaderResource.get(LMShaderID::NormalMapColoured);
    //m_normalMapShader->setUniform("u_ambientColour", sf::Glsl::Vec3(0.03f, 0.03f, 0.01f));

    setup();

    quitLoadingScreen();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::RequestState;
    msg->stateId = States::ID::MenuMain;
}

//public
bool MenuBackgroundState::update(float dt)
{
    //update lighting
    /*auto mousePos = getContext().appInstance.getMouseWorldPosition();*/
    m_lightEntity->setPosition(/*mousePos*/960.f, 540.f);
    auto light = m_lightEntity->getComponent<xy::PointLight>();

    m_normalMapShader->setUniform("u_pointLightPositions[0]", light->getWorldPosition());
    m_normalMapShader->setUniform("u_pointLights[0].intensity", light->getIntensity());
    m_normalMapShader->setUniform("u_pointLights[0].diffuseColour", sf::Glsl::Vec4(light->getDiffuseColour()));
    m_normalMapShader->setUniform("u_pointLights[0].specularColour", sf::Glsl::Vec4(light->getSpecularColour()));
    m_normalMapShader->setUniform("u_pointLights[0].inverseRange", light->getInverseRange());


    m_scene.update(dt);
    return true;
}

void MenuBackgroundState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    //rw.setView(getContext().defaultView);
}

bool MenuBackgroundState::handleEvent(const sf::Event&)
{
    return false;
}

void MenuBackgroundState::handleMessage(const xy::Message& msg)
{
    if (msg.id == xy::Message::UIMessage)
    {
        auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            m_scene.setView(getContext().defaultView);
            break;
        }
    }
    m_scene.handleMessage(msg);
}

//private
void MenuBackgroundState::setup()
{
    auto background = xy::Component::create<lm::Starfield>(m_messageBus, m_textureResource);
    m_scene.getLayer(xy::Scene::Layer::BackRear).addComponent(background);
    
    auto planet = xy::Component::create<lm::PlanetDrawable>(m_messageBus, 200.f);
    planet->setBaseNormal(m_textureResource.get("assets/images/background/sphere_normal.png"));
    planet->setDetailNormal(m_textureResource.get("assets/images/background/moon_normal.png"));
    planet->setDiffuseTexture(m_textureResource.get("assets/images/background/moon_diffuse.png"));
    planet->setMaskTexture(m_textureResource.get("assets/images/background/moon_mask.png"));
    planet->setPrepassShader(m_shaderResource.get(LMShaderID::Prepass));
    planet->setNormalShader(*m_normalMapShader);
    planet->setRotationVelocity({ -0.005f, 0.006f });
    planet->setTextureOffset({ 0.65f, 1.2f });
    planet->setColour({ 83u, 94u, 52u });

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(planet);
    entity->setPosition({ 1400.f, 100.f });

    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    auto moon = xy::Component::create<lm::PlanetDrawable>(m_messageBus, 500.f);
    moon->setBaseNormal(m_textureResource.get("assets/images/background/sphere_normal.png"));
    moon->setDetailNormal(m_textureResource.get("assets/images/background/moon_normal.png"));
    moon->setDiffuseTexture(m_textureResource.get("assets/images/background/moon_diffuse.png"));
    moon->setMaskTexture(m_textureResource.get("assets/images/background/moon_mask.png"));
    moon->setPrepassShader(m_shaderResource.get(LMShaderID::Prepass));
    moon->setNormalShader(*m_normalMapShader);

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(moon);
    entity->setPosition({ -100.f, 340.f });

    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    auto lc = xy::Component::create<xy::PointLight>(m_messageBus, 1200.f);
    lc->setDepth(700.f);
    lc->setDiffuseColour({ 240u, 255u, 255u });
    
    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(1160.f, 340.f);
    entity->addComponent(lc);
    m_lightEntity = m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);

    //music
    m_musicFiles = xy::FileSystem::listFiles("assets/sound/music");
    if (!m_musicFiles.empty())
    {
        m_musicFiles.erase(std::remove_if(m_musicFiles.begin(), m_musicFiles.end(),
            [](const std::string& str) 
        {
            return (xy::FileSystem::getFileExtension(str) != ".ogg"
                || str.substr(0, 4) != "menu");
        }), m_musicFiles.end());

        
        auto as = xy::Component::create<xy::AudioSource>(m_messageBus, m_soundResource);
        as->setSound("assets/sound/music/" + m_musicFiles[xy::Util::Random::value(0, m_musicFiles.size() - 1)], xy::AudioSource::Mode::Stream);
        const auto& settings = getContext().appInstance.getAudioSettings();
        const float volume = (settings.muted) ? 0.f : maxMusicVol * getContext().appInstance.getAudioSettings().volume;
        as->setVolume(volume);
        as->play(true);


        entity = xy::Entity::create(m_messageBus);
        auto music =  entity->addComponent(as);
        m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

        //TODO add audio events to xygine so we can pick a new track at random instead of looping
        //when the music has finished playing
        xy::Component::MessageHandler mh;
        mh.id = xy::Message::UIMessage;
        mh.action = [music](xy::Component* c, const xy::Message& msg)
        {
            auto& msgData = msg.getData<xy::Message::UIEvent>();
            switch (msgData.type)
            {
            default: break;
            case xy::Message::UIEvent::RequestAudioMute:
                music->setVolume(0.f);
                break;
            case xy::Message::UIEvent::RequestAudioUnmute:
            case xy::Message::UIEvent::RequestVolumeChange:
                music->setVolume(msgData.value * maxMusicVol);
                break;
            }
        };
        music->addMessageHandler(mh);
    }
}

void MenuBackgroundState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}