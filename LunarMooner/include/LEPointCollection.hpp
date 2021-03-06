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

#ifndef LE_POINT_COLLECTION_HPP_
#define LE_POINT_COLLECTION_HPP_

#include <LESelectableCollection.hpp>
#include <LESelectableItem.hpp>

#include <SFML/Graphics/Vertex.hpp>

#include <vector>
#include <array>
#include <memory>

namespace le
{
    class PointCollection final : public SelectableCollection
    {
    public:
        PointCollection();
        ~PointCollection() = default;

        SelectableItem* getSelected(const sf::Vector2f&) override;
        void update() override;
        SelectableItem* add(const sf::Vector2f&) override;
        void clear() override { for (auto& p : m_points) p->remove(); }

        std::vector<sf::Vector2f> getPoints() const;

    private:

        std::vector<std::unique_ptr<PointItem>> m_points;
        std::array<sf::Vertex, 50> m_vertices;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}


#endif //LE_POINT_COLLECTION_HPP_