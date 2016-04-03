//
//  Application.hpp
//  PDFA
//
//  Created by Yu Mao on 4/1/16.
//  Copyright © 2016 Carnegie Mellon University. All rights reserved.
//

#ifndef Application_hpp
#define Application_hpp

#include <GLFW/glfw3.h>
#include <stdio.h>

#define APPLICATION_WWIDTH  640.f
#define APPLICATION_WHEIGHT 480.f

namespace Application
{
    void appSetup(GLFWwindow *window);
    void appLoop();
    void appDestroy();
}

#endif /* Application_hpp */

