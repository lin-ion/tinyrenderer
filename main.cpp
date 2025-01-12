#include <vector>
#include <iostream>
#include "geometry.h"
#include "tgaimage.h"

Vec3f barycentric(Vec2i *pts, Vec2i P){;
  Vec2i AB = Vec2i(pts[2]-pts[0]);
  Vec2i AC = Vec2i(pts[1]-pts[0]);
  Vec2i PA = Vec2i(pts[0]-P);

  Vec3f u = Vec3f(AB.x, AC.x, PA.x)^Vec3f(AB.y, AC.y, PA.y);
  // u/u[2] represents barycentric coordinates

  // Vec2f _uAB = Vec2f((u.raw[0]/u.raw[2])*AB.x, (u.raw[0]/u.raw[2])*AB.y);
  // Vec2f _vAC = Vec2f((u.raw[1]/u.raw[2])*AC.x, (u.raw[1]/u.raw[2])*AC.y);
  // std::cout << "P ("<<pts[0].x+_uAB.x+_vAC.x<<", "<<pts[0].y+_uAB.y+_vAC.y <<")\n";

  if (std::abs(u.z)<1) return Vec3f(-1,1,1);
  return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
}

void triangle(Vec2i *pts, TGAImage &image, TGAColor color) {
  Vec2i bboxmin(image.get_width()-1, image.get_height()-1);
  Vec2i bboxmax(0, 0);
  Vec2i clamp(image.get_width()-1, image.get_height()-1);
  for (int i=0; i<3; i++){
    bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
    bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

    bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
    bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
  }
  Vec2i P;
  for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++){
    for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++){
      Vec3f bc_screen = barycentric(pts, P);
      if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
      image.set(P.x, P.y, color);
    }
  }
}

int main(int argc, char** argv) { 
    TGAImage frame(200, 200, TGAImage::RGB); 
    Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)}; 
    triangle(pts, frame, TGAColor(255, 0, 0, 255)); 
    frame.flip_vertically(); // to place the origin in the bottom left corner of the image 
    frame.write_tga_file("framebuffer.tga");
    return 0; 
}
