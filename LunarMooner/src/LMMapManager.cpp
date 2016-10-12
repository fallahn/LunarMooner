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

#include <LMMapManager.hpp>
#include <CommandIds.hpp>
#include <LMShaderIds.hpp>
#include <LESelectableItem.hpp>

#include <xygine/util/Random.hpp>
#include <xygine/Command.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/util/Json.hpp>

using namespace lm;

namespace
{
    const std::string mapDir("assets/maps/");
    const std::string fileExtension(".lmp");
}

MapManager::MapManager(xy::MessageBus& mb, MaterialMap& mm, ResourceCollection& rc, xy::MeshRenderer& mr)
    : xy::Component (mb, this),
    m_materialMap   (mm),
    m_resources     (rc),
    m_meshRenderer  (mr),
    m_level         (0)
{
    //list map files in dir and shuffle
    auto files = xy::FileSystem::listFiles(mapDir);
    files.erase(std::remove_if(files.begin(), files.end(),
        [](const std::string& str)
    {
        return (xy::FileSystem::getFileExtension(str) != ".lmp");
    }), files.end());

    std::shuffle(files.rbegin(), files.rend(), xy::Util::Random::rndEngine);

    //load up to 10 of them (g++ requires explicit type)
    std::size_t max = std::min(std::size_t(10u), files.size());
    for (auto i = 0u; i < max; ++i)
    {
        loadMap(mapDir + files[i]);
    }
    LOG("loaded " + std::to_string(m_chains.size()) + " maps", xy::Logger::Type::Info);
}

//public
void MapManager::entityUpdate(xy::Entity&, float) {}

const std::vector<MapManager::Platform>& MapManager::getPlatforms() const
{
    return m_platforms[m_level];
}

const std::vector<sf::Vector2f>& MapManager::getChain() const
{
    return m_chains[m_level];
}

void MapManager::setLevel(sf::Uint8 level, xy::Scene& scene)
{
    m_level = level % m_propData.size();
    updateScene(scene);
}

//private
void MapManager::loadMap(const std::string& path)
{
    //parse map file and store the properties in relevant array
    if (xy::FileSystem::getFileExtension(path) != fileExtension)
    {
        xy::Logger::log(path + ": Not a material file", xy::Logger::Type::Error);
        return;
    }

    std::ifstream file(path);
    if (!file.good() || !xy::Util::File::validLength(file))
    {
        xy::Logger::log("failed to open " + path + ", or file empty", xy::Logger::Type::Error);
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
        xy::Logger::log(path + "failed to read, or file empty", xy::Logger::Type::Error);
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

            m_chains.emplace_back();
            auto& chain = m_chains.back();

            m_platforms.emplace_back();
            auto& platforms = m_platforms.back();

            m_propData.emplace_back();
            auto& props = m_propData.back();

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
                            chain.push_back(position);
                            break;
                        case le::SelectableItem::Type::Platform:
                        {
                            platforms.emplace_back();
                            auto& platform = platforms.back();
                            
                            if (obj.get("value").is<double>())
                            {
                                platform.value = static_cast<int>(obj.get("value").get<double>());
                            }
                            if (obj.get("size").is<pj::object>())
                            {
                                if (obj.get("size").get("x").is<double>())
                                {
                                    platform.size.x = static_cast<float>(obj.get("size").get("x").get<double>());
                                    platform.size.x = std::min(500.f, std::max(4.f, platform.size.x));
                                }
                                if (obj.get("size").get("y").is<double>())
                                {
                                    platform.size.y = static_cast<float>(obj.get("size").get("y").get<double>());
                                    platform.size.y = std::min(500.f, std::max(4.f, platform.size.y));
                                }
                            }

                            platform.position = position;
                        }
                        break;
                        case le::SelectableItem::Type::Prop:
                        {
                            props.emplace_back();
                            auto& prop = props.back();
                            
                            if (obj.get("rotation").is<double>())
                            {
                                prop.rotation = static_cast<float>(obj.get("rotation").get<double>());
                            }

                            if (obj.get("scale").is<pj::object>())
                            {
                                if (obj.get("scale").get("x").is<double>())
                                {
                                    prop.scale.x = static_cast<float>(obj.get("scale").get("x").get<double>());
                                    prop.scale.x = std::min(10.f, std::max(0.1f, prop.scale.x));
                                }

                                if (obj.get("scale").get("y").is<double>())
                                {
                                    prop.scale.y = static_cast<float>(obj.get("scale").get("y").get<double>());
                                    prop.scale.y = std::min(10.f, std::max(0.1f, prop.scale.y));
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

                                prop.modelID = Mesh::ID::Count + i;
                            }

                            prop.position = position;
                        }
                        break;
                        }
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

void MapManager::updateScene(xy::Scene& scene)
{
    //clear old entities, add new
    xy::Command cmd;
    cmd.category = LMCommandID::InUse;
    cmd.action = [](xy::Entity& entity, float) {entity.destroy(); };
    scene.sendCommand(cmd);

    cmd.category = LMCommandID::Prop;
    cmd.action = [](xy::Entity& entity, float) {entity.addCommandCategories(LMCommandID::InUse); };
    scene.sendCommand(cmd);

    for (const auto& pd : m_propData[m_level])
    {
        //create entity and add to scene
        auto model = m_meshRenderer.createModel(pd.modelID, getMessageBus());
        const auto& materials = m_materialMap[pd.modelID].second;
        if (materials.size() == 1)
        {
            model->setBaseMaterial(m_resources.materialResource.get(materials[0]));
        }
        else
        {
            for (auto i = 0u; i < materials.size(); ++i)
            {
                model->setSubMaterial(m_resources.materialResource.get(materials[i]), i);
            }
        }

        auto entity = xy::Entity::create(getMessageBus());
        entity->addComponent(model);
        entity->setRotation(pd.rotation);
        entity->setScale(pd.scale);
        entity->setPosition(pd.position);
        entity->addCommandCategories(LMCommandID::Prop);

        scene.addEntity(entity, xy::Scene::Layer::FrontRear);
    }
}