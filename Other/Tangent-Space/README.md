切线空间(tangent sapce)

![image](https://github.com/yl-me/Notes-of-computer-graphics/blob/master/Other/Tangent-Space/normal_mapping_tbn_vectors.png)

![image](https://github.com/yl-me/Notes-of-computer-graphics/blob/master/Other/Tangent-Space/normal_mapping_surface_edges.png)

the tangent vector <a href="http://www.codecogs.com/eqnedit.php?latex=T" target="_blank"><img src="http://latex.codecogs.com/gif.latex?T" title="T" /></a> and bitangent vector <a href="http://www.codecogs.com/eqnedit.php?latex=B" target="_blank"><img src="http://latex.codecogs.com/gif.latex?B" title="B" /></a> are unit vector

<a href="http://www.codecogs.com/eqnedit.php?latex=E_{1}=\Delta&space;U_{1}T&plus;\Delta&space;V_{1}B" target="_blank"><img src="http://latex.codecogs.com/gif.latex?E_{1}=\Delta&space;U_{1}T&plus;\Delta&space;V_{1}B" title="E_{1}=\Delta U_{1}T+\Delta V_{1}B" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=E_{2}=\Delta&space;U_{2}T&plus;\Delta&space;V_{2}B" target="_blank"><img src="http://latex.codecogs.com/gif.latex?E_{2}=\Delta&space;U_{2}T&plus;\Delta&space;V_{2}B" title="E_{2}=\Delta U_{2}T+\Delta V_{2}B" /></a>

Which we can also write as:

<a href="http://www.codecogs.com/eqnedit.php?latex=(E_{1x},E_{1y},E_{1z})=\Delta&space;U_{1}(T_{x},T_{y},T_{z})&plus;\Delta&space;V_{1}(B_{x},B_{y},B_{z})" target="_blank"><img src="http://latex.codecogs.com/gif.latex?(E_{1x},E_{1y},E_{1z})=\Delta&space;U_{1}(T_{x},T_{y},T_{z})&plus;\Delta&space;V_{1}(B_{x},B_{y},B_{z})" title="(E_{1x},E_{1y},E_{1z})=\Delta U_{1}(T_{x},T_{y},T_{z})+\Delta V_{1}(B_{x},B_{y},B_{z})" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=(E_{2x},E_{2y},E_{2z})=\Delta&space;U_{2}(T_{x},T_{y},T_{z})&plus;\Delta&space;V_{2}(B_{x},B_{y},B_{z})" target="_blank"><img src="http://latex.codecogs.com/gif.latex?(E_{2x},E_{2y},E_{2z})=\Delta&space;U_{2}(T_{x},T_{y},T_{z})&plus;\Delta&space;V_{2}(B_{x},B_{y},B_{z})" title="(E_{2x},E_{2y},E_{2z})=\Delta U_{2}(T_{x},T_{y},T_{z})+\Delta V_{2}(B_{x},B_{y},B_{z})" /></a>

The matrix multiplication:

<a href="http://www.codecogs.com/eqnedit.php?latex=\begin{bmatrix}&space;E_{1x}&space;&&space;E_{1y}&space;&&space;E_{1z}\\&space;E_{2x}&space;&&space;E_{2y}&space;&&space;E_{2z}&space;\end{bmatrix}=\begin{bmatrix}&space;\Delta&space;U_{1}&space;&&space;\Delta&space;V_{1}\\&space;\Delta&space;U_{2}&space;&&space;\Delta&space;V_{2}&space;\end{bmatrix}\begin{bmatrix}&space;T_{x}&&space;T_{y}&space;&&space;T_{z}\\&space;B_{x}&&space;B_{y}&space;&&space;B_{z}&space;\end{bmatrix}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\begin{bmatrix}&space;E_{1x}&space;&&space;E_{1y}&space;&&space;E_{1z}\\&space;E_{2x}&space;&&space;E_{2y}&space;&&space;E_{2z}&space;\end{bmatrix}=\begin{bmatrix}&space;\Delta&space;U_{1}&space;&&space;\Delta&space;V_{1}\\&space;\Delta&space;U_{2}&space;&&space;\Delta&space;V_{2}&space;\end{bmatrix}\begin{bmatrix}&space;T_{x}&&space;T_{y}&space;&&space;T_{z}\\&space;B_{x}&&space;B_{y}&space;&&space;B_{z}&space;\end{bmatrix}" title="\begin{bmatrix} E_{1x} & E_{1y} & E_{1z}\\ E_{2x} & E_{2y} & E_{2z} \end{bmatrix}=\begin{bmatrix} \Delta U_{1} & \Delta V_{1}\\ \Delta U_{2} & \Delta V_{2} \end{bmatrix}\begin{bmatrix} T_{x}& T_{y} & T_{z}\\ B_{x}& B_{y} & B_{z} \end{bmatrix}" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=\begin{bmatrix}&space;\Delta&space;U_{1}&space;&&space;\Delta&space;V_{1}\\&space;\Delta&space;U_{2}&space;&&space;\Delta&space;V_{2}&space;\end{bmatrix}^{-1}\begin{bmatrix}&space;E_{1x}&space;&&space;E_{1y}&space;&&space;E_{1z}\\&space;E_{2x}&space;&&space;E_{2y}&space;&&space;E_{2z}&space;\end{bmatrix}=\begin{bmatrix}&space;T_{x}&&space;T_{y}&space;&&space;T_{z}\\&space;B_{x}&&space;B_{y}&space;&&space;B_{z}&space;\end{bmatrix}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\begin{bmatrix}&space;\Delta&space;U_{1}&space;&&space;\Delta&space;V_{1}\\&space;\Delta&space;U_{2}&space;&&space;\Delta&space;V_{2}&space;\end{bmatrix}^{-1}\begin{bmatrix}&space;E_{1x}&space;&&space;E_{1y}&space;&&space;E_{1z}\\&space;E_{2x}&space;&&space;E_{2y}&space;&&space;E_{2z}&space;\end{bmatrix}=\begin{bmatrix}&space;T_{x}&&space;T_{y}&space;&&space;T_{z}\\&space;B_{x}&&space;B_{y}&space;&&space;B_{z}&space;\end{bmatrix}" title="\begin{bmatrix} \Delta U_{1} & \Delta V_{1}\\ \Delta U_{2} & \Delta V_{2} \end{bmatrix}^{-1}\begin{bmatrix} E_{1x} & E_{1y} & E_{1z}\\ E_{2x} & E_{2y} & E_{2z} \end{bmatrix}=\begin{bmatrix} T_{x}& T_{y} & T_{z}\\ B_{x}& B_{y} & B_{z} \end{bmatrix}" /></a>

<a href="http://www.codecogs.com/eqnedit.php?latex=\begin{bmatrix}&space;T_{x}&&space;T_{y}&space;&&space;T_{z}\\&space;B_{x}&&space;B_{y}&space;&&space;B_{z}&space;\end{bmatrix}=\frac{1}{\Delta&space;U_{1}\Delta&space;V_{2}-\Delta&space;U_{2}\Delta&space;V_{1}}\begin{bmatrix}&space;\Delta&space;V_{2}&space;&&space;-\Delta&space;V_{1}\\&space;-\Delta&space;U_{2}&space;&&space;\Delta&space;U_{1}&space;\end{bmatrix}\begin{bmatrix}&space;E_{1x}&space;&&space;E_{1y}&space;&&space;E_{1z}\\&space;E_{2x}&space;&&space;E_{2y}&space;&&space;E_{2z}&space;\end{bmatrix}" target="_blank"><img src="http://latex.codecogs.com/gif.latex?\begin{bmatrix}&space;T_{x}&&space;T_{y}&space;&&space;T_{z}\\&space;B_{x}&&space;B_{y}&space;&&space;B_{z}&space;\end{bmatrix}=\frac{1}{\Delta&space;U_{1}\Delta&space;V_{2}-\Delta&space;U_{2}\Delta&space;V_{1}}\begin{bmatrix}&space;\Delta&space;V_{2}&space;&&space;-\Delta&space;V_{1}\\&space;-\Delta&space;U_{2}&space;&&space;\Delta&space;U_{1}&space;\end{bmatrix}\begin{bmatrix}&space;E_{1x}&space;&&space;E_{1y}&space;&&space;E_{1z}\\&space;E_{2x}&space;&&space;E_{2y}&space;&&space;E_{2z}&space;\end{bmatrix}" title="\begin{bmatrix} T_{x}& T_{y} & T_{z}\\ B_{x}& B_{y} & B_{z} \end{bmatrix}=\frac{1}{\Delta U_{1}\Delta V_{2}-\Delta U_{2}\Delta V_{1}}\begin{bmatrix} \Delta V_{2} & -\Delta V_{1}\\ -\Delta U_{2} & \Delta U_{1} \end{bmatrix}\begin{bmatrix} E_{1x} & E_{1y} & E_{1z}\\ E_{2x} & E_{2y} & E_{2z} \end{bmatrix}" /></a>

TBN = mat3(T, B, N);

    // tangnt space
	GLuint quadVAO = 0;
	GLuint quadVBO;
	void RenderQuad()
	{
		if (quadVAO == 0) {
			// position
			glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
			glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
			glm::vec3 pos3(1.0f, -1.0f, 0.0f);
			glm::vec3 pos4(1.0f, 1.0f, 0.0f);
			// texture coordinates
			glm::vec2 uv1(0.0f, 1.0f);
			glm::vec2 uv2(0.0f, 0.0f);
			glm::vec2 uv3(1.0f, 0.0f);
			glm::vec2 uv4(1.0f, 1.0f);
			// normal
			glm::vec3 nm(0.0f, 0.0f, 1.0f);
	
			// in triangle we calculate tangent and bitangent vectors
			glm::vec3 tangent1, bitangent1;
			glm::vec3 tangent2, bitangent2;
			// triangle1
			glm::vec3 edge1 = pos2 - pos1;
			glm::vec3 edge2 = pos3 - pos1;
			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;
	
			GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	
			tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			tangent1 = glm::normalize(tangent1);
	
			bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
			bitangent1 = glm::normalize(bitangent1);
			
			// triangle2
			edge1 = pos3 - pos1;
			edge2 = pos4 - pos1;
			deltaUV1 = uv3 - uv1;
			deltaUV2 = uv4 - uv1;
	
			f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
	
			tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			tangent2 = glm::normalize(tangent2);
	
	
			bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
			bitangent2 = glm::normalize(bitangent2);
	
			float quadVertices[] = {
				// positions            // normal         // texcoords  // tangent                          // bitangent
				pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
				pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
				pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
	
				pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
				pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
				pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
			};
			// config plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}