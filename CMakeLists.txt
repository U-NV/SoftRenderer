cmake_minimum_required (VERSION 2.6)
project (SoftRenderer)

aux_source_directory(. DIR_SRCS)


add_executable(SoftRenderer ${DIR_SRCS})