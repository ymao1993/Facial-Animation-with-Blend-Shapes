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

#define APPLICATION_WWIDTH  1280
#define APPLICATION_WHEIGHT 960

namespace Application
{
    void appSetup();
    void appLoop();
    void appDestroy();
	void bindWindow(GLFWwindow *window);
}

#endif /* Application_hpp */


