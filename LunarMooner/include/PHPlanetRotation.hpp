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

#ifndef PH_PLANET_ROTATION_HPP_
#define PH_PLANET_ROTATION_HPP_

#include <xygine/components/Component.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/Entity.hpp>

namespace ph
{
    class PlanetRotation final : public xy::Component
    {
    public:
        explicit PlanetRotation(xy::MessageBus& mb) : xy::Component(mb, this), m_model(nullptr) {}
        ~PlanetRotation() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity& entity, float dt) override
        {
            m_model->rotate(xy::Model::Axis::X, 10.f * dt);
        }
        void onDelayedStart(xy::Entity& e) override
        {
            m_model = e.getComponent<xy::Model>();
            XY_ASSERT(m_model, "Model component not found");
        }

    private:
        xy::Model* m_model;
    };
}

#endif //PH_PLANET_ROTATION_HPP_