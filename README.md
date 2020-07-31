# 项目使用VS2019 Release x64构建


项目需要动态链接SDL库运行，x86的sdl文件已经包含在文件夹 SDL2-2.0.12 中, 修改编译模式需要重新连接库文件。

>如果出现错误  
--C1083	无法打开包括文件: “SDL.h”: No such file or directory--  
请选择 **Release x64** 模式进行编译

# 摄像机运动：
wasd前后左右  
qe上下  
鼠标右键按住拖动移动视角  
鼠标滚轮改变fov  

# 实现功能
MVP矩阵变换  
自定义shader  
视锥剔除/齐次裁切/背面剔除  
深度测试
摄像机运动  
obj模型导入  
透明混合  
Blinn–Phong反射模型  
阴影贴图（Shadow mapping）

>数学库、贴图、模型导入的代码 与 所有的obj模型来自
https://github.com/ssloy/tinyrenderer  
感谢ssloy提供的教程与素材