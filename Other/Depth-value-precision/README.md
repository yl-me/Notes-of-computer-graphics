The following (linear) equation then transforms the z-value to a depth value between 0.0 and 1.0:

$$F_{depth}=\frac{z-near}{far-near}$$

The depth values are greatly determined by the small z-values thus giving us enormous depth precision to the objects close by(the non-linear relation):

$$ F_{depth}=\frac{\frac{1}{z}-\frac{1}{near}}{\frac{1}{far}-\frac{1}{near}}$$
