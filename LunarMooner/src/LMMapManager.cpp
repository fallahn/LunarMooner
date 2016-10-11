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

#include <xygine/util/Random.hpp>
#include <xygine/Command.hpp>
#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/components/Model.hpp>

using namespace lm;

namespace
{
    const std::string mapDir("assets/maps/");
}

MapManager::MapManager(xy::MessageBus& mb, MaterialMap& mm, ResourceCollection& rc, xy::MeshRenderer& mr)
    : xy::Component (mb, this),
    m_materialMap   (mm),
    m_resources     (rc),
    m_entity        (nullptr),
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
}

//public
void MapManager::entityUpdate(xy::Entity&, float) {}

void MapManager::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

const std::vector<MapManager::Platform>& MapManager::getPlatformData() const
{
    return m_platforms[m_level];
}

const std::vector<sf::Vector2f>& MapManager::getChains() const
{
    return m_chains[m_level];
}

void MapManager::setLevel(sf::Uint8 level)
{
    m_level = level % m_propData.size();
    updateScene();
}

//private
void MapManager::loadMap(const std::string& path)
{
    //parse map file and store the properties in relevant array


    /*std::uint32_t idx;
    auto result = std::find_if(m_materialMap.begin(), m_materialMap.end(),
        [](const std::pair<std::uint32_t, std::pair<std::string, std::vector<std::uint32_t>>>& pair)
    {
        return pair.second.first == modelString;
    });*/
}

void MapManager::updateScene()
{
    //clear old entities, add new
    xy::Command cmd;
    cmd.category = LMCommandID::InUse;
    cmd.action = [](xy::Entity& entity, float) {entity.destroy(); };
    m_entity->getScene()->sendCommand(cmd);

    cmd.category = LMCommandID::Prop;
    cmd.action = [](xy::Entity& entity, float) {entity.addCommandCategories(LMCommandID::InUse); };
    m_entity->getScene()->sendCommand(cmd);

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

        m_entity->getScene()->addEntity(entity, xy::Scene::Layer::FrontRear);
    }
}