#ifndef PRUEBA_SISTEMA_H
#define PRUEBA_SISTEMA_H

#if defined(_WIN32)
#define PLATFORM_NAME "windows" // Windows
#define PLATFORM_WINDOWS
#elif defined(_WIN64)
#define PLATFORM_NAME "windows" // Windows
#define PLATFORM_WINDOWS
#elif defined(__CYGWIN__) && !defined(_WIN32)
#define PLATFORM_NAME "windows" // Windows (Cygwin POSIX under Microsoft Window)
#define PLATFORM_WINDOWS

#elif defined(__ANDROID__)
#define PLATFORM_NAME "android" // Android (implies Linux, so it must come first)
#define PLATFORM_ANDROID

#elif defined(__linux__)
#define PLATFORM_NAME "linux" // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
#define PLATFORM_LINUX
//#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
//#include <sys/param.h>

/*#if defined(BSD)
#define PLATFORM_NAME "bsd" // FreeBSD, NetBSD, OpenBSD, DragonFly BSD
#endif
#elif defined(__hpux)
#define PLATFORM_NAME "hp-ux" // HP-UX
#elif defined(_AIX)
#define PLATFORM_NAME "aix" // IBM AIX
#elif defined(__APPLE__) && defined(__MACH__) // Apple OSX and iOS (Darwin)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1
#define PLATFORM_NAME "ios" // Apple iOS
#elif TARGET_OS_IPHONE == 1
#define PLATFORM_NAME "ios" // Apple iOS
#elif TARGET_OS_MAC == 1
#define PLATFORM_NAME "osx" // Apple OSX
#endif
#elif defined(__sun) && defined(__SVR4)
#define PLATFORM_NAME "solaris" // Oracle Solaris, Open Indiana*/
#else
#define PLATFORM_NAME 0
#endif

namespace prueba_sistema{
    inline const char *nombrePlataforma() {
        return (PLATFORM_NAME == 0) ? "" : PLATFORM_NAME;
    }
}; //fin de namespace prueba_sistema
/*
int main(){
    std::string sistema(get_platform_name());
    std::cout<<sistema<<std::endl;
}*/

#endif
