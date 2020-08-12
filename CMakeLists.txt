cmake_minimum_required (VERSION 2.6)
project (SoftRenderer)

#查找当前目录下的所有源文件
aux_source_directory(. DIR_SRCS)

#指定生成目标
add_executable(SoftRenderer ${DIR_SRCS})