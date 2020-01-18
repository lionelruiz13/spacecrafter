/*
 * extract_sphere.cpp
 *
 * Copyright 2018 Olivier <olivier@orion>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

const double  C_PI	=	3.14159265358979323846;

void computeSphere(int slices, int stacks, std::vector<float> &dataVertex, std::vector<float> &dataTexture, std::vector<float> &dataNormal)
{
	float x, y, z;
	float s, t;
	int i, j;

	t=1.0;

	const float drho = C_PI / (float) stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		double rho = i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const float dtheta = 2.0 * C_PI / (float) slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	const float ds = 1.0 / slices;
	const float dt = 1.0 / stacks;

	// draw intermediate as quad strips
	for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
		s = 0.0;
		for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices; j++,cos_sin_theta_p+=2) {

			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = 1.0 * cos_sin_rho_p[0];

			//~ ---Pour info:---
			//~ glTexCoord2f(s, t);
			//~ glNormal3f(x, y, z);
			//~ glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
			//~ v = sVertex3v(x * scaledRadius, y * scaledRadius, z * one_minus_oblateness * scaledRadius, mat);
			//~ glVertex3dv(v);

			//~ glTexCoord2f(s, t - dt);
			//~ glNormal3f(x, y, z);
			//~ glColor3f(x * radius,y * radius,z * one_minus_oblateness * radius);
			//~ v = sVertex3v(x * scaledRadius, y * scaledRadius, z * one_minus_oblateness * scaledRadius, mat);
			//~ glVertex3dv(v);

			dataTexture.push_back(s);
			dataTexture.push_back(t);
			//~ glTexCoord2f(s, t);
			dataNormal.push_back(x);
			dataNormal.push_back(y);
			dataNormal.push_back(z);
			//~ glNormal3f(x, y, z);
			dataVertex.push_back(x);
			dataVertex.push_back(y);
			dataVertex.push_back(z);

			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = 1.0 * cos_sin_rho_p[2];

			dataTexture.push_back(s);
			dataTexture.push_back(t-dt);
			//~ glTexCoord2f(s, t);
			dataNormal.push_back(x);
			dataNormal.push_back(y);
			dataNormal.push_back(z);
			//~ glNormal3f(x, y, z);
			dataVertex.push_back(x);
			dataVertex.push_back(y);
			dataVertex.push_back(z);

			s += ds;
		}
		t -= dt;
	}
}


void computeMilkyway(int slices, int stacks, std::vector<float> &datapos, std::vector<float> &datatex, std::vector<float> &dataNormal)
{
	float x, y, z;
	float s, t;
	int i, j;

	t=0.0; // from inside texture is reversed

	const float drho = C_PI / (float) stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		double rho = i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const float dtheta = 2.0 * C_PI / (float) slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	const float ds = 1.0 / slices;
	const float dt = -1.0 / stacks; // from inside texture is reversed

	// draw intermediate as quad strips
	for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
		s = 0.0;
		for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices; j++,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = -1.0 * cos_sin_rho_p[0];

			//~ glTexCoord2f(s, t);
			datatex.push_back(s);
			datatex.push_back(t);

			//~ v = sVertex3v(x * radius, y * radius, z * one_minus_oblateness * radius, mat);
			dataNormal.push_back(-1.0f);
			dataNormal.push_back(-1.0f);
			dataNormal.push_back(-1.0f);

			datapos.push_back(x);
			datapos.push_back(y);
			datapos.push_back(z);

			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = -1.0 * cos_sin_rho_p[2];

			//~ glTexCoord2f(s, t - dt);
			datatex.push_back(s);
			datatex.push_back(t-dt);

			dataNormal.push_back(-1.0f);
			dataNormal.push_back(-1.0f);
			dataNormal.push_back(-1.0f);

			//~ v= sVertex3v(x * radius, y * radius, z * one_minus_oblateness * radius, mat);
			datapos.push_back(x);
			datapos.push_back(y);
			datapos.push_back(z);

			s += ds;
		}
		t -= dt;
	}
}


int main(int argc, char **argv)
{
	std::vector<float> dataNormal, dataTexture, dataVertex;
	std::vector<int> indices;
	
	if (argc<2) {
		std::cout << "Usage" << std::endl;
		std::cout << argv[0] << " <slices> <stacks>" << std::endl;
		return 1;
	}		

	int slices = std::stoi(argv[1]);
	int stacks = std::stoi(argv[2]);
	std::string filenameMTL = "sphere.mtl";
	std::string filenameOBJ = "sphere.obj";
	computeSphere(slices, slices, dataVertex , dataTexture, dataNormal);

	//~ std::string filenameMTL = "milkyway.mtl";
	//~ std::string filenameOBJ = "milkyway.obj";
	//~ computeMilkyway(slices, stacks, dataVertex, dataTexture, dataNormal);

	std::cout << "nombre de vectex " << dataVertex.size() << std::endl;
	std::cout << "nombre de texture " << dataTexture.size() << std::endl;
	std::cout << "nombre de normal " << dataNormal.size() << std::endl;
	std::cout << "nombre de points " <<  dataVertex.size() /3 << std::endl;

	std::ofstream stream;


	stream.open(filenameMTL.c_str(),std::ios_base::out);
	if(!stream.is_open())
		return -1;

	//creation du mtl
	stream << "# usemtl " << filenameMTL << std::endl;
	stream << "newmtl mat1" << std::endl;
	stream << "Ka 1.0 1.0 1.0" << std::endl;
	stream << "Kd 1.0 1.0 1.0" << std::endl;
	stream << "Ks 1.0 1.0 1.0" << std::endl;
	stream << "Ns 10" << std::endl;

	stream.close();


	stream.open(filenameOBJ.c_str(),std::ios_base::out);
	if(!stream.is_open())
		return -1;
	stream<< "# extration d'une sphère historique de Spacecrafter"<<std::endl;
	stream<< "mtllib " << filenameMTL << std::endl;
	stream<< "usemtl mat1" << std::endl;
	stream<< std::endl<<std::endl;

	for(unsigned int i=0; i< dataVertex.size() /3 ; i++) {
		stream<< "v " << dataVertex[3*i] << " " << dataVertex[3*i+1] << " " << dataVertex[3*i+2] << std::endl;
	}

	for(unsigned int i=0; i< dataTexture.size() /2 ; i++) {
		stream<< "vt " << dataTexture[2*i] << " " << dataTexture[2*i+1]  << std::endl;
	}

	for(unsigned int i=0; i< dataNormal.size() /3 ; i++) {
		stream<< "vn " << dataNormal[3*i] << " " << dataNormal[3*i+1] << " " << dataNormal[3*i+2] << std::endl;
	}

	for(unsigned int i=0; i< dataVertex.size() /3-2 ; i++) {
		if (i%2==0) {
			stream << "f "  << i+1 << "/" << i+1 << "/" << i+1 << " "
			       << i+2 << "/" << i+2 << "/" << i+2 << " "
			       << i+3 << "/" << i+3 << "/" << i+3 << " " << std::endl;
		}
		else {
			stream << "f "  << i+1 << "/" << i+1 << "/" << i+1 << " "
			       << i+3 << "/" << i+3 << "/" << i+3 << " "
			       << i+2 << "/" << i+2 << "/" << i+2 << " " << std::endl;
		}
	}


	stream.close();

	return 0;
}

