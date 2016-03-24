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

#ifndef LM_BULLET_CONTROLLER_HPP_
#define LM_BULLET_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

namespace lm
{
    class CollisionComponent;
    class BulletController final : public xy::Component
    {
    public:
        explicit BulletController(xy::MessageBus&);
        ~BulletController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        void collisionCallback(CollisionComponent*);

    private:

        xy::Entity* m_entity;
    };
}

#endif //LM_BULLET_CONTROLER_HPP_