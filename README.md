# eco
a c++ distributed services framework.

<a href="https://scan.coverity.com/projects/ujoychou-eco">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/24090/badge.svg"/>
</a>







## 目录结构


doc
bin
  eco
    build.sh
  eco-persist
  eco-rpc

inc
  eco
  eco-pack
    cpp11.cmake
  eco-mysql
src
  eco
  module1/
  module2/

test
  case
  unit

README.md


ref
  github/boost
  github/fast_float

## CAST FLOAT/DOUBLE


## 易用性与性能
易用性的优先级最高。
具有高性能（采用最优算法方案，通过插件模式适配，高性能可能需要占用更多资源）
* 高性能忽略函数调用开销、虚函数开销等。
* 高性能不是极致性能。