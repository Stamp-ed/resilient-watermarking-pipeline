#include <cassert>
#include <cstdio>
#include "wm/watermark/pn.h"

using namespace wm;

int main() {
    uint64_t key = 123456789ULL;

    int8_t a = pn_chip(key, 0, 0, 0);
    int8_t b = pn_chip(key, 0, 0, 0);
    int8_t c = pn_chip(key, 0, 1, 0);
    int8_t d = pn_chip(key, 1, 0, 0);

    // Determinism
    assert(a == b);

    // Sensitivity
    assert(a != c || a != d);

    // Range
    assert(a == 1 || a == -1);

    printf("[PASS] PN generator\n");
    return 0;
}
