# DensityMap

DensityMap is a class that stores a 3D array of floats between 0 and 1 and allows them to be displayed using OpenGL.

## Functions

<b>DensityMap(int dim)</b>  
Initializes the DensityMap with a cubic array of side length dim.

<b>void addLine(glm::vec3 p1, glm::vec3 p2, std::vector&lt;float&gt; vals)</b>  
Adds a line of data t othe array along the line segment defined by p1 and p2.
The more values there are in vals, the smoother the line will be.

<b>void addLineSmoothed(glm::vec3 p1, glm::vec3 p2, std::vector&lt;float&gt; vals, int radius = 5)</b>  
Adds a line of data t othe array along the line segment defined by p1 and p2.
The more values there are in vals, the smoother the line will be.
The area around the line is blurred. This area will be larger when radius is increased.

This function is not recommended when using a lot of data, because it blurs the area around the line.
(like with ultrasound data !!!)

<b>void clear()</b>  
Fills the whole array with zeroes.

<b>std::vector&lt;float&gt; getVertices()</b>  
Returns a vector of vertices used to render the density map using OpenGL.
