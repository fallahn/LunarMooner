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

#ifndef LM_EDITOR_STATE_HPP_
#define LM_EDITOR_STATE_HPP_

#include <StateIds.hpp>
#include <ResourceCollection.hpp>
#include <LESelectableCollection.hpp>

#include <xygine/State.hpp>
#include <xygine/Scene.hpp>
#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/State.hpp>

#include <vector>

namespace xy
{
    class MessageBus;
}

class EditorState final : public xy::State
{
public:
    EditorState(xy::StateStack&, Context);
    ~EditorState();

    bool handleEvent(const sf::Event&) override;
    void handleMessage(const xy::Message&) override;
    bool update(float) override;
    void draw() override;

    xy::StateID stateID() const override { return States::ID::LevelEditor; }

private:

    xy::MessageBus& m_messageBus;
    xy::Scene m_scene;
    xy::MeshRenderer m_meshRenderer;

    ResourceCollection m_resources;

    //TODO convert to array
    le::SelectableItem* m_selectedItem;
    std::vector<std::unique_ptr<le::SelectableCollection>> m_collections;
    bool m_hasClicked;

    void loadMeshes();
    void buildScene();

    //TODO remove this and mesh caching
    void spawnDeadGuy(float, float, const sf::Vector2f&);

    void addWindows();
};

#endif //LM_EDITOR_STATE_HPP_