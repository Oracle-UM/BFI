#include <fmt/format.h>

extern "C" {
#   include <fcntl.h>
#   include <unistd.h>

#   include <sys/mman.h>
#   include <sys/stat.h>
#   include <sys/types.h>
}

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
