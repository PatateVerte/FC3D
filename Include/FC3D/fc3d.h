#ifndef FC3D_H_INCLUDED
#define FC3D_H_INCLUDED

#ifdef FC3D_MSVC
    #define OWL_MSVC

	#ifdef FC3D_BUILDING
		#define FC3D_DLL_EXPORT __declspec(dllexport)
	#else
		#define FC3D_DLL_EXPORT __declspec(dllimport)
	#endif
#else
	#define FC3D_DLL_EXPORT
#endif

#include <WF3D/wf3d.h>
#include <OWL/owl.h>

#endif // FC3D_H_INCLUDED
