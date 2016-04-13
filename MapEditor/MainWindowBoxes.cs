/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

Lunar Mooner Map Editor - Zlib license.

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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Input;

namespace MapEditor
{
    /// <summary>
    /// Contains functions for drawing the boxes in the editor
    /// </summary>
    public partial class MainWindow : Form
    {
        int m_currentBox = -1;
        int m_selectedBox = -1;
        SFML.Window.Vector2f m_boxOffset = new SFML.Window.Vector2f();

        enum BoxInputState
        {
            None,
            Add,
            Move,
            Remove
        }

        private void radioButtonAddBox_CheckedChanged(object sender, EventArgs e)
        {
            m_lineInputState = LineInputState.None;
            if(radioButtonAddBox.Checked)
            {
                m_boxInputState = BoxInputState.Add;
            }
        }

        private void radioButtonMoveBox_CheckedChanged(object sender, EventArgs e)
        {
            m_lineInputState = LineInputState.None;
            if(radioButtonMoveBox.Checked)
            {
                m_boxInputState = BoxInputState.Move;
            }
        }

        private void radioButtonDeleteBox_CheckedChanged(object sender, EventArgs e)
        {
            m_lineInputState = LineInputState.None;
            if(radioButtonDeleteBox.Checked)
            {
                m_boxInputState = BoxInputState.Remove;
            }
        }

        private void numericUpDownWidth_ValueChanged(object sender, EventArgs e)
        {
            if(m_selectedBox  >-1 && m_selectedBox < m_screenBoxes.Count)
            {
                var size = m_screenBoxes[m_selectedBox].Box.Size;
                size.X = (float)numericUpDownWidth.Value;
                m_screenBoxes[m_selectedBox].Box.Size = size;
            }
        }

        private void numericUpDownHeight_ValueChanged(object sender, EventArgs e)
        {
            if (m_selectedBox > -1 && m_selectedBox < m_screenBoxes.Count)
            {
                var size = m_screenBoxes[m_selectedBox].Box.Size;
                size.Y = (float)numericUpDownHeight.Value;
                m_screenBoxes[m_selectedBox].Box.Size = size;
            }
        }

        private void numericUpDownBoxVal_ValueChanged(object sender, EventArgs e)
        {
            if (m_selectedBox > -1 && m_selectedBox < m_screenBoxes.Count)
            {
                m_screenBoxes[m_selectedBox].Value = (int)numericUpDownBoxVal.Value;
            }
        }

        private BoxInputState m_boxInputState = BoxInputState.None;

        public class ScreenBox
        {
            public SFML.Graphics.RectangleShape Box { get; set; }
            public int Value { get; set; }
            public ScreenBox(SFML.Window.Vector2f position)
            {
                Box = new SFML.Graphics.RectangleShape(new SFML.Window.Vector2f(20, 20));
                Box.Position = position;
                Box.FillColor = SFML.Graphics.Color.Green;
                Box.OutlineColor = SFML.Graphics.Color.Red;
            }
        }
        private List<ScreenBox> m_screenBoxes = new List<ScreenBox>();

        public void DrawBoxes(SFML.Graphics.RenderWindow window)
        {
            foreach(var b in m_screenBoxes)
            {
                window.Draw(b.Box);
            }
        }
    }
}