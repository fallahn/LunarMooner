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

#include <LEPropCollection.hpp>
#include <LMShaderIds.hpp>

#include <xygine/Scene.hpp>
#include <xygine/Entity.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/components/Model.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

using namespace le;

namespace
{
    std::size_t maxProps = 12;
}

PropCollection::PropCollection(xy::Scene& scene, xy::MeshRenderer& mr, ResourceCollection& rc, xy::MessageBus& mb,
    std::map<std::uint32_t, std::vector<std::uint32_t>>& mm)
    : m_scene       (scene),
    m_meshRenderer  (mr),
    m_resources     (rc),
    m_messageBus    (mb),
    m_materialMap   (mm),
    m_propIndex     (0),
    m_rotation      (0.f)
{

}

//public
SelectableItem* PropCollection::getSelected(const sf::Vector2f& mousePos)
{
    for (auto& p : m_props)
    {
        if (p->globalBounds().contains(mousePos))
        {
            p->select();
            return p.get();
        }
    }
    return nullptr;
}

void PropCollection::update()
{
    for (auto& p : m_props) p->update();

    m_props.erase(std::remove_if(std::begin(m_props), std::end(m_props), 
        [](const std::unique_ptr<PropItem>& p) 
    {
        return p->deleted();
    }), std::end(m_props));
}

SelectableItem* PropCollection::add(const sf::Vector2f& position)
{
    if (m_props.size() < maxProps)
    {
        auto model = m_meshRenderer.createModel(Mesh::Count + m_propIndex, m_messageBus);
        
        //set model material
        const auto& matIDs = m_materialMap[Mesh::Count + m_propIndex];
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

        auto entity = xy::Entity::create(m_messageBus);
        entity->addComponent(model);
        entity->setPosition(position);
        auto e = m_scene.addEntity(entity, xy::Scene::Layer::FrontFront);
        
        m_props.emplace_back(std::make_unique<PropItem>(*e));
        m_props.back()->setPosition(position);
        m_props.back()->setRotation(m_rotation);
        m_props.back()->setScale(m_scale);
        return m_props.back().get();
    }
    else
    {
        xy::Logger::log("Maximum number of platforms is " + std::to_string(maxProps), xy::Logger::Type::Info);
    }
    return nullptr;
}

//private
void PropCollection::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    for (const auto& p : m_props) rt.draw(*p, states);
}