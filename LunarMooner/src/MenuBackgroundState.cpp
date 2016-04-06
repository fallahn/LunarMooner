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

#include <xygine/App.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/components/AnimatedDrawable.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/shaders/NormalMapped.hpp>
#include <xygine/PostBloom.hpp>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Mouse.hpp>

MenuBackgroundState::MenuBackgroundState(xy::StateStack& ss, Context context)
    : xy::State         (ss, context),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_scene             (m_messageBus),
    m_normalMapShader   (nullptr),
    m_lightEntity       (nullptr)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);
    //auto pp = xy::PostProcess::create<xy::PostBloom>();
    //m_scene.addPostProcess(pp);

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
    /*auto mousePos = getContext().appInstance.getMouseWorldPosition();
    m_lightEntity->setPosition(mousePos);
    auto light = m_lightEntity->getComponent<xy::PointLight>();

    m_normalMapShader->setUniform("u_pointLightPositions[0]", light->getWorldPosition());
    m_normalMapShader->setUniform("u_pointLights[0].intensity", light->getIntensity());
    m_normalMapShader->setUniform("u_pointLights[0].diffuseColour", sf::Glsl::Vec4(light->getDiffuseColour()));
    m_normalMapShader->setUniform("u_pointLights[0].specularColour", sf::Glsl::Vec4(light->getSpecularColour()));
    m_normalMapShader->setUniform("u_pointLights[0].inverseRange", light->getInverseRange());*/


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

    /*auto moon = xy::Component::create<xy::AnimatedDrawable>(m_messageBus);
    moon->setShader(*m_normalMapShader);
    moon->setNormalMap(m_textureResource.get("assets/images/background/sphere_normal.png"));
    moon->setTexture(m_textureResource.get("assets/images/background/moon_diffuse.png"));
    */

    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(moon);
    entity->setPosition({ -100.f, 340.f });

    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);

    auto light = xy::Component::create<xy::PointLight>(m_messageBus, 600.f);
    light->setDepth(200.f);
    light->setDiffuseColour({ 240u, 255u, 255u });
    
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(light);
    m_lightEntity = m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
}