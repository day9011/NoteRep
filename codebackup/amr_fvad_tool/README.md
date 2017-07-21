requirement:
-------------------------
1.安装 libfvad:https://github.com/dpirch/libfvad.git<br>
使用./configure --prefix=path, 将库安装到自定义位置. <br>
2.安装 opencore-amr-0.1.5<br>
与libfvad类似，将库安装到自定义位置<br>

usage:
-------------------------
1.修改Makefile，将INCLUDEDIR等参数，修改成requirement安装的指定path<br>
2.执行make
