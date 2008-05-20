
#include "shapes.inc"

camera {
/*   location <0.0, 1700.0, 1200.0>*/
   location <0.0, 1471.0, 1471.0>
   direction <0.0, 0.0, 1.0>
   up  <0.0, 1.0, 0.0>
   right <1.244, 0.0, 0.0>
   sky <0.0, 1.0, 0.0>
   look_at <0.0, 0.0, 0.0>
}

/* The maze-textured ground */
/* plane { y, 0.0 */
box { <-1408.0, 0, -800.0>, <1408.0, 0, 800.0>
   pigment {
      image_map { gif "memorial.gif" }
      rotate 90*x
      scale <2816.0, 1.0, 1600.0>
      translate <-1408.0, 0.0, -800.0>
   }
   normal {
      wrinkles 0.5
      scale 0.5
   }
   finish {
      ambient 0.15
      diffuse 0.7
   }
}

