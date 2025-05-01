# C-CUBE
C-CUBE is a rich and powerful programming language created for game development and being the default language of Python-based game engines developed by Sahne Dünya. The main purpose of this programming language is the game world. The C-CUBE programming language is Python-based, but there are differences as follows. This programming language only supports the Match statement as control structures. Another difference is that it is an Ahead-Of-Time compilable language. At the same time, there is an executable file .blocker developed for this programming language. Another difference is that the Standard Library automatically supports three Graphics APIs, Vulkan, OpenGL and OpenGL-ES. This makes game development easier. What about platform independence? Although the C-CUBE programming language is platform independent, some parts are not completely platform independent, and the factors that limit this are the native support of the Standard Libraries Vulkan, OpenGL and OpenGL-ES. The main reason why Sahne Dünya chooses these three in particular is that they are both open source and support a wide variety of platforms. When I say platform, I mean the operating system. Sahne Dünya uses a file with a .blocker extension specifically developed for this programming language instead of existing files such as .exe or .elf in order for the codes in the C-CUBE programming language to be completely platform independent. The blocker executable file can support all operating systems supported by Vulkan, OpenGL and OpenGL-ES and can also support all CPU architectures supported by that operating system. Thanks to this platform support, Sahne Dünya will be able to play games on many operating systems without any problems, but factors such as which operating system the game will be developed will support, which API was used, which operating system or CPU architecture it was compiled for, and which game engine was used determine this. 

# Basic features:
* Cross Platform: Yes
* Supported programming: OOP
* Source file extension: .c-cube
* Executable file: .bloker
* Standard Library: Yes
* Standard Package manager: Yes
* Automatically supported APIs: Vulkan, OpenGL and OpenGL-ES
* Underlying programming language: Phyton

# Target Hello World code
```
print("Hello, World!")
