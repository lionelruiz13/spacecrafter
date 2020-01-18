
#include "model3D.hpp"



namespace GPU {

//! load_obj: load an OBJ file.
void Model3D::load_OBJ(const char*filename,const char*texname)
{
	err = tinyobj::LoadObj(shapes, materials, filename);

	if (!err.empty()) {
		std::cerr << err << std::endl;
		exit(1);
	}

	sradius = 0;
	size_t i=0, v=0, s=0;

	bufferSize = 0;
	for (i = 0; i < shapes.size(); i++) {
		bufferSize+=shapes[i].mesh.indices.size(); // points number
	}

	// buffers method
	positionsBuffer = new float[bufferSize*3];
	texcoordsBuffer = new float[bufferSize*2];
	TBNBuffer       = new float[bufferSize*9];
	normalsBuffer   = new float[bufferSize*3];
	tangentsBuffer  = new float[bufferSize*3];
	bitangentBuffer = new float[bufferSize*3];
	//cout<<"Buffer size: "<< bufferSize<<endl;

	Vec3f normal, // to compute a rotation for tangent compute
	      bitangent,
	      up(0.0,0.0,1.0),
	      light, // from the point to the light, normalized
	      lightPosition(0.0,0.0,0.0); // only 1 light point for the moment

	Vec3f   v0,v1,v2;
	Vec3f   v0v1, v0v2;
	Vec2f   uv0,uv1,uv2;
	Vec2f   uv0uv1,
	        uv0uv2;
	float scaleFactor;
	Mat4f tempTBN=Mat4f::identity();

	Mat4f TBN =Mat4f::identity(); // tangent bitangent normal

	int   ip0, ip1, ip2;   // i as indice, p as point

	// for each shape
	i=0;
	for ( s = 0; s <shapes.size(); s++) {

		// for each triangle
		for(v = 0; v < shapes[s].mesh.indices.size()/3; v++) {

			int i1=shapes[s].mesh.indices[3*v+0], // index of
			    i2=shapes[s].mesh.indices[3*v+1], // the first, second
			    i3=shapes[s].mesh.indices[3*v+2]; // and third point of the triangle

			//cout<<"Buffer Cursor: "<<i*3<<endl;

			texcoordsBuffer[i*2 +0]= shapes[s].mesh.texcoords[2*i1+0];
			texcoordsBuffer[i*2 +1]= shapes[s].mesh.texcoords[2*i1+1];

			positionsBuffer[i*3 +0]= shapes[s].mesh.positions[3*i1+0];
			positionsBuffer[i*3 +1]= shapes[s].mesh.positions[3*i1+1];
			positionsBuffer[i*3 +2]= shapes[s].mesh.positions[3*i1+2];

			normalsBuffer  [i*3 +0]= shapes[s].mesh.normals[3*i1+0];
			normalsBuffer  [i*3 +1]= shapes[s].mesh.normals[3*i1+1];
			normalsBuffer  [i*3 +2]= shapes[s].mesh.normals[3*i1+2];

			// COMPUTING TANGENT:
			tangentsBuffer [i*3 +0]= 1.0;
			tangentsBuffer [i*3 +1]= 1.0;
			tangentsBuffer [i*3 +2]= 0.0;
			i++;

			texcoordsBuffer[i*2 +0]= shapes[s].mesh.texcoords[2*i2+0];
			texcoordsBuffer[i*2 +1]= shapes[s].mesh.texcoords[2*i2+1];

			positionsBuffer[i*3 +0]= shapes[s].mesh.positions[3*i2+0];
			positionsBuffer[i*3 +1]= shapes[s].mesh.positions[3*i2+1];
			positionsBuffer[i*3 +2]= shapes[s].mesh.positions[3*i2+2];

			normalsBuffer  [i*3 +0]= shapes[s].mesh.normals[3*i2+0];
			normalsBuffer  [i*3 +1]= shapes[s].mesh.normals[3*i2+1];
			normalsBuffer  [i*3 +2]= shapes[s].mesh.normals[3*i2+2];

			tangentsBuffer [i*3 +0]= 1.0;
			tangentsBuffer [i*3 +1]= 1.0;
			tangentsBuffer [i*3 +2]= 0.0;
			i++;

			texcoordsBuffer[i*2 +0]= shapes[s].mesh.texcoords[2*i3+0];
			texcoordsBuffer[i*2 +1]= shapes[s].mesh.texcoords[2*i3+1];

			positionsBuffer[i*3 +0]= shapes[s].mesh.positions[3*i3+0];
			positionsBuffer[i*3 +1]= shapes[s].mesh.positions[3*i3+1];
			positionsBuffer[i*3 +2]= shapes[s].mesh.positions[3*i3+2];

			normalsBuffer  [i*3 +0]= shapes[s].mesh.normals[3*i3+0];
			normalsBuffer  [i*3 +1]= shapes[s].mesh.normals[3*i3+1];
			normalsBuffer  [i*3 +2]= shapes[s].mesh.normals[3*i3+2];

			tangentsBuffer [i*3 +0]= 1.0;
			tangentsBuffer [i*3 +1]= 1.0;
			tangentsBuffer [i*3 +2]= 0.0;
			i++;

			//////////////////////////////////////////////////////////////////
			//                                                              //
			// WARNING: TBN compute zone, high mathematical level required! //
			//                                                              //
			//////////////////////////////////////////////////////////////////

			// see OpenGL SuperBible page 518-522

			// we come back to compute tangent cause we need the whole triangle
			ip0=i-3; // indice point x
			ip1=i-2;
			ip2=i-1;

			// see https://www.opengl.org/discussion_boards/showthread.php/174015-How-to-calculate-TBN-matrix
			v0 = positionsBuffer + ip0*3;
			v1 = positionsBuffer + ip1*3;
			v2 = positionsBuffer + ip2*3;
			v0v1 = v1-v0;
			v0v2 = v2-v0;
			uv0 = texcoordsBuffer + ip0*2;
			uv1 = texcoordsBuffer + ip1*2;
			uv2 = texcoordsBuffer + ip2*2;
			uv0uv1 = uv1-uv0;
			uv0uv2 = uv2-uv0;

			scaleFactor = uv0uv1.v[0]*uv0uv2.v[1]-uv0uv1.v[1]*uv0uv2.v[0];
			if(scaleFactor==0)
				TBN=Mat4f::identity();
			else {
				scaleFactor=1/scaleFactor;
				tempTBN.setVector(Vec4f(uv0uv2.v[1]*v0v1-uv0uv1.v[1]*v0v2)*scaleFactor,0);
				tempTBN.setVector(Vec4f(uv0uv1.v[0]*v0v2-uv0uv2.v[0]*v0v1)*scaleFactor,1);
				//cout<<"length:" <<tempTBN.getVector(0).length()<<endl;
			}



			// MATRIX CALCULATION
			//-------------------

			// getting this normal vector
			normal = normalsBuffer+ ip0*3;
			normal.normalize();

			TBN= tempTBN;
			TBN.setVector(normal,2);
			TBN.setAsOrthonormalFromZ();
			TBN.transpose();

			TBNBuffer[ip0*9+ 0]=TBN.r[0];
			TBNBuffer[ip0*9+ 1]=TBN.r[1];
			TBNBuffer[ip0*9+ 2]=TBN.r[2];
			TBNBuffer[ip0*9+ 3]=TBN.r[3];
			TBNBuffer[ip0*9+ 4]=TBN.r[4];
			TBNBuffer[ip0*9+ 5]=TBN.r[5];
			TBNBuffer[ip0*9+ 6]=TBN.r[6];
			TBNBuffer[ip0*9+ 7]=TBN.r[7];
			TBNBuffer[ip0*9+ 8]=TBN.r[8];

			tangentsBuffer[ip0*3+ 0]=TBN.r[0];
			tangentsBuffer[ip0*3+ 1]=TBN.r[1];
			tangentsBuffer[ip0*3+ 2]=TBN.r[2];

			bitangentBuffer[ip0*3+ 0]=TBN.r[4];
			bitangentBuffer[ip0*3+ 1]=TBN.r[5];
			bitangentBuffer[ip0*3+ 2]=TBN.r[6];

			normalsBuffer[ip0*3 +0]= TBN.r[8];
			normalsBuffer[ip0*3 +1]= TBN.r[9];
			normalsBuffer[ip0*3 +2]= TBN.r[10];
			// ////////////////////////////////////////////////////////////////////

			normal = normalsBuffer+ ip1*3;
			normal.normalize();

			TBN= tempTBN;
			TBN.setVector(normal,2);
			TBN.setAsOrthonormalFromZ();
			TBN.transpose();

			TBNBuffer[ip1*9+ 0]=TBN.r[0];
			TBNBuffer[ip1*9+ 1]=TBN.r[1];
			TBNBuffer[ip1*9+ 2]=TBN.r[2];
			TBNBuffer[ip1*9+ 3]=TBN.r[3];
			TBNBuffer[ip1*9+ 4]=TBN.r[4];
			TBNBuffer[ip1*9+ 5]=TBN.r[5];
			TBNBuffer[ip1*9+ 6]=TBN.r[6];
			TBNBuffer[ip1*9+ 7]=TBN.r[7];
			TBNBuffer[ip1*9+ 8]=TBN.r[8];

			tangentsBuffer[ip1*3+ 0]=TBN.r[0];
			tangentsBuffer[ip1*3+ 1]=TBN.r[1];
			tangentsBuffer[ip1*3+ 2]=TBN.r[2];

			bitangentBuffer[ip1*3+ 0]=TBN.r[4];
			bitangentBuffer[ip1*3+ 1]=TBN.r[5];
			bitangentBuffer[ip1*3+ 2]=TBN.r[6];

			normalsBuffer[ip1*3 +0]= TBN.r[8];
			normalsBuffer[ip1*3 +1]= TBN.r[9];
			normalsBuffer[ip1*3 +2]= TBN.r[10];
			// ////////////////////////////////////////////////////////////////////

			normal = normalsBuffer+ ip2*3;
			normal.normalize();

			TBN= tempTBN;
			TBN.setVector(normal,2);
			TBN.setAsOrthonormalFromZ();
			TBN.transpose();


			TBNBuffer[ip2*9+ 0]=TBN.r[0];
			TBNBuffer[ip2*9+ 1]=TBN.r[1];
			TBNBuffer[ip2*9+ 2]=TBN.r[2];
			TBNBuffer[ip2*9+ 3]=TBN.r[3];
			TBNBuffer[ip2*9+ 4]=TBN.r[4];
			TBNBuffer[ip2*9+ 5]=TBN.r[5];
			TBNBuffer[ip2*9+ 6]=TBN.r[6];
			TBNBuffer[ip2*9+ 7]=TBN.r[7];
			TBNBuffer[ip2*9+ 8]=TBN.r[8];

			tangentsBuffer[ip2*3+ 0]=TBN.r[0];
			tangentsBuffer[ip2*3+ 1]=TBN.r[1];
			tangentsBuffer[ip2*3+ 2]=TBN.r[2];

			bitangentBuffer[ip2*3+ 0]=TBN.r[4];
			bitangentBuffer[ip2*3+ 1]=TBN.r[5];
			bitangentBuffer[ip2*3+ 2]=TBN.r[6];

			normalsBuffer[ip2*3 +0]= TBN.r[8];
			normalsBuffer[ip2*3 +1]= TBN.r[9];
			normalsBuffer[ip2*3 +2]= TBN.r[10];
			// ////////////////////////////////////////////////////////////////////
		}

	}
	// Debuggage
	//cout<<"vertex number i: "<< i<<endl;
	//cout<<"Buffer size: "<< bufferSize<<endl;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(5,vbo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);


	// buffer bindings
	glBindBuffer(GL_ARRAY_BUFFER,vbo[VBO_POSITIONS]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(positionsBuffer)*bufferSize*3,positionsBuffer,GL_STATIC_DRAW);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,vbo[VBO_TEXCOORDS]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(positionsBuffer)*bufferSize*2,texcoordsBuffer,GL_STATIC_DRAW);
	glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,vbo[VBO_NORMALS]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(normalsBuffer)*bufferSize*3,normalsBuffer,GL_STATIC_DRAW);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,vbo[VBO_TANGENTS]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(tangentsBuffer)*bufferSize*3,tangentsBuffer,GL_STATIC_DRAW);
	glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,0,NULL);

	glBindBuffer(GL_ARRAY_BUFFER,vbo[VBO_BITANGENTS]);
	glBufferData(GL_ARRAY_BUFFER,sizeof(bitangentBuffer)*bufferSize*3,bitangentBuffer,GL_STATIC_DRAW);
	glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,0,NULL);
}

Model3D::~Model3D()
{
	glDeleteBuffers(5,vbo);
	glDeleteVertexArrays(1,&vao);

	delete positionsBuffer;
	delete texcoordsBuffer;
	delete normalsBuffer;
	delete tangentsBuffer;
	delete bitangentBuffer;
}

//! This function intends to feed two layouts called position and texcoord
//! This is in the case you care about the program object and uniforms.
//! @param mode must be GL_TRIANGLES or GL_PATCHES if the used program contains a tessellation evaluation shader.
void Model3D::bindBuffersAndDraw(GLenum mode)
{
	//glEnable(GL_CULL_FACE); // enabling face culling to optimize
	//glCullFace(GL_BACK);
	glBindVertexArray(vao);
	glDrawArrays(mode,0,bufferSize);
	//glDisable(GL_CULL_FACE); // but remove it after to avoid trouble.
}

}// namespace GPU
