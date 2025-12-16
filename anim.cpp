#include "ALL.H"

anim::anim(pic8* source_sheet, const char* error_filename) {
    frame_count = 0;
    for (int i = 0; i < ANIM_MAX_FRAMES; i++) {
        frames[i] = NULL;
    }

    if (source_sheet->getxsize() % ANIM_WIDTH) {
        char tmp[80];
        sprintf(tmp, "Picture xsize must be a multiply of %d!", ANIM_WIDTH);
        uzenet(tmp, error_filename);
    }
    frame_count = source_sheet->getxsize() / ANIM_WIDTH;
    if (frame_count < 0) {
        hiba("uiuiotb");
    }
    if (frame_count > ANIM_MAX_FRAMES) {
        char tmp[80];
        sprintf(tmp, "Too many frames in picture! Max frame is %d!", (int)ANIM_MAX_FRAMES);
        uzenet(tmp, error_filename);
    }

    for (int i = 0; i < frame_count; i++) {
        frames[i] = new pic8(ANIM_WIDTH, ANIM_WIDTH);
        blt8(frames[i], source_sheet, -ANIM_WIDTH * i, 0);
        forditkepet(frames[i]);
        spriteosit(frames[i], source_sheet->gpixel(0, 0));
    }
}

anim::~anim(void) {
    frame_count = 0;
    for (int i = 0; i < ANIM_MAX_FRAMES; i++) {
        if (frames[i]) {
            delete frames[i];
            frames[i] = NULL;
        }
    }
}

static const double Egyframeido = 0.014;

pic8* anim::get_frame_by_time(double time) {
    int sorszam = time / Egyframeido;
    int i = sorszam % frame_count;
    return frames[i];
}

pic8* anim::get_frame_by_index(int index) {
    if (index < 0 || index >= frame_count) {
        hiba("678673");
    }
    return frames[index];
}

// Menu sisakjanal egy kis piros vonalat odahuz fole:
void anim::make_helmet_top(void) {
    for (int i = 0; i < frame_count; i++) {
        pic8* ujpic = new pic8(ANIM_WIDTH, ANIM_WIDTH + 1);
        unsigned char atlatszo = frames[i]->gpixel(0, 0);
        ujpic->fillbox(atlatszo);
        blt8(ujpic, frames[i], 0, 1);
        // Most behuzzuk vonalat balrol jobb szelig:
        for (int x = 0; x < ANIM_WIDTH; x++) {
            if (ujpic->gpixel(x, 1) != atlatszo) {
                for (int j = x + 4; j < ANIM_WIDTH; j++) {
                    ujpic->ppixel(j, 0, ujpic->gpixel(x + 3, 1));
                }
                break;
            }
        }
        // Most behuzunk atlatszot jobbrol jobb szelig:
        for (int x = ANIM_WIDTH - 1; x > 0; x--) {
            if (ujpic->gpixel(x, 1) != atlatszo) {
                for (int j = x - 3; j < ANIM_WIDTH; j++) {
                    ujpic->ppixel(j, 0, atlatszo);
                }
                break;
            }
        }
        spriteosit(ujpic, ujpic->gpixel(0, 0));
        delete frames[i];
        frames[i] = ujpic;
    }
}
