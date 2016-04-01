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

#include <xygine/App.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/shaders/NormalMapped.hpp>

#include <SFML/Graphics/CircleShape.hpp>

MenuBackgroundState::MenuBackgroundState(xy::StateStack& ss, Context context)
    : xy::State         (ss, context),
    m_messageBus        (context.appInstance.getMessageBus()),
    m_scene             (m_messageBus),
    m_normalMapShader   (nullptr)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    m_shaderResource.preload(LMShaderID::NormalBlend, xy::Shader::Default::vertex, lm::normalBlendFrag);
    m_shaderResource.preload(LMShaderID::NormalMapColoured, xy::Shader::NormalMapped::vertex, NORMAL_FRAGMENT_TEXTURED_SPECULAR);

    m_normalMapShader = &m_shaderResource.get(LMShaderID::NormalMapColoured);

    setup();

    quitLoadingScreen();

    auto msg = m_messageBus.post<xy::Message::UIEvent>(xy::Message::UIMessage);
    msg->type = xy::Message::UIEvent::RequestState;
    msg->stateId = States::ID::MenuMain;
}

//public
bool MenuBackgroundState::update(float dt)
{
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

void MenuBackgroundState::handleMessage(const xy::Message&)
{

}

//private
void MenuBackgroundState::setup()
{
    auto planet = xy::Component::create<lm::PlanetDrawable>(m_messageBus);
    planet->setBaseNormal(m_textureResource.get("assets/images/background/sphere_normal.png"));
    planet->setDetailNormal(m_textureResource.get("assets/images/background/crater_normal.png"));
    planet->setDiffuseTexture(m_textureResource.get("assets/images/background/moon_diffuse.png"));
    planet->setBlendShader(m_shaderResource.get(LMShaderID::NormalBlend));
    planet->setNormalShader(*m_normalMapShader);

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(planet);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);
}