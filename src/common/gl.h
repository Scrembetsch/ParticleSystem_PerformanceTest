#pragma once

#ifdef _WIN32
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
#else
	#if __ANDROID_API__ >= 24
		#include <GLES3/gl32.h>
	#elif __ANDROID_API__ >= 21
		#include <GLES3/gl31.h>
	#elif __ANDROID_API__ >= 18
		#include <GLES3/gl3.h>
	#endif
	#include <EGL/egl.h>
#endif