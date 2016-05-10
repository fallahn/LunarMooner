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

#include <DemoRecorder.hpp>

#include <ctime>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iomanip>

DemoRecorder::DemoRecorder()
    : m_seed    (0),
    m_ptr       (nullptr),
    m_started   (false)
{

}

//public
sf::Uint32 DemoRecorder::getSeed()
{
    m_seed = static_cast<sf::Uint32>(std::time(nullptr));
    return m_seed;
}

void DemoRecorder::start(const lm::PlayerState& ps, xy::Difficulty diff)
{
    std::size_t size = static_cast<std::size_t>(std::ceil(ps.timeRemaining * 60.f)); //60fps
    size += sizeof(ps);
    size += sizeof(diff);
    size += sizeof(m_seed);

    m_buffer = std::vector<char>(size);
    m_ptr = m_buffer.data();

    std::memcpy(m_ptr, &ps, sizeof(ps));
    m_ptr += sizeof(ps);
    std::memcpy(m_ptr, &diff, sizeof(diff));
    m_ptr += sizeof(diff);
    std::memcpy(m_ptr, &m_seed, sizeof(m_seed));
    m_ptr += sizeof(m_seed);

    m_started = true;
}

void DemoRecorder::recordInput(sf::Uint8 input)
{
    if (m_started)
    {
        *m_ptr = input;
        m_ptr++;
    }
}

void DemoRecorder::stop(bool saveFile)
{
    m_started = false;
    if (saveFile)
    {
        auto time = std::time(nullptr);
        auto tm = *std::localtime(&time);

        std::stringstream ss;
        ss.imbue(std::locale());
        ss << std::put_time(&tm, "%H_%M_%S-%d%m%y.lmd");

        std::string filename = ss.str();

        std::fstream file(filename, std::ios::out | std::ios::binary);
        if (file.is_open() && file.good() && !file.fail())
        {
            file.write(m_buffer.data(), m_ptr - m_buffer.data());
        }
        file.close();
    }
}

//private