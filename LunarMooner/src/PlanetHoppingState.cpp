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

#include <PlanetHoppingState.hpp>
#include <PHPlayerController.hpp>

#include <xygine/App.hpp>
#include <xygine/components/QuadTreeComponent.hpp>
#include <xygine/util/Vector.hpp>
#include <xygine/util/Random.hpp>
#include <xygine/components/SfDrawableComponent.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

PlanetHoppingState::PlanetHoppingState(xy::StateStack& stack, Context context)
    : xy::State     (stack, context),
    m_messageBus    (context.appInstance.getMessageBus()),
    m_scene         (m_messageBus),
    m_collisionWorld(m_scene)
{
    m_loadingSprite.setTexture(m_textureResource.get("assets/images/ui/loading.png"));
    m_loadingSprite.setOrigin(sf::Vector2f(m_loadingSprite.getTexture()->getSize() / 2u));
    m_loadingSprite.setPosition(m_loadingSprite.getOrigin());

    launchLoadingScreen();

    m_scene.setView(context.defaultView);
    m_scene.drawDebug(true);
    //TODO post processes

    buildScene();

    quitLoadingScreen();
}

//public
bool PlanetHoppingState::update(float dt)
{
    m_collisionWorld.update();
    m_scene.update(dt);
    
    return false;
}

bool PlanetHoppingState::handleEvent(const sf::Event& evt)
{
    return false;
}

void PlanetHoppingState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);

    if (msg.id == xy::Message::UIMessage)
    {
        const auto& data = msg.getData<xy::Message::UIEvent>();
        switch (data.type)
        {
        default: break;
        case xy::Message::UIEvent::ResizedWindow:
            m_scene.setView(getContext().defaultView);
            break;
        }
    }
}

void PlanetHoppingState::draw()
{
    auto& rw = getContext().renderWindow;

    rw.draw(m_scene);
}

namespace
{
    const float boundsOffset = 40.f;
    const float boundsMargin = boundsOffset;

    const sf::Vector2f playerStart(100.f, xy::DefaultSceneSize.y / 2.f);
    const sf::FloatRect playerSize({ -10.f, -10.f }, { 20.f, 20.f });

    const float masterRadius = 100.f; //for start / end planets
}

//private
void PlanetHoppingState::buildScene()
{
    //set bounds
    m_scene.setSize({ 
        { -(boundsOffset + boundsMargin), -(boundsOffset + boundsMargin) },
        xy::DefaultSceneSize + (sf::Vector2f(boundsOffset + boundsMargin, boundsOffset + boundsMargin) * 2.f) });

    auto ct = m_collisionWorld.addComponent(m_messageBus,
    { {}, { xy::DefaultSceneSize.x + ((boundsOffset + boundsMargin) * 2.f), boundsOffset } },
        lm::CollisionComponent::ID::Bounds); //top
    auto qt = xy::Component::create<xy::QuadTreeComponent>(m_messageBus, ct->localBounds());

    auto cb = m_collisionWorld.addComponent(m_messageBus, 
    { { 0.f, xy::DefaultSceneSize.y + boundsOffset + (boundsMargin * 2.f) }, { xy::DefaultSceneSize.x + ((boundsOffset + boundsMargin) * 2.f), boundsOffset } },
        lm::CollisionComponent::ID::Bounds); //bottom
    auto qb = xy::Component::create<xy::QuadTreeComponent>(m_messageBus, cb->localBounds());

    auto cl = m_collisionWorld.addComponent(m_messageBus,
    {  {0.f, boundsOffset },{ boundsOffset, xy::DefaultSceneSize.y + (boundsMargin * 2.f) } },
        lm::CollisionComponent::ID::Bounds); //left
    auto ql = xy::Component::create<xy::QuadTreeComponent>(m_messageBus, cl->localBounds());

    auto cr = m_collisionWorld.addComponent(m_messageBus,
    { { xy::DefaultSceneSize.x + boundsOffset + (boundsMargin * 2.f), boundsOffset },{ boundsOffset, xy::DefaultSceneSize.y + (boundsMargin * 2.f) } },
        lm::CollisionComponent::ID::Bounds); //right
    auto qr = xy::Component::create<xy::QuadTreeComponent>(m_messageBus, cr->localBounds());

    auto ent = xy::Entity::create(m_messageBus);
    ent->addComponent(ct);
    ent->addComponent(cb);
    ent->addComponent(cl);
    ent->addComponent(cr);
    ent->addComponent(qt);
    ent->addComponent(qb);
    ent->addComponent(ql);
    ent->addComponent(qr);
    ent->setPosition(-(boundsOffset + boundsMargin), -(boundsOffset + boundsMargin));
    m_scene.addEntity(ent, xy::Scene::Layer::BackRear);


    //starting planet
    const float startOffset = masterRadius * 2.f;
    auto startBody = addBody({ startOffset, xy::Util::Random::value(startOffset, xy::DefaultSceneSize.y - startOffset) }, masterRadius);

    //ending planet
    auto endBody = addBody({xy::DefaultSceneSize.x - startOffset, xy::Util::Random::value(startOffset, xy::DefaultSceneSize.y - startOffset) }, masterRadius);

    //create intermediate bodies - evenly distribute this by repeating 4 times
    //once for each quadrant of the area
    std::vector<xy::Entity*> bodies;
    for (auto x = 0u; x < 2u; ++x)
    {
        float startX = x * (xy::DefaultSceneSize.x / 2.f);
        float endX = startX + (xy::DefaultSceneSize.x / 2.f);

        for (auto y = 0u; y < 2u; ++y)
        {
            float startY = y * (xy::DefaultSceneSize.y / 2.f);
            float endY = startY + (xy::DefaultSceneSize.y / 2.f);

            for (auto i = 0u; i < 2u; ++i)
            {
                auto body = addBody({ xy::Util::Random::value(startX, endX), xy::Util::Random::value(startY, endY) },
                    xy::Util::Random::value(10.f, 70.f));
                bodies.push_back(body);
            }
        }
    }
    //remove overlapping bodies, but keep nearby so that they orbit each other   
    for (auto body : bodies)
    {
        //move bodies in from far edges
        if (body->getWorldPosition().x < startOffset) body->move(startOffset, 0.f);
        if (body->getWorldPosition().x > (xy::DefaultSceneSize.x - startOffset)) body->move(-startOffset, 0.f);

        auto bounds = body->getComponent<lm::CollisionComponent>()->globalBounds();

        //use this to move away from start / end planets
        std::function<void(xy::Entity*, const xy::Entity*)> checkMaster = [&bounds](xy::Entity* body, const xy::Entity* master)
        {
            const float moveDistance = masterRadius * 4.f;
            if (bounds.intersects(master->getComponent<lm::CollisionComponent>()->globalBounds()))
            {
                if (master->getWorldPosition().y < xy::DefaultSceneSize.y / 2.f)
                {
                    //move the body to bottom half of screen
                    body->move(0.f, moveDistance);
                }
                else
                {
                    body->move(0.f, -moveDistance);
                }
            }
        };
        checkMaster(body, startBody);
        checkMaster(body, endBody);

        for (auto other : bodies)
        {
            if (body != other &&
                bounds.intersects(other->getComponent<lm::CollisionComponent>()->globalBounds()))
            {
                body->destroy();
                break;
            }
        }
    }

    //create player
    auto controller = xy::Component::create<ph::PlayerController>(m_messageBus);

    auto playerCollision = m_collisionWorld.addComponent(m_messageBus, playerSize, lm::CollisionComponent::ID::Player);
    lm::CollisionComponent::Callback callback = std::bind(&ph::PlayerController::collisionCallback, controller.get(), std::placeholders::_1);
    playerCollision->setCallback(callback);

    auto qtc = xy::Component::create<xy::QuadTreeComponent>(m_messageBus, playerCollision->localBounds());

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::RectangleShape>>(m_messageBus);
    drawable->getDrawable().setSize({ playerSize.width, playerSize.height });
    drawable->getDrawable().setOrigin(playerSize.width / 2.f, playerSize.height / 2.f);
    drawable->getDrawable().setFillColor(sf::Color::Red);

    ent = xy::Entity::create(m_messageBus);
    ent->addComponent(controller);
    ent->addComponent(playerCollision);
    ent->addComponent(qtc);
    ent->addComponent(drawable);
    ent->setPosition(startBody->getWorldPosition() + sf::Vector2f((masterRadius * 1.4f), 0.f));
    m_scene.addEntity(ent, xy::Scene::Layer::FrontMiddle);
}

xy::Entity* PlanetHoppingState::addBody(const sf::Vector2f& position, float radius)
{
    auto cc = m_collisionWorld.addComponent(m_messageBus, { {-radius, -radius}, {radius * 2.f, radius * 2.f} }, lm::CollisionComponent::ID::Body);
    auto qtc = xy::Component::create<xy::QuadTreeComponent>(m_messageBus, cc->localBounds());

    auto drawable = xy::Component::create<xy::SfDrawableComponent<sf::CircleShape>>(m_messageBus);
    drawable->getDrawable().setRadius(radius);
    drawable->getDrawable().setOrigin(radius, radius);


    //TODO fudge this so smaller bodies only orbit larger rather than more realistically
    //affecting each other (else chaos!!).

    auto ent = xy::Entity::create(m_messageBus);
    ent->addComponent(cc);
    ent->addComponent(qtc);
    ent->addComponent(drawable);
    ent->setPosition(position);

    return m_scene.addEntity(ent, xy::Scene::Layer::FrontMiddle);
}

void PlanetHoppingState::updateLoadingScreen(float dt, sf::RenderWindow& rw)
{
    m_loadingSprite.rotate(1440.f * dt);
    rw.draw(m_loadingSprite);
}