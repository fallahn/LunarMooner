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

#include <LevelEditorState.hpp>
#include <LMShaderIds.hpp>
#include <PHPlanetRotation.hpp>
#include <BGStarfield.hpp>

#include <xygine/App.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/QuadBuilder.hpp>
#include <xygine/mesh/CubeBuilder.hpp>
#include <xygine/util/Random.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace
{
#include "ConstParams.inl"

}

EditorState::EditorState(xy::StateStack& stack, Context context)
    : xy::State     (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_meshRenderer  ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene)
{
    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    loadMeshes();
    buildScene();

    quitLoadingScreen();

    context.appInstance.setMouseCursorVisible(true);
}

//public
bool EditorState::handleEvent(const sf::Event & evt)
{

    return false;
}

void EditorState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
    m_meshRenderer.handleMessage(msg);

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& msgData = msg.getData<xy::Message::UIEvent>();
        switch (msgData.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            m_scene.setView(getContext().defaultView);
            //m_meshRenderer.setView(getContext().defaultView);
            break;
        }
    }
}

bool EditorState::update(float dt)
{
    m_scene.update(dt);
    m_meshRenderer.update();
    return false;
}

void EditorState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
}

//private
void EditorState::loadMeshes()
{
    m_scene.setAmbientColour(SceneAmbientColour);
    m_scene.getSkyLight().setIntensity(SceneLightIntensity);
    m_scene.getSkyLight().setDiffuseColour(SceneDiffuseLight);
    m_scene.getSkyLight().setSpecularColour(SceneSpecularLight);
    m_scene.getSkyLight().setDirection(SceneLightDirection);

    m_meshRenderer.setNearFarRatios(0.2f, 10.8f);
    m_meshRenderer.setFOV(50.f);
    
    m_resources.shaderResource.preload(Shader::ID::MeshNormalMapped, DEFERRED_TEXTURED_BUMPED_VERTEX, DEFERRED_TEXTURED_BUMPED_FRAGMENT);
    m_resources.shaderResource.preload(Shader::ID::MeshTextured, DEFERRED_TEXTURED_VERTEX, DEFERRED_TEXTURED_FRAGMENT);
    m_resources.shaderResource.preload(Shader::ID::MeshVertexColoured, DEFERRED_VERTCOLOURED_VERTEX, DEFERRED_VERTCOLOURED_FRAGMENT);
    m_resources.shaderResource.preload(Shader::ID::Shadow, SHADOW_VERTEX, SHADOW_FRAGMENT);

    auto& groundMat = m_resources.materialResource.add(Material::ID::Ground, m_resources.shaderResource.get(Shader::ID::MeshNormalMapped));
    groundMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    groundMat.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/moon_diffuse.png") });
    groundMat.addProperty({ "u_normalMap", m_resources.textureResource.get("assets/images/game/textures/moon_normal.png") });
    groundMat.addRenderPass(xy::RenderPass::ID::ShadowMap, m_resources.shaderResource.get(Shader::ID::Shadow));
    groundMat.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto& wallMat01 = m_resources.materialResource.add(Material::ID::RockWall01, m_resources.shaderResource.get(Shader::ID::MeshNormalMapped));
    wallMat01.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    wallMat01.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/rockwall_01_diffuse.png") });
    wallMat01.addProperty({ "u_normalMap", m_resources.textureResource.get("assets/images/game/textures/rockwall_01_normal.png") });
    wallMat01.addRenderPass(xy::RenderPass::ID::ShadowMap, m_resources.shaderResource.get(Shader::ID::Shadow));

    auto& vertMat = m_resources.materialResource.add(Material::ID::DeadDoofer, m_resources.shaderResource.get(Shader::ID::MeshVertexColoured));
    vertMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    vertMat.addRenderPass(xy::RenderPass::ID::ShadowMap, m_resources.shaderResource.get(Shader::ID::Shadow));

    auto& platMat = m_resources.materialResource.add(Material::ID::Platform, m_resources.shaderResource.get(Shader::ID::MeshTextured));
    platMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    platMat.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/platform_diffuse.png") });
    platMat.addRenderPass(xy::RenderPass::ID::ShadowMap, m_resources.shaderResource.get(Shader::ID::Shadow));

    xy::IQMBuilder mb("assets/models/moon_surface.iqm");
    m_meshRenderer.loadModel(Mesh::ID::Ground, mb);

    xy::IQMBuilder ib("assets/models/rock_wall_01.iqm");
    m_meshRenderer.loadModel(Mesh::ID::RockWall01, ib);

    xy::IQMBuilder ib2("assets/models/island_rock_01.iqm");
    m_meshRenderer.loadModel(Mesh::ID::RockIsland01, ib2);

    xy::IQMBuilder ib3("assets/models/island_rock_02.iqm");
    m_meshRenderer.loadModel(Mesh::ID::RockIsland02, ib3);

    xy::CubeBuilder cb(1.f);
    m_meshRenderer.loadModel(Mesh::ID::Platform, cb);

    xy::IQMBuilder ib4("assets/models/corpse.iqm");
    m_meshRenderer.loadModel(Mesh::ID::DeadDoofer, ib4);
}

void EditorState::buildScene()
{
    //ground
    auto ground = m_meshRenderer.createModel(Mesh::ID::Ground, m_messageBus);
    ground->setBaseMaterial(m_resources.materialResource.get(Material::ID::Ground));
    ground->rotate(xy::Model::Axis::Y, 180.f);
    ground->setPosition({ 0.f, 0.f, 580.f });
    ground->setScale({ 1.2f, 1.f, 1.f });

    auto entity = xy::Entity::create(m_messageBus);
    entity->addComponent(ground);
    entity->setPosition(xy::DefaultSceneSize.x / 2.f, xy::DefaultSceneSize.y - groundOffset);

    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
    
    //rear walls
    for (auto i = 0u; i < 5u; ++i)
    {
        auto rockWall = m_meshRenderer.createModel(Mesh::ID::RockWall01, m_messageBus);
        rockWall->setPosition({ 0.f, 20.f, -960.f });
        rockWall->setScale({ 2.2f, xy::Util::Random::value(1.2f, 1.8f), 1.8f });

        auto& material = m_resources.materialResource.get(Material::ID::RockWall01);
        rockWall->setBaseMaterial(material);

        entity = xy::Entity::create(m_messageBus);
        entity->setPosition(-80 + (rockWall->getMesh().getBoundingBox().asFloatRect().width / 2.f) + (i * 600.f), xy::DefaultSceneSize.y - groundOffset);
        entity->addComponent(rockWall);
        m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);
    }

    //-----------------------------------------
    auto rock = m_meshRenderer.createModel(Mesh::ID::RockIsland01, m_messageBus);
    rock->setBaseMaterial(m_resources.materialResource.get(Material::ID::DeadDoofer));
    rock->setPosition({ 0.f, 0.f, playerOffsetDepth });
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(rock);
    entity->setPosition(500.f, xy::DefaultSceneSize.y - groundOffset);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    rock = m_meshRenderer.createModel(Mesh::ID::RockIsland02, m_messageBus);
    rock->setBaseMaterial(m_resources.materialResource.get(Material::ID::DeadDoofer));
    rock->setPosition({ 0.f, 0.f, playerOffsetDepth });
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(rock);
    entity->setPosition(1200.f, xy::DefaultSceneSize.y - groundOffset);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    auto box = m_meshRenderer.createModel(Mesh::ID::Platform, m_messageBus);
    box->setScale({ 300.f, 50.f, 120.f });
    box->setBaseMaterial(m_resources.materialResource.get(Material::ID::Platform));
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(box);
    entity->setPosition(900.f, 800.f);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);

    for (auto i = 0; i < 10; ++i)
    {
        spawnDeadGuy(xy::Util::Random::value(alienArea.left, alienArea.left + alienArea.width),
            xy::Util::Random::value(alienArea.top, alienArea.top + alienArea.height), 
            {xy::Util::Random::value(-300.f, 200.f), xy::Util::Random::value(-200.f, 300.f)});
    }
    //----------------------------------------

    auto meshRenderer = m_meshRenderer.createDrawable(m_messageBus);
    //meshRenderer->enableWater(true);
    //meshRenderer->setWaterColour(SceneWaterColour);
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(meshRenderer);
    m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);


    auto bg = xy::Component::create<lm::Starfield>(m_messageBus, m_resources.textureResource);
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(bg);
    m_scene.addEntity(entity, xy::Scene::Layer::BackRear);

    //add a couple of panels just to mark where the UI would be
    sf::Color c(117, 115, 99);
    auto rect = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
    rect->getDrawable().setSize({ alienArea.left, xy::DefaultSceneSize.y });
    rect->getDrawable().setFillColor(c);
    entity = xy::Entity::create(m_messageBus);
    entity->addComponent(rect);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);

    rect = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
    rect->getDrawable().setSize({ alienArea.left, xy::DefaultSceneSize.y });
    rect->getDrawable().setFillColor(c);
    entity = xy::Entity::create(m_messageBus);
    entity->setPosition(alienArea.left + alienArea.width, 0.f);
    entity->addComponent(rect);
    m_scene.addEntity(entity, xy::Scene::Layer::UI);
}

#include <LMAlienController.hpp>
void EditorState::spawnDeadGuy(float x, float y, const sf::Vector2f& vel)
{
    if (alienArea.contains(x, y))
    {
        auto model = m_meshRenderer.createModel(Mesh::DeadDoofer, m_messageBus);
        model->setScale({ 0.2f, 0.2f, 0.2f });
        model->setBaseMaterial(m_resources.materialResource.get(Material::DeadDoofer));

        auto controller = xy::Component::create<lm::AlienController>(m_messageBus, alienArea);
        controller->setVelocity(vel);
        auto entity = xy::Entity::create(m_messageBus);
        entity->addComponent(model);
        entity->addComponent(controller);
        entity->setPosition(x, y);
        m_scene.addEntity(entity, xy::Scene::Layer::BackMiddle);
    }
}