#include <vector>
#include <iostream>
#include <cmath>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

Model *model = NULL;
const int width = 800;
const int height = 800;

Vec3f barycentric(Vec3f *pts, Vec3f P){;
  Vec3f AB = Vec3f(pts[1]-pts[0]);
  Vec3f AC = Vec3f(pts[2]-pts[0]);
  Vec3f PA = Vec3f(pts[0]-P);

  Vec3f u = Vec3f(AB.x, AC.x, PA.x)^Vec3f(AB.y, AC.y, PA.y);

  // abs(u.z) means area of triangle
  if (std::abs(u.z)<1) return Vec3f(-1,1,1);
  // A: 1.f-(u.x+u.y)/u.z
  // B: u.x/u.z
  // C: u.y/u.z
  return Vec3f(1.f-(u.x+u.y)/u.z, u.x/u.z, u.y/u.z);
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
  Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
  Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
  Vec2f clamp(image.get_width()-1, image.get_height()-1);
  for (int i=0; i<3; i++){
    bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
    bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

    bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
    bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
  }
  Vec3f P;
  for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++){
    for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++){
      Vec3f bc_screen = barycentric(pts, P);
      if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
      
      // compute z-value
      // 삼각형의 정점들의 z-value와 각 정점에 대한 barycentric 가중치를 곱하여 더하면 P의 z-value가 계산된다. 
      P.z = 0.f;
      P.z += pts[0].z * bc_screen.raw[0];
      P.z += pts[1].z * bc_screen.raw[1];
      P.z += pts[2].z * bc_screen.raw[2];

      if (zbuffer[int(P.x+P.y*width)]<P.z){
        zbuffer[int(P.x+P.y*width)] = P.z;
        image.set(P.x, P.y, color);
      }
    }
  }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

int main(int argc, char **argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);

  Vec3f light_dir(0,0,-1);

  float *zbuffer = new float[width*height];
  for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

  for (int i=0; i<model->nfaces(); i++) { 
    std::vector<int> face = model->face(i); 
    Vec3f screen_coords[3]; 
    Vec3f world_coords[3];
    for (int j=0; j<3; j++) {
        Vec3f v = model->vert(face[j]);
        screen_coords[j] = world2screen(v);
        world_coords[j] = v;
    }
    Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
    n.normalize();
    float intencity = n*light_dir; // dot product

    // gamma collection
    intencity = std::pow(intencity, 2.2f);
    if(intencity>0) {
      triangle(screen_coords, zbuffer, image, TGAColor(intencity*255, intencity*255, intencity*255, 255));
    }
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  {
    TGAImage image_z(width, height, TGAImage::RGB);
    for (int x=0; x<width; x++){
      for (int y=0; y<height; y++){
        float z = zbuffer[x+y*width];
        z = std::max(-1.f, std::min(1.f, z)); // clamp
        z = (z+1.)/2.; // normalize
        z = std::pow(z, 2.2f); // gamma collection

        image_z.set(x,y,TGAColor(z*255,z*255,z*255,255));
      }
    }

    image_z.flip_vertically();
    image_z.write_tga_file("zbuffer.tga");
  }

  delete model;
  return 0;
}
