# 🎮 Tetris Game

## 📝 Overview
This is a terminal-based implementation of the classic **Tetris** game written in **C++**. It features smooth gameplay, scoring, and an **undo** feature.

## ✨ Features
- 🕹️ **Classic Tetris gameplay**
- 🎲 **Randomized tetromino pieces**
- 🎮 **Rotations and movement using keyboard inputs**
- 📊 **Scoring system based on cleared lines**
- 🏆 **High score persistence**
- 🔄 **Undo functionality**
- ⏸️ **Pause and restart options**

## 🔧 Installation
### ✅ Prerequisites
- 🛠️ A **C++ compiler** (GCC or Clang recommended)
- 🖥️ **Linux or macOS terminal** (Windows users can use WSL)

### ▶️ Build and Run
```sh
# 📥 Clone the repository
git clone https://github.com/yourusername/tetris-game.git
cd tetris-game

# 🔨 Compile the game
g++ -o tetris TetrisGame.cpp -std=c++11

# 🎯 Run the game
./tetris
```

## 🎮 Controls
| 🎹 Key | 🎯 Action |
|------|---------|
| A | Move left ⬅️ |
| D | Move right ➡️ |
| S | Move down faster ⬇️ |
| W | Rotate piece 🔄 |
| Space | Drop piece instantly ⚡ |
| P | Pause game ⏸️ |
| R | Restart game 🔁 |
| X | Exit game ❌ |

## 📊 Scoring
- ✅ **1 each peice drop**: **250 points**
- ✅ **2 each line cleared**: **100 points**


## 🏆 High Score
The game saves the highest score to **`highscore.txt`** and loads it on startup.

## 🤝 Contributing
Feel free to **submit pull requests** or **report issues** for improvements. 🚀

## 📜 License
This project is open-source under the **MIT License**. 📝
