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

#ifndef LM_DEMO_PLAYER_HPP_
#define LM_DEMO_PLAYER_HPP_

#include <LMPlayerState.hpp>

#include <xygine/Difficulty.hpp>

#include <string>
#include <vector>

class DemoPlayer final
{
public:
    DemoPlayer();
    ~DemoPlayer() = default;
    DemoPlayer(const DemoPlayer&) = delete;
    DemoPlayer& operator = (const DemoPlayer&) = delete;

    bool loadDemo(const std::string&);
    const lm::PlayerState& getPlayerState() const { return m_state; }
    sf::Uint32 getSeed() const { return m_seed; }
    xy::Difficulty getDifficulty() const { return m_difficulty; }
    sf::Uint8 getNextInput();
	bool isPlaying();

private:

    lm::PlayerState m_state;
    sf::Uint32 m_seed;
    xy::Difficulty m_difficulty;
    std::vector<char> m_inputData;
    char* m_ptr;
	bool m_playing = false;
};


#endif //LM_DEMO_PLAYER_HPP_