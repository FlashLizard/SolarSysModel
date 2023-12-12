# SolarSysModel (日地月运动模型)

## 运行程序

直接运行项目根目录下的SolarSysModel.exe即可。

## 实现功能

- 日地月运动轨迹
- 纹理映射
- 基础光照
- 基本控制

## 基本操作

- WASD移动位置
- 移动鼠标改变视角（隐藏了鼠标）
- ESC退出
- R重置镜头
- Q切换全屏/窗口
- P暂停/继续

## 文件夹结构

- res: 图片
- shader: shader代码
- src: cpp文件
- include：头文件

## 编译教程

### 安装xmake

本项目基于xmake构建所以需要先安装xmake，官网：<https://xmake.io/>

### 编译运行

- 编译项目

```
xmake build SolarSysModel
```

- 运行项目

```
xmake run SolarSysModel
```

注意，不要直接用在release文件夹下运行exe文件，因为路径不正确。如果要直接运行exe文件，请放置在项目根目录下运行。