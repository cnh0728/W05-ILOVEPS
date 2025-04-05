#include "QuadTexture.h"

uint32 quadTextureInices[] =
{
    0,1,2,
    1,3,2
};

FVertexTexture quadTextureVertices[] =
{
    {  -1.0f,  1.0f, 0.0f , 0.0f, 0.0f  }, // 좌상단: UV (0,0)  
    {   1.0f,  1.0f, 0.0f , 1.0f, 0.0f  }, // 우상단: UV (1,0)  
    {  -1.0f, -1.0f, 0.0f , 0.0f, 1.0f  }, // 좌하단: UV (0,1)  
    {   1.0f, -1.0f, 0.0f , 1.0f, 1.0f  }  // 우하단: UV (1,1)  
};