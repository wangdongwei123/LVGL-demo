# LVGL 初学者智能手表 Demo（Windows 可运行工程）

这是一个面向初学者的 LVGL 9.x Windows 模拟器工程，分辨率为 `320 x 320`。它不依赖 RTC、心率传感器或计步传感器，数据由 `lv_timer` 模拟，因此可以先专注学习 GUI。

工程基于 LVGL 官方 `lv_port_pc_vscode` 模板，使用 SDL2 创建模拟屏幕、使用鼠标模拟触摸。

## Windows 编译和运行

在 PowerShell 中执行（MinGW 安装在 `D:\ENV_TOOLS\mingw64\bin`）：

```powershell
$env:Path = "D:\ENV_TOOLS\mingw64\bin;$env:Path"
cmake -S . -B build
cmake --build build -j 4
.\bin\main.exe
```

首次配置后，修改 UI 时通常只需再次执行构建命令，然后运行程序。也可以双击 `simulator.code-workspace` 用 VS Code 打开工程。

当前工程的 `bin` 目录已经放置了运行所需的 `SDL2.dll`，因此也可以直接双击 `bin\main.exe`。

## 包含的页面

1. **TIME**：数字表盘，每秒更新时间。
   点击 `CALCULATOR` 可进入四则运算页面，点击 `BACK` 返回表盘。
2. **STEPS**：步数和每日目标进度条。
3. **HEART**：使用圆弧显示模拟心率。
4. **SET**：深色模式开关和亮度滑块。

底部标签栏可以点击；如果输入设备支持触摸，也可以左右滑动内容区切换页面。

## 接入现有 LVGL 9.x 工程

1. 将 `smartwatch_demo.c`、`smartwatch_demo.h` 和整个 `smartwatch/` 目录加入工程。
2. 确认 `lv_conf.h` 启用了 Montserrat 24 和 48 字体：

```c
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_48 1
```

3. 在 `lv_init()` 和显示/输入驱动初始化完成后调用：

```c
#include "smartwatch_demo.h"

smartwatch_demo_create();
```

4. 主循环中持续执行 LVGL 任务处理：

```c
while(1) {
    lv_timer_handler();
    platform_delay_ms(5); /* 替换为你的平台延时函数 */
}
```

> `main_example.c` 只是接入示例，不应与开发板原有的 `main.c` 重复编译为入口文件。

## 用到的 LVGL 功能

### 1. 对象树

所有控件都有父对象。例如心率文字是 Arc 的子对象，因此可以直接相对 Arc 居中。删除父对象时，其子对象也会一起删除。

### 2. Tabview 页面切换

`lv_tabview_create()` 创建页面容器，`lv_tabview_add_tab()` 添加 4 个页面。Tabview 同时完成标签按钮管理和滑动切换，适合初学者理解多页面 UI。

### 3. Flex 布局

页面使用纵向 Flex 自动排列控件，设置行使用横向 Flex 将文字和开关分布到两侧。它比手动计算每个坐标更便于适配不同屏幕。

### 4. 常用控件

- `lv_label`：时间、步数和说明文字。
- `lv_bar`：步数目标进度。
- `lv_arc`：心率圆环。
- `lv_switch`：深色模式开关。
- `lv_slider`：亮度设置示例。
- `lv_buttonmatrix`：用一个控件组成计算器键盘。

### 5. 事件

深色模式开关通过 `lv_obj_add_event_cb()` 注册 `LV_EVENT_VALUE_CHANGED` 事件。用户操作开关时，回调函数读取控件状态并更新主题。

### 6. Timer 与动画

`lv_timer_create(update_timer_cb, 5000, &watch)` 每 5 秒执行一次模拟更新，刷新时钟、步数和心率。步数条使用 `LV_ANIM_ON` 平滑移动。真实项目中，只需把模拟值替换成传感器数据。

### 7. 样式

示例设置了背景色、文字色、圆角、Arc 宽度和控件指示色。LVGL 的样式可应用于对象的不同 Part，例如进度条的填充部分是 `LV_PART_INDICATOR`。

## 推荐学习顺序

1. 修改表盘初始时间和颜色。
2. 注释 `tick_cb()` 中的步数增长，观察区别。
3. 给设置页面增加一个 `lv_checkbox`。
4. 将模拟步数替换为自己的变量或传感器读取函数。
5. 最后再添加图标、页面转场和真实 RTC，避免一开始引入过多概念。

## 模块说明

- `smartwatch_demo.c`：协调层，管理 Tabview、模拟数据、Timer、主题和页面导航。
- `smartwatch/clock_page.c`：Clock 页面和计算器入口。
- `smartwatch/steps_page.c`：步数页面。
- `smartwatch/heart_page.c`：心率页面。
- `smartwatch/settings_page.c`：设置、主题开关和亮度 Slider。
- `smartwatch/calculator_page.c`：计算器 UI、输入和运算逻辑。
- `smartwatch/watch_theme.h`：模块共享的主题数据类型，不包含页面逻辑。
- `smartwatch_demo.h`：整个 Demo 的对外启动接口。
- `main_example.c`：如何从平台工程启动 UI。

五个功能模块不会直接引用彼此。Clock 和 Settings 通过回调把用户操作通知协调层；协调层再决定是否打开计算器或更新全局主题。
