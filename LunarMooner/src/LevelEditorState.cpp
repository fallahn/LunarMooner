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

#include <LevelEditorState.hpp>

#include <xygine/App.hpp>

#include <SFML/Window/Event.hpp>

namespace
{

}

EditorState::EditorState(xy::StateStack& stack, Context context)
    : xy::State     (stack, context),
    m_messageBus    (context.appInstance.getMessageBus())
{
    launchLoadingScreen();
    context.renderWindow.setView(context.defaultView);

    loadMeshes();

    quitLoadingScreen();
}

//public
bool EditorState::handleEvent(const sf::Event & evt)
{

    return false;
}

void EditorState::handleMessage(const xy::Message& msg)
{

}

bool EditorState::update(float dt)
{
    return false;
}

void EditorState::draw()
{
    auto& rw = getContext().renderWindow;

}

//private
void EditorState::loadMeshes()
{
    //temp stuff to see how new models lay out
    /*for (auto i = 0u; i < 3u; ++i)
    {
        auto rockWall = m_meshRenderer.createModel(Mesh::ID::RockWall01, getMessageBus());
        rockWall->setPosition({ 0.f, 0.f, -340.f + xy::Util::Random::value(-50.f, 50.f) });
        rockWall->setScale({ 2.2f, xy::Util::Random::value(1.5f, 1.9f), 1.8f });
        rockWall->rotate(xy::Model::Axis::Y, xy::Util::Random::value(-10.f, 10.f));
        auto& material = m_resources.materialResource.get(Material::ID::RockWall01);
        rockWall->setBaseMaterial(material);
        entity = xy::Entity::create(getMessageBus());
        entity->setPosition(alienArea.left + (rockWall->getMesh().getBoundingBox().asFloatRect().width / 2.f) + (i * 520.f), xy::DefaultSceneSize.y);
        entity->addComponent(rockWall);
        m_scene.addEntity(entity, xy::Scene::Layer::FrontRear);
    }*/
}