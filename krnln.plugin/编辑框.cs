using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using wnxd.E_NET;

namespace krnln.plugin
{
    [LibGuid("d09f2340818511d396f6aaf844c7e325")]
    [LibType(4)]
    class 编辑框 : TextBox, Base, Plugin
    {
        编辑框()
        {
            this.TextChanged += 编辑框_TextChanged;
        }
        void 编辑框_TextChanged(object sender, EventArgs e)
        {
            if (this.内容被改变 != null) this.内容被改变();
        }
        //-继承属性-
        [LibTypeTag(0)]
        public int 左边
        {
            get
            {
                return this.Left;
            }
            set
            {
                this.Left = value;
            }
        }
        [LibTypeTag(1)]
        public int 顶边
        {
            get
            {
                return this.Top;
            }
            set
            {
                this.Top = value;
            }
        }
        [LibTypeTag(2)]
        public int 宽度
        {
            get
            {
                return this.Width;
            }
            set
            {
                this.Width = value;
            }
        }
        [LibTypeTag(3)]
        public int 高度
        {
            get
            {
                return this.Height;
            }
            set
            {
                this.Height = value;
            }
        }
        [LibTypeTag(4)]
        public string 标记 { get; set; }
        [LibTypeTag(5)]
        public bool 可视
        {
            get
            {
                return this.Visible;
            }
            set
            {
                this.Visible = value;
            }
        }
        [LibTypeTag(6)]
        public bool 禁止
        {
            get
            {
                return this.Enabled;
            }
            set
            {
                this.Enabled = value;
            }
        }
        [LibTypeTag(7)]
        public byte[] 鼠标指针 { get; set; }
        //-继承属性-
        //-属性-
        [LibTypeTag(8)]
        public string 内容
        {
            get
            {
                return this.Text;
            }
            set
            {
                this.Text = value;
            }
        }
        [LibTypeTag(9)]
        public int 边框 { get; set; }
        [LibTypeTag(10)]
        public int 文本颜色
        {
            get
            {
                return this.ForeColor.ToArgb();
            }
            set
            {
                if (value != -16777216)
                {
                    if ((value & 0xFF000000) == 0) value = (int)(value + 0xFF000000);
                    this.ForeColor = Color.FromArgb(value);
                }
            }
        }
        [LibTypeTag(11)]
        public int 背景颜色
        {
            get
            {
                return this.BackColor.ToArgb();
            }
            set
            {
                if (value != -16777216)
                {
                    if ((value & 0xFF000000) == 0) value = (int)(value + 0xFF000000);
                    this.BackColor = Color.FromArgb(value);
                }
            }
        }
        [LibTypeTag(12)]
        public byte[] 字体 { get; set; }
        [LibTypeTag(13)]
        public bool 隐藏选择
        {
            get
            {
                return this.HideSelection;
            }
            set
            {
                this.HideSelection = value;
            }
        }
        [LibTypeTag(14)]
        public int 最大允许长度
        {
            get
            {
                return this.MaxLength;
            }
            set
            {
                this.MaxLength = value;
            }
        }
        [LibTypeTag(15)]
        public bool 是否允许多行
        {
            get
            {
                return this.Multiline;
            }
            set
            {
                this.Multiline = value;
            }
        }
        [LibTypeTag(16)]
        public int 滚动条
        {
            get
            {
                return (int)this.ScrollBars;
            }
            set
            {
                this.ScrollBars = (ScrollBars)value;
            }
        }
        [LibTypeTag(17)]
        public int 对齐方式
        {
            get
            {
                switch (this.TextAlign)
                {
                    case HorizontalAlignment.Left:
                        return 0;
                    case HorizontalAlignment.Center:
                        return 1;
                    case HorizontalAlignment.Right:
                        return 2;
                    default:
                        return 0;
                }
            }
            set
            {
                switch (value)
                {
                    case 0:
                        this.TextAlign = HorizontalAlignment.Left;
                        break;
                    case 1:
                        this.TextAlign = HorizontalAlignment.Center;
                        break;
                    case 2:
                        this.TextAlign = HorizontalAlignment.Right;
                        break;
                }
            }
        }
        [LibTypeTag(18)]
        public int 输入方式
        {
            get
            {
                if (this.ReadOnly) return 1;
                else if (this.PasswordChar == null || this.PasswordChar == '\0') return 0;
                else return 2;
            }
            set
            {
                switch (value)
                {
                    case 0:
                        this.ReadOnly = false;
                        this.PasswordChar = '\0';
                        break;
                    case 1:
                        this.ReadOnly = true;
                        this.PasswordChar = '\0';
                        break;
                    case 2:
                        this.ReadOnly = false;
                        this.PasswordChar = this.密码遮盖字符.FirstOrDefault();
                        break;
                    default:
                        this.ReadOnly = false;
                        this.PasswordChar = '\0';
                        break;
                }
            }
        }
        [LibTypeTag(19)]
        public string 密码遮盖字符 { get; set; }
        [LibTypeTag(20)]
        public int 转换方式 { get; set; }
        [LibTypeTag(21)]
        public int 调节器方式 { get; set; }
        [LibTypeTag(22)]
        public int 调节器底限值 { get; set; }
        [LibTypeTag(23)]
        public int 调节器上限值 { get; set; }
        [LibTypeTag(24)]
        public int 起始选择位置
        {
            get
            {
                return this.SelectionStart;
            }
            set
            {
                this.SelectionStart = value;
            }
        }
        [LibTypeTag(25)]
        public int 被选择字符数
        {
            get
            {
                return this.SelectionLength;
            }
            set
            {
                this.SelectionLength = value;
            }
        }
        [LibTypeTag(26)]
        public string 被选择文本
        {
            get
            {
                return this.SelectedText;
            }
            set
            {
                this.SelectedText = value;
            }
        }
        [LibTypeTag(27)]
        public string 数据源 { get; set; }
        [LibTypeTag(28)]
        public string 数据列 { get; set; }
        //-属性-
        //-事件-
        [LibTypeTag(0)]
        public event Action 内容被改变;
        [LibTypeTag(1)]
        public event Action<int> 调节钮被按下;
        //-事件-
    }
}
