/*
    Copyright (C) 1998, 1999, 2000  Charles Mac Donald

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "shared.h"


t_bitmap bitmap;
t_cart cart;                
t_snd snd;
t_input input;
//OPLL *opll;

struct
{
    char reg[64];
}ym2413;

void emu_system_init(int rate)
{
    /* Initialize the VDP emulation */
    vdp_init();

    /* Initialize the SMS emulation */
    sms_init();

    /* Initialize the look-up tables and related data */
    render_init();

    /* Enable sound emulation if the sample rate was specified */
    audio_init(rate);

    /* Don't save SRAM by default */
    sms.save = 0;

    /* Clear emulated button state */
    memset(&input, 0, sizeof(t_input));
}

void audio_init(int rate)
{
    /* Clear sound context */
    memset(&snd, 0, sizeof(t_snd));

    /* Reset logging data */
    snd.log = 0;
    snd.callback = NULL;

    /* Oops.. sound is disabled */
    if(!rate) return;

    /* Calculate buffer size in samples */
    snd.bufsize = (rate / 60);

    /* Sound output */
    snd.buffer[0] = (signed short int *)malloc(snd.bufsize * 2);
    snd.buffer[1] = (signed short int *)malloc(snd.bufsize * 2);
    if(!snd.buffer[0] || !snd.buffer[1]) return;
    memset(snd.buffer[0], 0, snd.bufsize * 2);
    memset(snd.buffer[1], 0, snd.bufsize * 2);

    /* YM2413 sound stream */
//    snd.fm_buffer = (signed short int *)malloc(snd.bufsize * 2);
//    if(!snd.fm_buffer) return;
//    memset(snd.fm_buffer, 0, snd.bufsize * 2);

    /* SN76489 sound stream */
//    snd.psg_buffer[0] = (signed short int *)malloc(snd.bufsize * 2);
//    snd.psg_buffer[1] = (signed short int *)malloc(snd.bufsize * 2);
//    if(!snd.psg_buffer[0] || !snd.psg_buffer[1]) return;
//    memset(snd.psg_buffer[0], 0, snd.bufsize * 2);
//    memset(snd.psg_buffer[1], 0, snd.bufsize * 2);

    /* Set up SN76489 emulation */
    SN76496_init(0, MASTER_CLOCK, 255, rate);

    /* Set up YM2413 emulation */
//    OPLL_init(3579545, rate) ;
//    opll = OPLL_new() ;
//    OPLL_reset(opll) ;
//    OPLL_reset_patch(opll,0) ;            /* if use default voice data. */ 

    /* Inform other functions that we can use sound */
    snd.enabled = 1;
}


void system_shutdown(void)
{
    if(snd.enabled)
    {
//        OPLL_delete(opll);
//        OPLL_close();
    }
}

void system_reset(void)
{
    cpu_reset();
    vdp_reset();
    sms_reset();
    render_reset();
    system_load_sram();
    if(snd.enabled)
    {
//        OPLL_reset(opll) ;
//        OPLL_reset_patch(opll,0) ;            /* if use default voice data. */ 
    }
}

uint8_t SCREEN[192][256] = { 0 };

void system_save_state(void *fd)
{
    FIL* f = (FIL*)fd;
    UINT wb;
    /* Save VDP context */
    f_write(f, &vdp, sizeof(t_vdp), &wb);

    /* Save SMS context */
    f_write(f, &sms, sizeof(t_sms), &wb);

    /* Save Z80 context */
    save_z80(f);

    /* Save YM2413 registers */
    f_write(f, &ym2413.reg[0], 0x40, &wb);

    /* Save SN76489 context */
    f_write(f, &sn[0], sizeof(t_SN76496), &wb);

    f_write(f, SCREEN, sizeof(SCREEN), &wb);
    save_render(f);
}


void system_load_state(void *fd)
{
    int i;
    uint8 reg[0x40];

    /* Initialize everything */
///    cpu_reset();
///    system_reset();

    FIL* f = (FIL*)fd;
    UINT rb;
    /* Load VDP context */
    f_read(f, &vdp, sizeof(t_vdp), &rb);

    /* Load SMS context */
    f_read(f, &sms, sizeof(t_sms), &rb);

    /* Load Z80 context */
    load_z80(f);

    /* Load YM2413 registers */
    f_read(f, reg, 0x40, &rb);

    /* Load SN76489 context */
    f_read(f, &sn[0], sizeof(t_SN76496), &rb);

    f_read(f, SCREEN, sizeof(SCREEN), &rb);
    load_render(f);

    /* Restore callbacks */
    z80_set_irq_callback(sms_irq_callback);

///    cpu_readmap[0] = cart.rom + 0x0000; /* 0000-3FFF */
///    cpu_readmap[1] = cart.rom + 0x2000;
///    cpu_readmap[2] = cart.rom + 0x4000; /* 4000-7FFF */
///    cpu_readmap[3] = cart.rom + 0x6000;
///    cpu_readmap[4] = cart.rom + 0x0000; /* 0000-3FFF */
///    cpu_readmap[5] = cart.rom + 0x2000;
///    cpu_readmap[6] = sms.ram;
///    cpu_readmap[7] = sms.ram;
/**
    cpu_writemap[0] = sms.dummy;
    cpu_writemap[1] = sms.dummy;
    cpu_writemap[2] = sms.dummy;         
    cpu_writemap[3] = sms.dummy;
    cpu_writemap[4] = sms.dummy;         
    cpu_writemap[5] = sms.dummy;
    cpu_writemap[6] = sms.ram;           
    cpu_writemap[7] = sms.ram;

    sms_mapper_w(3, sms.fcr[3]);
    sms_mapper_w(2, sms.fcr[2]);
    sms_mapper_w(1, sms.fcr[1]);
    sms_mapper_w(0, sms.fcr[0]);
*/

    /* Force full pattern cache update */
//    is_vram_dirty = 1;
//    memset(vram_dirty, 1, 0x200);

    /* Restore palette */
    for(i = 0; i < PALETTE_SIZE; i += 1)
        palette_sync(i);

    /* Restore sound state */
    if(snd.enabled)
    {
#if 0
        /* Clear YM2413 context */
        OPLL_reset(opll) ;
        OPLL_reset_patch(opll,0) ;            /* if use default voice data. */ 

        /* Restore rhythm enable first */
        ym2413_write(0, 0, 0x0E);
        ym2413_write(0, 1, reg[0x0E]);

        /* User instrument settings */
        for(i = 0x00; i <= 0x07; i += 1)
        {
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }

        /* Channel frequency */
        for(i = 0x10; i <= 0x18; i += 1)
        {
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }

        /* Channel frequency + ctrl. */
        for(i = 0x20; i <= 0x28; i += 1)
        {
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }

        /* Instrument and volume settings  */
        for(i = 0x30; i <= 0x38; i += 1)
        {
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }
#endif
    }
}

void ym2413_write(int chip, int offset, int data)
{
//    static uint8 latch = 0;
//    if(offset & 1)
//        OPLL_writeReg(opll, latch, data);
//    else
//        latch = data;
}







