///----------------------------------------------------------------------------|
/// C::B LLVM Clang 12.0.1 - libgdi32.a
///----------------------------------------------------------------------------:
#pragma once
#include <stdio.h>
#include "fcntl.h"
static void coninit() {
    _setmode(_fileno(stdout), _O_U8TEXT);
    _setmode(_fileno(stdin), _O_U8TEXT);
}