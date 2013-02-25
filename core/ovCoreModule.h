
#ifndef OVCORE_EXPORT_H
#define OVCORE_EXPORT_H

#ifdef OVCORE_STATIC_DEFINE
#  define OVCORE_EXPORT
#  define OVCORE_NO_EXPORT
#else
#  ifndef OVCORE_EXPORT
#    ifdef ovCore_EXPORTS
        /* We are building this library */
#      define OVCORE_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define OVCORE_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef OVCORE_NO_EXPORT
#    define OVCORE_NO_EXPORT 
#  endif
#endif

#ifndef VTKCOMMONCORE_DEPRECATED
#  define VTKCOMMONCORE_DEPRECATED __declspec(deprecated)
#  define VTKCOMMONCORE_DEPRECATED_EXPORT VTKCOMMONCORE_EXPORT __declspec(deprecated)
#  define VTKCOMMONCORE_DEPRECATED_NO_EXPORT VTKCOMMONCORE_NO_EXPORT __declspec(deprecated)
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define VTKCOMMONCORE_NO_DEPRECATED
#endif



#endif
