# LVGL 智能手表 Demo：C 语言、LVGL 与 FreeRTOS 入门教程

这份教程面向刚接触 C 语言、LVGL 和 FreeRTOS 的学习者。阅读时不需要一次记住所有 API，先理解程序如何从 `main()` 走到界面，再逐步修改小功能即可。

---

## 1. 这个工程现在能做什么

运行 `bin\main.exe` 后，SDL2 会在 Windows 上创建一个 `320 × 320` 的模拟屏幕。鼠标相当于手指，可以点击底部标签和设置开关。

Demo 有四页：

| 页面 | 控件 | 作用 |
| --- | --- | --- |
| TIME | Label | 显示日期、时、分、秒 |
| STEPS | Label、Bar | 显示模拟步数和目标进度 |
| HEART | Arc、Label | 显示模拟心率 |
| SET | Switch、Slider | 切换主题、演示亮度设置 |

数据不来自真实硬件：当前 `lv_timer` 每 5 秒模拟更新时间、步数和心率。

> 重要：当前工程的 FreeRTOS 是关闭的。程序使用普通的 `while(1)` 循环运行 LVGL。这很适合第一阶段学习。

---

## 2. 建议的学习路线

不要一开始从头读完整个 LVGL 源码。推荐顺序是：

1. 运行程序，认识四个页面。
2. 阅读 `main/src/main.c`，理解程序入口。
3. 阅读 `smartwatch_demo_create()`，理解协调层如何创建五个模块。
4. 分别阅读 `smartwatch/` 下的页面模块，学习 Label、Bar、Arc、Switch、Slider。
5. 理解事件回调和 Timer 回调。
6. 做文末的小实验。
7. 最后再启用 FreeRTOS。

---

## 3. 编译和运行

在工程根目录打开 PowerShell：

```powershell
$env:Path = "D:\ENV_TOOLS\mingw64\bin;$env:Path"
cmake --build build -j 4
.\bin\main.exe
```

这两条命令的含义：

- `cmake --build build`：根据已经生成的构建配置编译代码。
- `-j 4`：最多使用 4 个并行编译任务。
- `.\bin\main.exe`：运行编译结果。

如果只修改了 `smartwatch_demo.c` 或 `smartwatch/` 下的某个模块，CMake 通常只重新编译相关文件，不会从头编译整个 LVGL。

---

## 4. 先补一点必需的 C 语言知识

### 4.1 变量

变量用于保存数据：

```c
uint8_t second = 0;
uint16_t steps = 2450;
bool dark_mode = true;
```

- `uint8_t`：无符号 8 位整数，范围为 0～255。
- `uint16_t`：无符号 16 位整数，范围为 0～65535。
- `bool`：布尔值，通常是 `true` 或 `false`。

这里用较小的整数类型，是因为嵌入式设备通常需要关注内存占用。

### 4.2 函数

函数是一段可以被调用的代码：

```c
void smartwatch_demo_create(void)
{
    /* 创建界面 */
}
```

第一个 `void` 表示函数不返回结果；括号里的 `void` 表示函数不接收参数。

带参数的函数示例：

```c
lv_obj_t *make_page(lv_obj_t *tabview, const char *name)
```

它接收页面容器和页面名称，返回新页面对象。

### 4.3 指针

LVGL 中经常看到：

```c
lv_obj_t *label;
```

`label` 不是整个控件的数据，而是控件在内存中的地址。可以把它理解成控件的“编号”或“遥控器”。以后通过这个地址修改控件：

```c
lv_label_set_text(label, "Hello");
```

初学阶段不需要手动释放 LVGL 控件。删除父对象时，LVGL 会删除它的子对象。

### 4.4 结构体

结构体把相关数据放在一起：

```c
typedef struct {
    lv_obj_t *time_label;
    uint8_t second;
    uint16_t steps;
    bool dark_mode;
} watch_state_t;
```

随后创建一个状态变量：

```c
static watch_state_t watch;
```

读取成员使用点号：

```c
watch.second = 0;
watch.steps = 2450;
```

这个结构体既保存界面控件的地址，也保存手表当前数据。

### 4.5 `static`

文件内函数和变量前面的 `static` 表示它们只在当前 `.c` 文件中使用：

```c
static watch_state_t watch;
static void tick_cb(lv_timer_t *timer);
```

这样可以减少命名冲突，也能隐藏组件内部实现。

### 4.6 头文件

`smartwatch_demo.h` 对外公布了一个函数：

```c
void smartwatch_demo_create(void);
```

其他文件使用 `#include "smartwatch_demo.h"` 后，就知道这个函数存在。`.h` 文件像目录，`.c` 文件才是具体实现。

### 4.7 回调函数

回调是“先把函数交给框架，在将来某个时刻由框架调用”。例如：

```c
lv_timer_create(tick_cb, 1000, NULL);
```

这里没有立即调用 `tick_cb()`，而是告诉 LVGL：每隔 1000 毫秒调用它一次。

---

## 5. 程序从哪里开始

入口位于 `main/src/main.c`：

```c
int main(int argc, char **argv)
{
    lv_init();
    hal_init(320, 320);
    smartwatch_demo_create();

    while(1) {
        lv_timer_handler();
        usleep(5 * 1000);
    }
}
```

执行顺序如下：

### 第一步：`lv_init()`

初始化 LVGL 内部模块。创建任何 LVGL 控件之前都必须调用它，而且通常只调用一次。

### 第二步：`hal_init(320, 320)`

HAL 是 Hardware Abstraction Layer，即硬件抽象层。在当前 PC 工程中，它负责：

- 创建 SDL2 窗口；
- 创建鼠标输入设备；
- 创建鼠标滚轮和键盘输入设备；
- 告诉 LVGL 默认显示器是哪一个。

在真实开发板上，这部分会替换为 LCD 刷屏、触摸屏读取等驱动代码，而手表 UI 基本不用变化。

### 第三步：`smartwatch_demo_create()`

创建智能手表页面及其所有控件。

### 第四步：无限循环

```c
while(1) {
    lv_timer_handler();
    usleep(5 * 1000);
}
```

嵌入式程序通常不会像普通命令行程序那样执行完就退出。`while(1)` 表示一直运行。

`lv_timer_handler()` 让 LVGL 处理：

- 控件重绘；
- 鼠标和触摸输入；
- 动画；
- LVGL Timer；
- 页面滚动和切换。

`usleep(5 * 1000)` 暂停约 5 毫秒，避免循环占满 CPU。

---

## 6. LVGL 对象与对象树

LVGL 中几乎所有界面元素都是 `lv_obj_t` 对象。创建 Label 时必须指定父对象：

```c
lv_obj_t *title = lv_label_create(page);
```

对象关系可以想象成：

```text
活动屏幕
└── Tabview
    ├── TIME 页面
    │   ├── 日期 Label
    │   ├── 时间 Label
    │   └── 秒数 Label
    ├── STEPS 页面
    │   ├── 步数 Label
    │   └── Bar
    ├── HEART 页面
    │   └── Arc
    │       └── 心率 Label
    └── SET 页面
        ├── 设置行
        │   ├── Label
        │   └── Switch
        └── Slider
```

父子关系的作用包括：

- 子对象的位置通常相对父对象计算；
- 子对象可以继承部分样式；
- 父对象滚动时，子对象一起移动；
- 删除父对象时，子对象一起删除。

---

## 7. 创建整个智能手表

核心入口在 `smartwatch_demo.c`：

```c
void smartwatch_demo_create(void)
{
    lv_obj_clean(lv_screen_active());
    /* 初始化 watch 数据 */
    /* 创建 Tabview */
    /* 创建四个页面 */
    /* 应用主题 */
    /* 创建定时器 */
}
```

### 7.1 清理当前屏幕

```c
lv_obj_clean(lv_screen_active());
```

- `lv_screen_active()` 获取当前活动屏幕。
- `lv_obj_clean()` 删除该对象的所有子对象。

这样可以避免官方示例控件和手表控件同时存在。

### 7.2 初始化状态

```c
watch.hour = 10;
watch.minute = 8;
watch.steps = 2450;
watch.dark_mode = true;
```

这些是模拟数据。以后接入 RTC 或传感器时，可以在定时更新函数中把它们替换为硬件读取结果。

### 7.3 创建 Tabview

```c
lv_obj_t *tabview = lv_tabview_create(lv_screen_active());
lv_obj_set_size(tabview, lv_pct(100), lv_pct(100));
lv_tabview_set_tab_bar_position(tabview, LV_DIR_BOTTOM);
lv_tabview_set_tab_bar_size(tabview, 52);
```

- `lv_tabview_create()` 创建多页面容器。
- `lv_pct(100)` 表示占父对象宽度或高度的 100%。
- `LV_DIR_BOTTOM` 把标签按钮放在底部。
- `52` 是底部标签栏高度，单位为像素。

### 7.4 创建页面

```c
create_clock_page(tabview);
create_steps_page(tabview);
create_heart_page(tabview);
create_settings_page(tabview);
```

把每页代码拆成一个函数，比把所有代码都写在入口函数里更容易阅读和维护。

### 7.5 创建 LVGL Timer

```c
lv_timer_create(tick_cb, 1000, NULL);
```

- `tick_cb`：到时间后执行的回调函数。
- `1000`：周期为 1000 毫秒。
- `NULL`：没有额外传入用户数据。

它不是 FreeRTOS 软件定时器，而是 LVGL 自己的 Timer。只有不断调用 `lv_timer_handler()`，它才能得到执行。

### 7.6 五个功能模块如何解耦

现在 UI 被拆成五个独立模块，`smartwatch_demo.c` 不再绘制各页面的具体控件，只充当协调层：

```text
                    smartwatch_demo.c
                  （状态、Timer、导航）
            ┌──────────┼──────────┐
            ▼          ▼          ▼
       Clock 模块   Steps 模块   Heart 模块
            │
            │ 回调：请求打开
            ▼
      Calculator 模块       Settings 模块
                              │
                              └─ 回调：主题发生变化
```

模块之间不直接调用。例如 Clock 模块不知道 Calculator 模块的存在。Clock 只保存一个由协调层传入的回调：

```c
clock_page_create(&watch.clock_page,
                  clock_tab,
                  &watch.theme,
                  open_calculator_cb,
                  &watch);
```

用户点击 Clock 中的按钮时，Clock 调用这个回调。随后协调层决定隐藏 Tabview 并打开 Calculator。这样以后即使替换计算器模块，也不需要修改 Clock 模块。

每个模块的接口分成两类：

- `xxx_page_create()`：创建该模块自己的控件。
- `xxx_page_set_xxx()`：由协调层传入新数据，例如时间、步数和心率。

`watch_theme.h` 只是公共数据协议，保存背景色、卡片色和文字色。它不包含页面逻辑，因此五个功能模块仍然彼此独立。

---

## 8. Flex 布局

公共页面创建函数中有：

```c
lv_obj_set_flex_flow(page, LV_FLEX_FLOW_COLUMN);
lv_obj_set_flex_align(page,
                      LV_FLEX_ALIGN_CENTER,
                      LV_FLEX_ALIGN_CENTER,
                      LV_FLEX_ALIGN_CENTER);
```

`LV_FLEX_FLOW_COLUMN` 表示控件从上向下排列。三个对齐参数分别控制主轴、交叉轴和行的对齐方式。

使用 Flex 后，添加新控件时通常不需要手动设置 `x`、`y` 坐标。页面大小变化时，布局也更容易适配。

设置行使用横向布局：

```c
lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(row,
                      LV_FLEX_ALIGN_SPACE_BETWEEN,
                      LV_FLEX_ALIGN_CENTER,
                      LV_FLEX_ALIGN_CENTER);
```

`SPACE_BETWEEN` 会把文字推到左边、开关推到右边。

---

## 9. 常用控件逐项讲解

### 9.1 Label：文字

创建并设置文字：

```c
lv_obj_t *title = lv_label_create(page);
lv_label_set_text(title, "WED 23 JUL");
```

动态格式化文字：

```c
lv_label_set_text_fmt(watch.time_label, "%02u:%02u",
                      watch.hour, watch.minute);
```

`%02u` 表示按无符号整数输出，不足两位时前面补 0。例如 `8` 显示成 `08`。

### 9.2 Bar：进度条

```c
watch.steps_bar = lv_bar_create(page);
lv_bar_set_range(watch.steps_bar, 0, 10000);
lv_bar_set_value(watch.steps_bar, 2450, LV_ANIM_OFF);
```

- Range 是 0～10000。
- 当前值是 2450。
- `LV_ANIM_OFF` 表示首次显示时不播放动画。

更新时使用：

```c
lv_bar_set_value(watch.steps_bar, watch.steps, LV_ANIM_ON);
```

`LV_ANIM_ON` 会让进度变化更平滑。

### 9.3 Arc：圆弧

```c
watch.heart_arc = lv_arc_create(page);
lv_arc_set_range(watch.heart_arc, 50, 130);
lv_arc_set_value(watch.heart_arc, 72);
```

心率范围设置为 50～130，当前值是 72。

```c
lv_obj_remove_flag(watch.heart_arc, LV_OBJ_FLAG_CLICKABLE);
```

这行取消 Arc 的点击交互，防止用户误拖圆弧改变模拟心率。

### 9.4 Switch：开关

```c
lv_obj_t *sw = lv_switch_create(row);
lv_obj_add_state(sw, LV_STATE_CHECKED);
```

`LV_STATE_CHECKED` 表示开关初始为开启状态。

### 9.5 Slider：滑块

```c
lv_obj_t *slider = lv_slider_create(page);
lv_slider_set_value(slider, 70, LV_ANIM_OFF);
```

当前 Slider 只用于展示 UI，并没有真正改变 Windows 显示器亮度。真实设备上可以在事件回调里把 Slider 数值转换成屏幕背光 PWM 占空比。

---

## 10. 事件回调

开关创建后注册事件：

```c
lv_obj_add_event_cb(sw,
                    theme_event_cb,
                    LV_EVENT_VALUE_CHANGED,
                    NULL);
```

意思是：当开关的值发生变化时，请调用 `theme_event_cb()`。

回调函数：

```c
static void theme_event_cb(lv_event_t *event)
{
    lv_obj_t *sw = lv_event_get_target_obj(event);
    watch.dark_mode = lv_obj_has_state(sw, LV_STATE_CHECKED);
    apply_theme();
}
```

执行步骤：

1. 从事件中取得触发事件的控件。
2. 检查 Switch 是否有 `CHECKED` 状态。
3. 把结果保存到 `watch.dark_mode`。
4. 重新应用主题颜色。

事件适合处理用户操作，例如点击、长按、值变化、滚动和获得焦点。

### 10.1 从 Clock 页面进入计算器

Clock 页面使用普通 Button 作为入口，并把点击事件转换成回调通知：

```c
lv_obj_t *calculator_button = lv_button_create(page);
lv_obj_add_event_cb(calculator_button,
                    calculator_button_event_cb,
                    LV_EVENT_CLICKED,
                    page);
```

Clock 模块的事件回调再调用协调层传入的 `open_calculator_cb()`。协调层隐藏 Tabview，并通过 `calculator_page_open()` 创建全屏计算器。`BACK` 也通过回调通知协调层；协调层重新显示 Tabview、把它移到最前面，再调用 `calculator_page_close()`。

计算器键盘使用 `lv_buttonmatrix`。它把多个按钮放进一个控件，比逐个创建数字按钮更简单：

```c
static const char *calculator_key_map[] = {
    "C", "DEL", "/", "*", "\n",
    "7", "8", "9", "-", "\n",
    "4", "5", "6", "+", "\n",
    "1", "2", "3", "=", "\n",
    "0", ".", ""
};
```

- 每个字符串是一个按键的文字。
- `"\n"` 表示键盘换到下一行。
- 最后的空字符串表示按键表结束。

矩阵按键共用 `calculator_key_event_cb()`。回调先读取被点击按键的文字，再用 `if` 判断它属于数字、运算符、清除、删除还是等号。独立的返回按钮使用 `calculator_back_event_cb()`。

`calculator_page_t` 中的 `input` 是字符数组，用来保存屏幕上正在输入的数字。例如按下 `1`、`2`、`.`、`5` 后，它保存字符串 `"12.5"`。计算前使用 `strtod()` 把字符串转换为数值；计算后使用 `snprintf()` 把数值重新转换为显示字符串。

一次 `2 + 3 =` 的状态变化是：

```text
输入 2      → 结果行显示 2
按下 +      → 计算行显示 "2 +"
输入 3      → 计算行显示 "2 + 3"，结果行显示 3
按下 =      → 计算行显示 "2 + 3 ="，结果行显示 5
```

除数为 0 时会显示 `Error`。这个计算器适合学习事件和状态管理，不用于需要高精度的小数或财务计算。

---

## 11. 每秒更新是怎样工作的

`tick_cb()` 每秒执行一次：

```c
watch.second++;
```

`++` 表示加 1。秒数达到 60 后归零，并给分钟加 1：

```c
if(watch.second >= 60) {
    watch.second = 0;
    watch.minute++;
}
```

步数更新：

```c
watch.steps = (watch.steps + 3) % 10001;
```

`%` 是取余运算。当数值达到 10001 时，结果会重新回到 0。这里仅用于让 PC Demo 可以一直演示。

模拟心率：

```c
watch.heart_rate = 72 + (watch.second % 7);
```

心率会在 72～78 之间变化。

最后调用各控件的 `set` 函数，把新数据写到界面。一般规律是：

- `create` 创建控件；
- `set` 修改控件属性；
- `get` 读取控件属性。

---

## 12. 样式与 Part

设置普通背景色：

```c
lv_obj_set_style_bg_color(screen, COLOR_BG_DARK, LV_PART_MAIN);
```

一个控件可能包含多个 Part。例如 Bar 有背景和填充部分：

```c
lv_obj_set_style_bg_color(watch.steps_bar,
                          COLOR_ACCENT,
                          LV_PART_INDICATOR);
```

- `LV_PART_MAIN`：控件主体。
- `LV_PART_INDICATOR`：进度条或圆弧已经填充的部分。

同一个样式属性应用到不同 Part，效果可能不同。

颜色宏：

```c
#define COLOR_ACCENT lv_color_hex(0x45d4a8)
```

`0x45d4a8` 是十六进制 RGB 颜色，分别对应红、绿、蓝三个分量。

---

## 13. LVGL Timer、系统 Tick 和 FreeRTOS Timer 的区别

这三个名字很容易混淆：

| 名称 | 作用 |
| --- | --- |
| LVGL Tick | 告诉 LVGL 经过了多少毫秒，是时间基准 |
| LVGL Timer | 在 `lv_timer_handler()` 中按周期调用回调 |
| FreeRTOS 软件定时器 | 由 FreeRTOS Timer Service Task 管理 |

本 Demo 使用的是 LVGL Timer：

```c
lv_timer_create(tick_cb, 1000, NULL);
```

它不创建线程，也不会与 `main()` 真正并行执行。回调通常是在 `lv_timer_handler()` 执行期间被调用。

---

## 14. FreeRTOS 是什么

在没有操作系统的程序中，常见结构是“裸机大循环”：

```c
while(1) {
    read_sensor();
    update_ui();
    handle_key();
}
```

功能增加后，大循环可能变得难以管理。FreeRTOS 允许把工作拆成多个任务：

```text
LVGL 任务       → 处理界面
传感器任务      → 读取心率和步数
通信任务        → 处理蓝牙或 Wi-Fi
存储任务        → 保存设置
```

FreeRTOS 调度器会让这些任务轮流使用 CPU。单核 CPU 上通常不是真的同时执行，而是快速切换，看起来像并行。

### 14.1 Task

任务通常是一个不会返回的函数：

```c
void sensor_task(void *parameter)
{
    while(1) {
        read_sensor();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
```

### 14.2 Delay

`vTaskDelay()` 会让当前任务休眠一段时间，让其他任务运行。它不同于忙等待，不会一直占用 CPU。

### 14.3 Queue

Queue 用来安全地把数据从一个任务传给另一个任务。例如传感器任务把心率发给 LVGL 任务。

### 14.4 Mutex

Mutex 用来保护共享资源，避免两个任务同时修改同一份数据。

---

## 15. 为什么现在不急着启用 FreeRTOS

当前 `CMakeLists.txt` 中：

```cmake
option(USE_FREERTOS "Enable FreeRTOS" OFF)
```

`OFF` 表示不编译 FreeRTOS 运行模式。`lv_conf.h` 中也使用 `LV_OS_NONE`。

先关闭 FreeRTOS 有三个好处：

1. 调试路径简单，所有 UI 逻辑都从一个循环执行。
2. 不需要立刻处理任务栈、优先级、互斥锁和线程安全。
3. 可以先判断问题属于 UI 代码还是 RTOS 调度。

等熟悉当前 Demo 后，再单独做“传感器任务向 UI 任务发送心率”的练习会更容易。

---

## 16. LVGL 与多任务的重要规则

LVGL 默认不是线程安全的。以后启用 FreeRTOS 时，推荐遵守：

- 只在一个专门的 LVGL 任务中调用 LVGL API；
- 其他任务通过 Queue 把数据发给 LVGL 任务；
- 不要让传感器任务直接执行 `lv_label_set_text()`；
- 如果确实要从多个任务调用 LVGL，必须使用 LVGL 锁或统一 Mutex，并严格遵守所用移植层规则。

推荐的数据流：

```text
传感器任务
    │ 发送 heart_rate
    ▼
FreeRTOS Queue
    │ 接收数据
    ▼
LVGL 任务
    │ lv_label_set_text_fmt()
    ▼
界面更新
```

---

## 17. 从 PC 移植到开发板时哪些代码会变化

通常保持不变：

- `smartwatch_demo.c`
- `smartwatch_demo.h`
- 页面布局、控件、样式、事件逻辑

需要替换或增加：

- SDL 显示驱动 → LCD 显示驱动；
- SDL 鼠标 → 触摸屏驱动；
- 模拟时间 → RTC 时间；
- 模拟步数和心率 → 传感器数据；
- Windows 延时 → HAL、FreeRTOS 或芯片 SDK 延时；
- 可能需要减小字体、动画、缓冲区来适配 RAM 和 Flash。

这就是 LVGL 将 UI 和硬件驱动分开的价值。

---

## 18. 推荐的小实验

每次只做一个修改，编译、运行、观察结果，然后再做下一个。

### 实验 1：修改初始时间

找到：

```c
watch.hour = 10;
watch.minute = 8;
```

改成自己的时间，重新编译运行。

### 实验 2：修改主题色

找到：

```c
#define COLOR_ACCENT lv_color_hex(0x45d4a8)
```

把颜色改为蓝色：

```c
#define COLOR_ACCENT lv_color_hex(0x3498db)
```

### 实验 3：修改步数速度

找到：

```c
watch.steps = (watch.steps + 3) % 10001;
```

把 `3` 改成 `50`，观察进度条变化。

### 实验 4：增加一句文字

在 `create_clock_page()` 最后添加：

```c
lv_obj_t *hello = lv_label_create(page);
lv_label_set_text(hello, "Hello LVGL");
```

因为页面使用 Flex，新 Label 会自动排到其他控件后面。

### 实验 5：读取 Slider 数值

给 Slider 注册 `LV_EVENT_VALUE_CHANGED`，在回调中使用：

```c
int32_t value = lv_slider_get_value(slider);
```

先把数值显示到 Label，不要立即控制真实硬件。

---

## 19. 常见错误和排查方法

### 修改后界面没有变化

确认重新执行了：

```powershell
cmake --build build -j 4
```

并确认运行的是当前工程的 `bin\main.exe`。

### 提示找不到 `SDL2.dll`

确认 `SDL2.dll` 与 `main.exe` 位于同一个 `bin` 目录。

### 提示找不到字体符号

检查 `lv_conf.h`：

```c
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_48 1
```

修改配置后应重新编译。

### 程序能启动但界面不刷新

检查主循环是否持续调用 `lv_timer_handler()`，并确认循环没有被其他耗时操作长时间阻塞。

### 点击控件没有反应

检查是否注册了正确事件，输入设备是否绑定到当前显示器，以及控件是否被其他对象遮挡。

### 编译很慢

LVGL 首次构建源文件很多，耗时较长是正常的。后续只修改应用代码时通常会快很多。

---

## 20. 重要文件索引

| 文件 | 初学阶段是否重点阅读 | 作用 |
| --- | --- | --- |
| `smartwatch_demo.c` | 是 | 五个模块的协调层：状态、Timer、主题和导航 |
| `smartwatch_demo.h` | 是 | 对外接口声明 |
| `smartwatch/clock_page.c` | 是 | Clock 页面和计算器入口 |
| `smartwatch/steps_page.c` | 是 | 步数页面及数据更新接口 |
| `smartwatch/heart_page.c` | 是 | 心率页面及数据更新接口 |
| `smartwatch/settings_page.c` | 是 | 设置页面、Slider 和主题回调 |
| `smartwatch/calculator_page.c` | 是 | 计算器 UI 和运算状态 |
| `smartwatch/watch_theme.h` | 了解 | 五个模块共享的主题数据协议 |
| `main/src/main.c` | 是 | 初始化 LVGL、SDL 和主循环 |
| `lv_conf.h` | 了解 | 开关 LVGL 功能和字体 |
| `CMakeLists.txt` | 了解 | 指定编译哪些源码、链接哪些库 |
| `lvgl/` | 暂时不要通读 | LVGL 库源码 |
| `FreeRTOS/` | 第二阶段学习 | FreeRTOS 内核源码 |

---

## 21. 第一阶段应掌握的核心结论

完成这份 Demo 的学习后，先确保理解以下几点：

1. `main()` 负责初始化和持续运行程序。
2. LVGL 控件由 `lv_xxx_create()` 创建，并有父子关系。
3. Flex 可以减少手动坐标计算。
4. 用户操作通过事件回调处理。
5. 周期更新可以使用 LVGL Timer。
6. `lv_timer_handler()` 必须持续执行。
7. 当前 Demo 没有启用 FreeRTOS。
8. 以后启用多任务时，应集中在 LVGL 任务中更新 UI。

不需要背诵函数名。能够找到创建位置、修改参数、重新编译并解释变化，就已经完成了很好的第一步。
