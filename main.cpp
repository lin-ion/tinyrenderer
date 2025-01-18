#include <vector>
#include <iostream>
#include <cmath>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

Model *model = NULL;
const int width = 800;
const int height = 800;
const int depth = 255;

Vec3f barycentric(Vec3f *pts, Vec3f P){;
  Vec3f AB = Vec3f(pts[1]-pts[0]);
  Vec3f AC = Vec3f(pts[2]-pts[0]);
  Vec3f PA = Vec3f(pts[0]-P);
  Vec3f u = Vec3f(AB.x, AC.x, PA.x)^Vec3f(AB.y, AC.y, PA.y);

  if (std::abs(u.z)<1) {
    return Vec3f(-1,1,1);
  } else {
    return Vec3f(1.f-(u.x+u.y)/u.z, u.x/u.z, u.y/u.z);
  }
}

void triangle(TGAImage &image, Vec3f *pts, float *zbuffer, Vec3f intensities, TGAImage &texture, Vec2f *t_pts) {
  Vec2i bboxmin( std::numeric_limits<int>::max(),  std::numeric_limits<int>::max());
  Vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
  Vec2i clamp(image.get_width()-1, image.get_height()-1);
  for (int i=0; i<3; i++){
    bboxmin.x = std::max(0, std::min(bboxmin.x, int(pts[i].x+.5f)));
    bboxmin.y = std::max(0, std::min(bboxmin.y, int(pts[i].y+.5f)));

    bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, int(pts[i].x+.5f)));
    bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, int(pts[i].y+.5f)));
  }
  Vec3f P;
  for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
    for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
      Vec3f bc_screen = barycentric(pts, P);
      if (bc_screen[0]<0 || bc_screen[1]<0 || bc_screen[2]<0) continue;

      P.z = bc_screen * Vec3f(pts[0].z, pts[1].z, pts[2].z);
      if (zbuffer[int(P.x+P.y*width)]<P.z) {
        zbuffer[int(P.x+P.y*width)] = P.z;

        float intensity = bc_screen * intensities;
        if (intensity > 0) {
          intensity = std::pow(intensity, 2.2f);

          Vec2f tP = Vec2f(0.f, 0.f);
          tP.x = bc_screen*Vec3f(t_pts[0].x, t_pts[1].x, t_pts[2].x);
          tP.y = bc_screen*Vec3f(t_pts[0].y, t_pts[1].y, t_pts[2].y);

          tP.x *= (texture.get_width()-1.);
          tP.y *= (texture.get_height()-1.);

          TGAColor color = texture.get(tP.x, tP.y);
          for (int i=0; i<3; i++) {
            color.raw[i] *= intensity;
          }
          image.set(P.x, P.y, color);
        }
      }
    }
  }
}

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

Matrix lookAt(Vec3f eye, Vec3f center, Vec3f up) {
  Vec3f z = (eye-center).normalize();
  Vec3f x = (up^z).normalize();
  Vec3f y = (z^x).normalize();
  Matrix Minv = Matrix::identity(4);
  Matrix Tr = Matrix::identity(4);
  for (int i=0; i<3; i++) {
    Minv[0][i] = x[i];
    Minv[1][i] = y[i];
    Minv[2][i] = z[i];
    Tr[i][3] = -center[i];
  }
  return Minv*Tr;
}

int main(int argc, char **argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);

  TGAImage texture;
  texture.read_tga_file("obj/african_head_diffuse.tga");
  texture.flip_vertically();

  float *zbuffer = new float[width*height];
  for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

  Vec3f light_dir = Vec3f(1.f, -1.f, 1.f).normalize();

  Vec3f eye(1.f, 1.f, 3.f);
  Vec3f center(0.f, 0.f, 0.f);
  Vec3f up(0.f, 1.f, 0.f);

  Matrix ModelView = lookAt(eye, center, up);
  Matrix Projection = Matrix::identity(4);
  Matrix ViewPort = viewport(width/8, height/8, width*3/4, height*3/4);
  Projection[3][2] = -1.f/(eye-center).z;

  for (int i=0; i<model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3f screen_coords[3];
    Vec3f intensities;
    std::vector<int> t_face = model->t_face(i);
    Vec2f texture_coords[3];
    for (int j=0; j<3; j++) {
      Vec3f v = model->vert(face[j]);
      screen_coords[j] = Vec3f(ViewPort*Projection*ModelView*Matrix(v));
      intensities[j] = model->vert_norm(face[j]) * light_dir;
      texture_coords[j] = model->t_vert(t_face[j]);
    }

    triangle(image, screen_coords, zbuffer, intensities, texture, texture_coords);
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  delete model;
  delete[] zbuffer;
  return 0;
}
