#pragma bank 20
#pragma disable_warning 110

#include <gb/gb.h>
#include <gb/cgb.h>
#include <string.h>
#include <stdlib.h>
#include "vm.h"
#include "ui.h"

// --- HEAP MEMORY MAP ---
#define Q_BASE ((uint8_t*)(script_memory + 100))

#define Q_M       (Q_BASE)         
#define Q_F       (Q_BASE + 172)   
#define Q_D       (Q_BASE + 344)   
#define Q_URL     ((char*)(Q_BASE + 478)) 
#define Q_TM      (Q_BASE + 598)   
#define Q_ECC     (Q_BASE + 700)   
#define Q_G       (Q_BASE + 726)   
#define Q_GEXP    (Q_BASE + 753)   
#define Q_GLOG    (Q_BASE + 1008)  

static uint16_t bo = 0;
static const char B36_CH[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// --- HELPERS ---

static uint8_t gfm(uint8_t a, uint8_t b) {
    if (!a || !b) return 0;
    return Q_GEXP[(uint16_t)(Q_GLOG[a] + Q_GLOG[b]) % 255];
}

static void init_gf(void) {
    uint16_t v = 1;
    for (int i = 0; i < 255; i++) {
        Q_GEXP[i] = (uint8_t)v; Q_GLOG[v] = (uint8_t)i;
        v <<= 1; if (v & 0x100) v ^= 0x11D;
    }
}

static void q_sf(uint8_t x, uint8_t y, uint8_t v) {
    if (x >= 37 || y >= 37) return;
    uint16_t o = (uint16_t)y * 37 + x;
    if (v) Q_M[o >> 3] |= (0x80 >> (o & 7));
    else Q_M[o >> 3] &= ~(0x80 >> (o & 7));
    Q_F[o >> 3] |= (0x80 >> (o & 7));
}

static void pack_bits(uint32_t val, int len) {
    for (int i = len - 1; i >= 0; i--) {
        if ((val >> i) & 1) Q_D[bo >> 3] |= (0x80 >> (bo & 7));
        bo++;
    }
}

// --- ENTRY POINT 1: CALCULATE HASH ---
void calculate_hash_c(SCRIPT_CTX * THIS) OLDCALL BANKED {
    (void)THIS;
    uint32_t h = 0; 
    unsigned char *p = ui_text_data;
    uint32_t pwr = 1679616UL;
    int i;

    // Read indices from stack (pushed 5, 4, 3, 2, 1)
    // Stack grows UP: [..., Var5, Var4, Var3, Var2, Var1] <-- stack_ptr points here
    uint16_t v_idx[5];
    v_idx[0] = *(THIS->stack_ptr - 1);   // Var1
    v_idx[1] = *(THIS->stack_ptr - 2);   // Var2
    v_idx[2] = *(THIS->stack_ptr - 3);   // Var3
    v_idx[3] = *(THIS->stack_ptr - 4);   // Var4
    v_idx[4] = *(THIS->stack_ptr - 5);   // Var5

    while (*p) { 
        if (*p <= 32) {
            p++;
            continue;
        }
        uint32_t high_bit = (h & 0x80000000UL) ? 1UL : 0UL;
        h = (h << 1) | high_bit;
        h ^= (uint32_t)(*p++); 
    }
    h %= 60466176UL; 

    for (i = 0; i < 4; i++) {
        int d = 0; uint32_t cur_pwr = pwr;
        while (h >= cur_pwr) { h -= cur_pwr; d++; }
        script_memory[v_idx[i]] = (uint16_t)B36_CH[d];
        pwr /= 36;
    }
    script_memory[v_idx[4]] = (uint16_t)B36_CH[h % 36];
}

    // --- ENTRY POINT 2: GENERATE QR ---
void hello_world_c(SCRIPT_CTX * THIS) OLDCALL BANKED {
    (void)THIS;
    init_gf();
    for(int i=0; i<598; i++) Q_BASE[i] = 0; 
    for(int i=700; i<753; i++) Q_BASE[i] = 0; 
    bo = 0;

    strncpy(Q_URL, ui_text_data, 110);
    Q_URL[109] = 0;

    // V5 Structures
    for (int k = 0; k < 3; k++) {
        int ox = (k == 1) ? 30 : 0, oy = (k == 2) ? 30 : 0;
        for (int y = -1; y <= 7; y++) for (int x = -1; x <= 7; x++) {
            int cx = ox + x, cy = oy + y;
            if (cx >= 0 && cx < 37 && cy >= 0 && cy < 37)
                q_sf(cx, cy, (x>=0&&x<=6&&y>=0&&y<=6)&&(x==0||x==6||y==0||y==6||(x>=2&&x<=4&&y>=2&&y<=4)));
        }
    }
    for (int i = 8; i < 29; i++) { q_sf(i, 6, !(i & 1)); q_sf(6, i, !(i & 1)); }
    for (int i=-2; i<=2; i++) for (int j=-2; j<=2; j++) q_sf(30+j, 30+i, (abs(i)==2||abs(j)==2||(i==0&&j==0)));
    q_sf(8, 29, 1);
    for (int i = 0; i <= 8; i++) { if(i!=6){uint16_t o1=i*37+8, o2=8*37+i; Q_F[o1>>3]|=(0x80>>(o1&7)); Q_F[o2>>3]|=(0x80>>(o2&7));} }
    for (int i = 0; i < 8; i++) { uint16_t o1=8*37+(36-i), o2=(36-i)*37+8; Q_F[o1>>3]|=(0x80>>(o1&7)); Q_F[o2>>3]|=(0x80>>(o2&7)); }

    pack_bits(4, 4); pack_bits(strlen(Q_URL), 8);
    for(int i=0; Q_URL[i]; i++) pack_bits(Q_URL[i], 8);
    pack_bits(0, 4); while(bo & 7) bo++;
    int db = bo / 8; for(int i=db; i<108; i++) Q_D[i] = ((i - db) & 1) ? 0x11 : 0xEC;

    Q_G[0] = 1;
    for (int i = 0; i < 26; i++) {
        uint8_t root = Q_GEXP[i];
        for (int j = i; j >= 0; j--) Q_G[j+1] ^= gfm(Q_G[j], root);
    }
    for (int i = 0; i < 108; i++) {
        uint8_t f = Q_D[i] ^ Q_ECC[0];
        for(int k=0; k<25; k++) Q_ECC[k] = Q_ECC[k+1];
        Q_ECC[25] = 0; if (f) for (int j = 0; j < 26; j++) Q_ECC[j] ^= gfm(f, Q_G[j+1]);
    }
    memcpy(Q_D + 108, Q_ECC, 26);

    int ic = 0, dr = 1;
    for (int x = 36; x > 0; x -= 2) {
        if (x == 6) x--;
        for (int vert = 0; vert < 37; vert++) {
            int y = dr ? (36 - vert) : vert;
            for (int i = 0; i < 2; i++) {
                int cx = x - i, o = y * 37 + cx;
                if (!(Q_F[o >> 3] & (0x80 >> (o & 7)))) {
                    uint8_t bt = (ic < 1072) ? ((Q_D[ic >> 3] >> (7 - (ic & 7))) & 1) : 0;
                    if (bt ^ (((cx + y) % 2) == 0)) Q_M[o >> 3] |= (0x80 >> (o & 7));
                    ic++;
                }
            }
        }
        dr = !dr;
    }
    uint16_t fv = 0x77C4;
    for (int i = 0; i < 15; i++) {
        uint8_t v = (fv >> i) & 1;
        int x1, y1; if (i < 6) { x1 = 8; y1 = i; } else if (i < 8) { x1 = 8; y1 = i + 1; } else if (i == 8) { x1 = 7; y1 = 8; } else { x1 = 14 - i; y1 = 8; }
        if (v) Q_M[(y1 * 37 + x1) >> 3] |= (0x80 >> ((y1 * 37 + x1) & 7)); else Q_M[(y1 * 37 + x1) >> 3] &= ~(0x80 >> ((y1 * 37 + x1) & 7));
        int x2, y2; if (i < 8) { x2 = 36 - i; y2 = 8; } else { x2 = 8; y2 = 30 + (i - 8); }
        if (v) Q_M[(y2 * 37 + x2) >> 3] |= (0x80 >> ((y2 * 37 + x2) & 7)); else Q_M[(y2 * 37 + x2) >> 3] &= ~(0x80 >> ((y2 * 37 + x2) & 7));
    }

    wait_vbl_done();
    
    // GENERATE TILES INTO 28-127 (0x91C0 range in Addressing Mode 0)
    // We start at 28 to avoid index 0, which is usually the 'blank' tile for the scene.
    for (uint8_t ty = 0; ty < 10; ty++) {
        for (uint8_t tx = 0; tx < 10; tx++) {
            uint8_t tile[16]; for(int i=0; i<16; i++) tile[i]=0;
            for (uint8_t py = 0; py < 8; py++) {
                int sy = ty * 8 + py, my = sy >> 1;
                if (sy < 74) {
                    for (uint8_t px = 0; px < 8; px++) {
                        int sx = tx * 8 + px, mx = sx >> 1;
                        if (sx < 74) {
                            if (Q_M[((uint16_t)my * 37 + mx) >> 3] & (0x80 >> (((uint16_t)my * 37 + mx) & 7))) {
                                tile[py * 2] |= (0x80 >> px); tile[py * 2 + 1] |= (0x80 >> px);
                            }
                        }
                    }
                }
            }
            // Use tile indices 28-127.
            set_bkg_data(28 + ty * 10 + tx, 1, tile);
        }
    }

    // SAVE BACKGROUND MAP (10x10 area at 5,4)
    for (uint8_t y = 0; y < 10; y++) {
        for (uint8_t x = 0; x < 10; x++) {
            Q_TM[y * 10 + x] = get_vram_byte((uint8_t*)(0x9800 + ((y + 4) * 32) + (x + 5)));
        }
    }

    // UPDATE MAP TO SHOW QR
    for (uint8_t y = 0; y < 10; y++) {
        for (uint8_t x = 0; x < 10; x++) {
            set_vram_byte((uint8_t*)(0x9800 + ((y + 4) * 32) + (x + 5)), 28 + y * 10 + x);
        }
    }

    // Ensure we are in Addressing Mode 0 (LCDC.4 = 0) so indices 0-99 point to 0x9000
    LCDC_REG &= ~0x10U; 

    // BUTTON WAIT LOOP
    // 1. Wait for release of A/START if they were already held
    while (joypad() & (J_A | J_START)) wait_vbl_done();
    // 2. Wait for a new press
    while (!(joypad() & (J_A | J_START))) wait_vbl_done();
    // 3. Wait for release
    while (joypad() & (J_A | J_START)) wait_vbl_done();

    // RESTORE BACKGROUND MAP
    for (uint8_t y = 0; y < 10; y++) {
        for (uint8_t x = 0; x < 10; x++) {
            set_vram_byte((uint8_t*)(0x9800 + ((y + 4) * 32) + (x + 5)), Q_TM[y * 10 + x]);
        }
    }
}

