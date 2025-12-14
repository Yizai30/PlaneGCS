#add_executable(aichater_exe)

# 三种可见性说明符 PRIVATE、PUBLIC、INTERFACE，用来修饰针对当前目标 A 的设置
# 当 B 通过 target_link_libraries(B A) 链接 A 时，对 A 的设置，分三种情况传递。
# - PRIVATE：私有的，仅用于构建 A 本身，不会传递给 B
# - PUBLIC：公开的，既用于构建 A，又是 A 接口的一部分，会传递给 B
# - INTERFACE：接口的，A 本身不使用，但 B 需要，会传递给 B
#target_sources(aichater_exe PRIVATE )

#target_include_directories(aichater )