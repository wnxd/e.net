using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using wnxd.E_NET;

namespace krnln.plugin
{
    [LibGuid("d09f2340818511d396f6aaf844c7e325")]
    [LibType(2)]
    class 菜单 : ToolStrip, Plugin
    {
        private ToolStrip menu;
        public void Create()
        {
            if (this.可视)
            {
                //this.menu = new MenuStrip();
            }
            else
            {
                //this.menu = new ContextMenuStrip();
            }
        }
        //-属性-
        [LibTypeTag(0)]
        public string 标题
        {
            get;
            set;
        }
        [LibTypeTag(1)]
        public bool 选中
        {
            get;
            set;
        }
        [LibTypeTag(2)]
        public bool 禁止
        {
            get;
            set;
        }
        [LibTypeTag(3)]
        public bool 可视
        {
            get;
            set;
        }
        //-属性-
        //-事件-
        [LibTypeTag(0)]
        public event Action 被选择;
        //-事件-
    }
}
