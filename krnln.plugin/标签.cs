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
    [LibType(9)]
    class 标签 : Label, Base, Plugin
    {
        protected override void WndProc(ref Message m)
        {
            if (this.反馈事件 != null) m.Result = (IntPtr)this.反馈事件((int)m.LParam, (int)m.WParam);
            else base.WndProc(ref m);
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
        public int 效果 { get; set; }
        [LibTypeTag(10)]
        public int 边框 { get; set; }
        [LibTypeTag(11)]
        public int 渐变边框宽度 { get; set; }
        [LibTypeTag(12)]
        public int 渐变边框颜色1 { get; set; }
        [LibTypeTag(13)]
        public int 渐变边框颜色2 { get; set; }
        [LibTypeTag(14)]
        public int 渐变边框颜色3 { get; set; }
        [LibTypeTag(15)]
        public byte[] 字体 { get; set; }
        [LibTypeTag(16)]
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
        [LibTypeTag(17)]
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
        [LibTypeTag(18)]
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
        [LibTypeTag(19)]
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
        [LibTypeTag(20)]
        public int 渐变背景方式 { get; set; }
        [LibTypeTag(21)]
        public int 渐变背景颜色1 { get; set; }
        [LibTypeTag(22)]
        public int 渐变背景颜色2 { get; set; }
        [LibTypeTag(23)]
        public int 渐变背景颜色3 { get; set; }
        [LibTypeTag(24)]
        public int 横向对齐方式
        {
            get
            {
                ContentAlignment ca = this.TextAlign;
                switch (ca)
                {
                    case ContentAlignment.TopLeft:
                    case ContentAlignment.MiddleLeft:
                    case ContentAlignment.BottomLeft:
                        return 0;
                    case ContentAlignment.TopCenter:
                    case ContentAlignment.MiddleCenter:
                    case ContentAlignment.BottomCenter:
                        return 1;
                    case ContentAlignment.TopRight:
                    case ContentAlignment.MiddleRight:
                    case ContentAlignment.BottomRight:
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
                        switch (this.纵向对齐方式)
                        {
                            case 0:
                                this.TextAlign = ContentAlignment.TopLeft;
                                break;
                            case 1:
                                this.TextAlign = ContentAlignment.MiddleLeft;
                                break;
                            case 2:
                                this.TextAlign = ContentAlignment.BottomLeft;
                                break;
                        }
                        break;
                    case 1:
                        switch (this.纵向对齐方式)
                        {
                            case 0:
                                this.TextAlign = ContentAlignment.TopCenter;
                                break;
                            case 1:
                                this.TextAlign = ContentAlignment.MiddleCenter;
                                break;
                            case 2:
                                this.TextAlign = ContentAlignment.BottomCenter;
                                break;
                        }
                        break;
                    case 2:
                        switch (this.纵向对齐方式)
                        {
                            case 0:
                                this.TextAlign = ContentAlignment.TopRight;
                                break;
                            case 1:
                                this.TextAlign = ContentAlignment.MiddleRight;
                                break;
                            case 2:
                                this.TextAlign = ContentAlignment.BottomRight;
                                break;
                        }
                        break;
                }
            }
        }
        [LibTypeTag(25)]
        public bool 是否自动折行
        {
            get
            {
                return (this.Dock == DockStyle.Fill) && (this.AutoSize == false);
            }
            set
            {
                this.AutoSize = false;
                this.Dock = value ? DockStyle.Fill : DockStyle.None;
            }
        }
        [LibTypeTag(26)]
        public int 纵向对齐方式
        {
            get
            {
                ContentAlignment ca = this.TextAlign;
                switch (ca)
                {
                    case ContentAlignment.TopLeft:
                    case ContentAlignment.TopCenter:
                    case ContentAlignment.TopRight:
                        return 0;
                    case ContentAlignment.MiddleLeft:
                    case ContentAlignment.MiddleCenter:
                    case ContentAlignment.MiddleRight:
                        return 1;
                    case ContentAlignment.BottomLeft:
                    case ContentAlignment.BottomCenter:
                    case ContentAlignment.BottomRight:
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
                        switch (this.横向对齐方式)
                        {
                            case 0:
                                this.TextAlign = ContentAlignment.TopLeft;
                                break;
                            case 1:
                                this.TextAlign = ContentAlignment.TopCenter;
                                break;
                            case 2:
                                this.TextAlign = ContentAlignment.TopRight;
                                break;
                        }
                        break;
                    case 1:
                        switch (this.横向对齐方式)
                        {
                            case 0:
                                this.TextAlign = ContentAlignment.MiddleLeft;
                                break;
                            case 1:
                                this.TextAlign = ContentAlignment.MiddleCenter;
                                break;
                            case 2:
                                this.TextAlign = ContentAlignment.MiddleRight;
                                break;
                        }
                        break;
                    case 2:
                        switch (this.横向对齐方式)
                        {
                            case 0:
                                this.TextAlign = ContentAlignment.BottomLeft;
                                break;
                            case 1:
                                this.TextAlign = ContentAlignment.BottomCenter;
                                break;
                            case 2:
                                this.TextAlign = ContentAlignment.BottomRight;
                                break;
                        }
                        break;
                }
            }
        }
        [LibTypeTag(27)]
        public string 数据源 { get; set; }
        [LibTypeTag(28)]
        public string 数据列 { get; set; }
        //-属性-
        //-方法-
        [LibTypeTag((uint)krnln_method.调用反馈事件)]
        public int 调用反馈事件(int 参数一 = 0, int 参数二 = 0, bool 事件传递方式 = true)
        {
            if (this.反馈事件 != null)
            {
                if (事件传递方式) return this.反馈事件(参数一, 参数二);
                else this.反馈事件.BeginInvoke(参数一, 参数二, ar => this.反馈事件.EndInvoke(ar), null);
            }
            return 0;
        }
        //-方法-
        //-事件-
        [LibTypeTag(0)]
        public event Func<int, int, int> 反馈事件;
        //-事件-
    }
}
