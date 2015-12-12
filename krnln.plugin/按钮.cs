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
    [LibType(10)]
    class 按钮 : Button, Base, Plugin
    {
        private int _类型;
        按钮()
        {
            this.ParentChanged += 按钮_ParentChanged;
            this.Click += 按钮_Click;
        }
        void 按钮_ParentChanged(object sender, EventArgs e)
        {
            Control col = this.Parent;
            Form form = null;
            do
            {
                form = col as Form;
                if (form != null)
                {
                    if (this._类型 == 1) form.AcceptButton = this;
                    else if (form.AcceptButton == this) form.AcceptButton = null;
                }
                col = col.Parent;
            } while (col != null);
        }
        void 按钮_Click(object sender, EventArgs e)
        {
            if (this.被单击 != null) this.被单击();
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
        public byte[] 图片 { get; set; }
        [LibTypeTag(9)]
        public int 类型
        {
            get
            {
                return this._类型;
            }
            set
            {
                this._类型 = value;
                if (this.Parent != null) 按钮_ParentChanged(null, null);
            }
        }
        [LibTypeTag(10)]
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
        [LibTypeTag(11)]
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
        [LibTypeTag(12)]
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
        [LibTypeTag(13)]
        public byte[] 字体 { get; set; }
        //-属性-
        //-事件-
        [LibTypeTag(0)]
        public event Action 被单击;
        //-事件-
    }
}
