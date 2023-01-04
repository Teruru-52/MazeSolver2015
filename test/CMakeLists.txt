message("================== MazeSolver2015 test ==================")

# CMakeのバージョンを設定
cmake_minimum_required(VERSION 3.16.3)

# プロジェクト名と使用する言語を設定
project(main CXX)

# 実行ファイルを作成
add_executable(main ../Agent.h ../Agent.cpp ../Maze.h ../Maze.cpp ../mazeData.h ../mazeData.cpp ../Operation.h ../Operation.cpp ../ShortestPath.h ../ShortestPath.cpp ../MazeSolver_conf.h main.cpp)

# コマンドラインで実行
# cmake -S . -B build
# cmake --build build
#  ./build/main ../maze_data/maze2.dat 