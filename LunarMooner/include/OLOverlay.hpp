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

#ifndef LM_OVERLAY_HPP_
#define LM_OVERLAY_HPP_

#include <xygine/Entity.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/View.hpp>

namespace xy
{
    class Scene;
}

struct ResourceCollection;
namespace lm
{
    class Overlay final : public sf::Drawable
    {
    public:
        Overlay(xy::MessageBus&, ResourceCollection&, xy::Scene&);
        ~Overlay() = default;

        Overlay(const Overlay&) = delete;
        Overlay& operator = (const Overlay&) = delete;

        void update(float);
        void handleMessage(const xy::Message&);

        void setView(const sf::View&);

    private:

        xy::MessageBus& m_messageBus;
        ResourceCollection& m_resources;
        xy::Entity::Ptr m_rootNode;

        sf::View m_view;
        void draw(sf::RenderTarget&, sf::RenderStates) const override;
    };
}

#endif //LM_OVERLAY_HPP_