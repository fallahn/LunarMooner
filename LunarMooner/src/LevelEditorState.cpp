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
#include <xygine/util/Json.hpp>
#include <xygine/FileSystem.hpp>

#include <xygine/imgui/imgui.h>
#include <xygine/imgui/CommonDialogues.hpp>

#include <xygine/parsers/picojson.h>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <fstream>

namespace
{
#include "ConstParams.inl"

    int currentItemIndex = 0;
    int currentPropIndex = 0;
    
    float nextPlatformSize[] = { 100.f, 25.f };
    int nextPlatValue = 10;

    float propScale[] = { 1.f, 1.f };
    float propRotation = 0.f;

    std::vector<std::string> modelFiles;

    const std::string fileExtension = ".lmp";
    const float autosaveTime = 180.f;
}

EditorState::EditorState(xy::StateStack& stack, Context context)
    : xy::State     (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_meshRenderer  ({ context.appInstance.getVideoSettings().VideoMode.width, context.appInstance.getVideoSettings().VideoMode.height }, m_scene),
    m_selectedItem  (nullptr)
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

    if (m_autosaveClock.getElapsedTime().asSeconds() > autosaveTime)
    {
        saveMap("autosave.lmp");
        m_autosaveClock.restart();
    }


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
namespace
{
    enum Mouse
    {
        Left = 0x1,
        Middle = 0x2,
        Right = 0x4
    };
}

void EditorState::doMouseEvent(const sf::Event& evt)
{
    static sf::Uint8 mouseButtons = 0;
    static sf::Vector2f middleMouseClickPosition;
    
    auto position = xy::App::getMouseWorldPosition();

    //check if we're over anything when clicking
    if (evt.type == sf::Event::MouseButtonPressed)
    {
        if (evt.mouseButton.button == sf::Mouse::Left)
        {
            if (m_selectedItem && m_selectedItem->globalBounds().contains(position))
            {
                mouseButtons |= Mouse::Left;
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
                    mouseButtons |= Mouse::Left;
                    m_clickedOffset = m_selectedItem->getPosition() - position;
                }
            }
        }
        else if (evt.mouseButton.button == sf::Mouse::Right)
        {
            addItem(position);
        }
        else if (evt.mouseButton.button == sf::Mouse::Middle)
        {
            mouseButtons |= Mouse::Middle;
            middleMouseClickPosition = position;
        }
    }

    if (evt.type == sf::Event::MouseButtonReleased)
    {
        switch (evt.mouseButton.button)
        {
        default: break;
        case sf::Mouse::Left:
            mouseButtons &= ~Mouse::Left;
            break;
        case sf::Mouse::Middle:
            mouseButtons &= ~Mouse::Middle;
            break;
        }
    }

    //if the mouse moves while left button pressed
    //move any selected item
    if (evt.type == sf::Event::MouseMoved)
    {
        if (m_selectedItem)
        {
            if (mouseButtons & Mouse::Left)
            {
                position.x = std::min(alienArea.left + alienArea.width, std::max(alienArea.left, position.x));
                position.y = std::min(xy::DefaultSceneSize.y, std::max(0.f, position.y)); //probably needs to be clamped smaller?

                m_selectedItem->setPosition(position + m_clickedOffset);
            }
            else if (mouseButtons & Mouse::Middle)
            {
                sf::Vector2f diff = position - middleMouseClickPosition;

                if (m_selectedItem->type() == le::SelectableItem::Type::Platform)
                {
                    //change size
                    auto item = dynamic_cast<le::PlatformItem*>(m_selectedItem);
                    auto newSize = item->getSize() + diff;
                    newSize.x = std::min(500.f, std::max(4.f, newSize.x));
                    newSize.y = std::min(500.f, std::max(4.f, newSize.y));
                    item->setSize(newSize);
                }
                else if (m_selectedItem->type() == le::SelectableItem::Type::Prop)
                {
                    //change scale
                    auto item = dynamic_cast<le::PropItem*>(m_selectedItem);
                    diff.y = -diff.y;
                    auto newScale = item->getScale() + (diff * 0.01f);
                    newScale.x = std::min(10.f, std::max(0.1f, newScale.x));
                    newScale.y = std::min(10.f, std::max(0.1f, newScale.y));
                    item->setScale(newScale);
                }
                middleMouseClickPosition = position;
            }
        }
    }

    //scroll mouse rotates items (only affects props)
    if (evt.type == sf::Event::MouseWheelScrolled)
    {
        if (m_selectedItem && m_selectedItem->type() == le::SelectableItem::Type::Prop)
        {
            m_selectedItem->rotate(evt.mouseWheelScroll.delta * 5.f);
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
            currentItemIndex = std::min(static_cast<int>(m_collections.size() - 1), ++currentItemIndex);
            break;
        case sf::Keyboard::A:
            currentPropIndex = std::max(0, --currentPropIndex);
            break;
        case sf::Keyboard::D:
            currentPropIndex = std::min(static_cast<int>(m_materialMap.size() - 1), ++currentPropIndex);
            break;
        case sf::Keyboard::F:
            m_collections[currentItemIndex]->setFrozen(!m_collections[currentItemIndex]->frozen());
            if (currentItemIndex == Collection::Props)
            {
                for (auto& m : m_materialMap)
                {
                    for (auto& i : m.second.second)
                    {
                        m_resources.materialResource.get(i).getProperty("u_colour")->setValue(m_collections[currentItemIndex]->frozen() ? sf::Color(107, 107, 255) : sf::Color::White);
                    }
                }
            }  
            if (m_selectedItem)
            {
                m_selectedItem->deselect();
                m_selectedItem = nullptr;
            }
            break;
        case sf::Keyboard::H:
            m_collections[currentItemIndex]->setHidden(!m_collections[currentItemIndex]->hidden());
            if (currentItemIndex == Collection::Props)
            {
                for (auto& m : m_materialMap)
                {
                    for (auto& i : m.second.second)
                    {
                        bool hidden = m_collections[currentItemIndex]->hidden();
                        m_resources.materialResource.get(i).getRenderPass(xy::RenderPass::Default)->setCullFace(hidden ? xy::FrontAndBack : xy::Back);
                        m_resources.materialResource.get(i).getRenderPass(xy::RenderPass::ShadowMap)->setCullFace(hidden ? xy::FrontAndBack : xy::Back);
                    }
                }
            }
            if (m_selectedItem)
            {
                m_selectedItem->deselect();
                m_selectedItem = nullptr;
            }
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
    groundMat.addProperty({ "u_colour", sf::Color::White });
    groundMat.addRenderPass(xy::RenderPass::ID::ShadowMap, m_resources.shaderResource.get(Shader::ID::Shadow));
    groundMat.getRenderPass(xy::RenderPass::ID::ShadowMap)->setCullFace(xy::CullFace::Front);

    auto& wallMat01 = m_resources.materialResource.add(Material::ID::RockWall01, m_resources.shaderResource.get(Shader::ID::MeshNormalMapped));
    wallMat01.addUniformBuffer(m_meshRenderer.getMatrixUniforms());
    wallMat01.addProperty({ "u_diffuseMap", m_resources.textureResource.get("assets/images/game/textures/rockwall_01_diffuse.png") });
    wallMat01.addProperty({ "u_normalMap", m_resources.textureResource.get("assets/images/game/textures/rockwall_01_normal.png") });
    wallMat01.addProperty({ "u_colour", sf::Color::White });
    wallMat01.addRenderPass(xy::RenderPass::ID::ShadowMap, m_resources.shaderResource.get(Shader::ID::Shadow));

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
                        mat.addProperty({ "u_colour", sf::Color::White });
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
                            mat.addProperty({ "u_colour", sf::Color::White });
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
                            mat.addProperty({ "u_colour", sf::Color::White });
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
                m_materialMap[Mesh::Count + i] = std::make_pair(f, std::move(materialIDs));
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
        nim::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoCollapse);

        static std::string currentFile;
        if (nim::BeginMenuBar())
        {
            if (nim::BeginMenu("Help"))
            {
                //if (nim::MenuItem("Keyboard Shortcuts", nullptr, &showHelp))
                {
                    nim::TextUnformatted("Right-click: Place selected item\nW/S: Scroll through items\n"
                        "A/D Choose item property\nF: Toggle layer frozen\nH: Toggle layer hidden\n"
                        "Delete: remove selected item\nMiddle Mouse Drag: Size/Scale\nMiddle Mouse Scroll: rotate prop");
                }
                nim::EndMenu();
            }
            nim::EndMenuBar();
        }

        if (nim::fileBrowseDialogue("Open File", currentFile, nim::Button("Load"))
            && !currentFile.empty())
        {
            loadMap(currentFile);

        }
        nim::SameLine();
        if (nim::fileBrowseDialogue("Save File", currentFile, nim::Button("Save##2"))
            && !currentFile.empty())
        {
            if (xy::FileSystem::getFileExtension(currentFile) != fileExtension)
            {
                currentFile.append(fileExtension);
            }
            saveMap(currentFile);
        }
        nim::SameLine();
        if (nim::Button("Exit"))
        {
            //TODO confirm/save box
            requestStackClear();
            requestStackPush(States::ID::MenuBackground);
        }
        nim::Separator();

        nim::Combo("", &currentItemIndex, "Prop\0Point\0Platform\0");
        nim::SameLine();
        if (nim::Button("Add", {40.f, 20.f}))
        {
            addItem(xy::DefaultSceneSize / 2.f);
        }
       
        bool frozen = m_collections[currentItemIndex]->frozen();
        nim::Checkbox("Freeze", &frozen);
        if (frozen != m_collections[currentItemIndex]->frozen())
        {
            m_collections[currentItemIndex]->setFrozen(frozen);
            if (currentItemIndex == Collection::Props)
            {
                for (auto& m : m_materialMap)
                {
                    for (auto& i : m.second.second)
                    {
                        m_resources.materialResource.get(i).getProperty("u_colour")->setValue(frozen ? sf::Color(107, 107, 255) : sf::Color::White);
                    }
                }
            }
            if (m_selectedItem)
            {
                m_selectedItem->deselect();
                m_selectedItem = nullptr;
            }
        }
        nim::SameLine();
        bool hidden = m_collections[currentItemIndex]->hidden();
        nim::Checkbox("Hide", &hidden);
        if (hidden != m_collections[currentItemIndex]->hidden())
        {
            m_collections[currentItemIndex]->setHidden(hidden);
            
            //hides props by face culling
            if (currentItemIndex == Collection::Props)
            {
                for (auto& m : m_materialMap)
                {
                    for (auto& i : m.second.second)
                    {
                        m_resources.materialResource.get(i).getRenderPass(xy::RenderPass::Default)->setCullFace(hidden ? xy::FrontAndBack : xy::Back);
                        m_resources.materialResource.get(i).getRenderPass(xy::RenderPass::ShadowMap)->setCullFace(hidden ? xy::FrontAndBack : xy::Back);
                    }
                }
            }

            if (m_selectedItem)
            {
                m_selectedItem->deselect();
                m_selectedItem = nullptr;
            }
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
                auto platformSize = dynamic_cast<le::PlatformItem*>(m_selectedItem)->getSize();
                float lastX = nextPlatformSize[0] = platformSize.x;
                float lastY = nextPlatformSize[1] = platformSize.y;

                nim::DragFloat2("Size", nextPlatformSize, 0.5f, 4.f, 500.f);
                if (lastX != nextPlatformSize[0] || lastY != nextPlatformSize[1])
                {
                    nextPlatformSize[0] = std::min(500.f, std::max(4.f, nextPlatformSize[0]));
                    nextPlatformSize[1] = std::min(500.f, std::max(4.f, nextPlatformSize[1]));
                    dynamic_cast<le::PlatformItem*>(m_selectedItem)->setSize({ nextPlatformSize[0], nextPlatformSize[1] });
                }

                int lastValue = nextPlatValue = dynamic_cast<le::PlatformItem*>(m_selectedItem)->getValue();
                nim::DragInt("Value", &nextPlatValue, 0.2f, 10, 200);
                if (lastValue != nextPlatValue)
                {
                    dynamic_cast<le::PlatformItem*>(m_selectedItem)->setValue(nextPlatValue);
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
                    //auto model = m_meshRenderer.createModel(Mesh::Count + currentPropIndex, m_messageBus);
                    ////set model material
                    //const auto& matIDs = m_materialMap[Mesh::Count + currentPropIndex].second;
                    //if (matIDs.size() == 1)
                    //{
                    //    model->setBaseMaterial(m_resources.materialResource.get(matIDs[0]));
                    //}
                    //else
                    //{
                    //    for (auto i = 0u; i < matIDs.size(); ++i)
                    //    {
                    //        model->setSubMaterial(m_resources.materialResource.get(matIDs[i]), i);
                    //    }
                    //}

                    //dynamic_cast<le::PropItem*>(m_selectedItem)->setModel(currentPropIndex, model);
                    dynamic_cast<le::PropCollection*>(m_collections[Collection::Props].get())->setPropIndex(currentPropIndex);   
                }
                //scale and rotation
                auto scale = dynamic_cast<le::PropItem*>(m_selectedItem)->getScale();
                float lastX = propScale[0] = scale.x;
                float lastY = propScale[1] = scale.y;
                nim::DragFloat2("Scale", propScale, 0.01f, 0.1f, 10.f);

                if (lastX != propScale[0] || lastY != propScale[1])
                {
                    dynamic_cast<le::PropItem*>(m_selectedItem)->setScale(propScale[0], propScale[1]);
                }

                float lastRot =  dynamic_cast<le::PropItem*>(m_selectedItem)->getRotation();
                while (lastRot > 180.f) { lastRot -= 360.f; }
                propRotation = lastRot;
                nim::DragFloat("Rotation", &propRotation, 0.1f, -180.f, 180.f);
                if (lastRot != propRotation)
                {
                    dynamic_cast<le::PropItem*>(m_selectedItem)->setRotation(propRotation);
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

                //scale
                nim::DragFloat2("Scale", propScale, 0.01f, 0.1f, 10.f);

                //rotation
                nim::DragFloat("Rotation", &propRotation, 0.1f, -180.f, 180.f);

                break;
            case Collection::Platforms:
            {
                nim::DragFloat2("Size", nextPlatformSize, 0.5f, 4.f, 500.f);
                nim::DragInt("Value", &nextPlatValue, 0.2f, 10, 200);
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
    if (m_collections[currentItemIndex]->frozen()) return;
    
    switch (currentItemIndex)
    {
    default: break;
    case Collection::Points: break;
    case Collection::Platforms:
        nextPlatformSize[0] = std::min(500.f, std::max(4.f, nextPlatformSize[0]));
        nextPlatformSize[1] = std::min(500.f, std::max(4.f, nextPlatformSize[1]));
        dynamic_cast<le::PlatformCollection*>(m_collections[currentItemIndex].get())->setNextSize({ nextPlatformSize[0], nextPlatformSize[1] });
        dynamic_cast<le::PlatformCollection*>(m_collections[currentItemIndex].get())->setNextValue(nextPlatValue);
        break;
    case Collection::Props:
    {
        auto props = dynamic_cast<le::PropCollection*>(m_collections[currentItemIndex].get());
        props->setPropIndex(currentPropIndex);
        props->setNextRotation(propRotation);
        props->setNextScale({ propScale[0], propScale[1] });
    }
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

void EditorState::saveMap(const std::string& path)
{
    //**outputs array of selectable item objects**//
    //****Type
    //****Position
    //****Size - for platforms
    //****Value - for platforms
    //****Rotation - for props
    //****Scale - for props
    //****File Name - for props

    std::ofstream file(path, std::ios::out);
    if (!file.good() || file.fail() || !file.is_open())
    {
        xy::Logger::log("Failed opening " + path + " for writing", xy::Logger::Type::Error, xy::Logger::Output::All);
        file.close();
        return;
    }

    pj::array outArray;
    const auto points = dynamic_cast<le::PointCollection*>(m_collections[Collection::Points].get())->getPoints();
    for (const auto& p : points)
    {
        pj::object item;
        item["type"] = pj::value(static_cast<double>(static_cast<int>(Collection::Points)));

        pj::object position;
        position["x"] = pj::value(p.x);
        position["y"] = pj::value(p.y);

        item["position"] = pj::value(position);

        outArray.push_back(pj::value(item));
    }

    const auto& plats = dynamic_cast<le::PlatformCollection*>(m_collections[Collection::Platforms].get())->getPlatforms();
    for (const auto& p : plats)
    {
        pj::object item;
        item["type"] = pj::value(static_cast<double>(static_cast<int>(Collection::Platforms)));

        pj::object position;
        position["x"] = pj::value(p->getPosition().x);
        position["y"] = pj::value(p->getPosition().y);

        item["position"] = pj::value(position);

        pj::object size;
        size["x"] = pj::value(p->getSize().x);
        size["y"] = pj::value(p->getSize().y);

        item["size"] = pj::value(size);

        item["value"] = pj::value(static_cast<double>(p->getValue()));

        outArray.push_back(pj::value(item));
    }

    const auto& props = dynamic_cast<le::PropCollection*>(m_collections[Collection::Props].get())->getProps();
    for (const auto& p : props)
    {
        pj::object item;
        item["type"] = pj::value(static_cast<double>(static_cast<int>(Collection::Props)));

        pj::object position;
        position["x"] = pj::value(p->getPosition().x);
        position["y"] = pj::value(p->getPosition().y);

        item["position"] = pj::value(position);

        item["rotation"] = pj::value(p->getRotation()); //remember when unpacking this we need to move to +- 180!

        pj::object scale;
        scale["x"] = pj::value(p->getScale().x);
        scale["y"] = pj::value(p->getScale().y);

        item["scale"] = pj::value(scale);

        item["name"] = pj::value(m_materialMap[Mesh::Count + p->getModelID()].first);

        outArray.push_back(pj::value(item));
    }

    auto json = pj::value(outArray).serialize();
    file.write(json.c_str(), json.size());
    file.close();
}

void EditorState::loadMap(const std::string& path)
{
    //clear existing items
    if (m_selectedItem)
    {
        m_selectedItem->deselect();
        m_selectedItem = nullptr;
    }
    for (auto& c : m_collections)
    {
        c->clear();
        c->setFrozen(false);
        c->setHidden(false);
        c->update();
    }
    for (auto& m : m_materialMap)
    {
        for (auto& i : m.second.second)
        {
            m_resources.materialResource.get(i).getProperty("u_colour")->setValue(sf::Color::White);
            m_resources.materialResource.get(i).getRenderPass(xy::RenderPass::Default)->setCullFace(xy::Back);
            m_resources.materialResource.get(i).getRenderPass(xy::RenderPass::ShadowMap)->setCullFace(xy::Back);
        }
    }


    
    //remember when unpacking this we need to move rotations to +- 180!
    if (xy::FileSystem::getFileExtension(path) != fileExtension)
    {
        xy::Logger::log(path + ": Not a material file", xy::Logger::Type::Error);
        return;
    }

    std::ifstream file(path);
    if (!file.good() || !xy::Util::File::validLength(file))
    {
        LOG("failed to open " + path + ", or file empty", xy::Logger::Type::Error);
        file.close();
        return;
    }

    std::string jsonString;
    while (!file.eof())
    {
        std::string temp;
        file >> temp;
        jsonString += temp;
    }
    if (jsonString.empty())
    {
        LOG(path + "failed to read, or file empty", xy::Logger::Type::Error);
        file.close();
        return;
    }
    file.close();

    pj::value pv;
    auto err = pj::parse(pv, jsonString);
    if (err.empty())
    {
        if (pv.is<pj::array>())
        {
            auto objArray = pv.get<pj::array>();
            for (const auto& obj : objArray)
            {
                sf::Vector2f position = xy::DefaultSceneSize / 2.f;
                if (obj.get("position").is<pj::object>())
                {
                    if (obj.get("position").get("x").is<double>())
                    {
                        position.x = static_cast<float>(obj.get("position").get("x").get<double>());
                    }
                    if (obj.get("position").get("y").is<double>())
                    {
                        position.y = static_cast<float>(obj.get("position").get("y").get<double>());
                    }
                }

                le::SelectableItem::Type itemType = le::SelectableItem::Type::Count;
                //remember if we don't get a valid type to skip!
                if (obj.get("type").is<double>())
                {
                    int type = static_cast<int>(obj.get("type").get<double>());
                    if (type < static_cast<int>(le::SelectableItem::Type::Count)
                        && type >= 0)
                    {
                        itemType = static_cast<le::SelectableItem::Type>(type);
                        switch (itemType)
                        {
                        default: continue;
                        case le::SelectableItem::Type::Point:
                            currentItemIndex = Collection::Points;
                            addItem(position);
                            break;
                        case le::SelectableItem::Type::Platform:
                        {
                            if (obj.get("value").is<double>())
                            {
                                nextPlatValue = static_cast<int>(obj.get("value").get<double>());
                            }
                            if (obj.get("size").is<pj::object>())
                            {
                                if (obj.get("size").get("x").is<double>())
                                {
                                    nextPlatformSize[0] = static_cast<float>(obj.get("size").get("x").get<double>());
                                }
                                if (obj.get("size").get("y").is<double>())
                                {
                                    nextPlatformSize[1] = static_cast<float>(obj.get("size").get("y").get<double>());
                                }
                            }

                            currentItemIndex = Collection::Platforms;
                            addItem(position);
                        }
                            break;
                        case le::SelectableItem::Type::Prop:
                        {
                            if (obj.get("rotation").is<double>())
                            {
                                propRotation = static_cast<float>(obj.get("rotation").get<double>());
                                while (propRotation > 180.f) { propRotation -= 360; }
                            }

                            if (obj.get("scale").is<pj::object>())
                            {
                                if (obj.get("scale").get("x").is<double>())
                                {
                                    propScale[0] = static_cast<float>(obj.get("scale").get("x").get<double>());
                                    propScale[0] = std::min(10.f, std::max(0.1f, propScale[0]));
                                }

                                if (obj.get("scale").get("y").is<double>())
                                {
                                    propScale[1] = static_cast<float>(obj.get("scale").get("y").get<double>());
                                    propScale[1] = std::min(10.f, std::max(0.1f, propScale[1]));
                                }
                            }

                            if (obj.get("name").is<std::string>())
                            {
                                auto name = obj.get("name").get<std::string>();
                                auto i = 0;
                                for (auto& m : m_materialMap)
                                {
                                    if (m.second.first == name)
                                    {
                                        break;
                                    }
                                    i++;
                                }

                                if (i >= m_materialMap.size())
                                {
                                    //skip this as we have invalid model ID
                                    xy::Logger::log(name + " model ID not found!", xy::Logger::Type::Warning);
                                    continue;
                                }

                                currentPropIndex = i;
                            }

                            currentItemIndex = Collection::Props;
                            addItem(position);
                        }
                            break;
                        }
                        if(m_selectedItem) m_selectedItem->deselect();
                        m_selectedItem = nullptr;
                    }
                    else
                    {
                        //invalid type!
                        continue;
                    }
                }
            }
        }
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