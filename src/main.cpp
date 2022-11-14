/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "ProceduralTerrain.h"


int main(int argc, char* argv[])
{
  sgl::Init();

  auto app = ProceduralTerrain();
  app.Run();
}
