static const uint8_t ter_v12n[] = {
#include "ter-v12n.inc"
};

static const uint8_t ter_v14b[] = {
#include "ter-v14b.inc"
};

static const uint8_t ter_v14n[] = {
#include "ter-v14n.inc"
};

static const uint8_t ter_v14v[] = {
#include "ter-v14v.inc"
};

static const uint8_t ter_v16b[] = {
#include "ter-v16b.inc"
};

static const uint8_t ter_v16n[] = {
#include "ter-v16n.inc"
};

static const uint8_t ter_v16v[] = {
#include "ter-v16v.inc"
};

static const uint8_t ter_v18b[] = {
#include "ter-v18b.inc"
};

static const uint8_t ter_v18n[] = {
#include "ter-v18n.inc"
};

static const uint8_t ter_v20b[] = {
#include "ter-v20b.inc"
};

static const uint8_t ter_v20n[] = {
#include "ter-v20n.inc"
};

static const uint8_t ter_v22b[] = {
#include "ter-v22b.inc"
};

static const uint8_t ter_v22n[] = {
#include "ter-v22n.inc"
};

static const uint8_t ter_v24b[] = {
#include "ter-v24b.inc"
};

static const uint8_t ter_v24n[] = {
#include "ter-v24n.inc"
};

static const uint8_t ter_v28b[] = {
#include "ter-v28b.inc"
};

static const uint8_t ter_v28n[] = {
#include "ter-v28n.inc"
};

static const uint8_t ter_v32b[] = {
#include "ter-v32b.inc"
};

static const uint8_t ter_v32n[] = {
#include "ter-v32n.inc"
};

const uint8_t* getFont(uint8_t font) {
    switch (font) {
    case FONT_TERMINUS_v12n:
        return ter_v12n;
    case FONT_TERMINUS_v14b:
        return ter_v14b;
    case FONT_TERMINUS_v14n:
        return ter_v14n;
    case FONT_TERMINUS_v14v:
        return ter_v14v;
    case FONT_TERMINUS_v16b:
        return ter_v16b;
    case FONT_TERMINUS_v16n:
        return ter_v16n;
    case FONT_TERMINUS_v16v:
        return ter_v16v;
    case FONT_TERMINUS_v18b:
        return ter_v18b;
    case FONT_TERMINUS_v18n:
        return ter_v18n;
    case FONT_TERMINUS_v20b:
        return ter_v20b;
    case FONT_TERMINUS_v20n:
        return ter_v20n;
    case FONT_TERMINUS_v22b:
        return ter_v22b;
    case FONT_TERMINUS_v22n:
        return ter_v22n;
    case FONT_TERMINUS_v24b:
        return ter_v24b;
    case FONT_TERMINUS_v24n:
        return ter_v24n;
    case FONT_TERMINUS_v28b:
        return ter_v28b;
    case FONT_TERMINUS_v28n:
        return ter_v28n;
    case FONT_TERMINUS_v32b:
        return ter_v32b;
    case FONT_TERMINUS_v32n:
        return ter_v32n;
    default:
        return ter_v12n;
    }
}
