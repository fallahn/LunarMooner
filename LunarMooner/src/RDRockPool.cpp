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

#include <RDRockPool.hpp>
#include <RDRockController.hpp>

using namespace rd;

#include <LMCollisionComponent.hpp>
#include <LMCollisionWorld.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Scene.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

#include <xygine/util/Random.hpp>

#include <SFML/Graphics/CircleShape.hpp>

namespace
{
    const std::size_t poolSize = 10;
    const float minSize = 60.f;
    const float maxSize = 120.f;
}

RockPool::RockPool(lm::CollisionWorld& cw, xy::Scene& scene, xy::MessageBus& mb)
    : m_pool    (poolSize)
{
    //init pool
    const float padding = maxSize;
    for (auto i = 0u; i < poolSize; ++i)
    {
        float size = xy::Util::Random::value(minSize, maxSize);
        float homeSize = (maxSize * 2.f) * i + padding;
        auto controller = xy::Component::create<RockController>(mb, sf::Vector2f(xy::DefaultSceneSize.x + homeSize, 0.f));
        auto collision = cw.addComponent(mb, { {-size, -size}, {size *2.f, size * 2.f} }, lm::CollisionComponent::ID::Tower, false);
        lm::CollisionComponent::Callback callback = std::bind(&RockController::collisionCallback, controller.get(), std::placeholders::_1);
        collision->setCallback(callback);

        auto qtc = xy::Component::create<xy::QuadTreeComponent>(mb, sf::FloatRect({ -size, -size },{ size * 2.f, size * 2.f }));

        auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(mb);
        drawable->getDrawable().setRadius(size);
        drawable->getDrawable().setOrigin(size, size);
        drawable->getDrawable().setFillColor(sf::Color::Red);

        auto entity = xy::Entity::create(mb);
        entity->addComponent(controller);
        entity->addComponent(collision);
        entity->addComponent(qtc);
        entity->addComponent(drawable);

        m_pool[i] = scene.addEntity(entity, xy::Scene::Layer::FrontMiddle);
        m_pool[i]->getComponent<RockController>()->reset();
    }
}

//public
void RockPool::spawn()
{
    //find a dead rock and bring it to life
    auto result = std::find_if(std::begin(m_pool), std::end(m_pool),
        [](xy::Entity* ep)
    {
        return !ep->getComponent<RockController>()->alive();
    });

    if (result != m_pool.end())
    {
        (*result)->getComponent<RockController>()->spawn({ 2200.f, xy::Util::Random::value(0.f, xy::DefaultSceneSize.y) });
    }
}