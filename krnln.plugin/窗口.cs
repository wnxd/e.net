using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using wnxd.E_NET;

namespace krnln.plugin
{
    [LibGuid("d09f2340818511d396f6aaf844c7e325")]
    [LibType(0)]
    class 窗口 : Form, Base, Plugin
    {
        public 窗口()
        {
            this.Load += 窗口_Load;
            this.FormClosing += 窗口_FormClosing;
        }
        void 窗口_Load(object sender, EventArgs e)
        {
            if (this.创建完毕 != null) this.创建完毕();
        }
        void 窗口_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (this.可否被关闭 != null) e.Cancel = !this.可否被关闭();
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
        public string 标题
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
        public int 边框
        {
            get
            {
                switch (this.FormBorderStyle)
                {
                    case FormBorderStyle.Fixed3D:
                        return 7;
                    case FormBorderStyle.FixedDialog:
                        return 6;
                    case FormBorderStyle.FixedSingle:
                        return 2;
                    case FormBorderStyle.FixedToolWindow:
                        return 4;
                    case FormBorderStyle.None:
                        return 0;
                    case FormBorderStyle.Sizable:
                        return 1;
                    case FormBorderStyle.SizableToolWindow:
                        return 3;
                    default:
                        return 0;
                }
            }
            set
            {
                switch (value)
                {
                    case 0:
                        this.FormBorderStyle = FormBorderStyle.None;
                        break;
                    case 1:
                        this.FormBorderStyle = FormBorderStyle.Sizable;
                        break;
                    case 2:
                        this.FormBorderStyle = FormBorderStyle.FixedSingle;
                        break;
                    case 3:
                        this.FormBorderStyle = FormBorderStyle.SizableToolWindow;
                        break;
                    case 4:
                        this.FormBorderStyle = FormBorderStyle.FixedToolWindow;
                        break;
                    case 5:
                        break;
                    case 6:
                        this.FormBorderStyle = FormBorderStyle.FixedDialog;
                        break;
                    case 7:
                        this.FormBorderStyle = FormBorderStyle.Fixed3D;
                        break;
                }
            }
        }
        [LibTypeTag(10)]
        public int 底色
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
        [LibTypeTag(11)]
        public byte[] 底图
        {
            get
            {
                using (MemoryStream ms = new MemoryStream())
                {
                    this.BackgroundImage.Save(ms, this.BackgroundImage.RawFormat);
                    byte[] buffer = new byte[ms.Length];
                    ms.Seek(0, SeekOrigin.Begin);
                    ms.Read(buffer, 0, buffer.Length);
                    return buffer;
                }
            }
            set
            {
                if (value != null && value.Length > 0)
                {
                    MemoryStream ms = new MemoryStream(value);
                    this.BackgroundImage.Dispose();
                    this.BackgroundImage = Image.FromStream(ms);
                }
            }
        }
        [LibTypeTag(12)]
        public int 底图方式
        {
            get
            {
                return (int)this.BackgroundImageLayout;
            }
            set
            {
                this.BackgroundImageLayout = (ImageLayout)value;
            }
        }
        [LibTypeTag(13)]
        public byte[] 背景音乐 { get; set; }
        [LibTypeTag(14)]
        public int 播放次数 { get; set; }
        [LibTypeTag(15)]
        public bool 控制按钮
        {
            get
            {
                return this.ControlBox;
            }
            set
            {
                this.ControlBox = value;
            }
        }
        [LibTypeTag(16)]
        public bool 最大化按钮
        {
            get
            {
                return this.MaximizeBox;
            }
            set
            {
                this.MaximizeBox = value;
            }
        }
        [LibTypeTag(17)]
        public bool 最小化按钮
        {
            get
            {
                return this.MinimizeBox;
            }
            set
            {
                this.MinimizeBox = value;
            }
        }
        [LibTypeTag(18)]
        public int 位置
        {
            get
            {
                switch (this.WindowState)
                {
                    case FormWindowState.Maximized:
                        return 3;
                    case FormWindowState.Minimized:
                        return 2;
                    case FormWindowState.Normal:
                        return 0;
                    default:
                        return 0;
                }
            }
            set
            {
                switch (value)
                {
                    case 0:
                        this.WindowState = FormWindowState.Normal;
                        break;
                    case 1:
                        break;
                    case 2:
                        this.WindowState = FormWindowState.Minimized;
                        break;
                    case 3:
                        this.WindowState = FormWindowState.Maximized;
                        break;
                }
            }
        }
        [LibTypeTag(19)]
        public bool 可否移动 { get; set; }
        [LibTypeTag(20)]
        public byte[] 图标
        {
            get
            {
                using (MemoryStream ms = new MemoryStream())
                {
                    this.Icon.Save(ms);
                    byte[] buffer = new byte[ms.Length];
                    ms.Seek(0, SeekOrigin.Begin);
                    ms.Read(buffer, 0, buffer.Length);
                    return buffer;
                }
            }
            set
            {
                if (value != null && value.Length > 0)
                {
                    using (MemoryStream ms = new MemoryStream(value))
                    {
                        this.Icon.Dispose();
                        this.Icon = Icon.FromHandle(new Bitmap(ms).GetHicon());
                    }
                }
            }
        }
        [LibTypeTag(21)]
        public bool 回车下移焦点 { get; set; }
        [LibTypeTag(22)]
        public bool Esc键关闭 { get; set; }
        [LibTypeTag(23)]
        public bool F1键打开帮助 { get; set; }
        [LibTypeTag(24)]
        public string 帮助文件名 { get; set; }
        [LibTypeTag(25)]
        public int 帮助标志值 { get; set; }
        [LibTypeTag(26)]
        public bool 在任务条中显示
        {
            get
            {
                return this.ShowInTaskbar;
            }
            set
            {
                this.ShowInTaskbar = value;
            }
        }
        [LibTypeTag(27)]
        public bool 随意移动 { get; set; }
        [LibTypeTag(28)]
        public int 外形 { get; set; }
        [LibTypeTag(29)]
        public bool 总在最前
        {
            get
            {
                return this.TopMost;
            }
            set
            {
                this.TopMost = value;
            }
        }
        [LibTypeTag(30)]
        public bool 保持标题条激活 { get; set; }
        [LibTypeTag(31)]
        public string 窗口类名 { get; set; }
        //-属性-
        //-方法-
        [LibTypeTag((uint)krnln_method.取窗口句柄)]
        public int 取窗口句柄()
        {
            return this.Handle.ToInt32();
        }
        //-方法-
        //-事件-
        [LibTypeTag(0)]
        public event Action 创建完毕;
        [LibTypeTag(1)]
        public event Func<bool> 可否被关闭;
        [LibTypeTag(2)]
        public event Action 将被销毁;
        [LibTypeTag(3)]
        public event Action 位置被改变;
        [LibTypeTag(4)]
        public event Action 尺寸被改变;
        [LibTypeTag(5)]
        public event Action 被激活;
        [LibTypeTag(6)]
        public event Action 被取消激活;
        [LibTypeTag(7)]
        public event Action 空闲;
        [LibTypeTag(8)]
        public event Action 首次激活;
        [LibTypeTag(9)]
        public event Action 托盘事件;
        [LibTypeTag(10)]
        public event Action 被显示;
        [LibTypeTag(11)]
        public event Action 被隐藏;
        //-事件-
    }
}
