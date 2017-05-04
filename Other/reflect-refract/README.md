![image](https://github.com/yl-me/Notes-of-computer-graphics/blob/master/Other/reflect-refract/data/1.png)

<a href="http://www.codecogs.com/eqnedit.php?latex=u'=&space;d\frac{v}{|v|}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?u'=&space;d\frac{v}{|v|}" title="u'= d\frac{v}{|v|}" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=d&space;=&space;|u|cos\theta" target="_blank"><img src="http://latex.codecogs.com/gif.latex?d&space;=&space;|u|cos\theta" title="d = |u|cos\theta" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=cos\theta&space;=&space;\frac{u\cdot&space;v}{|u||v|}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?cos\theta&space;=&space;\frac{u\cdot&space;v}{|u||v|}" title="cos\theta = \frac{u\cdot v}{|u||v|}" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=u'&space;=&space;\frac{u\cdot&space;v}{|v|^{2}}v" target="_blank"><img src="http://latex.codecogs.com/gif.latex?u'&space;=&space;\frac{u\cdot&space;v}{|v|^{2}}v" title="u' = \frac{u\cdot v}{|v|^{2}}v" /></a>

![image](https://github.com/yl-me/Notes-of-computer-graphics/blob/master/Other/reflect-refract/data/2.png)

<a href="http://www.codecogs.com/eqnedit.php?latex=N&space;=&space;2S" target="_blank"><img src="http://latex.codecogs.com/gif.latex?N&space;=&space;2S" title="N = 2S" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=R&space;=&space;I&space;&plus;&space;2S" target="_blank"><img src="http://latex.codecogs.com/gif.latex?R&space;=&space;I&space;&plus;&space;2S" title="R = I + 2S" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=S&space;=&space;-\frac{I\cdot&space;N}{|N|^{2}}N&space;=&space;-(I\cdot&space;N)N" target="_blank"><img src="http://latex.codecogs.com/gif.latex?S&space;=&space;-\frac{I\cdot&space;N}{|N|^{2}}N&space;=&space;-(I\cdot&space;N)N" title="S = -\frac{I\cdot N}{|N|^{2}}N = -(I\cdot N)N" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=R&space;=&space;I&space;-&space;2(I\cdot&space;N)N" target="_blank"><img src="http://latex.codecogs.com/gif.latex?R&space;=&space;I&space;-&space;2(I\cdot&space;N)N" title="R = I - 2(I\cdot N)N" /></a>

![image](https://github.com/yl-me/Notes-of-computer-graphics/blob/master/Other/reflect-refract/data/3.png)

[snell](https://en.wikipedia.org/wiki/Snell%27s_law)

<a href="http://www.codecogs.com/eqnedit.php?latex=\frac{sin\theta_{1}}{sin\theta_{2}}&space;=&space;\frac{n_{2}}{n_{1}}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\frac{sin\theta_{1}}{sin\theta_{2}}&space;=&space;\frac{n_{2}}{n_{1}}" title="\frac{sin\theta_{1}}{sin\theta_{2}} = \frac{n_{2}}{n_{1}}" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=R&space;=&space;R1&space;&plus;&space;R2&space;=&space;\frac{sin\theta&space;_{2}}{sin\theta&space;_{1}}[I-(N\cdot&space;I)N]&space;&plus;&space;R2" target="_blank"><img src="http://latex.codecogs.com/gif.latex?R&space;=&space;R1&space;&plus;&space;R2&space;=&space;\frac{sin\theta&space;_{2}}{sin\theta&space;_{1}}[I-(N\cdot&space;I)N]&space;&plus;&space;R2" title="R = R1 + R2 = \frac{sin\theta _{2}}{sin\theta _{1}}[I-(N\cdot I)N] + R2" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=R&space;=&space;\frac{\eta&space;1}{\eta&space;2}[I&space;-&space;(N\cdot&space;I)N]&plus;R2" target="_blank"><img src="http://latex.codecogs.com/gif.latex?R&space;=&space;\frac{\eta&space;1}{\eta&space;2}[I&space;-&space;(N\cdot&space;I)N]&plus;R2" title="R = \frac{\eta 1}{\eta 2}[I - (N\cdot I)N]+R2" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=R&space;=&space;\frac{\eta&space;1}{\eta&space;2}[I&space;-&space;(N\cdot&space;I)N]-Ncos\theta&space;_{2}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?R&space;=&space;\frac{\eta&space;1}{\eta&space;2}[I&space;-&space;(N\cdot&space;I)N]-Ncos\theta&space;_{2}" title="R = \frac{\eta 1}{\eta 2}[I - (N\cdot I)N]-Ncos\theta _{2}" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=R&space;=&space;\frac{\eta&space;1}{\eta&space;2}[I&space;-&space;(N\cdot&space;I)N]-N\sqrt{1-(\frac{\eta_{1}}{\eta_{2}})^{2}sin^{2}\theta&space;_{1}}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?R&space;=&space;\frac{\eta&space;1}{\eta&space;2}[I&space;-&space;(N\cdot&space;I)N]-N\sqrt{1-(\frac{\eta_{1}}{\eta_{2}})^{2}sin^{2}\theta&space;_{1}}" title="R = \frac{\eta 1}{\eta 2}[I - (N\cdot I)N]-N\sqrt{1-(\frac{\eta_{1}}{\eta_{2}})^{2}sin^{2}\theta _{1}}" /></a>

