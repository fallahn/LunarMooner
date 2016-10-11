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

#ifndef LM_MAP_MANAGER_HPP_
#define LM_MAP_MANAGER_HPP_

#include <ResourceCollection.hpp>

#include <xygine/components/Component.hpp>

#include <map>

namespace xy
{
    class MeshRenderer;
}

namespace lm
{
    /*
    Load up to 10 random map files from directory and store arrays
    of map data (platforms, prop positions etc) which can be returned
    to the game manager for the requested level.
    */
    class MapManager final : public xy::Component
    {
        using MaterialMap = std::map<std::uint32_t, std::pair<std::string, std::vector<std::uint32_t>>>;
    public:
        struct Platform final
        {
            sf::Vector2f size;
            sf::Vector2f position;
            sf::Uint16 value = 0;
        };
        
        MapManager(xy::MessageBus&, MaterialMap&, ResourceCollection&, xy::MeshRenderer&);
        ~MapManager() = default;

        xy::Component::Type type() const override { return xy::Component::Type::Script; }
        void entityUpdate(xy::Entity&, float) override;
        void onStart(xy::Entity&) override;

        const std::vector<Platform>& getPlatformData() const;
        const std::vector<sf::Vector2f>& getChains() const;

        void setLevel(sf::Uint8);

    private:
        MaterialMap& m_materialMap;
        ResourceCollection& m_resources;
        xy::Entity* m_entity;
        xy::MeshRenderer& m_meshRenderer;
        sf::Uint8 m_level;

        struct PropData final
        {
            std::uint32_t modelID;
            float rotation;
            sf::Vector2f position;
            sf::Vector2f scale;
        };
        std::vector<std::vector<PropData>> m_propData; //< array of prop data for each loaded level

        std::vector<std::vector<Platform>> m_platforms;
        std::vector<std::vector<sf::Vector2f>> m_chains;

        void loadMap(const std::string&);
        void updateScene();
    };
}

#endif // LM_MAP_MANAGER_HPP_