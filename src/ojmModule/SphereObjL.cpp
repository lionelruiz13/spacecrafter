/*
 * SphereObjL
 *
 * Copyright 2020 Association Sirius & Association Andromède
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

#include "SphereObjL.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

constexpr double icosahedron_G = 0.5*(1.0+sqrt(5.0));
constexpr double icosahedron_b = 1.0/sqrt(1.0+icosahedron_G*icosahedron_G);
constexpr double icosahedron_a = icosahedron_b*icosahedron_G;
constexpr double segment = icosahedron_b * 2;
constexpr double height = segment * sqrt(3)/2;
constexpr double halfHeight = height / 2;
constexpr double radius = segment / (2 * sin(M_PI / 5));
constexpr double interTex = asin(halfHeight) / M_PI;
constexpr double PI_MUL_2 = M_PI * 2;

// Counter-clockwise triangles
const SphereObjL::Triangle icosahedron_triangles[20]= {
	{0, 2, 4},
	{0, 4, 6},
	{0, 6, 8},
	{0, 8, 10},
	{0, 10, 12},
	{1, 13, 11},
	{1, 11, 9},
	{1, 9, 7},
	{1, 7, 5},
	{1, 5, 3},
	{2, 3, 4},
	{4, 3, 5},
	{4, 5, 6},
	{6, 5, 7},
	{6, 7, 8},
	{8, 7, 9},
	{8, 9, 10},
	{10, 9, 11},
	{10, 11, 12},
	{12, 11, 13}
};

SphereObjL::SphereObjL()
{
	auto &context = *Context::instance;
	SubBuffer indexLow, indexMedium, indexHigh, tmpBuffer;
	unsigned int indexCountLow, indexCountMedium, indexCountHigh, tmp;
	unsigned long *src, *dst;
	// Construct sphere and upload indices

	construct(SUBDIVISE_LOW_RES);
	indexCountLow = triangles.size() * 3;
	indexLow = context.indexBufferMgr->acquireBuffer(indexCountLow * sizeof(int));
	tmp = indexCountLow / 2;
	src = (unsigned long *) triangles.data();
	dst = (unsigned long *) context.transfer->planCopy(indexLow);
	while (tmp--)
		*(dst++) = *(src++);

	while (subdiviseLevel < SUBDIVISE_MEDIUM_RES)
		subdivise();
	indexCountMedium = triangles.size() * 3;
	indexMedium = context.indexBufferMgr->acquireBuffer(indexCountMedium * sizeof(int));
	tmp = indexCountMedium / 2;
	src = (unsigned long *) triangles.data();
	dst = (unsigned long *) context.transfer->planCopy(indexMedium);
	while (tmp--)
		*(dst++) = *(src++);

	while (subdiviseLevel < SUBDIVISE_HIGH_RES)
		subdivise();
	indexCountHigh = triangles.size() * 3;
	indexHigh = context.indexBufferMgr->acquireBuffer(indexCountHigh * sizeof(int));
	tmp = indexCountHigh / 2;
	src = (unsigned long *) triangles.data();
	dst = (unsigned long *) context.transfer->planCopy(indexHigh);
	while (tmp--)
		*(dst++) = *(src++);

	// Upload vertices
	auto vertex = std::shared_ptr<VertexBuffer>(context.ojmVertexArray->newBuffer(0, points.size(), context.ojmBufferMgr.get()));
	tmpBuffer = context.stagingMgr->acquireBuffer(vertex->get().size);
	tmp = points.size() * sizeof(OjmPoint) / sizeof(*dst);
	dst = (unsigned long *) context.stagingMgr->getPtr(tmpBuffer);
	src = (unsigned long *) points.data();
	while (tmp--)
		*(dst++) = *(src++);
	context.transfer->planCopyBetween(tmpBuffer, vertex->get());
	context.transientBuffer[context.frameIdx].push_back(tmpBuffer);

	// Generate OjmL
	low = std::make_unique<OjmL>(vertex, indexLow, indexCountLow);
	medium = std::make_unique<OjmL>(vertex, indexMedium, indexCountMedium);
	high = std::make_unique<OjmL>(vertex, indexHigh, indexCountHigh);
}

SphereObjL::~SphereObjL()
{
}

void SphereObjL::construct(int nb_subdivision)
{
	points.clear();
	points.push_back({{0, 0, 1}, {0.5, 1}, {0, 0, 1}});
	points.push_back({{0, 0,-1}, {0.5, 0}, {0, 0,-1}});
	OjmPoint p;
	for (int i = 0; i < 12; ++i) {
		p.tex[0] = i/10.f - 0.25f;
		p.pos[0] = cos(i*PI_MUL_2/10.f) * radius;
		p.pos[1] = sin(i*PI_MUL_2/10.f) * radius;
		if (i & 1) {
			p.tex[1] = 0.5 - interTex;
			p.pos[2] = -halfHeight;
		} else {
			p.tex[1] = 0.5 + interTex;
			p.pos[2] = halfHeight;
		}
		points.push_back({p.pos, p.tex, p.pos});
	}
	for (int i = 0; i < 20; ++i)
		triangles.push_back(icosahedron_triangles[i]);
	while (nb_subdivision--)
		subdivise();
}

void SphereObjL::subdivise()
{
	std::vector<Triangle> swapTriangles;
	swapTriangles.reserve(triangles.size() * 4);
	for (auto &t : triangles) {
		Triangle m {getIntersect(t.p1, t.p2), getIntersect(t.p2, t.p3), getIntersect(t.p3, t.p1)};
		swapTriangles.push_back({t.p1, m.p1, m.p3});
		swapTriangles.push_back({t.p2, m.p2, m.p1});
		swapTriangles.push_back({t.p3, m.p3, m.p2});
		swapTriangles.push_back(m);
	}
	triangles.swap(swapTriangles);
	++subdiviseLevel;
}