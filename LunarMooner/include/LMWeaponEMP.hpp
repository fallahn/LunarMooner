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

#ifndef LM_WEAPON_EMP_HPP_
#define LM_WEAPON_EMP_HPP_

#include <xygine/components/Component.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/CircleShape.hpp>

namespace xy
{
    class Scene;
}

namespace lm
{
    class CollisionComponent;
    class WeaponEMP final : public xy::Component, public sf::Drawable
    {
    public:
        WeaponEMP(xy::MessageBus&, const xy::Scene&);
        ~WeaponEMP() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Drawable; }
        void entityUpdate(xy::Entity&, float) override;

    private:

        const xy::Scene& m_scene;

        float m_radiusIncrease;

        sf::CircleShape m_shape;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;

        void killThing(CollisionComponent*);
    };
}

#endif //LM_WEAPON_EMP_HPP_