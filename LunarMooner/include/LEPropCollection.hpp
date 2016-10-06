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

#ifndef LE_PROP_COLLECTION_HPP_
#define LE_PROP_COLLECTION_HPP_

#include <LESelectableCollection.hpp>
#include <LESelectableItem.hpp>
#include <ResourceCollection.hpp>

#include <vector>
#include <memory>
#include <map>

namespace xy
{
    class Scene;
    class MeshRenderer;
    class MessageBus;
}

namespace le
{
    class PropCollection final :public SelectableCollection
    {
    public:
        PropCollection(xy::Scene&, xy::MeshRenderer&, ResourceCollection&, xy::MessageBus&,
            std::map<std::uint32_t, std::vector<std::uint32_t>>&);
        ~PropCollection() = default;

        SelectableItem* getSelected(const sf::Vector2f&) override;
        void update() override;
        SelectableItem* add(const sf::Vector2f&) override;

        void setPropIndex(int idx) { m_propIndex = idx; }

    private:
        xy::Scene& m_scene;
        xy::MeshRenderer& m_meshRenderer;
        ResourceCollection& m_resources;
        xy::MessageBus& m_messageBus;
        std::map<std::uint32_t, std::vector<std::uint32_t>>& m_materialMap;

        int m_propIndex;

        std::vector<std::unique_ptr<PropItem>> m_props;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LE_PROP_COLLECTION_HPP_