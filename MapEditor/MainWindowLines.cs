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
    /// Contains functions for drawing the lines in the editor
    /// </summary>
    public partial class MainWindow : Form
    {
        private bool m_mouseDown = false;
        private int m_currentHandle = -1;

        enum LineInputState
        {
            None,
            Add,
            Move,
            Remove
        }
        private LineInputState m_lineInputState = LineInputState.None;

        private void radioButtonAddPoint_CheckedChanged(object sender, EventArgs e)
        {
            if(radioButtonAddPoint.Checked)
            {
                m_lineInputState = LineInputState.Add;
                m_boxInputState = BoxInputState.None;
            }
        }

        private void radioButtonMovePoint_CheckedChanged(object sender, EventArgs e)
        {
            if(radioButtonMovePoint.Checked)
            {
                m_lineInputState = LineInputState.Move;
                m_boxInputState = BoxInputState.None;
            }
        }

        private void radioButtonDeletePoint_CheckedChanged(object sender, EventArgs e)
        {
            if(radioButtonDeletePoint.Checked)
            {
                m_lineInputState = LineInputState.Remove;
                m_boxInputState = BoxInputState.None;
            }
        }

        private void sfmlControl_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                m_mouseDown = false;

                var sfPosition = m_sfmlControl.MouseWorldPosition;
                if (sfPosition.X > 0 && sfPosition.X < m_maxTextureSize.X
                    && sfPosition.Y > 0 && sfPosition.Y < m_maxTextureSize.Y)
                {
                    switch (m_lineInputState)
                    {
                        case LineInputState.None:
                        default: break;
                        case LineInputState.Add:
                            m_screenPoints.Add(new ScreenPoint(sfPosition));
                            break;
                        case LineInputState.Move:
                            m_currentHandle = -1;
                            break;
                        case LineInputState.Remove:
                            int index = -1;
                            for (var i = 0; i < m_screenPoints.Count; ++i)
                            {
                                if(m_screenPoints[i].Handle.GetGlobalBounds().Contains(sfPosition.X, sfPosition.Y))
                                {
                                    index = i;
                                    break;
                                }
                            }
                            if(index != -1)
                            {
                                m_screenPoints.RemoveAt(index);
                            }
                            break;
                    }

                    switch(m_boxInputState)
                    {
                        case BoxInputState.None:
                        default: break;
                        case BoxInputState.Add:
                            var box = new ScreenBox(sfPosition);
                            box.Box.Size = new SFML.Window.Vector2f((float)numericUpDownWidth.Value, (float)numericUpDownHeight.Value);
                            box.Value = (int)numericUpDownBoxVal.Value;
                            m_screenBoxes.Add(box);
                            break;
                        case BoxInputState.Move:
                            m_currentBox = -1;
                            break;
                        case BoxInputState.Remove:
                            int idx = -1;
                            for (var i = 0; i < m_screenBoxes.Count; ++i)
                            {
                                if(m_screenBoxes[i].Box.GetGlobalBounds().Contains(sfPosition.X, sfPosition.Y))
                                {
                                    idx = i;
                                    break;
                                }
                            }
                            if(idx != -1)
                            {
                                m_screenBoxes.RemoveAt(idx);
                            }
                            break;
                    }
                }
            }
        }

        private void sfmlControl_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                m_mouseDown = true;
                var mousePos = m_sfmlControl.MouseWorldPosition;
                if(m_lineInputState == LineInputState.Move)
                {
                    for(var i = 0; i < m_screenPoints.Count; ++i)
                    {
                        if (m_screenPoints[i].Handle.GetGlobalBounds().Contains(mousePos.X, mousePos.Y))
                        {
                            m_currentHandle = i;
                            break;
                        }
                    }
                }
                else if(m_boxInputState == BoxInputState.Move)
                {
                    m_selectedBox = -1;
                    for(var i = 0; i < m_screenBoxes.Count; ++i)
                    {
                        if (m_screenBoxes[i].Box.GetGlobalBounds().Contains(mousePos.X, mousePos.Y))
                        {
                            m_currentBox = i;
                            m_boxOffset = m_screenBoxes[i].Box.Position - mousePos;
                            m_screenBoxes[i].Box.OutlineThickness = 2;
                            m_selectedBox = i;

                            var size = m_screenBoxes[i].Box.Size;
                            numericUpDownWidth.Value = (decimal)size.X;
                            numericUpDownHeight.Value = (decimal)size.Y;
                            numericUpDownBoxVal.Value = m_screenBoxes[i].Value;
                        }
                        else
                        {
                            m_screenBoxes[i].Box.OutlineThickness = 0;
                        }
                    }
                }
            }
        }

        private void sfmlControl_MouseMove(object sender, MouseEventArgs e)
        {
            if (m_mouseDown)
            {
                if (m_lineInputState == LineInputState.Move
                    && m_currentHandle < m_screenPoints.Count
                    && m_currentHandle > -1)
                {
                    m_screenPoints[m_currentHandle].Position = m_sfmlControl.MouseWorldPosition;
                }
                else if(m_boxInputState == BoxInputState.Move
                    && m_currentBox < m_screenBoxes.Count
                    && m_currentBox > -1)
                {
                    var mousePos = m_sfmlControl.MouseWorldPosition;
                    m_screenBoxes[m_currentBox].Box.Position = mousePos + m_boxOffset;
                }
            }
        }

        public class ScreenPoint
        {
            private SFML.Window.Vector2f m_position = new SFML.Window.Vector2f();
            public SFML.Window.Vector2f Position
            {
                get
                {
                    return m_position;
                }

                set
                {
                    m_position = value;
                    Handle.Position = value;
                }
            }
            public SFML.Graphics.RectangleShape Handle { get; set; }
            public ScreenPoint(SFML.Window.Vector2f position)
            {
                Handle = new SFML.Graphics.RectangleShape(new SFML.Window.Vector2f(6, 6));
                Handle.FillColor = SFML.Graphics.Color.Transparent;
                Handle.OutlineColor = SFML.Graphics.Color.Yellow;
                Handle.OutlineThickness = 1f;
                Handle.Origin = new SFML.Window.Vector2f(3, 3);

                Position = position;
            }
        }
        private List<ScreenPoint> m_screenPoints = new List<ScreenPoint>();

        private SFML.Graphics.Vertex[] m_vertices;
        public void UpdateLines(float dt)
        {
            m_vertices = new SFML.Graphics.Vertex[m_screenPoints.Count];
            for(var i = 0; i < m_vertices.Length; ++i)
            {
                m_vertices[i].Position = m_screenPoints[i].Position;
                m_vertices[i].Color = SFML.Graphics.Color.Red;
            }
        }

        public void DrawLines(SFML.Graphics.RenderWindow window)
        {
            //draw vertex array
            window.Draw(m_vertices, SFML.Graphics.PrimitiveType.LinesStrip);

            //draw points
            foreach(var p in m_screenPoints)
            {
                window.Draw(p.Handle);
            }
        }
    }
}
