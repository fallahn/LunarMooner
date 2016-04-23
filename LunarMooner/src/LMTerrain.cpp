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
    m_level         (0u),
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

void Terrain::init(const std::string& mapDir, xy::TextureResource& tr)
{
    //list map files in dir and shuffle
    auto files = xy::FileSystem::listFiles(mapDir);
    files.erase(std::remove_if(files.begin(), files.end(),
        [](const std::string& str)
    {
        return (xy::FileSystem::getFileExtension(str) != ".lmm");
    }), files.end());

    std::random_shuffle(files.begin(), files.end());

    //load up to 10 of them (g++ requires explicit type)
    std::size_t max = std::min(std::size_t(10u), files.size());
    for (auto i = 0u; i < max; ++i)
    {
        load(mapDir + "/" + files[i], tr);
    }
}

const std::vector<sf::Vector2f>& Terrain::getChain() const
{
    if (!m_chains[m_level].first)
    {
        m_chains[m_level].first = true;
        const auto& tx = m_entity->getTransform();
        for (auto& c : m_chains[m_level].second)
        {
            c = tx.transformPoint(c);
        }
    }
    return m_chains[m_level].second;
}

const std::vector<Terrain::Platform>& Terrain::getPlatforms() const
{
    if (!m_platforms[m_level].first)
    {
        const auto& tx = m_entity->getTransform();
        for (auto& p : m_platforms[m_level].second)
        {
            p.position = tx.transformPoint(p.position);
        }
        m_platforms[m_level].first = true;
    }
    return m_platforms[m_level].second;
}

void Terrain::setLevel(sf::Uint8 level)
{
    level -= 1;
    if (!m_textures.empty())
    {
        m_level = level % m_textures.size();
    }
}

Terrain::WaterData Terrain::getWaterData() const
{
    return WaterData(m_textures[m_level], m_waterLevels[m_level]);
}

//private
void Terrain::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    states.texture = &m_textures[m_level];
    rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
}

bool Terrain::load(const std::string& path, xy::TextureResource& tr)
{


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

    std::vector<sf::Vector2f> points;
    std::vector<Platform> platforms;
    sf::Texture texture;
    float waterLevel = 0.f;

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
                texture = tr.get(mapDir + textureFile);
            }
            else
            {
                xy::Logger::log("No texture name given in map file: " + path, xy::Logger::Type::Warning);
                return false;
            }
        }
        else
        {
            xy::Logger::log("Missing texture property in map file: " + path, xy::Logger::Type::Warning);
            return false;
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
                        platforms.emplace_back();
                        Platform& platform = platforms.back();
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
                    points.push_back(getVecFromObject(v.get<picojson::object>()));
                }
            }
        }

        if (pv.get("WaterLevel").is<double>())
        {
            waterLevel = static_cast<float>(pv.get("WaterLevel").get<double>());
        }
    }
    else
    {
        xy::Logger::log("Map Loader - " + err, xy::Logger::Type::Error, xy::Logger::Output::All);
        return false;
    }

    if (points.empty() || platforms.empty())
    {
        return false;
    }
    m_chains.emplace_back(std::make_pair(false, points));
    m_platforms.emplace_back(std::make_pair(false, platforms));
    m_waterLevels.push_back(waterLevel);
    m_textures.push_back(texture);

    return true;
}
