#include <glad/glad.h>

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) x; (GLLogCall(#x, __FILE__, __LINE__));

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

GLenum glCheckError_(const char* file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__) 