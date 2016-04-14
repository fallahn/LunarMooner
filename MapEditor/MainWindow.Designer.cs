namespace MapEditor
{
    partial class MainWindow
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.quitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.buttonTextureDown = new System.Windows.Forms.Button();
            this.buttonTextureUp = new System.Windows.Forms.Button();
            this.buttonAddTexture = new System.Windows.Forms.Button();
            this.listBoxTextures = new System.Windows.Forms.ListBox();
            this.numericUpDownBoxVal = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.numericUpDownHeight = new System.Windows.Forms.NumericUpDown();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.numericUpDownWidth = new System.Windows.Forms.NumericUpDown();
            this.radioButtonDeleteBox = new System.Windows.Forms.RadioButton();
            this.radioButtonMoveBox = new System.Windows.Forms.RadioButton();
            this.radioButtonAddBox = new System.Windows.Forms.RadioButton();
            this.radioButtonDeletePoint = new System.Windows.Forms.RadioButton();
            this.radioButtonMovePoint = new System.Windows.Forms.RadioButton();
            this.radioButtonAddPoint = new System.Windows.Forms.RadioButton();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
            this.splitContainer2.Panel1.SuspendLayout();
            this.splitContainer2.Panel2.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownBoxVal)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHeight)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownWidth)).BeginInit();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.menuStrip1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
            this.splitContainer1.Size = new System.Drawing.Size(1156, 541);
            this.splitContainer1.SplitterDistance = 340;
            this.splitContainer1.TabIndex = 0;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1156, 24);
            this.menuStrip1.TabIndex = 0;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.openToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.saveAsToolStripMenuItem,
            this.quitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(36, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.N)));
            this.newToolStripMenuItem.Size = new System.Drawing.Size(202, 22);
            this.newToolStripMenuItem.Text = "New";
            this.newToolStripMenuItem.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
            this.openToolStripMenuItem.Size = new System.Drawing.Size(202, 22);
            this.openToolStripMenuItem.Text = "Open";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.S)));
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(202, 22);
            this.saveToolStripMenuItem.Text = "Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Shift) 
            | System.Windows.Forms.Keys.S)));
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(202, 22);
            this.saveAsToolStripMenuItem.Text = "Save As...";
            this.saveAsToolStripMenuItem.Click += new System.EventHandler(this.saveAsToolStripMenuItem_Click);
            // 
            // quitToolStripMenuItem
            // 
            this.quitToolStripMenuItem.Name = "quitToolStripMenuItem";
            this.quitToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Q)));
            this.quitToolStripMenuItem.Size = new System.Drawing.Size(202, 22);
            this.quitToolStripMenuItem.Text = "Quit";
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            // 
            // splitContainer2.Panel1
            // 
            this.splitContainer2.Panel1.Controls.Add(this.buttonTextureDown);
            this.splitContainer2.Panel1.Controls.Add(this.buttonTextureUp);
            this.splitContainer2.Panel1.Controls.Add(this.buttonAddTexture);
            this.splitContainer2.Panel1.Controls.Add(this.listBoxTextures);
            // 
            // splitContainer2.Panel2
            // 
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownBoxVal);
            this.splitContainer2.Panel2.Controls.Add(this.label3);
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownHeight);
            this.splitContainer2.Panel2.Controls.Add(this.label2);
            this.splitContainer2.Panel2.Controls.Add(this.label1);
            this.splitContainer2.Panel2.Controls.Add(this.numericUpDownWidth);
            this.splitContainer2.Panel2.Controls.Add(this.radioButtonDeleteBox);
            this.splitContainer2.Panel2.Controls.Add(this.radioButtonMoveBox);
            this.splitContainer2.Panel2.Controls.Add(this.radioButtonAddBox);
            this.splitContainer2.Panel2.Controls.Add(this.radioButtonDeletePoint);
            this.splitContainer2.Panel2.Controls.Add(this.radioButtonMovePoint);
            this.splitContainer2.Panel2.Controls.Add(this.radioButtonAddPoint);
            this.splitContainer2.Size = new System.Drawing.Size(1156, 197);
            this.splitContainer2.SplitterDistance = 206;
            this.splitContainer2.TabIndex = 0;
            // 
            // buttonTextureDown
            // 
            this.buttonTextureDown.Location = new System.Drawing.Point(165, 32);
            this.buttonTextureDown.Name = "buttonTextureDown";
            this.buttonTextureDown.Size = new System.Drawing.Size(22, 23);
            this.buttonTextureDown.TabIndex = 3;
            this.buttonTextureDown.Text = "D";
            this.buttonTextureDown.UseVisualStyleBackColor = true;
            // 
            // buttonTextureUp
            // 
            this.buttonTextureUp.Location = new System.Drawing.Point(165, 3);
            this.buttonTextureUp.Name = "buttonTextureUp";
            this.buttonTextureUp.Size = new System.Drawing.Size(22, 23);
            this.buttonTextureUp.TabIndex = 2;
            this.buttonTextureUp.Text = "U";
            this.buttonTextureUp.UseVisualStyleBackColor = true;
            // 
            // buttonAddTexture
            // 
            this.buttonAddTexture.Location = new System.Drawing.Point(3, 156);
            this.buttonAddTexture.Name = "buttonAddTexture";
            this.buttonAddTexture.Size = new System.Drawing.Size(156, 23);
            this.buttonAddTexture.TabIndex = 1;
            this.buttonAddTexture.Text = "Add Texture";
            this.buttonAddTexture.UseVisualStyleBackColor = true;
            this.buttonAddTexture.Click += new System.EventHandler(this.buttonAddTexture_Click);
            // 
            // listBoxTextures
            // 
            this.listBoxTextures.FormattingEnabled = true;
            this.listBoxTextures.Location = new System.Drawing.Point(3, 3);
            this.listBoxTextures.Name = "listBoxTextures";
            this.listBoxTextures.Size = new System.Drawing.Size(156, 147);
            this.listBoxTextures.TabIndex = 0;
            // 
            // numericUpDownBoxVal
            // 
            this.numericUpDownBoxVal.Increment = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDownBoxVal.Location = new System.Drawing.Point(190, 130);
            this.numericUpDownBoxVal.Minimum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDownBoxVal.Name = "numericUpDownBoxVal";
            this.numericUpDownBoxVal.Size = new System.Drawing.Size(53, 20);
            this.numericUpDownBoxVal.TabIndex = 11;
            this.numericUpDownBoxVal.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDownBoxVal.ValueChanged += new System.EventHandler(this.numericUpDownBoxVal_ValueChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(163, 132);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(17, 13);
            this.label3.TabIndex = 10;
            this.label3.Text = "V:";
            // 
            // numericUpDownHeight
            // 
            this.numericUpDownHeight.Location = new System.Drawing.Point(190, 104);
            this.numericUpDownHeight.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownHeight.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownHeight.Name = "numericUpDownHeight";
            this.numericUpDownHeight.Size = new System.Drawing.Size(52, 20);
            this.numericUpDownHeight.TabIndex = 9;
            this.numericUpDownHeight.Value = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.numericUpDownHeight.ValueChanged += new System.EventHandler(this.numericUpDownHeight_ValueChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(163, 106);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(18, 13);
            this.label2.TabIndex = 8;
            this.label2.Text = "H:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(163, 80);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(21, 13);
            this.label1.TabIndex = 7;
            this.label1.Text = "W:";
            // 
            // numericUpDownWidth
            // 
            this.numericUpDownWidth.Location = new System.Drawing.Point(190, 78);
            this.numericUpDownWidth.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDownWidth.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownWidth.Name = "numericUpDownWidth";
            this.numericUpDownWidth.Size = new System.Drawing.Size(52, 20);
            this.numericUpDownWidth.TabIndex = 6;
            this.numericUpDownWidth.Value = new decimal(new int[] {
            50,
            0,
            0,
            0});
            this.numericUpDownWidth.ValueChanged += new System.EventHandler(this.numericUpDownWidth_ValueChanged);
            // 
            // radioButtonDeleteBox
            // 
            this.radioButtonDeleteBox.AutoSize = true;
            this.radioButtonDeleteBox.Location = new System.Drawing.Point(166, 55);
            this.radioButtonDeleteBox.Name = "radioButtonDeleteBox";
            this.radioButtonDeleteBox.Size = new System.Drawing.Size(77, 17);
            this.radioButtonDeleteBox.TabIndex = 5;
            this.radioButtonDeleteBox.TabStop = true;
            this.radioButtonDeleteBox.Text = "Delete Box";
            this.radioButtonDeleteBox.UseVisualStyleBackColor = true;
            this.radioButtonDeleteBox.CheckedChanged += new System.EventHandler(this.radioButtonDeleteBox_CheckedChanged);
            // 
            // radioButtonMoveBox
            // 
            this.radioButtonMoveBox.AutoSize = true;
            this.radioButtonMoveBox.Location = new System.Drawing.Point(166, 32);
            this.radioButtonMoveBox.Name = "radioButtonMoveBox";
            this.radioButtonMoveBox.Size = new System.Drawing.Size(73, 17);
            this.radioButtonMoveBox.TabIndex = 4;
            this.radioButtonMoveBox.TabStop = true;
            this.radioButtonMoveBox.Text = "Move Box";
            this.radioButtonMoveBox.UseVisualStyleBackColor = true;
            this.radioButtonMoveBox.CheckedChanged += new System.EventHandler(this.radioButtonMoveBox_CheckedChanged);
            // 
            // radioButtonAddBox
            // 
            this.radioButtonAddBox.AutoSize = true;
            this.radioButtonAddBox.Location = new System.Drawing.Point(166, 8);
            this.radioButtonAddBox.Name = "radioButtonAddBox";
            this.radioButtonAddBox.Size = new System.Drawing.Size(65, 17);
            this.radioButtonAddBox.TabIndex = 3;
            this.radioButtonAddBox.TabStop = true;
            this.radioButtonAddBox.Text = "Add Box";
            this.radioButtonAddBox.UseVisualStyleBackColor = true;
            this.radioButtonAddBox.CheckedChanged += new System.EventHandler(this.radioButtonAddBox_CheckedChanged);
            // 
            // radioButtonDeletePoint
            // 
            this.radioButtonDeletePoint.AutoSize = true;
            this.radioButtonDeletePoint.Location = new System.Drawing.Point(12, 55);
            this.radioButtonDeletePoint.Name = "radioButtonDeletePoint";
            this.radioButtonDeletePoint.Size = new System.Drawing.Size(83, 17);
            this.radioButtonDeletePoint.TabIndex = 2;
            this.radioButtonDeletePoint.TabStop = true;
            this.radioButtonDeletePoint.Text = "Delete Point";
            this.radioButtonDeletePoint.UseVisualStyleBackColor = true;
            this.radioButtonDeletePoint.CheckedChanged += new System.EventHandler(this.radioButtonDeletePoint_CheckedChanged);
            // 
            // radioButtonMovePoint
            // 
            this.radioButtonMovePoint.AutoSize = true;
            this.radioButtonMovePoint.Location = new System.Drawing.Point(12, 32);
            this.radioButtonMovePoint.Name = "radioButtonMovePoint";
            this.radioButtonMovePoint.Size = new System.Drawing.Size(79, 17);
            this.radioButtonMovePoint.TabIndex = 1;
            this.radioButtonMovePoint.TabStop = true;
            this.radioButtonMovePoint.Text = "Move Point";
            this.radioButtonMovePoint.UseVisualStyleBackColor = true;
            this.radioButtonMovePoint.CheckedChanged += new System.EventHandler(this.radioButtonMovePoint_CheckedChanged);
            // 
            // radioButtonAddPoint
            // 
            this.radioButtonAddPoint.AutoSize = true;
            this.radioButtonAddPoint.Location = new System.Drawing.Point(12, 9);
            this.radioButtonAddPoint.Name = "radioButtonAddPoint";
            this.radioButtonAddPoint.Size = new System.Drawing.Size(71, 17);
            this.radioButtonAddPoint.TabIndex = 0;
            this.radioButtonAddPoint.TabStop = true;
            this.radioButtonAddPoint.Text = "Add Point";
            this.radioButtonAddPoint.UseVisualStyleBackColor = true;
            this.radioButtonAddPoint.CheckedChanged += new System.EventHandler(this.radioButtonAddPoint_CheckedChanged);
            // 
            // MainWindow
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1156, 541);
            this.Controls.Add(this.splitContainer1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "MainWindow";
            this.Text = "Map Editor - Lunar Mooner";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel1.PerformLayout();
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.splitContainer2.Panel1.ResumeLayout(false);
            this.splitContainer2.Panel2.ResumeLayout(false);
            this.splitContainer2.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
            this.splitContainer2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownBoxVal)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownHeight)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownWidth)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem quitToolStripMenuItem;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.Button buttonAddTexture;
        private System.Windows.Forms.ListBox listBoxTextures;
        private System.Windows.Forms.Button buttonTextureDown;
        private System.Windows.Forms.Button buttonTextureUp;
        private System.Windows.Forms.RadioButton radioButtonDeletePoint;
        private System.Windows.Forms.RadioButton radioButtonMovePoint;
        private System.Windows.Forms.RadioButton radioButtonAddPoint;
        private System.Windows.Forms.RadioButton radioButtonDeleteBox;
        private System.Windows.Forms.RadioButton radioButtonMoveBox;
        private System.Windows.Forms.RadioButton radioButtonAddBox;
        private System.Windows.Forms.NumericUpDown numericUpDownHeight;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown numericUpDownWidth;
        private System.Windows.Forms.NumericUpDown numericUpDownBoxVal;
        private System.Windows.Forms.Label label3;
    }
}

