# 项目使用VS2019 Release x64构建

**编译时请选择Release x64模式**  
项目需要动态链接SDL库运行，x86的sdl文件已经包含在文件夹 SDL2-2.0.12 中, 修改编译模式需要重新连接库文件。

# 摄像机运动：
wasd前后左右  
qe上下  
鼠标右键按住拖动移动视角  
鼠标滚轮改变fov  

# 实现功能
MVP矩阵变换  
自定义shader  
视锥剔除/齐次裁切/背面剔除  
摄像机运动  
obj模型导入  
透视矫正  
blinn phong 光照模型  
alpha blending  
shadow Map  

>数学库、贴图、模型导入的代码 与 所有的obj模型来自
https://github.com/ssloy/tinyrenderer  
感谢ssloy提供的教程与素材