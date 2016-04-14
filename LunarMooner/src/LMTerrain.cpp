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

#include <LMTerrain.hpp>

#include <xygine/Entity.hpp>
#include <xygine/Resource.hpp>
#include <xygine/util/Json.hpp>
#include <xygine/FileSystem.hpp>

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

using namespace lm;

namespace
{
    const sf::Vector2f size(1362.f, 320.f);
}

Terrain::Terrain(xy::MessageBus& mb)
    :xy::Component  (mb, this),
    m_transformed   (false),
    m_entity        (nullptr)
{
    m_vertices[1].position.x = size.x;
    m_vertices[1].texCoords = m_vertices[1].position;

    m_vertices[2].position = size;
    m_vertices[2].texCoords = size;

    m_vertices[3].position.y = size.y;
    m_vertices[3].texCoords = m_vertices[3].position;

    m_bounds.width = size.x;
    m_bounds.height = size.y;
}

//public
void Terrain::entityUpdate(xy::Entity&, float)
{

}

void Terrain::onStart(xy::Entity& entity)
{
    m_entity = &entity;
}

bool Terrain::load(const std::string& path, xy::TextureResource& tr)
{
    m_platforms.clear();
    m_chain.clear();
    m_transformed = false;

    //load / parse json file
    std::ifstream file(path);
    if (!file.good() || !xy::Util::File::validLength(file))
    {
        LOG("failed to open " + path + ", or file empty", xy::Logger::Type::Error);
        file.close();
        return false;
    }

    std::string jsonString;
    while (!file.eof())
    {
        std::string temp;
        file >> temp;
        jsonString += temp;
    }
    if (jsonString.empty())
    {
        LOG(path + "failed to read, or file empty", xy::Logger::Type::Error);
        file.close();
        return false;
    }
    file.close();
    
    //function for parsing vector values from objects
    std::function<sf::Vector2f(const picojson::object&)> getVecFromObject = []
        (const picojson::object& o)-> sf::Vector2f
    {
        sf::Vector2f retVal;
        for (const auto& p : o)
        {
            if (p.first == "X")
            {
                retVal.x = (p.second.is<double>()) ?
                    static_cast<float>(p.second.get<double>()) : 0.f;
            }
            else if (p.first == "Y")
            {
                retVal.y = (p.second.is<double>()) ?
                    static_cast<float>(p.second.get<double>()) : 0.f;
            }
        }
        return retVal;
    };

    picojson::value pv;
    auto err = picojson::parse(pv, jsonString);
    if (err.empty())
    {
        if (pv.get("TextureName").is<std::string>())
        {
            std::string textureFile = pv.get("TextureName").get<std::string>();
            if (!textureFile.empty())
            {
                std::string mapDir = xy::FileSystem::getFilePath(path);
                m_texture = tr.get(mapDir + textureFile);
            }
            else
            {
                xy::Logger::log("No texture name given in map file: " + path, xy::Logger::Type::Warning);
            }
        }
        else
        {
            xy::Logger::log("Missing texture property in map file: " + path, xy::Logger::Type::Warning);
        }

        if (pv.get("Platforms").is<picojson::array>())
        {
            const auto& arr = pv.get("Platforms").get<picojson::array>();
            if (!arr.empty())
            {
                for (const auto& v : arr)
                {
                    if (v.is<picojson::object>())
                    {
                        m_platforms.emplace_back();
                        Platform& platform = m_platforms.back();
                        const auto& o = v.get<picojson::object>();
                        for (const auto& property : o)
                        {
                            if (property.first == "Position")
                            {
                                if (property.second.is<picojson::object>())
                                {
                                    platform.position = getVecFromObject(property.second.get<picojson::object>());
                                }
                            }
                            else if (property.first == "Size")
                            {
                                if (property.second.is<picojson::object>())
                                {
                                    platform.size = getVecFromObject(property.second.get<picojson::object>());
                                }
                            }
                            else if (property.first == "Value")
                            {
                                if (property.second.is<double>())
                                {
                                    platform.value = static_cast<sf::Uint16>(property.second.get<double>());
                                }
                            }
                        }
                    }
                }
            }
        }

        if (pv.get("Points").is<picojson::array>())
        {
            const auto& arr = pv.get("Points").get<picojson::array>();
            for (const auto& v : arr)
            {
                if (v.is<picojson::object>())
                {
                    m_chain.push_back(getVecFromObject(v.get<picojson::object>()));
                }
            }
        }
    }
    else
    {
        xy::Logger::log("Map Loader - " + err, xy::Logger::Type::Error, xy::Logger::Output::All);
        return false;
    }

    return true;
}

const std::vector<sf::Vector2f>& Terrain::getChain() const
{
    if (!m_transformed)
    {
        const auto& tx = m_entity->getTransform();
        for (auto& p : m_chain)
        {
            p = tx.transformPoint(p);
        }
        m_transformed = true;
    }
    return m_chain;
}

std::vector<Terrain::Platform> Terrain::getPlatforms() const
{
    std::vector<Platform> platforms(m_platforms.size());

    const auto& tx = m_entity->getTransform();
    for (auto i = 0u; i < m_platforms.size(); ++i)
    {
        platforms[i].position = tx.transformPoint(m_platforms[i].position);
        platforms[i].size = m_platforms[i].size;
        platforms[i].value = m_platforms[i].value;
    }

    return std::move(platforms);
}

//private
void Terrain::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = &m_texture;
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}