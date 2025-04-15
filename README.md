# 🛠️ My2DEngine

A simple, custom-built 2D game engine written in modern C++ using SDL2 and Lua. Developed as part of a deep-dive into game engine architecture inspired by the Pikuma course on C++ Game Engine Programming.

---

## ⚙️ Features

- ECS (Entity-Component-System) framework built from scratch  
- Rendering using SDL2  
- Lua scripting via Sol2  
- Asset and memory management  
- Input handling  
- Simple collision system  
- Built with Visual Studio (.sln)

---

## 🏁 Getting Started

### 🔧 Requirements

Make sure you have the following installed:

- Visual Studio 2019 or later (with C++ workload)  
- SDL2 development libraries  
- Lua + Sol2  
- GLM (for math)  
- Dear ImGui (optional, if GUI is implemented)

### 🛠️ Build Instructions

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/My2DEngine.git
   ```
2. Open the `.sln` file in Visual Studio.

3. Set the correct configuration (`Debug` or `Release`)

4. Build and run the project.

💡 Make sure all dependencies are properly linked in the project settings (include directories, library directories, etc.).

## 📁 Project Structure
```
2DGameEngine/
├── 2DGameEngine/   # With .dlls
├   ├── assets  # All the assets
├   ├── libs    # Libaries like GLM, SDL etc.
├   ├── src     # C++ and Lua Source of Engine + Game
├── 2DGameEngine.sln
└── README.md
```

## 📜 License
This project is released under the MIT License. See `LICENSE` for more details.

## 🙏 Credits
Built as a learning project inspired by the
C++ Game Engine Programming course from Pikuma.
Thanks to Gustavo Pezzi for the guidance and course material.

## ⭐️ Like this project?
Give it a ⭐ on GitHub if you find it interesting or useful!
