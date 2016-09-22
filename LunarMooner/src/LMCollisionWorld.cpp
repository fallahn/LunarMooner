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

#include <LMCollisionWorld.hpp>

#include <xygine/Scene.hpp>

using namespace lm;

CollisionWorld::CollisionWorld(const xy::Scene& scene)
: m_scene(scene){}

//public
CollisionComponent::Ptr CollisionWorld::addComponent(xy::MessageBus& mb, sf::FloatRect area, CollisionComponent::ID id, bool isCollider)
{
    auto cc = xy::Component::create<CollisionComponent>(mb, area, id);
    if (isCollider)
    {
        m_colliders.push_back(cc.get());
    }

    return std::move(cc);
}

void CollisionWorld::update()
{
    flush();

    for (auto ca : m_colliders)
    {
        const auto collidees = m_scene.queryQuadTree(ca->globalBounds());
        for (auto c : collidees)
        {
            if (auto entity = c->getEntity())
            {
                auto cbs = entity->getComponents<CollisionComponent>();
                if (!c->destroyed())
                {
                    for (const auto cb : cbs)
                    {
                        if (ca == cb) continue;
                        if (ca->globalBounds().intersects(cb->globalBounds()))
                        {
                            ca->addCollider(cb);
                            cb->addCollider(ca);
                            //break;
                        }
                    }
                }
            }
        }
    }
}

void CollisionWorld::flush()
{
    m_colliders.erase(std::remove_if(m_colliders.begin(), m_colliders.end(),
        [](const CollisionComponent* cp)
    {
        return cp->destroyed();
    }), m_colliders.end());
}