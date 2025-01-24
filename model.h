#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<std::vector<int> > faces_;
	std::vector<Vec2f> t_verts_;
	std::vector<std::vector<int> > t_faces_;
	std::vector<Vec3f> vert_norms_;
public:
	Model(const char *filename);
	~Model();

	int nverts();
	Vec3f vert(int idx);

	int nfaces();
	std::vector<int> face(int idx);

	int nt_verts();
	Vec2f t_vert(int idx);
	
	int nt_faces();
	std::vector<int> t_face(int idx);

	Vec3f vert_norm(int idx);
};

#endif //__MODEL_H__
