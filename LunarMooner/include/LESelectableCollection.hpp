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

#ifndef LE_SELECTABLE_COLLECTION_HPP_
#define LE_SELECTABLE_COLLECTION_HPP_

#include <SFML/Graphics/Drawable.hpp>

namespace le
{
    class SelectableItem;
    class SelectableCollection : public sf::Drawable
    {
    public:
        SelectableCollection() : m_frozen(false), m_hidden(false) {}
        virtual ~SelectableCollection() = default;

        virtual SelectableItem* getSelected(const sf::Vector2f&) = 0;
        virtual void update() = 0;
        virtual SelectableItem* add(const sf::Vector2f&) = 0;
        virtual void clear() = 0;

        virtual void setFrozen(bool frozen) { m_frozen = frozen; }
        bool frozen() const { return m_frozen || m_hidden; }

        void setHidden(bool hidden) { m_hidden = hidden; }
        bool hidden() const { return m_hidden; }

    protected:
        void draw(sf::RenderTarget&, sf::RenderStates) const override = 0;
        
    private:
        bool m_frozen;
        bool m_hidden;
    };
}

#endif //LE_SELECTABLE_COLLECTION_HPP_