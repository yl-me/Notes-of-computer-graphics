###Calculating a light's radius

attenuation functions:

<a href="http://www.codecogs.com/eqnedit.php?latex=F_{light}=\frac{I}{K_{c}&plus;K_{l}*d&plus;k_{q}*d^{2}}&space;\quad&space;F_{light}!=0.0" target="_blank"><img src="http://latex.codecogs.com/gif.latex?F_{light}=\frac{I}{K_{c}&plus;K_{l}*d&plus;k_{q}*d^{2}}&space;\quad&space;F_{light}!=0.0" title="F_{light}=\frac{I}{K_{c}+K_{l}*d+k_{q}*d^{2}} \quad F_{light}!=0.0" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=I_{max}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?I_{max}" title="I_{max}" /></a> is the light source's brightest color component.

<a href="http://www.codecogs.com/eqnedit.php?latex=\frac{5}{256}=\frac{I_{max}}{Attenuation}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\frac{5}{256}=\frac{I_{max}}{Attenuation}" title="\frac{5}{256}=\frac{I_{max}}{Attenuation}" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=K_{q}*d^{2}&plus;k_{l}*d&plus;K_{c}-I_{max}*\frac{256}{5}=0" target="_blank"><img src="http://latex.codecogs.com/gif.latex?K_{q}*d^{2}&plus;k_{l}*d&plus;K_{c}-I_{max}*\frac{256}{5}=0" title="K_{q}*d^{2}+k_{l}*d+K_{c}-I_{max}*\frac{256}{5}=0" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=x=\frac{-K_{l}\sqrt{K_{l}^{2}-4*K_{q}*(K_{c}-I_{max}*\frac{256}{5})}}{2*K_{q}}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?x=\frac{-K_{l}\sqrt{K_{l}^{2}-4*K_{q}*(K_{c}-I_{max}*\frac{256}{5})}}{2*K_{q}}" title="x=\frac{-K_{l}\sqrt{K_{l}^{2}-4*K_{q}*(K_{c}-I_{max}*\frac{256}{5})}}{2*K_{q}}" /></a>

