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

#include <DemoPlayer.hpp>

#include <xygine/Log.hpp>

#include <fstream>
#include <cstring>

DemoPlayer::DemoPlayer()
    : m_seed    (0),
    m_difficulty(xy::Difficulty::Easy),
    m_inputData (18000),
    m_ptr       (nullptr)
{

}

//public
bool DemoPlayer::loadDemo(const std::string& path)
{
    std::fstream file(path, std::ios::in | std::ios::binary);

    if (!file.good() || !file.is_open() || file.fail())
    {
        LOG("Failed opening demo " + path, xy::Logger::Type::Error);

        m_ptr = nullptr;
        file.close();
        return false;
    }

    file.seekg(0, std::ios::end);
    int fileSize = static_cast<int>(file.tellg());
    file.seekg(0, std::ios::beg);

    if (fileSize < static_cast<int>(sizeof(lm::PlayerState)))
    {
        LOG("Unexpected demo file size", xy::Logger::Type::Error);
        file.close();
        return false;
    }

    file.read(m_inputData.data(), fileSize);
    file.close();

    m_ptr = m_inputData.data();
    std::memcpy(&m_state, m_ptr, sizeof(m_state));
    m_ptr += sizeof(m_state);
    std::memcpy(&m_difficulty, m_ptr, sizeof(m_difficulty));
    m_ptr += sizeof(m_difficulty);
    std::memcpy(&m_seed, m_ptr, sizeof(m_seed));
    m_ptr += sizeof(m_seed);
    return true;
}

sf::Uint8 DemoPlayer::getNextInput()
{
    return (m_ptr) ? *m_ptr++ : 0xff;
}