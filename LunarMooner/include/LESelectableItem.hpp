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

#ifndef LE_SELECTABLE_HPP_
#define LE_SELECTABLE_HPP_

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <memory>

namespace xy
{
    class Entity;
    class Model;
}

namespace le
{
    class SelectableItem : public sf::Transformable, public sf::Drawable
    {
    public:
        enum class Type
        {
            Prop, Point, Platform
        };
        SelectableItem() : m_deleted(false) {}
        virtual ~SelectableItem() = default;

        virtual Type type() const = 0;
        SelectableItem* remove() { m_deleted = true; return nullptr; }
        virtual void select() = 0;
        virtual void deselect() = 0;
        bool deleted() const { return m_deleted; }
        virtual sf::FloatRect globalBounds() const = 0;
    
    protected:
        virtual void draw(sf::RenderTarget&, sf::RenderStates) const override = 0;

    private:
        bool m_deleted;
    };

    class PointItem final : public SelectableItem
    {
    public:
        PointItem();
        ~PointItem() = default;
        Type type() const override { return Type::Point; }
        void select() override;
        void deselect() override;
        sf::FloatRect globalBounds() const override;
    private:

        sf::RectangleShape m_shape;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };

    class PlatformItem final : public SelectableItem
    {
    public:
        PlatformItem();
        ~PlatformItem() = default;

        Type type() const override { return Type::Platform; }
        void select() override;
        void deselect() override;
        sf::FloatRect globalBounds() const override;
        
        void setSize(const sf::Vector2f&);
        const sf::Vector2f& getSize() const;
        
        void setValue(int v) { m_value = v; }
        int getValue() const { return m_value; }

        void setFrozen(bool);
    private:
        int m_value;
        bool m_frozen;
        sf::RectangleShape m_shape;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };

    class PropItem final : public SelectableItem
    {
    public:
        PropItem(xy::Entity&, std::uint32_t);
        ~PropItem();

        Type type() const { return Type::Prop; }
        void select() override;
        void deselect() override;
        sf::FloatRect globalBounds() const override;

        void update();
        void setModel(std::uint32_t, std::unique_ptr<xy::Model>&);
        std::uint32_t getModelID() const { return m_modelID; }

    private:

        xy::Entity& m_entity;
        std::uint32_t m_modelID;

        sf::RectangleShape m_shape;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LE_SELECTABLE_HPP_