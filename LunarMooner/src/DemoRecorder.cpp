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

#include <xygine/Log.hpp>
#include <xygine/Assert.hpp>

#include <ctime>
#include <cmath>
#include <cstring>
#include <sstream>
#include <fstream>
#include <iomanip>

DemoRecorder::DemoRecorder()
    : m_seed    (0),
    m_buffer    (18000), //allows up to 300 seconds
    m_ptr       (nullptr),
    m_enabled   (true)
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
    if (!m_enabled) return;
    
    std::size_t size = static_cast<std::size_t>(std::ceil((ps.timeRemaining + 1.f) * 60.f)); //60fps
    size += sizeof(ps);
	auto humansRemainingSize(ps.humansRemaining.size());
	if (!ps.humansRemaining.empty())
	{
		size += sizeof(ps.humansRemaining[0])*humansRemainingSize;
	}
    size += sizeof(diff);
    size += sizeof(m_seed);

    m_ptr = m_buffer.data();

    std::memcpy(m_ptr, &ps.alienCount, sizeof(ps.alienCount));
    m_ptr += sizeof(ps.alienCount);
	std::memcpy(m_ptr, &ps.ammo, sizeof(ps.ammo));
	m_ptr += sizeof(ps.ammo);
	std::memcpy(m_ptr, &ps.cooldownTime, sizeof(ps.cooldownTime));
	m_ptr += sizeof(ps.cooldownTime);
	std::memcpy(m_ptr, &ps.humansSaved, sizeof(ps.humansSaved));
	m_ptr += sizeof(ps.humansSaved);
	std::memcpy(m_ptr, &ps.level, sizeof(ps.level));
	m_ptr += sizeof(ps.level);
	std::memcpy(m_ptr, &ps.lives, sizeof(ps.lives));
	m_ptr += sizeof(ps.lives);
	std::memcpy(m_ptr, &ps.previousScore, sizeof(ps.previousScore));
	m_ptr += sizeof(ps.previousScore);
	std::memcpy(m_ptr, &ps.score, sizeof(ps.score));
	m_ptr += sizeof(ps.score);
	std::memcpy(m_ptr, &ps.special, sizeof(ps.special));
	m_ptr += sizeof(ps.special);
	std::memcpy(m_ptr, &ps.startNewRound, sizeof(ps.startNewRound));
	m_ptr += sizeof(ps.startNewRound);
	std::memcpy(m_ptr, &ps.timeRemaining, sizeof(ps.timeRemaining));
	m_ptr += sizeof(ps.timeRemaining);
	std::memcpy(m_ptr, &humansRemainingSize, sizeof(humansRemainingSize));
	m_ptr += sizeof(humansRemainingSize);
	while (humansRemainingSize--)
	{
		std::memcpy(m_ptr, &ps.humansRemaining[humansRemainingSize], sizeof(ps.humansRemaining[humansRemainingSize]));
		m_ptr += sizeof(ps.humansRemaining[humansRemainingSize]);
	}
    std::memcpy(m_ptr, &diff, sizeof(diff));
    m_ptr += sizeof(diff);
    std::memcpy(m_ptr, &m_seed, sizeof(m_seed));
    m_ptr += sizeof(m_seed);

    LOG("Started Recording Demo", xy::Logger::Type::Info);
}

void DemoRecorder::recordInput(sf::Uint8 input)
{
    //XY_ASSERT(m_started, "Demo recording not enabled");
    if(m_ptr)
    {
        *m_ptr = input;
        m_ptr++;
    }
}

void DemoRecorder::stop(bool saveFile)
{
    if (!m_enabled || !m_ptr) return;
    
    *m_ptr = 0xff; //termination value
    if (saveFile)
    {
        auto time = std::time(nullptr);
        auto tm = *std::localtime(&time);

        std::stringstream ss;
        ss.imbue(std::locale());
        ss << std::put_time(&tm, "%H_%M_%S-%d%m%y.lmd");

        std::string filename = ss.str();

        std::fstream file("test.lmd", std::ios::out | std::ios::binary);
        if (file.is_open() && file.good() && !file.fail())
        {
            file.write(m_buffer.data(), m_ptr - m_buffer.data());
        }
        file.close();

        LOG("Save demo file to " + filename, xy::Logger::Type::Info);
    }

    m_ptr = nullptr;
    m_buffer.clear();

    LOG("Stopped Recording Demo", xy::Logger::Type::Info);
}

//private