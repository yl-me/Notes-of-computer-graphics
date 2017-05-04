### Phong reflection model：
![image](https://github.com/yl-me/Notes-of-computer-graphics/blob/master/Other/Phong/Phong%20reflection%20model.png)

[The Normal Matrix](http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/) </br>
要想转换后的(模型矩阵乘)法线与转换后的模型表面切线的点积为零(垂直)，那么模型矩阵的左上3x3矩阵必须为正交矩阵(矩阵的逆的转置与原矩阵相同，或者说，矩阵的逆与矩阵的转置相同)。正交矩阵既可以保存向量角度关系，也可以保存向量长度。证明过程见链接。