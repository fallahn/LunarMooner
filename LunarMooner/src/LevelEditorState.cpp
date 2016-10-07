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

#include <LEPointCollection.hpp>
#include <LEPlatformCollection.hpp>
#include <LEPropCollection.hpp>

#include <xygine/App.hpp>
#include <xygine/Entity.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/components/MeshDrawable.hpp>
#include <xygine/components/SfDrawableComponent.hpp>
#include <xygine/mesh/shaders/DeferredRenderer.hpp>
#include <xygine/mesh/IQMBuilder.hpp>
#include <xygine/mesh/QuadBuilder.hpp>
#include <xygine/mesh/CubeBuilder.hpp>
#include <xygine/mesh/MaterialDefinition.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/FileSystem.hpp>

#include <xygine/imgui/imgui.h>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

namespace
{
#include "ConstParams.inl"

    int currentItemIndex = 0;
    int currentPropIndex = 0;
    sf::Vector2f nextPlatformSize(100.f, 25.f);
    std::vector<std::string> modelFiles;
}

EditorState::EditorState(xy::StateStack& stack, Context context)
    : xy::State     (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_meshRenderer  ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene),
    m_selectedItem  (nullptr),
    m_hasClicked    (false)
{
    m_loadingSprite.setTexture(m_resources.textureResource.get("assets/images/game/meteor.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();
    m_scene.setView(context.defaultView);

    loadMeshes();
    buildScene();
    addWindows();

    m_collections[Collection::Points] = std::make_unique<le::PointCollection>();
    m_collections[Collection::Platforms] = std::make_unique<le::PlatformCollection>();
    m_collections[Collection::Props] = std::make_unique<le::PropCollection>(m_scene, m_meshRenderer, m_resources, m_messageBus, m_materialMap);

    quitLoadingScreen();

    context.appInstance.setMouseCursorVisible(true);
}

EditorState::~EditorState()
{
    xy::App::removeUserWindows(this);
}

//public
bool EditorState::handleEvent(const sf::Event & evt)
{    
    doMouseEvent(evt);
    doKeyEvent(evt);

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
    for (auto& c : m_collections)
    {
        c->update();
    }
    
    m_scene.update(dt);
    m_meshRenderer.update();
    return false;
}

void EditorState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);

    rw.setView(getContext().defaultView);
    for (const auto& c : m_collections)
    {
        rw.draw(*c);
    }
}

//private
void EditorState::doMouseEvent(const sf::Event& evt)
{
    auto position = xy::App::getMouseWorldPosition();

    //check if we're over anything when clicking
    if (evt.type == sf::Event::MouseButtonPressed)
    {
        if (evt.mouseButton.button == sf::Mouse::Left)
        {
            if (m_selectedItem && m_selectedItem->globalBounds().contains(position))
            {
                m_hasClicked = true;
                m_clickedOffset = m_selectedItem->getPosition() - position;
            }
            else
            {
                if (m_selectedItem)
                {
                    m_selectedItem->deselect();
                    m_selectedItem = nullptr;
                }

                std::size_t i = 0;
                while (i < m_collections.size() && !m_selectedItem)
                {
                    m_selectedItem = m_collections[i++]->getSelected(position);
                }
                if (m_selectedItem)
                {
                    m_hasClicked = true;
                    m_clickedOffset = m_selectedItem->getPosition() - position;
                }
            }
        }
        else if (evt.mouseButton.button == sf::Mouse::Right)
        {
            /*if (auto i = m_collections[currentItemIndex]->add(position))
            {
                if (m_selectedItem)
                {
                    m_selectedItem->deselect();
                }
                m_selectedItem = i;
                m_selectedItem->select();
            }*/
            addItem(position);
        }
    }

    if (evt.type == sf::Event::MouseButtonReleased &&
        evt.mouseButton.button == sf::Mouse::Left)
    {
        m_hasClicked = false;
    }

    //if the mouse moves while left button pressed
    //move any selected item
    if (evt.type == sf::Event::MouseMoved && m_hasClicked)
    {
        if (m_selectedItem)
        {
            position.x = std::min(alienArea.left + alienArea.width, std::max(alienArea.left, position.x));
            position.y = std::min(xy::DefaultSceneSize.y, std::max(0.f, position.y)); //probably needs to be clamped smaller?

            m_selectedItem->setPosition(position + m_clickedOffset);
        }
    }
}

void EditorState::doKeyEvent(const sf::Event& evt)
{
    if (evt.type == sf::Event::KeyReleased)
    {
        switch (evt.key.code)
        {
        default: break;
        case sf::Keyboard::W:
            currentItemIndex = std::max(0, --currentItemIndex);
            break;
        case sf::Keyboard::S:
            currentItemIndex = std::min(2, ++currentItemIndex); //TODO const here! this changes with the number of items :/
            break;
        case sf::Keyboard::A:

            break;
        case sf::Keyboard::D:

            break;
        case sf::Keyboard::Delete:
            if (m_selectedItem)
            {
                m_selectedItem = m_selectedItem->remove();
            }
            break;
        }
    }
}

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

    auto& platMat = m_resources.materialResource.add(Material::ID::Platform, m_resources.shaderResource.get(Shader::ID::MeshTextured));
    platMat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    platMat.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/platform_diffuse.png") });
    platMat.addRenderPass(xy::RenderPass::ID::ShadowMap, m_resources.shaderResource.get(Shader::ID::Shadow));

    xy::IQMBuilder mb("assets/models/moon_surface.iqm");
    m_meshRenderer.loadModel(Mesh::ID::Ground, mb);

    xy::IQMBuilder ib("assets/models/rock_wall_01.iqm");
    m_meshRenderer.loadModel(Mesh::ID::RockWall01, ib);


    //go through props folder and load each valid mesh
    modelFiles = xy::FileSystem::listFiles(propsDirectory);
    modelFiles.erase(std::remove_if(std::begin(modelFiles), std::end(modelFiles),
        [](const std::string& str)
    {
        return (xy::FileSystem::getFileExtension(str) != ".iqm");
    }), std::end(modelFiles));

    auto i = 0u;
    for (const auto& f : modelFiles)
    {
        if (xy::FileSystem::getFileExtension(f) == ".iqm")
        {
            //look for material with same name and load it if possible
            auto matname = f;
            matname.replace(matname.find(".iqm"), 4, ".xym");
            auto materials = xy::MaterialDefinition::loadFile(propsDirectory + matname);
            for (const auto& def : materials)
            {
                std::vector<std::uint32_t> materialIDs;
                
                switch (def.shaderType)
                {
                default:
                case xy::MaterialDefinition::VertexColoured:
                {
                    auto id = def.uid();
                    if (!m_resources.materialResource.hasMaterial(id))
                    {
                        auto& mat = m_resources.materialResource.add(id, m_resources.shaderResource.get(Shader::MeshVertexColoured));
                        mat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
                        if (def.castShadows)
                        {
                            mat.addRenderPass(xy::RenderPass::ShadowMap, m_resources.shaderResource.get(Shader::Shadow));
                        }
                    }
                    materialIDs.push_back(id);
                }
                    break;
                case xy::MaterialDefinition::Textured:
                {
                    auto id = def.uid();
                    if (!m_resources.materialResource.hasMaterial(id))
                    {
                        if (def.textures[2].empty())
                        {
                            //no normal map
                            auto& mat = m_resources.materialResource.add(id, m_resources.shaderResource.get(Shader::MeshTextured));
                            mat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());

                            m_resources.textureResource.setFallbackColour(sf::Color::Magenta);
                            mat.addProperty({ "u_diffuseMap", m_resources.textureResource.get(def.textures[0]) });
                            m_resources.textureResource.setFallbackColour(sf::Color::Black);
                            mat.addProperty({ "u_maskMap", m_resources.textureResource.get(def.textures[1]) });

                            if (def.castShadows)
                            {
                                mat.addRenderPass(xy::RenderPass::ShadowMap, m_resources.shaderResource.get(Shader::Shadow));
                            }
                        }
                        else
                        {
                            auto& mat = m_resources.materialResource.add(id, m_resources.shaderResource.get(Shader::MeshNormalMapped));
                            mat.addUniformBuffer(m_meshRenderer.getMatrixUniforms());

                            m_resources.textureResource.setFallbackColour(sf::Color::Magenta);
                            mat.addProperty({ "u_diffuseMap", m_resources.textureResource.get(def.textures[0]) });
                            m_resources.textureResource.setFallbackColour(sf::Color::Black);
                            mat.addProperty({ "u_maskMap", m_resources.textureResource.get(def.textures[1]) });
                            m_resources.textureResource.setFallbackColour(sf::Color(127, 127, 255));
                            mat.addProperty({ "u_normalMap", m_resources.textureResource.get(def.textures[2]) });

                            if (def.castShadows)
                            {
                                mat.addRenderPass(xy::RenderPass::ShadowMap, m_resources.shaderResource.get(Shader::Shadow));
                            }
                        }
                    }
                    materialIDs.push_back(id);
                }
                    break;
                }
                //map all IDs to the mesh ID
                m_materialMap[Mesh::Count + i] = std::move(materialIDs);
            }

            //load the mesh
            xy::IQMBuilder builder(propsDirectory + f);
            m_meshRenderer.loadModel(Mesh::ID::Count + i++, builder);
        }
    }
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

void EditorState::addWindows()
{
    xy::App::addUserWindow(
        [this]()
    {
        nim::SetNextWindowSize({ 240.f, 368.f });
        nim::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_ShowBorders);

        if (nim::BeginMenuBar())
        {
            if (nim::BeginMenu("Help"))
            {
                //if (nim::MenuItem("Keyboard Shortcuts", nullptr, &showHelp))
                {
                    nim::TextUnformatted("Right-click: Place selected item\nW/S: Scroll through items\nA/D Choose item property\nDelete: remove selected item");
                }
                nim::EndMenu();
            }

            if (nim::BeginMenu("Exit"))
            {
                //TODO save map on exit
                requestStackClear();
                requestStackPush(States::ID::MenuBackground);
                nim::EndMenu();
            }
            nim::EndMenuBar();
        }

        nim::Combo("", &currentItemIndex, "Prop\0Point\0Platform\0");
        nim::SameLine();
        if (nim::Button("Add", {40.f, 20.f}))
        {
            addItem(xy::DefaultSceneSize / 2.f);
        }

        if (nim::Button("Remove Selected", { 120, 20.f }))
        {
            //remove the currently selected item
            if (m_selectedItem)
            {
                m_selectedItem = m_selectedItem->remove();
            }
        }

        nim::NewLine();
        if (m_selectedItem)
        {
            switch (m_selectedItem->type())
            {
            default: break;
            case le::SelectableItem::Type::Platform:
            {
                nextPlatformSize = dynamic_cast<le::PlatformItem*>(m_selectedItem)->getSize();
                sf::Vector2f lastSize = nextPlatformSize;
                nim::Text("Size");
                nim::InputFloat("x", &nextPlatformSize.x, 1.f, 10.f);
                nim::InputFloat("y", &nextPlatformSize.y, 1.f, 10.f);
                if (lastSize != nextPlatformSize)
                {
                    nextPlatformSize.x = std::min(500.f, std::max(0.f, nextPlatformSize.x));
                    nextPlatformSize.y = std::min(500.f, std::max(0.f, nextPlatformSize.y));
                    dynamic_cast<le::PlatformItem*>(m_selectedItem)->setSize(nextPlatformSize);
                }
            }
                break;
            case le::SelectableItem::Type::Prop:
            {
                int idx = currentPropIndex;
                nim::Combo("Prop:", &currentPropIndex, 
                    [](void* data, int idx, const char** out)
                {
                    *out = (*(const std::vector<std::string>*)data)[idx].c_str();
                    return true;
                }, (void*)&modelFiles, modelFiles.size());

                if (idx != currentPropIndex && m_selectedItem) //selected prop changed
                {
                    auto model = m_meshRenderer.createModel(Mesh::Count + currentPropIndex, m_messageBus);
                    //set model material
                    const auto& matIDs = m_materialMap[Mesh::Count + currentPropIndex];
                    if (matIDs.size() == 1)
                    {
                        model->setBaseMaterial(m_resources.materialResource.get(matIDs[0]));
                    }
                    else
                    {
                        for (auto i = 0; i < matIDs.size(); ++i)
                        {
                            model->setSubMaterial(m_resources.materialResource.get(matIDs[i]), i);
                        }
                    }

                    dynamic_cast<le::PropItem*>(m_selectedItem)->setModel(model);
                    dynamic_cast<le::PropCollection*>(m_collections[Collection::Props].get())->setPropIndex(currentPropIndex);
                }
            }
            break;
            }
        }
        else //display the dropdown for current type
        {
            switch (currentItemIndex)
            {
            default:break;
            case Collection::Props:
                nim::Combo("Prop:", &currentPropIndex,
                    [](void* data, int idx, const char** out)
                {
                    *out = (*(const std::vector<std::string>*)data)[idx].c_str();
                    return true;
                }, (void*)&modelFiles, modelFiles.size());
                break;
            case Collection::Platforms:
            {
                nim::Text("Size");
                nim::InputFloat("x", &nextPlatformSize.x, 1.f, 10.f);
                nim::InputFloat("y", &nextPlatformSize.y, 1.f, 10.f);
            }
                break;
            }
        }

        nim::NewLine();
        static float fov = 52.f;
        float lastVal = fov;
        nim::SliderFloat("FOV", &fov, 10.f, 60.f);
        if (lastVal != fov) m_meshRenderer.setFOV(fov);
        nim::End();
    }, this);
}

void EditorState::addItem(const sf::Vector2f& position)
{
    switch (currentItemIndex)
    {
    default: break;
    case Collection::Points: break;
    case Collection::Platforms:
        nextPlatformSize.x = std::min(500.f, std::max(0.f, nextPlatformSize.x));
        nextPlatformSize.y = std::min(500.f, std::max(0.f, nextPlatformSize.y));
        dynamic_cast<le::PlatformCollection*>(m_collections[currentItemIndex].get())->setNextSize(nextPlatformSize);
        break;
    case Collection::Props:
        dynamic_cast<le::PropCollection*>(m_collections[currentItemIndex].get())->setPropIndex(currentPropIndex);
        break;
    }

    if (auto i = m_collections[currentItemIndex]->add(position))
    {
        if (m_selectedItem)
        {
            m_selectedItem->deselect();
        }
        m_selectedItem = i;
        m_selectedItem->select();
    }
}

void EditorState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    static float time = 0.f;
    time += dt;
    float value = std::sin(time * 12.f) * 128.f + 127.f;
    
    m_loadingSprite.setColor({ 255, 255, 255, static_cast<sf::Uint8>(value) });
    rw.draw(m_loadingSprite, sf::BlendAdd);
}