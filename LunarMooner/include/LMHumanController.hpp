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

#ifndef LM_HUMAN_CONTROLLER_HPP_
#define LM_HUMAN_CONTROLLER_HPP_

#include <xygine/components/Component.hpp>

namespace xy
{
    class AnimatedDrawable;
}

namespace lm
{
    class HumanController final : public xy::Component
    {
    public:
        enum AnimationID
        {
            Climb = 0,
            Wave,
            RunLeft,
            RunRight
        };

        HumanController(xy::MessageBus&, xy::AnimatedDrawable&);
        ~HumanController() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;

        void setDestination(const sf::Vector2f&);

        const sf::Vector2f& getPosition() const { return m_position; }

    private:
        xy::AnimatedDrawable& m_drawable;
        bool m_gotoDestination;
        sf::Vector2f m_destination;
        sf::Vector2f m_position;

        std::size_t m_waveTableIndex;
    };
}

#endif //LM_HUMAN_CONTROLLER_HPP_